#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"

#include "LoAgent.h"
#include "LevelObjSrc.h"

#include "LevelDeal.h"

#include "behaviorgraph/BehaviorParam.h"

#include "spline/CubicSpline.h"

#include "CentipedeRope.h"

#define CLASSUID_Centipede 53

enum CentipedeNodeType
{
	CentipedeNode_None,
	CentipedeNode_Head,
	CentipedeNode_Body,
	CentipedeNode_Tail,

	CentipedeNode_ForceDword=0xffffffff,
};

enum CentipedeWorkingMode
{
	CentipedeWorkingMode_None,
	CentipedeWorkingMode_Resting,
	CentipedeWorkingMode_Combat,

	CentipedeWorkingMode_Reserved01,
	CentipedeWorkingMode_Reserved02,
	CentipedeWorkingMode_Reserved03,
	CentipedeWorkingMode_Reserved04,

	CentipedeWorkingMode_Max,

	CentipedeWorkingMode_ForceDword=0xffffffff,
};

struct CentipedeNodeParam
{
	BEGIN_GOBJ_PURE(CentipedeNodeParam,1);

	GELEM_VAR_INIT(CentipedeNodeType,tp,CentipedeNode_Body);
		GELEM_EDITVAR("类型",GVT_S,GSem(GSem_Interger,"头部:1,体节:2,尾部:3"),"类型");

	END_GOBJ();

	CentipedeNodeType tp;
};

struct LosCentipede;
struct LopCentipede;
struct CentipedePose
{

	BEGIN_GOBJ_PURE(CentipedePose,1);

		GELEM_VAR_INIT(AnimTick,t,ANIMTICK_FROM_SECOND(0.0f));
			GELEM_EDITVAR("时间",GVT_U,GSem(GSem_AnimTick,"0,1000,0.01"),"时间");
		GELEM_VARVECTOR(i_math::vector3df,path); 
			GELEM_EDITVAR("路径",GVT_Fx3,GSem(GSem_Unknown,"MatSet"),"路径");

	END_GOBJ();

	AnimTick t;
	std::vector<i_math::vector3df> path;

	void BuildCache(LosCentipede *los,LopCentipede *lop);
	struct Cache
	{
		std::vector<LevelFaceYaw> yaws;
	} cache;
};

struct CentipedeAct
{
	BEGIN_GOBJ_PURE(CentipedeAct,1);

		GELEM_VAR_INIT( StringID,nm,StringID_Invalid);	
			GELEM_EDITVAR( "名称", GVT_U, GSem(GSem_StringID,"蜈蚣Act名称"), "名称" );
		GELEM_OBJVECTOR(CentipedePose,poses);
			GELEM_EDITOBJ("Pose列表","Pose列表");

	END_GOBJ();

	BOOL ValidatePoses();

	AnimTick GetDur();

	StringID nm;
	std::vector<CentipedePose> poses;
};

struct CentipedeCystParam
{
	BEGIN_GOBJ_PURE(CentipedeCystParam,1);

		GELEM_VAR_INIT(float,sideoff,1.0f);
			GELEM_EDITVAR("横向偏移",GVT_F,GSem(GSem_Float,"0.1,10.0,0.05"),"横向偏移");

	END_GOBJ();

	float sideoff;

};

struct CentipedeTouchParam
{

	BEGIN_GOBJ_PURE(CentipedeTouchParam,1);
		GELEM_VAR_INIT(float,radius,1.4f);
			GELEM_EDITVAR("触碰敌人半径",GVT_F,GSem(GSem_Float,"0.01,10.0,0.05"),"触碰敌人半径");
		GELEM_DYNOBJPTR_DEAL(CLevelDeal,deal,Deal_Dmg, "触碰敌人Deal", "触碰敌人Deal" );
			GELEMS_LEVELDEAL_CANDIDATES();
	END_GOBJ();

	float radius;
	CLevelDeal *deal;
};


struct LopCentipede:public CLevelObjParam
{
public:
	DEFINE_LEVELOBJPARAM_CLASS(LopCentipede,CLASSUID_Centipede);

	BEGIN_GOBJ_PURE(LopCentipede,1);

		GELEM_ALLOWDISABLE();

		GELEM_VAR_INIT(BOOL,bLeftArm,TRUE);GELEM_UID(7);
			GELEM_EDITVAR("左臂还是右臂",GVT_S,GSem(GSem_Interger,"左臂:1,右臂:0"),"是否左臂");

		GELEM_VAR_INIT( CentipedeWorkingMode,modeWorking,CentipedeWorkingMode_Resting);GELEM_UID(4);
			GELEM_EDITVAR("工作模式",GVT_U,GSem(GSem_Interger,"休息模式:1,战斗模式:2"),"工作模式");

		GELEM_OBJVECTOR(CentipedeNodeParam,paramsNode); GELEM_UID(1);
			GELEM_EDITOBJ("节点列表","节点列表");

		GELEM_VARVECTOR(i_math::vector3df,path); GELEM_UID(5);
			GELEM_EDITVAR("路径",GVT_Fx3,GSem(GSem_Unknown,"MatSet"),"路径");

		GELEM_VAR_INIT(float,off,0.0f); GELEM_UID(3);
			GELEM_EDITVAR("起点偏移值",GVT_F,GSem(GSem_Float,"0.0,100.0,0.05"),"单位为米");

		GELEM_OBJVECTOR(CentipedeAct,acts); GELEM_UID(6);
			GELEM_EDITOBJ("Act列表","Act列表");

		//Next GELEM_UID: 8

	END_GOBJ();

	BOOL bLeftArm;

	CentipedeWorkingMode modeWorking;

	std::vector<CentipedeNodeParam> paramsNode;
	std::vector<i_math::vector3df> path;
	float off;
	std::vector<CentipedeAct> acts;

	void BuildCache(LosCentipede *los);
	struct Cache
	{
		Cache()
		{
			length=0.0f;
		}
		float length;
	}cache;

};

struct LosCentipede:public LosAgent
{
public:
	DEFINE_LEVELOBJSRC_CLASS(LosCentipede,CLASSUID_Centipede);

	BEGIN_GOBJ_PURE(LosCentipede,1);

		GELEM_ALLOWDISABLE();
		GELEM_AGENTRECORD();

		GELEM_BEHAVIORMEM_OBJID(varCentipedeAgent,"记录蜈蚣对象的变量","蜈蚣节点单位里记录蜈蚣对象的变量"); GELEM_UID(2);

		GELEM_VARARRAY_INIT(StringID,senariosWorkingMode,StringID_Invalid); 
			GELEM_VERSION(10);
			GELEM_EDITVAR("AI方案",GVT_U,GSem(GSem_StringID,
				"$Lable{//n/a,休息模式,战斗模式,//Reserve01,//Reserve02,//Reserve03,//Reserve04}AIScenario"),
				"不同工作模式下的AI方案");

		GELEM_VAR_INIT(float,radiusHead,0.5f); GELEM_UID(1);
			GELEM_EDITVAR("头部半径",GVT_F,GSem(GSem_Float,"0,10,0.001"),"头部半径");
		GELEM_VAR_INIT(float,radiusBody,0.5f); GELEM_UID(3);
			GELEM_EDITVAR("体节半径",GVT_F,GSem(GSem_Float,"0,10,0.001"),"体节半径");
		GELEM_VAR_INIT(float,radiusTail,0.5f); GELEM_UID(4);
			GELEM_EDITVAR("尾部半径",GVT_F,GSem(GSem_Float,"0,10,0.001"),"尾部半径");
		GELEM_VAR_INIT(RecordID,idUnit_Node,RecordID_Invalid); GELEM_UID(5);
			GELEM_EDITVAR("节点单位",GVT_U,GSem(GSem_RecordID,"units"),"节点单位");
		GELEM_VAR_INIT(RecordID,idUnit_Cyst,RecordID_Invalid); GELEM_UID(16);
			GELEM_EDITVAR("囊单位",GVT_U,GSem(GSem_RecordID,"units"),"囊单位");
		GELEM_VAR_INIT(RecordID,idItem_HeadTail,RecordID_Invalid); GELEM_UID(6);
			GELEM_EDITVAR("有头有尾Item",GVT_U,GSem(GSem_RecordID,"items"),"有头有尾Item");
		GELEM_VAR_INIT(RecordID,idItem_Head,RecordID_Invalid); GELEM_UID(7);
			GELEM_EDITVAR("有头无尾Item",GVT_U,GSem(GSem_RecordID,"items"),"有头无尾Item");
		GELEM_VAR_INIT(RecordID,idItem_Tail,RecordID_Invalid); GELEM_UID(8);
			GELEM_EDITVAR("有尾无头Item",GVT_U,GSem(GSem_RecordID,"items"),"有尾无头Item");
		GELEM_VAR_INIT(RecordID,idItem_NoTailNoTail,RecordID_Invalid); GELEM_UID(9);
			GELEM_EDITVAR("无头无尾Item",GVT_U,GSem(GSem_RecordID,"items"),"无头无尾Item");
		GELEM_VAR_INIT(RecordID,idSkill_NodeBreakL,RecordID_Invalid); GELEM_UID(18);
			GELEM_EDITVAR("从左侧被打断Skill",GVT_U,GSem(GSem_RecordID,"skills"),"从左侧被打断Skill");
		GELEM_VAR_INIT(RecordID,idSkill_NodeBreakR,RecordID_Invalid); GELEM_UID(19);
			GELEM_EDITVAR("从右侧被打断Skill",GVT_U,GSem(GSem_RecordID,"skills"),"从右侧被打断Skill");
		GELEM_VAR_INIT(RecordID,idSkill_NodeBreakB,RecordID_Invalid); GELEM_UID(20);
			GELEM_EDITVAR("从后侧被打断Skill",GVT_U,GSem(GSem_RecordID,"skills"),"从后侧被打断Skill");
		GELEM_VAR_INIT(RecordID,idBuff_Tendrils,RecordID_Invalid); GELEM_UID(15);
			GELEM_EDITVAR("触须Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"触须Buff");
		GELEM_VAR_INIT(AnimTick,durStandUpStep,ANIMTICK_FROM_SECOND(0.1f));GELEM_UID(11);
			GELEM_EDITVAR("起身间隔",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"相邻Node间的起身间隔");
		GELEM_VAR_INIT(AnimTick,durStandUp,ANIMTICK_FROM_SECOND(0.5f));GELEM_UID(12);
			GELEM_EDITVAR("起身时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"相邻Node间的起身间隔");
		GELEM_VAR_INIT(float,spdMarch,5.0f);GELEM_UID(13);
			GELEM_EDITVAR("March速度",GVT_F,GSem(GSem_Float,"0.1,100.0,0.05"),"March速度");
		GELEM_OBJ(CentipedeRopeProp,propRope);GELEM_UID(14);
			GELEM_EDITOBJ("Rope参数","Rope参数");
		GELEM_OBJ(CentipedeCystParam,paramCyst);GELEM_UID(17);
			GELEM_EDITOBJ("囊参数","囊参数");
		GELEM_OBJ(CentipedeTouchParam,paramTouch);GELEM_UID(21);
			GELEM_EDITOBJ("触碰伤害参数","触碰伤害参数");
		//GELEM_UID_NEXT 22

	END_GOBJ();

	BOOL IsValid();

	float GetNodeRadius(CentipedeNodeType tp);
	RecordID GetUnitID_Node(CentipedeNodeType tp);
	RecordID GetUnitID_Cyst();
	RecordID GetEquip(CentipedeNodeType tp);

	virtual BOOL NeedSyncGUID()	{		return TRUE;	}

	StringID varCentipedeAgent;
	StringID senariosWorkingMode[CentipedeWorkingMode_Max];

	float radiusHead;
	float radiusBody;
	float radiusTail;
	RecordID idUnit_Node;
	RecordID idUnit_Cyst;
	RecordID idItem_HeadTail;
	RecordID idItem_Head;
	RecordID idItem_Tail;
	RecordID idItem_NoTailNoTail;
	RecordID idBuff_Tendrils;
	RecordID idSkill_NodeBreakL;
	RecordID idSkill_NodeBreakR;
	RecordID idSkill_NodeBreakB;
	AnimTick durStandUpStep;
	AnimTick durStandUp;
	float spdMarch;
	CentipedeRopeProp propRope;
	CentipedeCystParam paramCyst;
	CentipedeTouchParam paramTouch;

};

class CLoCentipede;
struct CentipedeNode
{
	CentipedeNode()
	{
		memset(this,0,sizeof(*this));
	}

	CLoCentipede *owner;
	CentipedeNodeParam *param;

	LevelObjID idLo;

	struct Cyst
	{
		BOOL bLeft;
		LevelObjID id;
	};
	Cyst cyst;
};

struct CentipedeCystSync
{
	CentipedeCystSync()
	{
		memset(this,0,sizeof(*this));
	}
	LevelObjID idNode;
	LevelObjID idCyst;
	BOOL bLeft;
};


struct CentipedeNodesLoc
{
	CentipedeNodesLoc()
	{
		stretch.Zero();
		Zero();
	}
	void Zero()
	{
		bDirty=FALSE;
		distTotal=0.0f;
	}
	void Clear()
	{
		positions.clear();
		dists.clear();
		faces.clear();
		Zero();
	}

	void WriteSync(CBitPacket *bp,BOOL &bContent);
	void ReadSync(CBitPacket *bp);

	void SetDirty()	{		bDirty=TRUE;	}
	void ClearDirty()	{		bDirty=FALSE;	}

	std::vector<LevelPos> positions;
	std::vector<LevelFace> faces;
	BOOL bDirty;

	//Dists
	void CalcDists();
	float distTotal;
	std::vector<float> dists;

	//Stretch
	struct Stretch
	{
		Stretch()
		{
			Zero();
		}
		void Zero()
		{
			tStart=ANIMTICK_INFINITE;
			bOut=FALSE;
			rateStart=0.0f;
		}

		void StretchOut(AnimTick t,AnimTick dur_)
		{
			rateStart=GetRate(t);
			tStart=t;
			dur=(AnimTick)((1.0f-rateStart)*(float)dur_);
			bOut=TRUE;
		}

		void StretchIn(AnimTick t,AnimTick dur_)
		{
			rateStart=GetRate(t);
			tStart=t;
			dur=(AnimTick)(rateStart*(float)dur_);
			bOut=FALSE;
		}


		BOOL IsStretchingOut()		{			return bOut;		}

		BOOL IsFullyStretchedOut(AnimTick t)
		{
			if (bOut&&(GetRate(t)>=1.0f))
				return TRUE;
			return FALSE;
		}

		BOOL IsFullyStretchedIn(AnimTick t)
		{
			if ((!bOut)&&(GetRate(t)>=1.0f))
				return TRUE;
			return FALSE;
		}


		float GetRate(AnimTick tCur)
		{
			if (bOut)
			{
				if (tStart==ANIMTICK_INFINITE)
					return 1.0f;
				AnimTick t=ANIMTICK_SAFE_MINUS(tCur,tStart);
				if (t>dur)
					return 1.0f;

				return rateStart+(1.0f-rateStart)*((float)t)/(float)dur;
			}
			else
			{
				if (tStart==ANIMTICK_INFINITE)
					return 0.0f;
				AnimTick t=ANIMTICK_SAFE_MINUS(tCur,tStart);
				if (t>dur)
					return 0.0f;

				return rateStart*(1.0f-((float)t)/(float)dur);
			}
		}

		AnimTick tStart;
		AnimTick dur;
		float rateStart;
		BOOL bOut;
	};
	Stretch stretch;


};

class CCentipedeState_Resting
{
public:
	CCentipedeState_Resting()
	{
		Zero();
	}
	void Zero()
	{
		_owner=NULL;
		_bDirty=FALSE;
		_status=None;
		_tStatusStart=0;
		_idFrom=LevelObjID_Invalid;
	}

	void Clear();

	enum Status
	{
		None,
		Resting,
		Standing,
		Marching,
	};

	void Reset(CLoCentipede *owner,AnimTick t);
	void StandUp(LevelObjID idFrom,AnimTick t);
	void StartMarch(AnimTick t);

	void Update(AnimTick t);

	//不需要同步的数据
	CLoCentipede *_owner;
	AnimTick _durStandUp;//所有node起身完毕所需时间
	AnimTick _tStatusStart;
	CCubicSpline _splinePath;


	//需要同步的数据
	Status _status;
	LevelObjID _idFrom;

	void WriteSync(CBitPacket *bp,BOOL &bContent);

	void SetDirty()	{		_bDirty=TRUE;	}
	void ClearDirty()	{		_bDirty=FALSE;	}
	BOOL _bDirty;
};

class CCentipedeState_Combat
{
public:
	CCentipedeState_Combat()
	{
		Zero();
	}
	void Zero()
	{
		_owner=NULL;
		_bDirty=FALSE;
		_t=0;
	}

	void Clear();

	void Reset(CLoCentipede *owner,AnimTick t);

	void PlayAct(StringID nm,BOOL bLoop,AnimTick t);
	BOOL GetTopActProgress(AnimTick &tCur,AnimTick &dur);

	void SpawnCyst(int idxNode,BOOL bLeft);
	void NotifyCystKilled(int idxNode,LevelObjID idCyst);

	void BreakFromCyst(int idxNode,LevelOpLink &link);//Ensure idxNode is broken with idxNode+1
	void Break(int idxNode,LevelOpLink &link);//Ensure idxNode is broken with idxNode+1

	void Update(AnimTick t);

	DWORD GetUnbroken()	{		return _nUnbroken;	}

public://Take it as protected
	void _RefreshRopeUnit();

	void _Break(int idxNode,RecordID idSkill,LevelOpLink &link);//Ensure idxNode is broken with idxNode+1

	struct ActState
	{
		ActState()
		{
			idxAct=-1;
		}
		float CalcBlendRate(AnimTick t)
		{
			if (durBlend<=0)
				return 1.0f;
			AnimTick tLocal=ANIMTICK_SAFE_MINUS(t,tStart);
			return i_math::clamp_f(((float)tLocal)/(float)durBlend,0.0f,1.0f);
		}
		int idxAct;
		BOOL bLoop;
		AnimTick tStart;
		AnimTick durBlend;
	};

	//不需要同步的数据
	CLoCentipede *_owner;
	CCentipedeRope _rope;
	AnimTick _t;
	DWORD _nUnbroken;

	void _UpdateAct();
	void _CalcActYaws(ActState &state,AnimTick t,LevelFaceYaw *yaws);
	std::deque<ActState> _acts;

	//需要同步的数据

	void WriteSync(CBitPacket *bp,BOOL &bContent);

	void SetDirty()	{		_bDirty=TRUE;	}
	void ClearDirty()	{		_bDirty=FALSE;	}
	BOOL _bDirty;
};

class CLoCentipede:public CLoAgent
{
public:
	CLoCentipede()
	{
		_bhv=NULL;
	}
	DEFINE_LEVELOBJ_CLASS(CLoCentipede,CLASSUID_Centipede);

	virtual const char *GetShowName()	{		return "蜈蚣";	}

	virtual void PostCreate();
	virtual void OnDestroy();

	virtual BOOL OnActivate();
	virtual void OnDeactivate();

	virtual void Update();

	virtual LevelObjShapeType GetShapeType()	{		return LevelObjShape_SingleCircle;	}

	virtual BOOL IsServerOnly()	{		return FALSE;	}

	void Activate(CLevelObj *loFrom);

	RecordID GetNodeUnitID();

	BOOL IsLeftArm();
	CentipedeWorkingMode GetWorkingMode();

	CCentipedeState_Combat &GetCombatState()	{		return _stateCombat;	}

	DWORD GetNodeCount()	{		return _nodes.size();	}

	BOOL GetNodeLoc(LevelObjID id,LevelPos &pos,LevelFace &face);
	BOOL GetNodeIndex(LevelObjID id,DWORD &idx);
	LevelObjID GetNodeFromIndex(DWORD idx);
	AnimTick CalcStandUpDur(LevelObjID idFrom);
	BOOL GetCystLoc(LevelObjID id,LevelPos &pos,LevelFace &face);
	BOOL GetNodeFromCyst(LevelObjID idCyst,LevelObjID &id);
	BOOL GetCystLeftOrRight(LevelObjID idCyst,BOOL &bLeft);

	BOOL Break(LevelObjID id,LevelOpLink &link);
	BOOL BreakFromCyst(LevelObjID id,LevelOpLink &link);

	void StretchOut(AnimTick dur);
	void StretchIn(AnimTick dur);

	float GetUnbrokenRate();

protected:

	virtual void _OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer);
	virtual void _OnWriteSyncL(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer);
	virtual void _OnPostWriteSync();

	void _CalcLength();
	float _length;

	void _BuildNodes();
	void _BuildNode(CentipedeNode &node,LevelPos3D &pos,LevelFace face,CentipedeNodeParam &param);
	std::deque<CentipedeNode> _nodes;

	std::unordered_map<LevelObjID,int> _lookupNodeIndex;//Obj ID 查找node的索引

	CentipedeNodesLoc _locsNode;
	CCentipedeState_Resting _stateResting;	
	CCentipedeState_Combat _stateCombat;	

	void _AddCentipedeCystSync(int idxNode);
	std::vector<CentipedeCystSync> _syncsCentipedeCyst;

	void _SetAgentVar(CLevelObj *lo);

	void _UpdateTouch();

	//Behavior
	LevelSimpleMem _memSimple;
	CLevelBehavior *_bhv;

	struct Touched
	{
		Touched()
		{
			lo=NULL;
			bTouchedByUnBroken=FALSE;
			nTouchedByBroken=0;
		}
		CLevelObj *lo;
		LevelPos pos;
		BOOL bTouchedByUnBroken;
		DWORD nTouchedByBroken;
	};
	std::vector<Touched> _bufTouched;


	friend class CCentipedeState_Resting;
	friend class CCentipedeState_Combat;
};
