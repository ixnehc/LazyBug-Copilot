#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

#include "behaviorgraph/BehaviorCustomConst.h"
#include "behaviorgraph/BehaviorParam.h"

#include "LevelTroops.h"

struct BP_MakeBuff
{
	BEGIN_GOBJ_PURE(BP_MakeBuff,1);

		GELEM_VAR_INIT(int,special,0);
			GELEM_EDITVAR("特殊类型Buff",GVT_S,GSem(GSem_Interger,
				"n/a:0"		","
				"化为灰烬:1"	"|要加哪个Buff"
				),"特殊类型Buff");
		GELEM_VAR_INIT(RecordID,idBuff,RecordID_Invalid);
			GELEM_EDITVAR("要加哪个Buff", GVT_U, GSem(GSem_RecordID,"buffs"), "要加哪个Buff" );

		GELEM_VAR_INIT( float,speed,0.0f);	
			GELEM_EDITVAR( "添加Buff速度", GVT_F, GSem(GSem_Float,"0,100,0.05"), "每秒为几个单位添加Buff,0表示无限个" );

	END_GOBJ();

	int special;
	RecordID idBuff;
	float speed;


};


class CBgpTroop_MakeBuff:public CBehaviorGraphPad
{
	virtual const char *GetTypeName()	{		return "添加Buff";	}
	virtual DWORD GetStubCount()
	{
		return 2;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"结束");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Troop;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		FormatString(s,"为Troop(%s)中的(%s)添加Buff",
			GetBVRDesc_StringID(BVR_ARG(_troop),assist),
			LevelTroopRankFlag_GetDesc(BVR_ARG(_flagsRank))
			);
	}

    BEGIN_GOBJ_PURE(CBgpTroop_MakeBuff,1);

		GELEM_OBJ(BP_MakeBuff,_param);
			GELEM_EDITOBJ("添加Buff参数","添加Buff参数");
			GELEM_BVR();
		GELEM_BEHAVIOR_TROOPREF(_troop,"Troop名称","为哪个Troop里的单位添加Buff");
			GELEM_BVR();
		GELEM_VAR_INIT(LevelTroopRankFlags,_flagsRank,LevelTroopRankFlag_All);
			GELEM_EDITVAR("职级",GVT_U,GSem(GSem_Flags,LevelTroopRankFlags_GSemContrains),"哪些职级");
			GELEM_BVR();

    END_GOBJ();    

public: //当作protected

	DEFINE_BVR(BP_MakeBuff,_param);
	DEFINE_BVR(StringID,_troop);
	DEFINE_BVR(LevelTroopRankFlags,_flagsRank);

};


class CBgnTroop_MakeBuff:public CLevelBgn
{
public:
	CBgnTroop_MakeBuff()
	{
		_nMade=0;
		_tStart=0;
		_idBuff=RecordID_Invalid;
		_specialBuff=0;
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);

	struct MakeBuffInfo
	{
		MakeBuffInfo()
		{
			memset(this,0,sizeof(*this));
		}
		LevelObjID idUnit;
	};

protected:

	virtual void _MakeBuff(MakeBuffInfo &info)=0;

	void _DoMakeBuff(MakeBuffInfo &info,LevelBuffArg *arg,AnimTick dur);

	std::deque<MakeBuffInfo> _infos;
	std::vector<short>_indices;
	DWORD _nMade;
	AnimTick _tStart;
	RecordID _idBuff;
	int _specialBuff;

};


