#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

#include "behaviorgraph/BehaviorCustomConst.h"
#include "behaviorgraph/BehaviorParam.h"

struct BP_SetTroopVar
{
	BEGIN_GOBJ_PURE(BP_SetTroopVar,1);

		GELEM_VAR_INIT( StringID,nmVar,StringID_Invalid);
			GELEM_EDITVAR( "变量名", GVT_U, GSem(GSem_StringID,"行为图内存变量名称"), "设置那个变量");

		GELEM_VAR_INIT( float,speed,0.0f);	
			GELEM_EDITVAR( "设置变量速度", GVT_F, GSem(GSem_Float,"0,100,0.05"), "每秒为几个单位设置变量值,0表示无限个" );

		GELEM_VAR_INIT(int,value,0);
			GELEM_EDITVAR("值",GVT_S,GSem_Interger,"设什么值");

	END_GOBJ();

	StringID nmVar;
	float speed;
	int value;


};


class CBgpTroop_SetVar:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgpTroop_SetVar);
	virtual const char *GetTypeName()	{		return "设置Troop的变量";	}
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
		s="n/a";
		if (_troop!=StringID_Invalid)
			AppendFmtString(s,"为 %s 中的单位设置变量值",assist->GetStr(_troop));
	}

    BEGIN_GOBJ_PURE(CBgpTroop_SetVar,1);

		GELEM_OBJ(BP_SetTroopVar,_param);
			GELEM_EDITOBJ("设置变量参数","设置变量参数");
			GELEM_BVR();
		GELEM_BEHAVIOR_TROOPREF(_troop,"Troop名称","为哪个Troop里的单位设置变量");
    END_GOBJ();    

public: //当作protected

	DEFINE_BVR(BP_SetTroopVar,_param);
	StringID _troop;

};


class CBgnTroop_SetVar:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnTroop_SetVar);
	CBgnTroop_SetVar()
	{
		_nMade=0;
		_tStart=0;
		_idBuff=RecordID_Invalid;
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);

	struct SetVarInfo
	{
		SetVarInfo()
		{
			memset(this,0,sizeof(*this));
		}
		LevelObjID idUnit;
		StringID nmVar;
		int value;
	};

protected:

	virtual void _SetVar(SetVarInfo &info);

	std::deque<SetVarInfo> _infos;
	std::vector<short>_indices;
	DWORD _nMade;
	AnimTick _tStart;
	RecordID _idBuff;

};


