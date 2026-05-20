#pragma once

#include "class/class.h"
#include "gds/GObj.h"
#include "gds/GObjUID.h"
#include "strlib/strlibdefines.h"
#include "strlib/strlib.h"
#include "linkpad/LinkPadDefines.h"

#include "LevelDefines.h"
#include "LevelSlateDefines.h"

#include "behaviorgraph/Behavior.h"
#include "behaviorgraph/BehaviorParam.h"
#include "behaviorgraph/BehaviorValue.h"
#include "behaviorgraph/BehaviorDebug.h"

class CLevelBehavior;
class CLevel;
class CLevelObj;
class CLevelNPC;
class CLevelAI;
struct MagicBoardAIContext;
struct LevelBehaviorContext
{
	LevelBehaviorContext()
	{
		bg=NULL;
		behavior=NULL;

		idPlayerLock=LevelPlayerID_Invalid;
		idPlayerTalk=LevelPlayerID_Invalid;
		level=NULL;
		npc=NULL;
		lo=NULL;
		aiLvl=NULL;
		ctxMB=NULL;
		memSimple=NULL;
		idxSlate=LevelSlateIdx_Invalid;
	}

	CBehaviorGraph*bg;
	CLevelBehavior *behavior;
	CLevel *level;

	LevelPlayerID idPlayerLock;//当前的Behavior是针对哪个Player的,注意不是自己的Player
	LevelPlayerID idPlayerTalk;//
	CLevelNPC *npc;
	CLevelObj *lo;
	CLevelAI *aiLvl;
	LevelSimpleMem *memSimple;
	MagicBoardAIContext *ctxMB;
	LevelSlateIdx idxSlate;
};

struct BgnLevelSkillTarget
{
	enum Type
	{
		Threat,
		Custom,
		CustomPos,
		Me,
		SkillTarget,

		ForceDword=0xffffffff,
	};
	Type tp;
	StringID nmCustom;
	StringID nmCustomPos;

    BEGIN_GOBJ_PURE_UID(BgnLevelSkillTarget,1);

		GELEM_VAR_INIT(Type,tp,Threat);
		GELEM_EDITVAR("类型",GVT_S,GSem(GSem_Interger,
			"Threat:0"	"|指定对象&指定位置," 
			"技能目标:4"	"|指定对象&指定位置," 
			"自己:3"	"|指定对象&指定位置," 
			"指定对象:1"	"|指定位置," 
			"指定位置:2"	"|指定对象" 
			),"目标类型");
		GELEM_BEHAVIORMEM_OBJID(nmCustom,"指定对象","指定对象")
		GELEM_BEHAVIORMEM_POS(nmCustomPos,"指定位置","指定位置")

    END_GOBJ();    

	static const char *GetDesc(BgnLevelSkillTarget &target,StringID nmRef)
	{
		if (nmRef==0xffffffff)//StringID_BhvValInvalidRef
		{
			return target.GetDesc();
		}
		else
		{
			static std::string nm;
			extern const char *StrLib_GetStr(StringID id);
			nm=std::string("[")+StrLib_GetStr(nmRef)+"]";
			return nm.c_str();
		}
	}

	const char *GetDesc()
	{
		static std::string s;
		if (tp==Threat)
			return "Threat";
		if (tp==Me)
			return "自己";
		if (tp==SkillTarget)
			return "技能目标";
		if (nmCustom!=StringID_Invalid)
		{
			FormatString(s,"指定对象:[%s]",StrLib_GetStr(nmCustom));
			return s.c_str();
		}
		if (nmCustomPos!=StringID_Invalid)
		{
			FormatString(s,"指定位置:[%s]",StrLib_GetStr(nmCustomPos));
			return s.c_str();
		}

		return "[n/a]";
	}
};


struct BGNOutputs;
struct LevelBehaviorContext;
class CBehaviorGraphPad;
class CLevelTalks;
struct LevelRoute;
class CLevelObj;
class CLevelPlayer;
class CLevelSkillDriver;
class CLevelBuffs;
class CLevelObjSrc;
class CLevelObjParam;
struct BehaviorValueDeclare;
class CLevelTroop;
class CLevelTroops;
struct LevelAIContext;
struct TroopCombatContext;
class CLevelBgn:public CBehaviorGraphNode
{
public:
	CLevelBgn()
	{
		_result=A_Pending;
		_bInPending=FALSE;
		_lpad=NULL;
	}
	BOOL IsValid()	{		return _GetCtx()!=NULL;	}

	virtual CClass*GetClass()=0;

	virtual void Create(){}
	virtual void Destroy(){}
	virtual void Break(BGNOutputs &outputs){}

	virtual void Start(DWORD iStb,BGNOutputs &outputs){}
	virtual void StartPending(DWORD iStb){}
	virtual void Trigger(DWORD iStb,BGNOutputs &outputs){}
	virtual void Update(BGNOutputs &outputs){}
	virtual void RewindOk(WORD keyRewind,BGNOutputs &outputs){}//因为执行成功导致的Rewind
	virtual void RewindFail(WORD keyRewind,BGNOutputs &outputs){}//因为执行失败导致的Rewind

	virtual BOOL GetCIn()	{		return FALSE;	}

	CLevelObj *GetLo()	{		return _GetLo();	}

protected:
	LevelBehaviorContext *_GetCtx();
	MagicBoardAIContext  *_GetCtxMB()
	{
		LevelBehaviorContext*ctx=_GetCtx();
		if (ctx)
			return ctx->ctxMB;
		return NULL;
	}

	virtual void _ResolvePad(CBehaviorGraphPad *pad);
	template<typename T>
	T*_GetPad()
	{
		return (T*)_GetPad();
	}

	CBehaviorGraphPad *_GetPad();


	CLevel *_GetLevel()	{		return _GetCtx()->level;	}

	CLevelObj *_GetLo();
	CLevelPlayer *_GetOwnerPlayer();

	CLevelPlayer *_GetLockPlayer();
	CLevelObj *_GetLockLo();
	float _GetLockPlayerSpeed();

	CLevelPlayer *_GetTalkPlayer();
	CLevelObj *_GetTalkLo();
	LevelPlayerID _GetTalkPlayerID();

	CLevelObj *_GetThreat();
	CLevelObj *_GetLoFromVar(StringID nm);
	CLevelObj *_GetLevelSkillTarget_Obj(BgnLevelSkillTarget &target);
	BOOL _GetLevelSkillTarget_Pos(BgnLevelSkillTarget &target,LevelPos &posTarget);

	CLevelObjSrc *_GetLos();
	CLevelObjParam *_GetLop();
	CLevelSkillDriver *_GetSkillDriver();
	CLevelBuffs*_GetBuffs();
	CLevelTroops*_ObtainTroops();
	CLevelTroop *_ObtainTroop(StringID nmTroop);
	CLevelTroops*_GetTroops();
	CLevelTroop *_GetTroop(StringID nmTroop);
	TroopCombatContext *_GetTcc();

	CLevelTalks *_GetTalks();
	BOOL _IsTalkActive();

	LevelAIContext *_GetAIContext();

	LevelSimpleMem *_GetSimpleMem();

	//Var Access
	virtual BOOL _SetBit(StringID nmVar,BOOL b);
	virtual BOOL _SetNumber(StringID nmVar,short n);
	virtual BOOL _SetID(StringID nmVar,BehaviorMemType tp,DWORD id);
	virtual BOOL _GetBit(StringID nmVar,BOOL &b);
	virtual BOOL _GetNumber(StringID nmVar,short &n);
	virtual BOOL _GetID(StringID nmVar,BehaviorMemType tp,DWORD &id);

	//BPR Access
	void _FindConstValue(StringID nm,BhvVal*&value,BhvValDeclare *&declare);
	void _FindParamValue(StringID nm,BhvVal*&value);
	void _FindValue(StringID nm,BhvVal*&value,BhvValDeclare*&declare);

	virtual BOOL _GetBPR(BPR_Bool &bpr);
	virtual int _GetBPR(BPR_Int &bpr);
	virtual float _GetBPR(BPR_Float &bpr);
	RecordID _GetBPR(BPR_ItemID &bpr);
	RecordID _GetBPR(BPR_SkillID &bpr);
	RecordID _GetBPR(BPR_BuffID &bpr);
	RecordID _GetBPR(BPR_UnitID &bpr);
	RecordID _GetBPR(BPR_EoID &bpr);
	StringID _GetBPR(BPR_StringID &bpr);
	//XXXXX:more BPR
	template <typename T>
	T *_GetBPR(BPR_Custom &bpr)
	{
// 		BehaviorValue *value;
// 		BehaviorValueDeclare *declare;
// 
// 		_FindValue(bpr.nmRef,value,declare);
// 
// 		if (value)
// 		{
// 			if(value->tp==BhvValueType_Custom)
// 			{
// 				if (value->bcc)
// 				{
// 					if (value->bcc->GetClass()->IsSameWith(Class_Ptr2(T)))
// 						return (T*)value->bcc;
// 				}
// 			}
// 			assert(FALSE);
// 			return NULL;//缺省值
// 		}
// 
// 		if (declare)
// 		{
// 			if(declare->tp==BhvValueType_Custom)
// 			{
// 				if (declare->bcc)
// 				{
// 					if (declare->bcc->GetClass()->IsSameWith(Class_Ptr2(T)))
// 						return (T*)declare->bcc;
// 				}
// 			}
// 			assert(FALSE);
// 			return NULL;//缺省值
// 		}

		return NULL;//缺省值
	}

	template <typename T>
	T *_GetBPR(BPR_CustomObj<T> &bpr)
	{
// 		if (bpr.bRef)
// 		{
// 			BehaviorValue *value;
// 			BehaviorValueDeclare*declare;
// 
// 			_FindValue(bpr.nmRef,value,declare);
// 
// 			if (value)
// 			{
// 				if(value->tp==BhvValueType_Custom)
// 				{
// 					if (value->bcc)
// 					{
// 						if (value->bcc->GetClass()->IsSameWith(Class_Ptr2(T)))
// 							return (T*)value->bcc;
// 					}
// 				}
// 				assert(FALSE);
// 				return NULL;//缺省值
// 			}
// 
// 			if (declare)
// 			{
// 				if(declare->tp==BhvValueType_Custom)
// 				{
// 					if (declare->bcc)
// 					{
// 						if (declare->bcc->GetClass()->IsSameWith(Class_Ptr2(T)))
// 							return (T*)declare->bcc;
// 					}
// 				}
// 				assert(FALSE);
// 				return NULL;//缺省值
// 			}
// 
// 			return NULL;//缺省值
// 		}
// 		else
// 			return &bpr.obj;
		return NULL;
	}


	BOOL _GetLocPos(StringID idLoc,LevelPos &pos);
	BOOL _GetLocXfm(StringID idLoc,LevelPos &pos,LevelFace &face);
	BOOL _GetRouteStartPos(StringID idRoute,LevelPos &pos,float &face);
	LevelRoute *_GetRoute(StringID idRoute);

	friend class CLevelBehavior;

};





struct BehaviorEvent
{
	BYTE tp;//BgpEvent类型
	LevelPlayerID idPlayerLock;
};


class CLevelBehavior:public CBehavior
{
public:
	DEFINE_CLASS(CLevelBehavior);
	CLevelBehavior()
	{
		Zero();
	}
	~CLevelBehavior()
	{
		Clear();
	}
	void Zero()
	{
		_nEvents=0;
		_bStarted=0;
	}
	void CLevelBehavior::Init(LevelBehaviorContext &ctx);
	virtual void Clear();

	virtual void Start();

	virtual void Update();


	void AddEvent(BYTE e);
	BehaviorEvent *FetchEvents(DWORD &c);//返回的值为BgpEvent的类型

	LevelBehaviorContext *GetContext()	{		return &_ctx;	}

protected:

	virtual AnimTick _GetT();

	//context
	LevelBehaviorContext _ctx;
	//Events
	BehaviorEvent _events[8];
	DWORD _nEvents:4;
	DWORD _bStarted:1;

};