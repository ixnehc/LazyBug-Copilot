#pragma once

#include "LevelSkill.h"

#include "LevelAttrs_Weak.h"

#include "LevelGesture.h"

#include "anim/KeySet.h"

struct GeneralSkillEoEntry
{
	StringID nmEvent;
	BOOL bEnable;
	RecordID idEO;
	StringID nmChecker;
	BOOL bGround;
	BEGIN_GOBJ_PURE(GeneralSkillEoEntry,1);

		GELEM_VAR_INIT(StringID,nmEvent,RecordID_Invalid);GELEM_UID(1);
			GELEM_EDITVAR("触发EO的事件",GVT_U,GSem(GSem_StringID,"动画事件"),"触发EO的事件");
		GELEM_VAR_INIT(BOOL,bEnable,TRUE);GELEM_UID(2);
			GELEM_EDITVAR("是否有效",GVT_S,GSem_Boolean,"是否有效");
		GELEM_VAR_INIT( StringID,nmChecker,StringID_Invalid);	GELEM_UID(6);
			GELEM_EDITVAR( "行为图检测入口", GVT_U, GSem(GSem_StringID,"行为图中继名称"), "一个行为图中继的名称,用来执行一段更新逻辑" );
		GELEM_VAR_INIT(RecordID,idEO,RecordID_Invalid);GELEM_UID(4);
			GELEM_EDITVAR("EO",GVT_U,GSem(GSem_RecordID,"eos"),"EO的ID");
		GELEM_VAR_INIT(BOOL,bGround,FALSE); GELEM_UID(5)
			GELEM_EDITVAR("投射到地面上",GVT_S,GSem_Boolean,"投射到地面上");
	END_GOBJ();
};

struct GeneralSkillOpEntry
{
	enum Op
	{
		Op_None,
		Op_SetFacingModeToNone,
		Op_SetFacingModeToFaceTarget,
		Op_SetFacingModeToFaceTargetFixedPos,
		Op_AllowCancel,
		Op_OverrideWeaks,
		Op_CleanOverrideWeaks,
		Op_OpenBlocking,
		Op_CloseBlocking,
		Op_Landing,//着陆
		Op_TakeOff,//起飞
		Op_SetPathFacingModeToNone,
		Op_SetPathFacingModeToFaceTarget,

		Op_ForceDword=0xffffffff,
	};

	StringID nmEvent;
	BOOL bEnable;
	Op op;
	WeaksEx weaks;

	BEGIN_GOBJ_PURE(GeneralSkillOpEntry,1);

		GELEM_VAR_INIT(StringID,nmEvent,StringID_Invalid);GELEM_UID(1);
			GELEM_EDITVAR("触发事件",GVT_U,GSem(GSem_StringID,"动画事件"),"触发Operation的事件");
		GELEM_VAR_INIT(BOOL,bEnable,TRUE);GELEM_UID(2);
			GELEM_EDITVAR("是否有效",GVT_S,GSem_Boolean,"是否有效");
		GELEM_VAR_INIT(Op,op,Op_None);GELEM_UID(4);
			GELEM_EDITVAR("Op",GVT_U,GSem(GSem_Interger,
				"n/a:0" "|弱点,"
				"保持朝向不变:1"  "|弱点,"
				"朝向目标位置:2"	 "|弱点,"
				"朝向目标固定位置:3"  "|弱点,"
				"保持路径朝向不变:11"  "|弱点,"
				"路径朝向目标位置:12"  "|弱点,"
				"允许取消:4"  "|弱点,"
				"重载弱点:5"   "路径方向最大修正速度,"
				"取消弱点修改:6"   "|弱点,"
				"开始格挡:7"   "|弱点,"
				"结束格挡:8"   "|弱点,"
				"着陆:9"   "|弱点,"
				"起飞:10"   "|弱点"
				),"操作");
		GELEM_OBJ(WeaksEx,weaks);GELEM_UID(5);
			GELEM_EDITOBJ("弱点","弱点");
	END_GOBJ();


};

struct SkillParam_General_ZMatch
{
	BEGIN_GOBJ_PURE_UID(SkillParam_General_ZMatch,1);
		GELEM_VAR_INIT(BOOL,bEnable,FALSE); GELEM_UID(1);
			GELEM_EDITVAR("可用",GVT_S,GSem(GSem_Boolean,"开始点,结束点,最大调整速度"),"可用");
		GELEM_VAR_INIT(StringID,nmStart,StringID_Invalid);GELEM_UID(2);
			GELEM_EDITVAR("开始点",GVT_U,GSem(GSem_StringID,"动画事件"),"开始点");
		GELEM_VAR_INIT(StringID,nmEnd,StringID_Invalid);GELEM_UID(3);
			GELEM_EDITVAR("结束点",GVT_U,GSem(GSem_StringID,"动画事件"),"结束点");
		GELEM_VAR_INIT(float,speedAdjustLimit,10.0f);GELEM_UID(4);
			GELEM_EDITVAR("最大调整速度",GVT_F,GSem(GSem_Float,"0.1,10.0,0.05"),"最大调整速度(米/秒");
	END_GOBJ();//Cur GLEM_UID:4
	
	BOOL bEnable;
	StringID nmStart;
	StringID nmEnd;
	float speedAdjustLimit;
};


struct SkillParam_General:public LevelSkillParam
{
	DEFINE_SKILLPARAM_CLASS(SkillParam_General);

	enum ObstacleMethod
	{
		ObstacleMethod_NotCheck=0,
		ObstacleMethod_StopAtStaticObstacleOrEnemyObstacle,
		ObstacleMethod_StopAtStaticObstacle,
		ObstacleMethod_StopAtStaticObstacleAndBumpEnemyObstacle,

		ObstacleMethod_ForceDword=0xffffffff,
	};

	enum PathAdaptMode
	{
		PathAdaptMode_None=0,
		PathAdaptMode_ScaleAlongZAxis=1,
		PathAdaptMode_LockInitialFacing=2,

		PathAdaptMode_ForceDword=0xffffffff,
	};

	enum PathMatchMode
	{
		PathMatchMode_None=0,//不Match
		PathMatchMode_TargetDirection,//技能朝向目标方向施放
		PathMatchMode_TargetPos,//技能的结束点匹配目标位置

	};

	BEGIN_GOBJ_PURE(SkillParam_General,1);

		GELEM_VAR_INIT(RecordID,idPathRes,RecordID_Invalid);
			GELEM_EDITVAR("移动路径资源",GVT_U,GSem(GSem_RecordID,"resources"),"移动路径资源");
		GELEM_VAR_INIT(BOOL,bStartAtFixedPos,FALSE);GELEM_UID(11)
			GELEM_EDITVAR("从固定位置开始释放",GVT_S,GSem_Boolean,"用于从固定位置对目标对象施放的情况");
		GELEM_VAR_INIT(LevelSkillTargetFacingMode,modeInitialFacing,LevelSkillTargetFacingMode_None); GELEM_UID(2)
			GELEM_EDITVAR("初始朝向模式",GVT_U,GSem(GSem_Interger,LevelSkillTargetFacingMode_SemConstraint),"初始朝向模式");
		GELEM_VAR_INIT(float,angleMaxInitialFacingAdjust,180.0f);GELEM_UID(5);GELEM_VERSION(2)
			GELEM_EDITVAR("最大初始朝向调整角度",GVT_F,GSem(GSem_Float,"0.0,180.0,0.05"),"最大初始朝向调整角度");
		GELEM_VAR_INIT(LevelSkillTargetFacingMode,modeInitialPathFacing,LevelSkillTargetFacingMode_None); GELEM_UID(9)
			GELEM_EDITVAR("初始路径朝向模式",GVT_U,GSem(GSem_Interger,LevelSkillTargetFacingMode_SemConstraint),"初始路径朝向模式");
		GELEM_VAR_INIT(float,angleMaxInitialPathFacingAdjust,180.0f);GELEM_UID(6)
			GELEM_EDITVAR("最大初始路径朝向调整角度",GVT_F,GSem(GSem_Float,"0.0,180.0,0.05"),"最大初始路径朝向调整角度");
		GELEM_VAR_INIT(float,speedMaxFacingAdjust,1800.0f);GELEM_UID(7);
			GELEM_EDITVAR("最大朝向调整速度",GVT_F,GSem(GSem_Float,"0.0,1800.0,0.05"),"最大朝向调整速度");
		GELEM_VAR_INIT(float,speedMaxPathFacingAdjust,1800.0f);GELEM_UID(8);
			GELEM_EDITVAR("最大路径朝向调整速度",GVT_F,GSem(GSem_Float,"0.0,1800.0,0.05"),"最大路径朝向调整速度");
		GELEM_VAR_INIT(LevelSkillTargetFacingMode,modeFinalFacing,LevelSkillTargetFacingMode_None); GELEM_UID(4)
			GELEM_EDITVAR("最终朝向模式",GVT_U,GSem(GSem_Interger,LevelSkillTargetFacingMode_SemConstraint),"最终朝向模式");
		GELEM_OBJ(SkillParam_General_ZMatch,zmatch);GELEM_UID(10)
			GELEM_EDITOBJ("Z轴匹配","在Z轴(前向)上的位置匹配");
		GELEM_VAR_INIT(AnimTick,dur,ANIMTICK_FROM_SECOND(0.0f));
			GELEM_EDITVAR("持续时间",GVT_U,GSem(GSem_AnimTick,"0,1000,0.1"),"如果为0,由路径持续时间决定");
		GELEM_OBJVECTOR(GeneralSkillEoEntry,entriesEo);
			GELEM_EDITOBJ("EO参数","EO参数");
		GELEM_OBJVECTOR(GeneralSkillOpEntry,entriesOp);
			GELEM_EDITOBJ("Op参数","Op参数");
		GELEM_VAR_INIT(ObstacleMethod,methodObstacle,ObstacleMethod_NotCheck);
			GELEM_EDITVAR("障碍处理方式",GVT_U,GSem(GSem_Interger,"不检测障碍:0,遇到Static障碍或Enemy障碍停止移动:1,遇到Static障碍停止移动:2,遇到Static障碍停止移动并撞开动态障碍:3"),"障碍处理方式");
		GELEM_VAR_INIT(float,ratioColliding,1.0f);
			GELEM_EDITVAR("碰撞半径缩放系数",GVT_F,GSem(GSem_Float,"0.1,1.0,0.01"),"碰撞半径缩放系数");
		GELEM_VAR_INIT(PathAdaptMode,adaptPath,PathAdaptMode_None); GELEM_UID(1)
			GELEM_EDITVAR("路径适配模式",GVT_U,GSem(GSem_Flags,"沿Z轴(角色正面方向)适配:1,锁定初始Facing:2"),"路径适配模式");
		GELEM_VAR_INIT(PathMatchMode,modePathMatch,PathMatchMode_TargetDirection); GELEM_UID(3)
			GELEM_EDITVAR("路径匹配模式",GVT_U,GSem(GSem_Interger,
			"不匹配:0"	"|最大初始路径朝向调整角度,"
			"匹配方向:1" ","
			"匹配结束点:2" "|最大初始路径朝向调整角度"
			),"路径匹配模式");

		//Cur GLEM_UID:11
	END_GOBJ();

	RecordID idPathRes;
	AnimTick dur;
	BOOL bStartAtFixedPos;
	LevelSkillTargetFacingMode modeInitialFacing;
	float angleMaxInitialFacingAdjust;
	LevelSkillTargetFacingMode modeInitialPathFacing;
	float angleMaxInitialPathFacingAdjust;
	float speedMaxFacingAdjust;
	float speedMaxPathFacingAdjust;
	LevelSkillTargetFacingMode modeFinalFacing;
	SkillParam_General_ZMatch zmatch;
	ObstacleMethod methodObstacle;
	PathAdaptMode adaptPath;
	PathMatchMode modePathMatch;
	float ratioColliding;
// 	LevelSkillTarget::TypeMask maskTargetType;

	std::vector<GeneralSkillEoEntry> entriesEo;
	std::vector<GeneralSkillOpEntry> entriesOp;

};


class CUnitMgrNavMesh;
class CSkillGesture_Path:public CLevelGesture_BuildIn
{
public:
	IMPLEMENT_REFCOUNT_C;
	DEFINE_CLASS(CSkillGesture_Path);

	CSkillGesture_Path()
	{
		Zero();
	}

	void Zero()
	{
		_dur=0;
		_tCur=0.0f;
		_bFinished=FALSE;
		_bAlive=FALSE;
	}

	void Create(KeySet *ksPos,KeySet *ksFace,AnimTick dur);

	virtual void Destroy()	{		Zero();	Release();}
	virtual void Update(CUnit3D *unit,float dt){		return;}//不支持	}
	virtual void Update(CUnit *unit,float dt);
	virtual BOOL IsFinished()	{		return _bFinished;	}


	BOOL IsAlive()	{		return _bAlive;	}
	void Stop()	{		_bFinished=TRUE;	}

	void GetCurStep(i_math::line2df &step);
	float GetCurT()	{		return _tCur;	}

protected:

	KeySet *_ksPos;
	KeySet *_ksFace;
	AnimTick _dur;

	float _tCur;

	DWORD _bAlive;
	DWORD _bFinished;

	LevelPos _posLast;
	LevelPos _posCur;

	friend class Skill_General;

};

struct LevelPathesEvent;
class Skill_General:public CLevelSkill
{
public:
	DEFINE_SKILL_CLASS(Skill_General,28);

	Skill_General()
	{
		_ges=NULL;
		_tCasting=0;
		_iNextEvent=0;
		_events=NULL;
	}


	LevelSkillTarget::TypeMask GetTargetTypes()
	{
		return (1<<LevelSkillTarget::Target_Aim)|(1<<LevelSkillTarget::Target_None)|(1<<LevelSkillTarget::Target_DefObj);
	}
	virtual CastMoving GetCastMoving()	{		return CastMoving_Control;	}
	virtual AnimTick GetCastingTime()	{		return _tCasting;	}//返回经过IAS修正的casting time

	virtual BOOL CheckCastingEvent(StringID nmEvent)	{		return _eventsFrame.find(nmEvent)!=_eventsFrame.end();	}
	virtual AnimTick GetCastingEventTime(StringID nmEvent);

	virtual BOOL PreInitStartCheck(CLevelObj *owner,LevelRecordSkill *rec,LevelSkillTarget &target);//在初始化前检查一下是否可以开始

	static void CalcStartFace(CLevelObj *lo,LevelSkillTarget &target,SkillParam_General *param,LevelFace &face);
	static void CalcStartXfm(CLevelObj *lo,LevelSkillTarget &target,SkillParam_General *param,LevelXfm &xfm);

protected:
	virtual void _OnStart();
	virtual void _OnBreak();
	virtual void _OnUpdate(AnimTick dt);

	virtual void _OnFinish()	{		_Finish();	}

	void _CalcEventXfm(LevelPathesEvent &e,LevelXfm &xfmEvent);
	void _CalcEventXfm(LevelPathesEvent &e,i_math::xformf &xfmEvent);


	void _Finish();

	AnimTick _tCasting;

	LevelXfm _xfmBase;
	CSkillGesture_Path *_ges;

	KeySet _ksPos;
	KeySet _ksFace;
	AnimTick _dur;

	std::unordered_set<StringID> _eventsFrame;
	std::vector<LevelPathesEvent> *_events;
	int _iNextEvent;//下一个事件
};

