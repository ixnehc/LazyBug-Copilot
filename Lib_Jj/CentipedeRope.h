#pragma once

#include "class/class.h"

#include "LevelDefines.h"

#include "rope/rope.h"

struct CentipedeRopeProp:public RopeProp
{
	BEGIN_GOBJ_PURE(CentipedeRopeProp,1);
		GELEM_VAR_INIT(float,m,1.0f);
			GELEM_EDITVAR("质点质量",GVT_F,GSem(GSem_Float,"0.001,10000.0,0.001"),"质点质量");
		GELEM_VAR_INIT(float,radius,0.1f);
			GELEM_EDITVAR("质点半径",GVT_F,GSem(GSem_Float,"0.00,10000.0,0.001"),"速度衰减");
		GELEM_VAR_INIT(float,stiffnessSpring,1.0f);
			GELEM_EDITVAR("弹簧stiffness",GVT_F,GSem(GSem_Float,"0.001,10000.0,0.001"),"弹簧stiffness");
		GELEM_VAR_INIT(float,lengthSpring,0.2f);
			GELEM_EDITVAR("弹簧长度",GVT_F,GSem(GSem_Float,"0.001,10000.0,0.001"),"弹簧长度");
		GELEM_VAR_INIT(float,frictionSpring,1.0f);
			GELEM_EDITVAR("弹簧摩擦系数",GVT_F,GSem(GSem_Float,"0.001,10000.0,0.001"),"弹簧摩擦系数");
		GELEM_VAR_INIT(float,stiffnessTwist,1.0f);
			GELEM_EDITVAR("扭转stiffness",GVT_F,GSem(GSem_Float,"0.001,10000.0,0.001"),"扭转stiffness");
		GELEM_VAR_INIT(float,frictionAir,2.0f);
			GELEM_EDITVAR("空气摩擦系数",GVT_F,GSem(GSem_Float,"0.000,10000.0,0.001"),"空气摩擦系数");
		GELEM_VAR_INIT(float,repulsionGround,1.0f);
//			GELEM_EDITVAR("地面弹性",GVT_F,GSem(GSem_Float,"0.00,10000.0,0.001"),"地面弹性");
		GELEM_VAR_INIT(float,frictionGround,2.0f);
//			GELEM_EDITVAR("地面摩擦系数",GVT_F,GSem(GSem_Float,"0.00,10000.0,0.001"),"地面摩擦系数");
		GELEM_VAR_INIT(float,absorptionGround,2.0f);
//			GELEM_EDITVAR("地面冲量吸收",GVT_F,GSem(GSem_Float,"0.00,10000.0,0.001"),"地面冲量吸收");
		GELEM_VAR_INIT(float,dampingVel,0.2f);
			GELEM_EDITVAR("速度衰减",GVT_F,GSem(GSem_Float,"0.00,10000.0,0.001"),"速度衰减");
		GELEM_VAR_INIT(float,bury,0.0f);
//			GELEM_EDITVAR("可以陷入地表多深",GVT_F,GSem(GSem_Float,"0.00,10000.0,0.001"),"可以陷入地表多深");
		GELEM_VAR_INIT(float,noise,0.0f);
			GELEM_EDITVAR("随机扰动",GVT_F,GSem(GSem_Float,"0.00,1.0,0.001"),"随机扰动");
		GELEM_VAR_INIT(float,repulsionObstacle,1.0f); 
			GELEM_EDITVAR("障碍弹性",GVT_F,GSem(GSem_Float,"0.00,10000.0,0.001"),"边界弹性");
		GELEM_VAR_INIT(int,nIterate,10);
			GELEM_EDITVAR("迭代次数",GVT_S,GSem_Interger,"迭代次数");
		GELEM_VAR_INIT(RecordID,idRopeUnitBuff,RecordID_Invalid);
			GELEM_EDITVAR("Rope单位的特有Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"Rope单位的特有Buff");
	END_GOBJ();

	float stiffnessTwist;
	float repulsionObstacle;
	RecordID idRopeUnitBuff;

	int nIterate;
};

class CUnitMgrNavMesh;
class CUnit;
class CCentipedeRope:public CRope
{
public:
	CCentipedeRope()
	{
		_unitmgr=NULL;
		_prop=NULL;
		_faceRoot=0.0f;
	}

	void Init(int numOfMasses,CentipedeRopeProp &prop,CUnitMgrNavMesh *unitmgr);
	void Clear();

	void SetRootFace(LevelFace face)	{		_faceRoot=face;	}
	LevelFace GetRootFace()	{		return _faceRoot;	}

	void SetTwists(LevelFaceYaw *yaws);

	void SetUnit(int idx,CUnit *unit);
	void Break(int idx);//ensure idx is broken with idx-1

	void Solve(float dt);
	void Simulate(float dt);

protected:
	struct Twist
	{
		Twist()
		{
			memset(this,0,sizeof(*this));
		}
		Mass* mass1;
		Mass* mass2;

		LevelFaceYaw yaw;
		CentipedeRopeProp *prop;

		void Init(Mass* mass1, Mass* mass2,	CentipedeRopeProp *prop);
		void Solve(float dt);
	};

	std::vector<Twist> _twists;
	std::vector<CUnit*> _units;
	LevelFace _faceRoot;

	CUnitMgrNavMesh *_unitmgr;
	CentipedeRopeProp *_prop;

	BOOL _CalcSegRepulsion(CUnit *unitMe,CUnit *unitNb,CUnit *unit,i_math::vector3df &force);
	BOOL _IsRopeUnit(CUnit *unit);


};

