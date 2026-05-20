#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"
#include "behaviorgraph/BehaviorParam.h"
#include "behaviorgraph/BehaviorCustomConst.h"

#include "records/recordsdefine.h"


struct DiscipleParam
{
	RecordID idSpell;
	int price;
	int nCrystal;

	BEGIN_GOBJ_PURE(DiscipleParam,1);

		GELEM_VAR_INIT(RecordID,idSpell,RecordID_Invalid);GELEM_UID(1)
			GELEM_EDITVAR("Spell奖励",GVT_U,GSem(GSem_RecordID,"upgrades"),"Spell");
		GELEM_VAR_INIT(int,price,1);GELEM_UID(2);
			GELEM_EDITVAR("价格",GVT_S,GSem_Interger,"价格");
		GELEM_VAR_INIT(int,nCrystal,1);GELEM_UID(3);
			GELEM_EDITVAR("魔晶奖励",GVT_S,GSem_Interger,"魔晶奖励");
	END_GOBJ();

};

struct BMO_DiscipleData:public CBehaviorMemObj
{
	DECLARE_CLASS(BMO_DiscipleData);
	BEGIN_GOBJ_PURE(BMO_DiscipleData,1);

		GELEM_OBJ(DiscipleParam,param);
			GELEM_UID(1);

	END_GOBJ();

	DiscipleParam param;
};



class CBgpGA_Diciple_Init:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_Diciple_Init);

	virtual const char *GetTypeName()	{		return "魔法使徒_初始化单位";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Misc;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{

	}

    BEGIN_GOBJ_PURE_UID2(CBgpGA_Diciple_Init,451,1);

		GELEM_OBJ(DiscipleParam,param);
			GELEM_EDITOBJ("初始化参数","初始化参数");
			GELEM_BVR();
		GELEM_VAR_INIT( StringID,nmVar,StringID_Invalid);	
			GELEM_EDITVAR( "变量名称", GVT_U, GSem(GSem_StringID,"行为图内存变量名称"), "变量名称" );
		GELEM_BEHAVIOR_TROOPREF(troop,"Troop名称","目标Troop");

    END_GOBJ();    

public: //当作protected

	DEFINE_BVR(DiscipleParam,param);
	StringID nmVar;
	StringID troop;


};


class CBgnGA_Diciple_Init:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_Diciple_Init);

	CBgnGA_Diciple_Init()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);

protected:

};

