#pragma once

#include "sparsearray/sparsearray2D.h"


#include "PositionBasedDynamics/Simulation/Simulation.h"
#include "PositionBasedDynamics/Simulation/SimulationModel.h"

#include "PositionBasedDynamics/Simulation/TimeManager.h"

#include <unordered_set>

typedef int TongueParticleHandle;
#define TongueParticleHandle_Invalid (-1)
typedef int TongueConstraintHandle;
#define TongueConstraintHandle_Invalid (-1)
typedef DWORD TongueParticleCollisionGroup;
#define TongueParticleCollisionGroup_None (0)
#define TongueParticleCollisionGroup_Obstacle (0xffffffff)

class CTonguePhysEnv
{
public:
	CTonguePhysEnv()
	{
		Zero();
	}
	void Zero()
	{
		_model=NULL;
		_seedCollisionGrp=0;
		_bAllowInterParticleCollision=FALSE;
	}
	void Init(PBD::SimulationModel * model);
	void Clear();

	TongueParticleCollisionGroup NewCollisionGroup()	{		return ++_seedCollisionGrp;	}

	PBD::SimulationModel *GetSimModel()	{		return _model;	}

	TongueParticleHandle AddParticle(i_math::vector3df &pos,float radius,DWORD collision);
	TongueParticleHandle AddObstacle(i_math::vector3df &pos,float radius);

	void RemoveParticle(TongueParticleHandle particle);
	void RemoveObstacle(TongueParticleHandle particle);

	void SetParticleCollision(TongueParticleHandle particle,TongueParticleCollisionGroup collision);

	void Update();

public:
	struct Entry
	{
		TongueParticleHandle particle;
		float radius;
		TongueParticleCollisionGroup collision;
		i_math::pos2di ptCell;
	};

	struct Cell
	{
		void Add(Entry *e)
		{
			entries.insert(e);
		}
		void Remove(Entry *e)
		{
			std::unordered_set<void*>::iterator it=entries.find(e);
			if (it!=entries.end())
				entries.erase(it);
		}
		std::unordered_set<void*> entries;
	};

	i_math::pos2di _ToCellCoord(float x,float y)
	{
		const float lenCell=8.0f;

		i_math::pos2di pt;
		pt.x=(int)floor(x/lenCell);
		pt.y=(int)floor(y/lenCell);

		return pt;
	}

	void _GenContacts();

	//是否允许非Obstacle的Particle之间的碰撞
	BOOL _bAllowInterParticleCollision;

	std::unordered_map<TongueParticleHandle,Entry> _entries;
	SparseArray2D<Cell,4,0,FALSE> _cells;

	std::deque<Entry*> _obstacles;

	PBD::SimulationModel *_model;
	DWORD _seedCollisionGrp;

};

class CTongueBranchPhys
{
public:
	CTongueBranchPhys()
	{
		Zero();
	}
	void Zero()
	{
		_env=NULL;
		_radiusNode=1.0f;
		_bTargetPos=FALSE;
		_collision=TongueParticleCollisionGroup_None;
		_speedScale=1.0f;
	}
	void Init(CTonguePhysEnv &env,DWORD nNodes,float radiusNode,i_math::vector3df &posRoot,i_math::vector3df &dirRoot);
	void Clear();

	void Update(PBD::SimulationModel &model);

	void SetTargetPos(i_math::vector3df &pos);
	void ClearTargetPos();
	BOOL GetTargetPos(i_math::vector3df &pos);

	float GetNodeRadius()	{		return _radiusNode;	}

	float GetNodeDist()	{		return _radiusNode*2.0f;	}

	int GetLastPulledIn()	{		return _GetLastPulledIn();	}

	void SetSpeedScale(float scale)	{		_speedScale=scale;	}

	float GetFullLength();

public:
	struct Node
	{
		Node()
		{
			stage=Stage_Static;
			particle=TongueParticleHandle_Invalid;
			constraintPulled=TongueConstraintHandle_Invalid;
			constraintDist=TongueConstraintHandle_Invalid;
			constraintBend=TongueConstraintHandle_Invalid;
		}
		enum Stage
		{
			Stage_Static,
			Stage_PulledIn,//这个Node正被前一个Node拉拽
			Stage_PulledOut,//这个Node正被前后两个Node拉拽,并主要是被后一个Node拉拽
		};

		Stage stage;

		TongueParticleHandle particle;
		TongueConstraintHandle constraintPulled;
		TongueConstraintHandle constraintDist;
		TongueConstraintHandle constraintBend;

		i_math::vector3df velTemp;
		i_math::vector3df velRecent;
	};
	BOOL _CheckTargetPosObstacle(PBD::SimulationModel &model,int &nNodeObstacled,int &idxObstacle);
	void _UpdateVelocityToTargetPos(PBD::SimulationModel &model);
	void _UpdateVelocityToTargetPos2(PBD::SimulationModel &model);
	void _UpdateVelocityToTargetPos3(PBD::SimulationModel &model);
	void _UpdateBendTolerance(PBD::SimulationModel &model);
	int _GetLastPulledIn();
	void _CalcEnd(PBD::SimulationModel &model,i_math::vector3df &velEnd,float &dEnd);
	void _AverageVelocity(PBD::SimulationModel &model,int iStart);
	void _DumpRecentVel(PBD::SimulationModel &model);

	CTonguePhysEnv *_env;

	TongueParticleCollisionGroup _collision;

	std::deque<Node> _nodes;
	float _radiusNode;

	i_math::vector3df _posRoot;
	i_math::vector3df _dirRoot;

	float _speedScale;

	BOOL _bTargetPos;
	i_math::vector3df _posTarget;

};

