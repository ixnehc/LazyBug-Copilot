/*
 * RvoUnit.h
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

#pragma once

/**
 * \file       RvoUnit.h
 * \brief      Contains the RvoUnit class.
 */

#include "Definitions.h"
#include "RvoSimulator.h"

class CUnit;
class CUnitMgr;

/**
 * \brief      Defines an unit in the simulation.
 */
class RvoUnit:public CUnitBase
{
public:
	DEFINE_CLASS(RvoUnit);
	RvoUnit();

	float getObstacleNeighborRange()		{			return _timeHorizonObst * _maxSpeed + _radius;		}
	float getUnitNeighborRange()		{			return _neighborDist;		}

	void SetPos(const RvoVec2 &pos)		{			_pos.set(pos.x_,pos.y_);		}
	RvoVec2 GetPos() const
	{
		RvoVec2 pos;
		pos.x_=_pos.x;
		pos.y_=_pos.y;
		return pos;
	}

	i_math::vector2df &Pos()	{		return _pos;	}

	void setPrefVelocity(i_math::vector2df &vel)
	{
		_prefVelocity.x_=vel.x;
		_prefVelocity.y_=vel.y;
	}

	void setMaxSpeed(float speed)	{		_maxSpeed=speed;	}
	void setWeight(float wt)	{		_weight=wt;	}

	void setTimeHorizon(float timeHorizon)	{		_timeHorizon=timeHorizon;	}

	void setGhostCollide(BOOL bGhostCollide)	{		_bGhostCollide=bGhostCollide;	}

	CUnit *getMirror()	{		return _mirror;	}
	void setPlayerID(int idPlayer)	{		_idPlayer=idPlayer;	}

	float getLastAvoidRad()	{		return _radLastAvoid;	}

	BOOL isStuck()const	{		return _bStuck;	}

private:
	/**
	 * \brief      Constructs an unit instance.
	 * \param      sim             The simulator instance.
	 */

	void setSim(RvoSimulator *sim)		{			_sim=sim;		}

	/**
	 * \brief      Computes the new velocity of this unit.
	 */
	void computeNewVelocity();

	/**
	 * \brief      Inserts an unit neighbor into the set of neighbors of
	 *             this unit.
	 * \param      unit           A pointer to the unit to be inserted.
	 * \param      rangeSq         The squared range around this unit.
	 */
	void insertUnitNeighbor(const RvoUnit *unit, float &rangeSq);

	/**
	 * \brief      Inserts a static obstacle neighbor into the set of neighbors
	 *             of this unit.
	 * \param      obstacle        The number of the static obstacle to be
	 *                             inserted.
	 * \param      rangeSq         The squared range around this unit.
	 */
	void insertObstacleNeighbor(const RvoObstacle *obstacle, float rangeSq);

	/**
	 * \brief      Updates the two-dimensional position and two-dimensional
	 *             velocity of this unit.
	 */
	void update();

	//被镜像的Unit
	BOOL isMirrored()const	{		return (!_mirror);	}//自己没有镜像的Unit

	std::vector<std::pair<float, const RvoUnit *> > _unitNeighbors;
	size_t _maxNeighbors;
	float _maxSpeed;
	float _neighborDist;
	RvoVec2 _newVelocity;
	std::vector<std::pair<float, const RvoObstacle *> > _obstacleNeighbors;
	std::vector<RVO::Line> _orcaLines;
	std::vector<RVO::Line> _orcaLinesColliding;
// 		RvoVec2 _position;
	RvoVec2 _prefVelocity;
	float _radius;
	RvoSimulator *_sim;
	float _timeHorizon;
	float _timeHorizonObst;
	RvoVec2 _velocity;
	float _weight;

	BOOL _bGhostCollide;
	BOOL _bStuck;

	float _radLastAvoid;

	CUnit *_mirror;

	size_t _id;
	size_t _idPlayer;

	friend class RvoObstacleMap;
	friend class RvoSimulator;
};

namespace RVO 
{

	/**
	 * \relates    Unit
	 * \brief      Solves a one-dimensional linear program on a specified line
	 *             subject to linear constraints defined by lines and a circular
	 *             constraint.
	 * \param      lines         Lines defining the linear constraints.
	 * \param      lineNo        The specified line constraint.
	 * \param      radius        The radius of the circular constraint.
	 * \param      optVelocity   The optimization velocity.
	 * \param      directionOpt  True if the direction should be optimized.
	 * \param      result        A reference to the result of the linear program.
	 * \return     True if successful.
	 */
	bool linearProgram1(const std::vector<Line> &lines, size_t lineNo,
						float radius, const RvoVec2 &optVelocity,
						bool directionOpt, RvoVec2 &result);

	/**
	 * \relates    Unit
	 * \brief      Solves a two-dimensional linear program subject to linear
	 *             constraints defined by lines and a circular constraint.
	 * \param      lines         Lines defining the linear constraints.
	 * \param      radius        The radius of the circular constraint.
	 * \param      optVelocity   The optimization velocity.
	 * \param      directionOpt  True if the direction should be optimized.
	 * \param      result        A reference to the result of the linear program.
	 * \return     The number of the line it fails on, and the number of lines if successful.
	 */
	size_t linearProgram2(const std::vector<Line> &lines, float radius,
						  const RvoVec2 &optVelocity, bool directionOpt,
						  RvoVec2 &result);

	/**
	 * \relates    Unit
	 * \brief      Solves a two-dimensional linear program subject to linear
	 *             constraints defined by lines and a circular constraint.
	 * \param      lines         Lines defining the linear constraints.
	 * \param      numObstLines  Count of obstacle lines.
	 * \param      beginLine     The line on which the 2-d linear program failed.
	 * \param      radius        The radius of the circular constraint.
	 * \param      result        A reference to the result of the linear program.
	 */
	void linearProgram3(const std::vector<Line> &lines, size_t numObstLines, size_t beginLine,
						float radius, RvoVec2 &result);
}

