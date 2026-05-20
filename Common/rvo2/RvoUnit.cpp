/*
 * Unit.cpp
 * RVO2 Library
 *
 * Copyright 2008 University of North Carolina at Chapel Hill
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please send all bug reports to <geom@cs.unc.edu>.
 *
 * The authors may be contacted via:
 *
 * Jur van den Berg, Stephen J. Guy, Jamie Snape, Ming C. Lin, Dinesh Manocha
 * Dept. of Computer Science
 * 201 S. Columbia St.
 * Frederick P. Brooks, Jr. Computer Science Bldg.
 * Chapel Hill, N.C. 27599-3175
 * United States of America
 *
 * <http://gamma.cs.unc.edu/RVO2/>
 */

#include "stdh.h"
#include "RvoUnit.h"

#include "RvoObstacleMap.h"
#include "RvoObstacle.h"


namespace RVO 
{

	bool linearProgram1(const std::vector<Line> &lines, size_t lineNo, float radius, const RvoVec2 &optVelocity, bool directionOpt, RvoVec2 &result)
	{
		const float dotProduct = lines[lineNo].point * lines[lineNo].direction;
		const float discriminant = sqr(dotProduct) + sqr(radius) - absSq(lines[lineNo].point);

		if (discriminant < 0.0f) {
			/* Max speed circle fully invalidates line lineNo. */
			return false;
		}

		const float sqrtDiscriminant = std::sqrtf(discriminant);
		float tLeft = -dotProduct - sqrtDiscriminant;
		float tRight = -dotProduct + sqrtDiscriminant;

		for (size_t i = 0; i < lineNo; ++i) {
			const float denominator = det(lines[lineNo].direction, lines[i].direction);
			const float numerator = det(lines[i].direction, lines[lineNo].point - lines[i].point);

			if (std::fabsf(denominator) <= RVO_EPSILON) {
				/* Lines lineNo and i are (almost) parallel. */
				if (numerator < 0.0f) {
					return false;
				}
				else {
					continue;
				}
			}

			const float t = numerator / denominator;

			if (denominator >= 0.0f) 
			{
				/* Line i bounds line lineNo on the right. */
				tRight = min(tRight, t);
			}
			else 
			{
				/* Line i bounds line lineNo on the left. */
				tLeft = max(tLeft, t);
			}

			if (tLeft > tRight) {
				return false;
			}
		}

		if (directionOpt) {
			/* Optimize direction. */
			if (optVelocity * lines[lineNo].direction > 0.0f) {
				/* Take right extreme. */
				result = lines[lineNo].point + tRight * lines[lineNo].direction;
			}
			else {
				/* Take left extreme. */
				result = lines[lineNo].point + tLeft * lines[lineNo].direction;
			}
		}
		else {
			/* Optimize closest point. */
			const float t = lines[lineNo].direction * (optVelocity - lines[lineNo].point);

			if (t < tLeft) {
				result = lines[lineNo].point + tLeft * lines[lineNo].direction;
			}
			else if (t > tRight) {
				result = lines[lineNo].point + tRight * lines[lineNo].direction;
			}
			else {
				result = lines[lineNo].point + t * lines[lineNo].direction;
			}
		}

		return true;
	}

	size_t linearProgram2(const std::vector<Line> &lines, float radius, const RvoVec2 &optVelocity, bool directionOpt, RvoVec2 &result)
	{
		if (directionOpt) {
			/*
			 * Optimize direction. Note that the optimization velocity is of unit
			 * length in this case.
			 */
			result = optVelocity * radius;
		}
		else if (absSq(optVelocity) > sqr(radius)) {
			/* Optimize closest point and outside circle. */
			result = normalize(optVelocity) * radius;
		}
		else {
			/* Optimize closest point and inside circle. */
			result = optVelocity;
		}

		for (size_t i = 0; i < lines.size(); ++i) {
			if (det(lines[i].direction, lines[i].point - result) > 0.0f) {
				/* Result does not satisfy constraint i. Compute new optimal result. */
				const RvoVec2 tempResult = result;

				if (!linearProgram1(lines, i, radius, optVelocity, directionOpt, result)) {
					result = tempResult;
					return i;
				}
			}
		}

		return lines.size();
	}

	void linearProgram3(const std::vector<Line> &lines, size_t numObstLines, size_t beginLine, float radius, RvoVec2 &result)
	{
		float distance = 0.0f;

		for (size_t i = beginLine; i < lines.size(); ++i) {
			if (det(lines[i].direction, lines[i].point - result) > distance) {
				/* Result does not satisfy constraint of line i. */
				std::vector<Line> projLines(lines.begin(), lines.begin() + static_cast<ptrdiff_t>(numObstLines));

				for (size_t j = numObstLines; j < i; ++j) {
					Line line;

					float determinant = det(lines[i].direction, lines[j].direction);

					if (std::fabsf(determinant) <= RVO_EPSILON) {
						/* Line i and line j are parallel. */
						if (lines[i].direction * lines[j].direction > 0.0f) {
							/* Line i and line j point in the same direction. */
							continue;
						}
						else {
							/* Line i and line j point in opposite direction. */
							line.point = 0.5f * (lines[i].point + lines[j].point);
						}
					}
					else {
						line.point = lines[i].point + (det(lines[j].direction, lines[i].point - lines[j].point) / determinant) * lines[i].direction;
					}

					line.direction = normalize(lines[j].direction - lines[i].direction);
					projLines.push_back(line);
				}

				const RvoVec2 tempResult = result;

				if (linearProgram2(projLines, radius, RvoVec2(-lines[i].direction.y(), lines[i].direction.x()), true, result) < projLines.size()) {
					/* This should in principle not happen.  The result is by definition
					 * already in the feasible region of this linear program. If it fails,
					 * it is due to small floating point error, and the current result is
					 * kept.
					 */
					result = tempResult;
				}

				distance = det(lines[i].direction, lines[i].point - result);
			}
		}
	}
}


RvoUnit::RvoUnit() : _maxNeighbors(0), _maxSpeed(0.0f), _neighborDist(0.0f), _radius(0.0f), _timeHorizon(0.0f), _timeHorizonObst(0.0f), _id(0), _weight(0.0f),_mirror(NULL)
{
	_bGhostCollide=FALSE;
	_radLastAvoid=-1.0f;
	_idPlayer=0xff;
	_bStuck=FALSE;
}


/* Search for the best new velocity. */
void RvoUnit::computeNewVelocity()
{
	_orcaLines.clear();
	_orcaLinesColliding.clear();


	const float invTimeHorizonObst = 1.0f / _timeHorizonObst;

	RvoVec2 sumColliding(0.0f,0.0f);

	/* Create obstacle ORCA lines. */
	for (size_t i = 0; i < _obstacleNeighbors.size(); ++i) 
	{

		const RvoObstacle *obstacle1 = _obstacleNeighbors[i].second;
		const RvoObstacle *obstacle2 = obstacle1->_nextObstacle;

		RvoVec2 relativePosition1 = obstacle1->_point - GetPos();
		RvoVec2 relativePosition2 = obstacle2->_point - GetPos();

		if ((relativePosition1.x_==0.0f)&&(relativePosition1.y_==0.0f))
			relativePosition1.x_=relativePosition1.y_=0.001f;
		if ((relativePosition2.x_==0.0f)&&(relativePosition2.y_==0.0f))
			relativePosition2.x_=relativePosition2.y_=0.001f;

		/*
		 * Check if velocity obstacle of obstacle is already taken care of by
		 * previously constructed obstacle ORCA lines.
		 */
		bool alreadyCovered = false;

		for (size_t j = 0; j < _orcaLines.size(); ++j) 
		{
			if (det(invTimeHorizonObst * relativePosition1 - _orcaLines[j].point, _orcaLines[j].direction) - invTimeHorizonObst * _radius >= -RVO_EPSILON && 
				det(invTimeHorizonObst * relativePosition2 - _orcaLines[j].point, _orcaLines[j].direction) - invTimeHorizonObst * _radius >=  -RVO_EPSILON) 
			{
				alreadyCovered = true;
				break;
			}
		}

		if (!alreadyCovered)
		{
			for (size_t j = 0; j < _orcaLinesColliding.size(); ++j) 
			{
				if (det(invTimeHorizonObst * relativePosition1 - _orcaLinesColliding[j].point, _orcaLinesColliding[j].direction) - invTimeHorizonObst * _radius >= -RVO_EPSILON && 
					det(invTimeHorizonObst * relativePosition2 - _orcaLinesColliding[j].point, _orcaLinesColliding[j].direction) - invTimeHorizonObst * _radius >=  -RVO_EPSILON) 
				{
					alreadyCovered = true;
					break;
				}
			}
		}


		if (alreadyCovered) 
		{
			continue;
		}

		/* Not yet covered. Check for collisions. */

		const float distSq1 = absSq(relativePosition1);
		const float distSq2 = absSq(relativePosition2);

		const float radius=0.01f;//_radius
		const float radiusSq = RVO::sqr(radius);

		const RvoVec2 obstacleVector = obstacle2->_point - obstacle1->_point;
		const float s = (-relativePosition1 * obstacleVector) / absSq(obstacleVector);
		const float distSqLine = absSq(-relativePosition1 - s * obstacleVector);

		RVO::Line line;

		if (s < 0.0f && distSq1 <= radiusSq) 
		{
			sumColliding+=(-obstacle1->_unitDir)*obstacle1->getLen();
			/* Collision with left vertex. Ignore if non-convex. */
			if (obstacle1->_isConvex) 
			{
				line.point = RvoVec2(0.0f, 0.0f);
				line.direction = normalize(RvoVec2(-relativePosition1.y(), relativePosition1.x()));
				_orcaLinesColliding.push_back(line);
			}

			continue;
		}
		else if (s > 1.0f && distSq2 <= radiusSq) 
		{
			sumColliding+=(-obstacle1->_unitDir)*obstacle1->getLen();
			/* Collision with right vertex. Ignore if non-convex
			 * or if it will be taken care of by neighoring obstace */
			if (obstacle2->_isConvex && det(relativePosition2, obstacle2->_unitDir) >= 0.0f) 
			{
				line.point = RvoVec2(0.0f, 0.0f);
				line.direction = normalize(RvoVec2(-relativePosition2.y(), relativePosition2.x()));
				_orcaLinesColliding.push_back(line);
			}

			continue;
		}
		else if (s >= 0.0f && s < 1.0f && distSqLine <= radiusSq) 
		{
			sumColliding+=(-obstacle1->_unitDir)*obstacle1->getLen();

			/* Collision with obstacle segment. */
			line.point = RvoVec2(0.0f, 0.0f);
			line.direction = -obstacle1->_unitDir;
			_orcaLinesColliding.push_back(line);
			continue;
		}

		/*
		 * No collision.
		 * Compute legs. When obliquely viewed, both legs can come from a single
		 * vertex. Legs extend cut-off line when nonconvex vertex.
		 */

		RvoVec2 leftLegDirection, rightLegDirection;

		if (s < 0.0f && distSqLine <= radiusSq) 
		{
			/*
			 * Obstacle viewed obliquely so that left vertex
			 * defines velocity obstacle.
			 */
			if (!obstacle1->_isConvex) 
			{
				/* Ignore obstacle. */
				continue;
			}

			obstacle2 = obstacle1;

			const float leg1 = std::sqrtf(distSq1 - radiusSq);
			leftLegDirection = RvoVec2(relativePosition1.x() * leg1 - relativePosition1.y() * radius, relativePosition1.x() * radius + relativePosition1.y() * leg1) / distSq1;
			rightLegDirection = RvoVec2(relativePosition1.x() * leg1 + relativePosition1.y() * radius, -relativePosition1.x() * radius + relativePosition1.y() * leg1) / distSq1;
		}
		else if (s > 1.0f && distSqLine <= radiusSq) 
		{
			/*
			 * Obstacle viewed obliquely so that
			 * right vertex defines velocity obstacle.
			 */
			if (!obstacle2->_isConvex) 
			{
				/* Ignore obstacle. */
				continue;
			}

			obstacle1 = obstacle2;

			const float leg2 = std::sqrtf(distSq2 - radiusSq);
			leftLegDirection = RvoVec2(relativePosition2.x() * leg2 - relativePosition2.y() * radius, relativePosition2.x() * radius + relativePosition2.y() * leg2) / distSq2;
			rightLegDirection = RvoVec2(relativePosition2.x() * leg2 + relativePosition2.y() * radius, -relativePosition2.x() * radius + relativePosition2.y() * leg2) / distSq2;
		}
		else 
		{
			/* Usual situation. */
			if (obstacle1->_isConvex) 
			{
				const float leg1 = std::sqrtf(distSq1 - radiusSq);
				leftLegDirection = RvoVec2(relativePosition1.x() * leg1 - relativePosition1.y() * radius, relativePosition1.x() * radius + relativePosition1.y() * leg1) / distSq1;
			}
			else 
			{
				/* Left vertex non-convex; left leg extends cut-off line. */
				leftLegDirection = -obstacle1->_unitDir;
			}

			if (obstacle2->_isConvex) 
			{
				const float leg2 = std::sqrtf(distSq2 - radiusSq);
				rightLegDirection = RvoVec2(relativePosition2.x() * leg2 + relativePosition2.y() * radius, -relativePosition2.x() * radius + relativePosition2.y() * leg2) / distSq2;
			}
			else 
			{
				/* Right vertex non-convex; right leg extends cut-off line. */
				rightLegDirection = obstacle1->_unitDir;
			}
		}

		/*
		 * Legs can never point into neighboring edge when convex vertex,
		 * take cutoff-line of neighboring edge instead. If velocity projected on
		 * "foreign" leg, no constraint is added.
		 */

		const RvoObstacle *const leftNeighbor = obstacle1->_prevObstacle;

		bool isLeftLegForeign = false;
		bool isRightLegForeign = false;

		if (obstacle1->_isConvex && det(leftLegDirection, -leftNeighbor->_unitDir) >= 0.0f) 
		{
			/* Left leg points into obstacle. */
			leftLegDirection = -leftNeighbor->_unitDir;
			isLeftLegForeign = true;
		}

		if (obstacle2->_isConvex && det(rightLegDirection, obstacle2->_unitDir) <= 0.0f) 
		{
			/* Right leg points into obstacle. */
			rightLegDirection = obstacle2->_unitDir;
			isRightLegForeign = true;
		}

		/* Compute cut-off centers. */
		const RvoVec2 leftCutoff = invTimeHorizonObst * (obstacle1->_point - GetPos());
		const RvoVec2 rightCutoff = invTimeHorizonObst * (obstacle2->_point - GetPos());
		const RvoVec2 cutoffVec = rightCutoff - leftCutoff;

		/* Project current velocity on velocity obstacle. */

		/* Check if current velocity is projected on cutoff circles. */
		const float t = (obstacle1 == obstacle2 ? 0.5f : ((_velocity - leftCutoff) * cutoffVec) / absSq(cutoffVec));
		const float tLeft = ((_velocity - leftCutoff) * leftLegDirection);
		const float tRight = ((_velocity - rightCutoff) * rightLegDirection);

		if ((t < 0.0f && tLeft < 0.0f) || (obstacle1 == obstacle2 && tLeft < 0.0f && tRight < 0.0f)) 
		{
			/* Project on left cut-off circle. */
			const RvoVec2 unitW = normalize(_velocity - leftCutoff);

			line.direction = RvoVec2(unitW.y(), -unitW.x());
			line.point = leftCutoff + radius * invTimeHorizonObst * unitW;
			_orcaLines.push_back(line);
			continue;
		}
		else if (t > 1.0f && tRight < 0.0f) 
		{
			/* Project on right cut-off circle. */
			const RvoVec2 unitW = normalize(_velocity - rightCutoff);

			line.direction = RvoVec2(unitW.y(), -unitW.x());
			line.point = rightCutoff + radius * invTimeHorizonObst * unitW;
			_orcaLines.push_back(line);
			continue;
		}

		/*
		 * Project on left leg, right leg, or cut-off line, whichever is closest
		 * to velocity.
		 */
		const float distSqCutoff = ((t < 0.0f || t > 1.0f || obstacle1 == obstacle2) ? std::numeric_limits<float>::infinity() : absSq(_velocity - (leftCutoff + t * cutoffVec)));
		const float distSqLeft = ((tLeft < 0.0f) ? std::numeric_limits<float>::infinity() : absSq(_velocity - (leftCutoff + tLeft * leftLegDirection)));
		const float distSqRight = ((tRight < 0.0f) ? std::numeric_limits<float>::infinity() : absSq(_velocity - (rightCutoff + tRight * rightLegDirection)));

		if (distSqCutoff <= distSqLeft && distSqCutoff <= distSqRight) 
		{
			/* Project on cut-off line. */
			line.direction = -obstacle1->_unitDir;
			line.point = leftCutoff + radius * invTimeHorizonObst * RvoVec2(-line.direction.y(), line.direction.x());
			_orcaLines.push_back(line);
			continue;
		}
		else if (distSqLeft <= distSqRight) 
		{
			/* Project on left leg. */
			if (isLeftLegForeign) {
				continue;
			}

			line.direction = leftLegDirection;
			line.point = leftCutoff + radius * invTimeHorizonObst * RvoVec2(-line.direction.y(), line.direction.x());
			_orcaLines.push_back(line);
			continue;
		}
		else 
		{
			/* Project on right leg. */
			if (isRightLegForeign) 
			{
				continue;
			}

			line.direction = -rightLegDirection;
			line.point = rightCutoff + radius * invTimeHorizonObst * RvoVec2(-line.direction.y(), line.direction.x());
			_orcaLines.push_back(line);
			continue;
		}
	}

	if (_orcaLinesColliding.size()>0)
	{
		float len=abs(sumColliding);

		RVO::Line line;
		if (len>0.001f)
		{
			sumColliding/=len;
			line.point = RvoVec2(0.0f, 0.0f);
			line.direction = sumColliding;
			_orcaLines.push_back(line);
		}
		else
			_orcaLines.push_back(_orcaLinesColliding[0]);
	}


	const size_t numObstLines = _orcaLines.size();

	const float invTimeHorizon = 1.0f / _timeHorizon;

	/* Create unit ORCA lines. */
	for (size_t i = 0; i < _unitNeighbors.size(); ++i) 
	{
		const RvoUnit *const other = _unitNeighbors[i].second;

		const RvoVec2 relativePosition = other->GetPos() - GetPos();
		const RvoVec2 relativeVelocity = _velocity - other->_velocity;
		const float distSq = absSq(relativePosition);

		float radius=_radius;
		float radiusOther=other->_radius;

		if (_radLastAvoid>=0.0f)
			radius*=0.3f;
		if (other->_radLastAvoid>=0.0f)
			radiusOther*=0.3f;

		const float combinedRadius = radius+ radiusOther;
		const float combinedRadiusSq = RVO::sqr(combinedRadius);

		RVO::Line line;
		RvoVec2 u;

		if (distSq > combinedRadiusSq) 
		{
			/* No collision. */
			const RvoVec2 w = relativeVelocity - invTimeHorizon * relativePosition;
			/* Vector from cutoff center to relative velocity. */
			const float wLengthSq = absSq(w);

			const float dotProduct1 = w * relativePosition;

			if (dotProduct1 < 0.0f && RVO::sqr(dotProduct1) > combinedRadiusSq * wLengthSq) 
			{
				/* Project on cut-off circle. */
				const float wLength = std::sqrtf(wLengthSq);

				const RvoVec2 unitW = w / wLength;

				line.direction = RvoVec2(unitW.y(), -unitW.x());
				u = (combinedRadius * invTimeHorizon - wLength) * unitW;
			}
			else 
			{
				/* Project on legs. */
				const float leg = std::sqrtf(distSq - combinedRadiusSq);

				if (det(relativePosition, w) > 0.0f) 
				{
					/* Project on left leg. */
					line.direction = RvoVec2(relativePosition.x() * leg - relativePosition.y() * combinedRadius, relativePosition.x() * combinedRadius + relativePosition.y() * leg) / distSq;
				}
				else 
				{
					/* Project on right leg. */
					line.direction = -RvoVec2(relativePosition.x() * leg + relativePosition.y() * combinedRadius, -relativePosition.x() * combinedRadius + relativePosition.y() * leg) / distSq;
				}

				const float dotProduct2 = relativeVelocity * line.direction;

				u = dotProduct2 * line.direction - relativeVelocity;
			}
		}
		else 
		{
			/* Collision. Project on cut-off circle of time timeStep. */
			const float invTimeStep = 1.0f / _sim->_timeStep;

			/* Vector from cutoff center to relative velocity. */
			const RvoVec2 w = relativeVelocity - invTimeStep * relativePosition;

			const float wLength = abs(w);
			const RvoVec2 unitW = w / wLength;

			line.direction = RvoVec2(unitW.y(), -unitW.x());
			u = (combinedRadius * invTimeStep - wLength) * unitW;
		}

        float alpha = 0.5f;
        if (_weight + other->_weight > 0.0f)
            alpha = _weight / (_weight + other->_weight);
		line.point = _velocity + alpha * u;
		_orcaLines.push_back(line);
	}

	size_t lineFail = linearProgram2(_orcaLines, _maxSpeed, _prefVelocity, false, _newVelocity);

	if (lineFail < _orcaLines.size()) 
	{
		linearProgram3(_orcaLines, numObstLines, lineFail, _maxSpeed, _newVelocity);
	}

	
	if (isnan(_newVelocity.x_)||isnan(_newVelocity.y_))
	{
		_newVelocity.x_=_newVelocity.y_=0.01f;
	}
}

void RvoUnit::insertUnitNeighbor(const RvoUnit *unit, float &rangeSq)
{
	if (this != unit) 
	{
		const float distSq = absSq(GetPos() - unit->GetPos());

		if (distSq < rangeSq) 
		{
			if (_unitNeighbors.size() < _maxNeighbors) 
			{
				_unitNeighbors.push_back(std::make_pair(distSq, unit));
			}

			size_t i = _unitNeighbors.size() - 1;

			while (i != 0 && distSq < _unitNeighbors[i - 1].first) 
			{
				_unitNeighbors[i] = _unitNeighbors[i - 1];
				--i;
			}

			_unitNeighbors[i] = std::make_pair(distSq, unit);

			if (_unitNeighbors.size() == _maxNeighbors) 
			{
				rangeSq = _unitNeighbors.back().first;
			}
		}
	}
}

void RvoUnit::insertObstacleNeighbor(const RvoObstacle *obstacle, float rangeSq)
{
	const RvoObstacle *const nextObstacle = obstacle->_nextObstacle;

	const float distSq = RVO::distSqPointLineSegment(obstacle->_point, nextObstacle->_point, GetPos());

	if (distSq < rangeSq) {
		_obstacleNeighbors.push_back(std::make_pair(distSq, obstacle));

		size_t i = _obstacleNeighbors.size() - 1;

		while (i != 0 && distSq < _obstacleNeighbors[i - 1].first) {
			_obstacleNeighbors[i] = _obstacleNeighbors[i - 1];
			--i;
		}

		_obstacleNeighbors[i] = std::make_pair(distSq, obstacle);
	}
}

void RvoUnit::update()
{
	_velocity = _newVelocity;
	SetPos(GetPos()+_velocity * _sim->_timeStep);
}

