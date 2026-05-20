#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

#include "gds/GObjUID.h"


class CBgp_创建飞妖:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_创建飞妖);

	virtual const char *GetTypeName()	{		return "创建飞妖";	}
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

    BEGIN_GOBJ_PURE_UID(CBgp_创建飞妖,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(RecordID,idUnit,RecordID_Invalid);
			GELEM_EDITVAR("飞妖ID",GVT_U,GSem(GSem_RecordID,"units"),"飞妖ID");
		GELEM_BEHAVIORMEM_OBJID(varSummoned,"创建出的飞妖对象保存在哪个变量","创建出的飞妖对象保存变量");
		GELEM_VAR_INIT( StringID,varSummoner,StringID_Invalid);
			GELEM_EDITVAR( "自己保存到飞妖行为图中的哪个变量", GVT_U, GSem(GSem_StringID,"行为图内存变量名称"), "自己保存到飞妖行为图中的哪个变量");

    END_GOBJ();    

public: //当作protected
	RecordID idUnit;
	
	StringID varSummoned;
	StringID varSummoner;

};


class CBgn_创建飞妖:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_创建飞妖);

	CBgn_创建飞妖()
	{
		_tStart=0;
		_idSummoned=LevelObjID_Invalid;
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs) override;
	virtual void Update(BGNOutputs &outputs) override;

protected:

	LevelObjID _idSummoned;

	AnimTick _tStart;

};

