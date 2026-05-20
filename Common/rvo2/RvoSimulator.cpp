/*
 * RVOSimulator.cpp
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

#include "math/line2d.h"

#include "commondefines/general_stl.h"


#include "RvoSimulator.h"

#include "RvoUnit.h"
#include "RvoObstacleMap.h"
#include "RvoObstacle.h"

#include "nav/navdata.h"

#include "unitmgr/unitmgr.h"

#ifdef _OPENMP
#include <omp.h>
#endif

//////////////////////////////////////////////////////////////////////////
//CAvoidHintTrail
void CAvoidHintTrail::Init(float distMax)
{
	_distMax=distMax;
}

void CAvoidHintTrail::Clear()
{
	_nodes.clear();
	Zero();
}

void CAvoidHintTrail::AddPos(i_math::vector2df &pos)
{
	Node node;
	node.pos=pos;
	_nodes.push_back(node);

	if (_nodes.size()<=1)
		return;

	while(_nodes.size()>1)
		_nodes.pop_front();


// 	_nodes[_nodes.size()-2].dist=_nodes[_nodes.size()-2].pos.getDistanceFrom(pos);
// 	_distTotal+=_nodes[_nodes.size()-2].dist;
// 
// 	while(_distTotal>_distMax)
// 	{
// 		_distTotal-=_nodes[0].dist;
// 		_nodes.pop_front();
// 		if (_nodes.empty())
// 			break;
// 	}
// 
// 	//重新更新一遍,以防浮点运算误差累计
// 	_distTotal=0.0f;
// 	for (int i=0;i<_nodes.size();i++)
// 		_distTotal+=_nodes[i].dist;

}


//////////////////////////////////////////////////////////////////////////
//RvoSimulator


RvoSimulator::RvoSimulator() : _defaultUnit(NULL), _globalTime(0.0f), _nmesh(NULL), _timeStep(0.0f),_mirror(NULL)
{
}

RvoSimulator::RvoSimulator(float timeStep, float neighborDist, size_t maxNeighbors, float timeHorizon, float timeHorizonObst, float radius, float maxSpeed, const RvoVec2 &velocity) : _defaultUnit(NULL), _globalTime(0.0f), _timeStep(timeStep)
{
	_nmesh = NULL;
	_mirror =NULL;

	_defaultUnit = Class_New2(RvoUnit);
	_defaultUnit->setSim(this);

	_defaultUnit->_maxNeighbors = maxNeighbors;
	_defaultUnit->_maxSpeed = maxSpeed;
	_defaultUnit->_neighborDist = neighborDist;
	_defaultUnit->_radius = radius;
	_defaultUnit->_timeHorizon = timeHorizon;
	_defaultUnit->_timeHorizonObst = timeHorizonObst;
	_defaultUnit->_velocity = velocity;
}

RvoSimulator::~RvoSimulator()
{
	Safe_Class_Delete(_defaultUnit);

	for (size_t i = 0; i < _units.size(); ++i) 
	{
		Safe_Class_Delete(_units[i]);
	}
}

void RvoSimulator::init(navMesh *nmesh,float timeStep)
{
	_nmesh=nmesh;
	i_math::recti rc;
	int lenTile=nmesh->getTileLen();
	rc.Left()=nmesh->getLeft()*lenTile;
	rc.Right()=nmesh->getRight()*lenTile;
	rc.Top()=nmesh->getTop()*lenTile;
	rc.Bottom()=nmesh->getBottom()*lenTile;

	_unitmap.Create(rc);

	_globalTime=0.0f;
	_timeStep=timeStep;

	_trailAvoidHint.Init(5.0f);

}

void RvoSimulator::clear()
{
	for (size_t i = 0; i < _units.size(); ++i) 
	{
		_unitmap.RemoveUnit(_units[i]);
		Safe_Class_Delete(_units[i]);
	}

	_unitmap.Destroy();

	_trailAvoidHint.Clear();

}



RvoUnit *RvoSimulator::addUnit(const i_math::vector2df&position,BOOL bAllowMirror)
{
	if (_defaultUnit == NULL) 
	{
		return NULL;
	}

	RvoUnit *unit = Class_New2(RvoUnit);
	unit->setSim(this);

	unit->Pos()=position;
	unit->_maxNeighbors = _defaultUnit->_maxNeighbors;
	unit->_maxSpeed = _defaultUnit->_maxSpeed;
	unit->_neighborDist = _defaultUnit->_neighborDist;
	unit->_radius = _defaultUnit->_radius;
	unit->_timeHorizon = _defaultUnit->_timeHorizon;
	unit->_timeHorizonObst = _defaultUnit->_timeHorizonObst;
	unit->_velocity = _defaultUnit->_velocity;

	unit->_id = _units.size();

	_units.push_back(unit);
	_unitmap.AddUnit(unit);

	if (bAllowMirror)
	{
		if (_mirror)
		{
			unit->_mirror=_mirror->CreateUnit(unit->Pos(),0.0f,unit->_radius,0.0f,NULL,FALSE);

			if (TRUE)
			{
				extern void UnitCollide_SetStatic(UnitCollide &collide,BOOL bStatic);
				UnitCollide collide=UnitCollide_Empty;
				UnitCollide_SetStatic(collide,TRUE);
				unit->_mirror->SetCollide(collide);
			}
		}
	}

	return unit;
}

RvoUnit* RvoSimulator::addUnit(const i_math::vector2df&position, 
							   float neighborDist, size_t maxNeighbors, 
							   float timeHorizon, float timeHorizonObst, 
							   float radius, float maxSpeed, 
							   BOOL bAllowMirror,const RvoVec2 &velocity)
{
	RvoUnit *unit = Class_New2(RvoUnit);
	unit->setSim(this);

	unit->Pos()=position;
	unit->_maxNeighbors = maxNeighbors;
	unit->_maxSpeed = maxSpeed;
	unit->_neighborDist = neighborDist;
	unit->_radius = radius;
	unit->_timeHorizon = timeHorizon;
	unit->_timeHorizonObst = timeHorizonObst;
	unit->_velocity = velocity;

	unit->_id = _units.size();

	_units.push_back(unit);

	_unitmap.AddUnit(unit);

	if (bAllowMirror)
	{
		if (_mirror)
			unit->_mirror=_mirror->CreateUnit(unit->Pos(),0.0f,unit->_radius,0.0f,NULL,FALSE);
	}

	return unit;
}

void RvoSimulator::removeUnit(RvoUnit *unit)
{
	if (!unit)
		return;

	int idx;
	VEC_FIND(_units,unit,idx);

	if (idx!=-1)
	{
		_unitmap.RemoveUnit(unit);
		_units.erase(_units.begin()+idx);

		if (_mirror)
		{
			if (unit->_mirror)
				unit->_mirror->Destroy();
		}

		Safe_Class_Delete(unit);
	}
}

void RvoSimulator::_collectObstacleNeighbors(RvoUnit *unit)
{
	unit->_obstacleNeighbors.clear();

	float range=unit->getObstacleNeighborRange();
	float range2=range*range;
	i_math::rectf rc;
	rc.set(unit->GetPos().x_,unit->GetPos().y_,unit->GetPos().x_,unit->GetPos().y_);
	rc.inflate(range);

	RvoObstacleMap *mp=(RvoObstacleMap *)_nmesh->getRvoObstacleMap();
	if (mp)
		mp->collectNeighbors(unit,range,_bufWorking);
}

void RvoSimulator::_collectUnitNeighbors(RvoUnit *unit)
{
	unit->_unitNeighbors.clear();
	if (unit->_maxNeighbors>0)
	{
		float range=unit->getUnitNeighborRange();
		float range2=range*range;

		_unitmap.Enum(unit,range);
		DWORD c;
		CUnitBase **units=_unitmap.GetEnums(c);
		
		for (int i=0;i<c;i++)
		{
			RvoUnit *unit2=((RvoUnit*)units[i]);
			if (unit==unit2)
				continue;

			if (unit2->_bGhostCollide)
				continue;

			unit->insertUnitNeighbor(unit2,range2);
		}
	}
}

void RvoSimulator::_makeAvoid(RvoUnit *unit)
{
	_ranges.Clear();

	RvoVec2 dirMove=unit->_prefVelocity*_timeStep;

	float distMove=abs(dirMove);
	float speedMove=distMove/_timeStep;

	if (distMove<=0.0f)
	{
		unit->_radLastAvoid=-1.0f;
		unit->_bStuck=FALSE;
		return;
	}

	float radian;//unit移动方向的角度
	radian=atan2f(dirMove.y(),dirMove.x());

	for (int i=0;i<unit->_unitNeighbors.size();i++)
	{
		const RvoUnit*unitNeighbor=unit->_unitNeighbors[i].second;
		if (unitNeighbor->_maxNeighbors>0)
			continue;
		if (unitNeighbor->_idPlayer==unit->_idPlayer)
			continue;//不绕队友及主人
		float distNeighbor=sqrtf(unit->_unitNeighbors[i].first);
		if (distNeighbor>unit->_radius+unitNeighbor->_radius+distMove)
			continue;//距离很远,没有碰撞

		float rSum=unit->_radius+unitNeighbor->_radius;
		RvoVec2 dir=unitNeighbor->GetPos()-unit->GetPos();

		//unitNeighbor相对于unit的角度
		float radianOff=atan2f(dir.y(),dir.x())-radian;
		radianOff=i_math::normalize_radian(radianOff);

		float scatter;
		if (rSum>distNeighbor)
		{
			//embedded
			scatter=i_math::Pi/2.0f;
		}
		else
			scatter=asin(rSum/distNeighbor);

		_ranges.Add(radianOff-scatter,radianOff+scatter,(CUnitBase*)unitNeighbor);
	}

	unit->_bStuck=FALSE;
	CUnitMgr::CircumRanges::Range *range=_ranges.FindRange(0.0f);
	if (range)
	{
		if (range->IsFull())
		{
			//Stuck
			unit->_prefVelocity.x_=unit->_prefVelocity.y_=0.0f;
			unit->_bStuck=TRUE;
		}
		else
		{
			float rLow,rHi;
			rLow=radian+range->low;
			rHi=radian+range->hi;

			BOOL bLow=TRUE;

			float radianAvoid;
			if (unit->_radLastAvoid<0.0f)
			{//上次没有在绕
				int nHi=0,nLow=0;

				for (int i=0;i<_trailAvoidHint._nodes.size();i++)
				{
					i_math::vector2df dir=_trailAvoidHint._nodes[i].pos-unit->_pos;
					float rad=atan2f(dir.y,dir.x);
					if (i_math::get_radian_dist(rLow,rad)<i_math::get_radian_dist(rHi,rad))
						nLow++;
					else
						nHi++;
				}
				if (nLow>nHi)
					radianAvoid=i_math::wrap_radian(rLow);
				else
				{
					radianAvoid=i_math::wrap_radian(rHi);
					bLow=FALSE;
				}
			}
			else
			{
				//选择一个和上次的选择更接近的方向
				if (i_math::get_radian_dist(rLow,unit->_radLastAvoid)<i_math::get_radian_dist(rHi,unit->_radLastAvoid))
					radianAvoid=i_math::wrap_radian(rLow);
				else
				{
					radianAvoid=i_math::wrap_radian(rHi);

					bLow=FALSE;
				}
			}

			RvoVec2 dirMoveNew;
			dirMoveNew.x_=cosf(radianAvoid)*distMove;
			dirMoveNew.y_=sinf(radianAvoid)*distMove;

			//检查障碍
			BOOL bObstacled=FALSE;
			if (TRUE)
			{
				i_math::line2df lineMove;
				lineMove.start=unit->_pos;
				lineMove.end.set(dirMove.x(),dirMove.y());
				lineMove.end+=lineMove.start;

				i_math::line2df lineObstacle;
				for (int i=0;i<unit->_obstacleNeighbors.size();i++)
				{
					const RvoObstacle *obstacle=unit->_obstacleNeighbors[i].second;
					RvoObstacle *obstacleNext=obstacle->_nextObstacle;
					if (obstacle&&obstacleNext)
					{
						lineObstacle.start.set(obstacle->_point.x(),obstacle->_point.y());
						lineObstacle.end.set(obstacleNext->_point.x(),obstacleNext->_point.y());

						i_math::vector2df posIntersect;
						if (lineMove.getIntersectionPoint(lineObstacle,posIntersect))
						{
							if (lineObstacle.classifyPoint(lineMove.start)>=0)
							{
								bObstacled=TRUE;
								break;
							}
						}
					}
				}
			}

			if (bObstacled)
			{
				if (bLow)
					radianAvoid=rHi;
				else
					radianAvoid=rLow;
			}

			unit->_radLastAvoid=radianAvoid;
			unit->_prefVelocity.x_=cosf(radianAvoid)*speedMove;
			unit->_prefVelocity.y_=sinf(radianAvoid)*speedMove;
		}
	}
	else
		unit->_radLastAvoid=-1.0f;


}


void RvoSimulator::doStep()
{
	for (int i = 0; i < static_cast<int>(_units.size()); ++i) 
	{
		if (_units[i]->_maxNeighbors<=0)
			continue;
		_collectObstacleNeighbors(_units[i]);
		_collectUnitNeighbors(_units[i]);

		_makeAvoid(_units[i]);

		_units[i]->computeNewVelocity();
	}

	for (int i = 0; i < static_cast<int>(_units.size()); ++i) 
	{
		RvoUnit *unit=_units[i];
		unit->update();
// 		unit->_velocity*=0.2f;

		if (_mirror)
		{
			if (unit->_mirror)
				unit->_mirror->Reset(unit->_pos,0.0f);
		}
		_unitmap.UpdateUnit(unit);

	}

	_globalTime += _timeStep;
}

size_t RvoSimulator::getUnitNeighbor(size_t unitNo, size_t neighborNo) const
{
	return _units[unitNo]->_unitNeighbors[neighborNo].second->_id;
}

size_t RvoSimulator::getUnitMaxNeighbors(size_t unitNo) const
{
	return _units[unitNo]->_maxNeighbors;
}

float RvoSimulator::getUnitMaxSpeed(size_t unitNo) const
{
	return _units[unitNo]->_maxSpeed;
}

float RvoSimulator::getUnitNeighborDist(size_t unitNo) const
{
	return _units[unitNo]->_neighborDist;
}

size_t RvoSimulator::getUnitNumUnitNeighbors(size_t unitNo) const
{
	return _units[unitNo]->_unitNeighbors.size();
}

size_t RvoSimulator::getUnitNumObstacleNeighbors(size_t unitNo) const
{
	return _units[unitNo]->_obstacleNeighbors.size();
}

size_t RvoSimulator::getUnitNumORCALines(size_t unitNo) const
{
	return _units[unitNo]->_orcaLines.size();
}

size_t RvoSimulator::getUnitObstacleNeighbor(size_t unitNo, size_t neighborNo) const
{
	return _units[unitNo]->_obstacleNeighbors[neighborNo].second->_id;
}

const RVO::Line &RvoSimulator::getUnitORCALine(size_t unitNo, size_t lineNo) const
{
	return _units[unitNo]->_orcaLines[lineNo];
}

const RvoVec2 &RvoSimulator::getUnitPrefVelocity(size_t unitNo) const
{
	return _units[unitNo]->_prefVelocity;
}

float RvoSimulator::getUnitRadius(size_t unitNo) const
{
	return _units[unitNo]->_radius;
}

float RvoSimulator::getUnitTimeHorizon(size_t unitNo) const
{
	return _units[unitNo]->_timeHorizon;
}

float RvoSimulator::getUnitTimeHorizonObst(size_t unitNo) const
{
	return _units[unitNo]->_timeHorizonObst;
}

const RvoVec2 &RvoSimulator::getUnitVelocity(size_t unitNo) const
{
	return _units[unitNo]->_velocity;
}

float RvoSimulator::getGlobalTime() const
{
	return _globalTime;
}

size_t RvoSimulator::getNumUnits() const
{
	return _units.size();
}

float RvoSimulator::getTimeStep() const
{
	return _timeStep;
}

bool RvoSimulator::queryVisibility(const RvoVec2 &point1, const RvoVec2 &point2, float radius) const
{
// 		return _obstacleTree->queryVisibility(point1, point2, radius);
	return false;
}

void RvoSimulator::setUnitDefaults(float neighborDist, size_t maxNeighbors, float timeHorizon, float timeHorizonObst, float radius, float maxSpeed, const RvoVec2 &velocity)
{
	if (_defaultUnit == NULL) 
	{
		_defaultUnit = Class_New2(RvoUnit);
		_defaultUnit->setSim(this);
	}

	_defaultUnit->_maxNeighbors = maxNeighbors;
	_defaultUnit->_maxSpeed = maxSpeed;
	_defaultUnit->_neighborDist = neighborDist;
	_defaultUnit->_radius = radius;
	_defaultUnit->_timeHorizon = timeHorizon;
	_defaultUnit->_timeHorizonObst = timeHorizonObst;
	_defaultUnit->_velocity = velocity;
}


void RvoSimulator::setTimeStep(float timeStep)
{
	_timeStep = timeStep;
}
