/*
 * Obstacle.h
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
 * \file       Obstacle.h
 * \brief      Contains the Obstacle class.
 */

#include "Definitions.h"

#include "class/class.h"

/**
 * \brief      Defines static obstacles in the simulation.
 */
//注意:线段的右侧为可走区域,左侧为不可走区域
class RvoObstacle 
{
public:
	DEFINE_CLASS(RvoObstacle);
	RvoObstacle();

	RvoObstacle *getNext()	{		return _nextObstacle;	}
	RvoVec2 &getPoint()	{		return _point;	}
	size_t getId()	{		return _id;	}
	float getLen() const
	{
		if (!_nextObstacle)
			return 0.0f;
		return abs(_point-_nextObstacle->_point);
	}
private:
	/**
	 * \brief      Constructs a static obstacle instance.
	 */

	bool _isConvex;
	RvoObstacle *_nextObstacle;
	RvoVec2 _point;
	RvoObstacle *_prevObstacle;
	RvoVec2 _unitDir;

	size_t _id;

	BOOL _bEnum;


	friend class RvoUnit;
	friend class RvoSimulator;
	friend class RvoObstacleMap;
};
