/*
 * RVOSimulator.h
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
 * \file       RVOSimulator.h
 * \brief      Contains the RVOSimulator class.
 */

#include <cstddef>
#include <limits>
#include <vector>

#include "Definitions.h"
#include "RvoVec2.h"

#include "unitmgr/UnitMap.h"
#include "unitmgr/UnitMgr.h"


struct navMesh;

class CUnitMgr;

class RvoUnit;
class RvoTree;
class RvoObstacle;

class CAvoidHintTrail
{
public:
	CAvoidHintTrail()
	{
		Zero();
	}

	void Zero()
	{
		_distMax=0.0f;
		_distTotal=0.0f;
	}
	void Init(float distMax);
	void Clear();

	void AddPos(i_math::vector2df &pos);

protected:
	struct Node
	{
		Node()
		{
			dist=0.0f;
		}
		i_math::vector2df pos;
		float dist;//到下一个Node的距离
	};

	float _distMax;
	float _distTotal;
	std::deque<Node> _nodes;

	friend class RvoSimulator;
};


/**
 * \brief      Defines the simulation.
 *
 * The main class of the library that contains all simulation functionality.
 */
class RvoSimulator 
{
public:
	/**
	 * \brief      Constructs a simulator instance.
	 */
	RvoSimulator();

	/**
	 * \brief      Constructs a simulator instance and sets the default
	 *             properties for any new unit that is added.
	 * \param      timeStep        The time step of the simulation.
	 *                             Must be positive.
	 * \param      neighborDist    The default maximum distance (center point
	 *                             to center point) to other agents a new unit
	 *                             takes into account in the navigation. The
	 *                             larger this number, the longer he running
	 *                             time of the simulation. If the number is too
	 *                             low, the simulation will not be safe. Must be
	 *                             non-negative.
	 * \param      maxNeighbors    The default maximum number of other agents a
	 *                             new unit takes into account in the
	 *                             navigation. The larger this number, the
	 *                             longer the running time of the simulation.
	 *                             If the number is too low, the simulation
	 *                             will not be safe.
	 * \param      timeHorizon     The default minimal amount of time for which
	 *                             a new unit's velocities that are computed
	 *                             by the simulation are safe with respect to
	 *                             other agents. The larger this number, the
	 *                             sooner an unit will respond to the presence
	 *                             of other agents, but the less freedom the
	 *                             unit has in choosing its velocities.
	 *                             Must be positive.
	 * \param      timeHorizonObst The default minimal amount of time for which
	 *                             a new unit's velocities that are computed
	 *                             by the simulation are safe with respect to
	 *                             obstacles. The larger this number, the
	 *                             sooner an unit will respond to the presence
	 *                             of obstacles, but the less freedom the unit
	 *                             has in choosing its velocities.
	 *                             Must be positive.
	 * \param      radius          The default radius of a new unit.
	 *                             Must be non-negative.
	 * \param      maxSpeed        The default maximum speed of a new unit.
	 *                             Must be non-negative.
	 * \param      velocity        The default initial two-dimensional linear
	 *                             velocity of a new unit (optional).
	 */
	RvoSimulator(float timeStep, float neighborDist, size_t maxNeighbors,
				 float timeHorizon, float timeHorizonObst, float radius,
				 float maxSpeed, const RvoVec2 &velocity = RvoVec2());

	/**
	 * \brief      Destroys this simulator instance.
	 */
	~RvoSimulator();

	void init(navMesh *nmesh,float timeStep);
	void clear();

	void setMirror(CUnitMgr *mirror)	{		_mirror=mirror;	}

	CAvoidHintTrail &GetAvoidHintTrail()	{		return _trailAvoidHint;	}

	/**
	 * \brief      Adds a new unit with default properties to the
	 *             simulation.
	 * \param      position        The two-dimensional starting position of
	 *                             this unit.
	 * \return     The number of the unit, or RVO::RVO_ERROR when the unit
	 *             defaults have not been set.
	 */
	RvoUnit *addUnit(const i_math::vector2df&position,BOOL bAllowMirror=TRUE);

	/**
	 * \brief      Adds a new unit to the simulation.
	 * \param      position        The two-dimensional starting position of
	 *                             this unit.
	 * \param      neighborDist    The maximum distance (center point to
	 *                             center point) to other agents this unit
	 *                             takes into account in the navigation. The
	 *                             larger this number, the longer the running
	 *                             time of the simulation. If the number is too
	 *                             low, the simulation will not be safe.
	 *                             Must be non-negative.
	 * \param      maxNeighbors    The maximum number of other agents this
	 *                             unit takes into account in the navigation.
	 *                             The larger this number, the longer the
	 *                             running time of the simulation. If the
	 *                             number is too low, the simulation will not
	 *                             be safe.
	 * \param      timeHorizon     The minimal amount of time for which this
	 *                             unit's velocities that are computed by the
	 *                             simulation are safe with respect to other
	 *                             agents. The larger this number, the sooner
	 *                             this unit will respond to the presence of
	 *                             other agents, but the less freedom this
	 *                             unit has in choosing its velocities.
	 *                             Must be positive.
	 * \param      timeHorizonObst The minimal amount of time for which this
	 *                             unit's velocities that are computed by the
	 *                             simulation are safe with respect to
	 *                             obstacles. The larger this number, the
	 *                             sooner this unit will respond to the
	 *                             presence of obstacles, but the less freedom
	 *                             this unit has in choosing its velocities.
	 *                             Must be positive.
	 * \param      radius          The radius of this unit.
	 *                             Must be non-negative.
	 * \param      maxSpeed        The maximum speed of this unit.
	 *                             Must be non-negative.
	 * \param      velocity        The initial two-dimensional linear velocity
	 *                             of this unit (optional).
	 * \return     The number of the unit.
	 */
	RvoUnit *addUnit(const i_math::vector2df&position, float neighborDist,
					size_t maxNeighbors, float timeHorizon,
					float timeHorizonObst, float radius, float maxSpeed,BOOL bAllowMirror=TRUE,
					const RvoVec2 &velocity = RvoVec2());

	//Never keep the RvoUnit* ptr after calling this function
	void removeUnit(RvoUnit *unit);



	/**
	 * \brief      Lets the simulator perform a simulation step and updates the
	 *             two-dimensional position and two-dimensional velocity of
	 *             each unit.
	 */
	void doStep();

	/**
	 * \brief      Returns the specified unit neighbor of the specified
	 *             unit.
	 * \param      unitNo         The number of the unit whose unit
	 *                             neighbor is to be retrieved.
	 * \param      neighborNo      The number of the unit neighbor to be
	 *                             retrieved.
	 * \return     The number of the neighboring unit.
	 */
	size_t getUnitNeighbor(size_t unitNo, size_t neighborNo) const;

	/**
	 * \brief      Returns the maximum neighbor count of a specified unit.
	 * \param      unitNo         The number of the unit whose maximum
	 *                             neighbor count is to be retrieved.
	 * \return     The present maximum neighbor count of the unit.
	 */
	size_t getUnitMaxNeighbors(size_t unitNo) const;

	/**
	 * \brief      Returns the maximum speed of a specified unit.
	 * \param      unitNo         The number of the unit whose maximum speed
	 *                             is to be retrieved.
	 * \return     The present maximum speed of the unit.
	 */
	float getUnitMaxSpeed(size_t unitNo) const;

	/**
	 * \brief      Returns the maximum neighbor distance of a specified
	 *             unit.
	 * \param      unitNo         The number of the unit whose maximum
	 *                             neighbor distance is to be retrieved.
	 * \return     The present maximum neighbor distance of the unit.
	 */
	float getUnitNeighborDist(size_t unitNo) const;

	/**
	 * \brief      Returns the count of unit neighbors taken into account to
	 *             compute the current velocity for the specified unit.
	 * \param      unitNo         The number of the unit whose count of unit
	 *                             neighbors is to be retrieved.
	 * \return     The count of unit neighbors taken into account to compute
	 *             the current velocity for the specified unit.
	 */
	size_t getUnitNumUnitNeighbors(size_t unitNo) const;

	/**
	 * \brief      Returns the count of obstacle neighbors taken into account
	 *             to compute the current velocity for the specified unit.
	 * \param      unitNo         The number of the unit whose count of
	 *                             obstacle neighbors is to be retrieved.
	 * \return     The count of obstacle neighbors taken into account to
	 *             compute the current velocity for the specified unit.
	 */
	size_t getUnitNumObstacleNeighbors(size_t unitNo) const;


	/**
	 * \brief      Returns the count of ORCA constraints used to compute
	 *             the current velocity for the specified unit.
	 * \param      unitNo         The number of the unit whose count of ORCA
	 *                             constraints is to be retrieved.
	 * \return     The count of ORCA constraints used to compute the current
	 *             velocity for the specified unit.
	 */
	size_t getUnitNumORCALines(size_t unitNo) const;

	/**
	 * \brief      Returns the specified obstacle neighbor of the specified
	 *             unit.
	 * \param      unitNo         The number of the unit whose obstacle
	 *                             neighbor is to be retrieved.
	 * \param      neighborNo      The number of the obstacle neighbor to be
	 *                             retrieved.
	 * \return     The number of the first vertex of the neighboring obstacle
	 *             edge.
	 */
	size_t getUnitObstacleNeighbor(size_t unitNo, size_t neighborNo) const;

	/**
	 * \brief      Returns the specified ORCA constraint of the specified
	 *             unit.
	 * \param      unitNo         The number of the unit whose ORCA
	 *                             constraint is to be retrieved.
	 * \param      lineNo          The number of the ORCA constraint to be
	 *                             retrieved.
	 * \return     A line representing the specified ORCA constraint.
	 * \note       The halfplane to the left of the line is the region of
	 *             permissible velocities with respect to the specified
	 *             ORCA constraint.
	 */
	const RVO::Line &getUnitORCALine(size_t unitNo, size_t lineNo) const;

	/**
	 * \brief      Returns the two-dimensional preferred velocity of a
	 *             specified unit.
	 * \param      unitNo         The number of the unit whose
	 *                             two-dimensional preferred velocity is to be
	 *                             retrieved.
	 * \return     The present two-dimensional preferred velocity of the unit.
	 */
	const RvoVec2 &getUnitPrefVelocity(size_t unitNo) const;

	/**
	 * \brief      Returns the radius of a specified unit.
	 * \param      unitNo         The number of the unit whose radius is to
	 *                             be retrieved.
	 * \return     The present radius of the unit.
	 */
	float getUnitRadius(size_t unitNo) const;

	/**
	 * \brief      Returns the time horizon of a specified unit.
	 * \param      unitNo         The number of the unit whose time horizon
	 *                             is to be retrieved.
	 * \return     The present time horizon of the unit.
	 */
	float getUnitTimeHorizon(size_t unitNo) const;

	/**
	 * \brief      Returns the time horizon with respect to obstacles of a
	 *             specified unit.
	 * \param      unitNo         The number of the unit whose time horizon
	 *                             with respect to obstacles is to be
	 *                             retrieved.
	 * \return     The present time horizon with respect to obstacles of the
	 *             unit.
	 */
	float getUnitTimeHorizonObst(size_t unitNo) const;

	/**
	 * \brief      Returns the two-dimensional linear velocity of a
	 *             specified unit.
	 * \param      unitNo         The number of the unit whose
	 *                             two-dimensional linear velocity is to be
	 *                             retrieved.
	 * \return     The present two-dimensional linear velocity of the unit.
	 */
	const RvoVec2 &getUnitVelocity(size_t unitNo) const;

	/**
	 * \brief      Returns the global time of the simulation.
	 * \return     The present global time of the simulation (zero initially).
	 */
	float getGlobalTime() const;

	/**
	 * \brief      Returns the count of agents in the simulation.
	 * \return     The count of agents in the simulation.
	 */
	size_t getNumUnits() const;


	/**
	 * \brief      Returns the time step of the simulation.
	 * \return     The present time step of the simulation.
	 */
	float getTimeStep() const;

	/**
	 * \brief      Performs a visibility query between the two specified
	 *             points with respect to the obstacles
	 * \param      point1          The first point of the query.
	 * \param      point2          The second point of the query.
	 * \param      radius          The minimal distance between the line
	 *                             connecting the two points and the obstacles
	 *                             in order for the points to be mutually
	 *                             visible (optional). Must be non-negative.
	 * \return     A boolean specifying whether the two points are mutually
	 *             visible. Returns true when the obstacles have not been
	 *             processed.
	 */
	bool queryVisibility(const RvoVec2 &point1, const RvoVec2 &point2,
						 float radius = 0.0f) const;

	/**
	 * \brief      Sets the default properties for any new unit that is
	 *             added.
	 * \param      neighborDist    The default maximum distance (center point
	 *                             to center point) to other agents a new unit
	 *                             takes into account in the navigation. The
	 *                             larger this number, the longer he running
	 *                             time of the simulation. If the number is too
	 *                             low, the simulation will not be safe.
	 *                             Must be non-negative.
	 * \param      maxNeighbors    The default maximum number of other agents a
	 *                             new unit takes into account in the
	 *                             navigation. The larger this number, the
	 *                             longer the running time of the simulation.
	 *                             If the number is too low, the simulation
	 *                             will not be safe.
	 * \param      timeHorizon     The default minimal amount of time for which
	 *                             a new unit's velocities that are computed
	 *                             by the simulation are safe with respect to
	 *                             other agents. The larger this number, the
	 *                             sooner an unit will respond to the presence
	 *                             of other agents, but the less freedom the
	 *                             unit has in choosing its velocities.
	 *                             Must be positive.
	 * \param      timeHorizonObst The default minimal amount of time for which
	 *                             a new unit's velocities that are computed
	 *                             by the simulation are safe with respect to
	 *                             obstacles. The larger this number, the
	 *                             sooner an unit will respond to the presence
	 *                             of obstacles, but the less freedom the unit
	 *                             has in choosing its velocities.
	 *                             Must be positive.
	 * \param      radius          The default radius of a new unit.
	 *                             Must be non-negative.
	 * \param      maxSpeed        The default maximum speed of a new unit.
	 *                             Must be non-negative.
	 * \param      velocity        The default initial two-dimensional linear
	 *                             velocity of a new unit (optional).
	 */
	void setUnitDefaults(float neighborDist, size_t maxNeighbors,
						  float timeHorizon, float timeHorizonObst,
						  float radius, float maxSpeed,
						  const RvoVec2 &velocity = RvoVec2());



	/**
	 * \brief      Sets the time step of the simulation.
	 * \param      timeStep        The time step of the simulation.
	 *                             Must be positive.
	 */
	void setTimeStep(float timeStep);

	float getTimeStep()	{		return _timeStep;	}

private:

	void _collectObstacleNeighbors(RvoUnit *unit);
	void _collectUnitNeighbors(RvoUnit *unit);
	void _makeAvoid(RvoUnit *unit);
	std::vector<RvoUnit *> _units;
	RvoUnit *_defaultUnit;
	float _globalTime;
	float _timeStep;

	CUnitMap _unitmap;
	navMesh *_nmesh;

	CUnitMgr *_mirror;

	std::vector<RvoObstacle *>_bufWorking;

	CUnitMgr::CircumRanges _ranges;

	CAvoidHintTrail _trailAvoidHint;//一系列用来帮助Avoid的参考点

	friend class RvoUnit;
	friend class RvoTree;
	friend class RvoObstacle;
};
