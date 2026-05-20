#pragma once

#include "class/class.h"
#include <unordered_map>

#include "LevelDefines.h"

#include "LevelUnitArg.h"

enum LevelTroopRank
{
	LevelTroopRank_None,
	LevelTroopRank_Leader,
	LevelTroopRank_Minion,

	LevelTroopRank_ForceDword=0xffffffff
};

typedef DWORD LevelTroopRankFlags;
#define LevelTroopRankFlag_Leader ((LevelTroopRankFlags)(1<<(int)LevelTroopRank_Leader))
#define LevelTroopRankFlag_Minion ((LevelTroopRankFlags)(1<<(int)LevelTroopRank_Minion))
#define LevelTroopRankFlag_All (LevelTroopRankFlag_Leader|LevelTroopRankFlag_Minion)

#define LevelTroopRankFlags_GSemContrains "Leader:2,Minion:4"

extern const char *LevelTroopRank_GetDesc(LevelTroopRank rank);
extern const char *LevelTroopRankFlag_GetDesc(LevelTroopRankFlags flags,StringID nmRef);




struct LevelTroopDesc
{
	BOOL bEnable;
	std::vector<i_math::matrix43f> mats;
	float hang;//离地高度
	RecordID idUnit;
	LevelTroopRank rank;
	LevelSkillGrade grd;
	RecordID idBirthBuff;
	StringID senarioAI;
	LevelUnitArg *arg;
	LevelPlayerID idPlayer;

	int nMin;
	int nMax;
	float wt;

	BOOL IsEnabled()	{		return bEnable;	}

	BOOL IsValid()
	{
		return idUnit!=RecordID_Invalid;
	}


	const char *GetBrief(void *param);

    BEGIN_GOBJ_PURE(LevelTroopDesc,1);
		GOBJ_GETBRIEF_FUNC(GetBrief);

		GELEM_VAR_INIT(RecordID,idUnit,RecordID_Invalid);
			GELEM_EDITVAR("单位",GVT_U,GSem(GSem_RecordID,"units"),"创建哪个单位");

		GELEM_VAR_INIT(BOOL,bEnable,TRUE)
			GELEM_EDITVAR("Enable",GVT_S,GSem(GSem_Boolean,"PlayerID,位点,等级,出生Buff,AI方案,最小值,最大值"),"是否Enable");

		GELEM_VAR_INIT(LevelPlayerID,idPlayer,LevelPlayerID_Wild);
			GELEM_EDITVAR("PlayerID",GVT_B,GSem(GSem_Interger,"Wild:15,NeutalWild:14,PlayerWild:13"),"隶属于哪个Player");

		GELEM_VARVECTOR(i_math::matrix43f,mats)
			GELEM_EDITVAR("位点",GVT_Fx12,GSem(GSem_Unknown,"MatSet"),"位点");

		GELEM_VAR_INIT(float,hang,0.0f);
				GELEM_EDITVAR("离地高度",GVT_F,GSem(GSem_Float,"0.0,10.0,0.05"),"离地高度");

		GELEM_VAR_INIT(LevelTroopRank,rank,LevelTroopRank_Minion);
			GELEM_EDITVAR("职级",GVT_S,GSem(GSem_Interger,"Leader:1,Minion:2"),"单位的职级");

		GELEM_VAR_INIT(LevelGrade,grd,LevelSkillGrade_Invalid);
			GELEM_EDITVAR("等级",GVT_B,GSem(GSem_Interger,LevelGradeBase_SemConstraint),"单位的等级");

		GELEM_VAR_INIT(RecordID,idBirthBuff,RecordID_Invalid);
			GELEM_EDITVAR("出生Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"出生Buff");

		GELEM_VAR_INIT( StringID,senarioAI,StringID_Invalid);
			GELEM_EDITVAR( "AI方案", GVT_U, GSem(GSem_StringID,"AIScenario"), "AI方案" );

		GELEM_VAR_INIT( int,nMin,1);	
			GELEM_EDITVAR( "最小值", GVT_S, GSem_Interger, "最小值" );

		GELEM_VAR_INIT( int,nMax,5);	
			GELEM_EDITVAR( "最大值", GVT_S, GSem_Interger, "最大值" );

		GELEM_VAR_INIT(float,wt,1.0f);
				GELEM_EDITVAR("权重",GVT_F,GSem(GSem_Float,"0.0,10.0,0.05"),"权重");

		GELEM_DYNOBJPTR_UNITPARAM(LevelUnitArg,arg,UnitArg_Null, "特定单位参数", "特定单位参数" );
			GELEM_DYNOBJPTR_CLASS_UNITPARAM_LIST();

    END_GOBJ();    

};

//Frame代表编制
struct LevelTroopFrame
{
	LevelTroopFrame()
	{
		desc=NULL;
		rank=LevelTroopRank_None;
		idUnit=LevelObjID_Invalid;
	}

	BOOL CheckRank(LevelTroopRankFlags flags)
	{
		if (desc)
			return ((1<<desc->rank)&flags)!=0;
		if (rank!=LevelTroopRank_None)
			return ((1<<rank)&flags)!=0;
		return FALSE;
	}
	LevelTroopDesc *desc;
	LevelTroopRank rank;
	LevelObjID idUnit;
};

struct TroopCombatContext
{
	void Clear();
	std::vector<CLevelObj *> detects;
};

struct LevelAICmd;
class CLevelTroop
{
public:
	DEFINE_CLASS(CLevelTroop);

	CLevelTroop()
	{
		Zero();
	}
	void Init(CLevel *level);
	void Clear();
	void Zero()	
	{		
		_level=NULL;	
	}

	BOOL IsEmpty()	{		return _frames.empty();	}

	void ClearUnitsAndFrames(BOOL bDestroy);

	void AddFrame(LevelTroopDesc *desc);//传入的指针需要保证永久有效
	void FlushDeadFrames();

	DWORD GetFrameCount()	{		return _frames.size();	}
	LevelTroopFrame *GetFrame(DWORD idxFrame);

	void DetachUnit(LevelObjID idUnit);
	void DetachAllUnits(BOOL bDestroy);
	void FlushDeadUnits();
	BOOL AddUnit(LevelTroopDesc *desc,LevelObjID idUnit);//添加一个编制以及unit
	BOOL FillUnit(LevelTroopDesc *desc,LevelObjID idUnit);//将一个unit填入编制
	BOOL AddUnit(LevelTroopRank rank,LevelObjID idUnit);//添加一个编制以及unit

	BOOL IsAllDead(LevelTroopRankFlags  flagsRank);
	BOOL CheckHealthRatio_AnyBelow(LevelTroopRankFlags flagsRank,float ratio);
	BOOL CheckHealthRatio_AllBelow(LevelTroopRankFlags flagsRank,float ratio);


	void SetCmdToUnits(LevelTroopRankFlags flagsRank,StringID idCmd);
	void DiscardCmdFromUnits(LevelTroopRankFlags flagsRank,StringID idCmd);

	void SetCmdToAllUnits(StringID idCmd)	{		SetCmdToUnits(LevelTroopRankFlag_All,idCmd);	}
	void DiscardCmdFromAllUnits(StringID idCmd)	{		DiscardCmdFromUnits(LevelTroopRankFlag_All,idCmd);	}

	TroopCombatContext *GetCombatContext()	{		return &_tcc;	}

protected:

	BOOL _CheckDead(LevelObjID idUnit);

	std::deque<LevelTroopFrame> _frames;
	TroopCombatContext _tcc;
	CLevel *_level;
};

class CLevelTroops
{
public:
	DEFINE_CLASS(CLevelTroops);
	
	CLevelTroops()
	{
		Zero();
	}
	~CLevelTroops()
	{
		Clear();
	}
	void Zero()
	{
		_level=NULL;
	}
	void Init(CLevel *level);
	void Clear();

	CLevelTroop *Obtain(StringID nm);//返回临时指针,不要保留这个指针
	CLevelTroop *Get(StringID nm);//返回临时指针,不要保留这个指针
	CLevelTroop *GetFirst();
	void Remove(StringID nm);

protected:

	std::unordered_map<StringID,CLevelTroop*>_buf;

	CLevel *_level;
};

