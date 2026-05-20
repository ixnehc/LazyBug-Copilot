#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"
#include "behaviorgraph/BehaviorParam.h"

#include "records/recordsdefine.h"

#include "LoAgentRef.h"


class CBgpGA_LanternRoute_Spawn:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_LanternRoute_Spawn);

	virtual const char *GetTypeName()	{		return "灯塔路线_Spawn";	}
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
		s="n/a";
		if ((idEo!=RecordID_Invalid)&&(idPathRes!=RecordID_Invalid)&&(nmVar!=StringID_Invalid))
		{
			FormatString(s,"创建灯塔路线对象(%s),保存在%s中",assist->GetEoName(idEo),assist->GetStr(nmVar));
		}

	}

    BEGIN_GOBJ_PURE_UID2(CBgpGA_LanternRoute_Spawn,456,1);

		GELEM_VAR_INIT(RecordID,idEo,RecordID_Invalid);GELEM_UID(1)
			GELEM_EDITVAR("Eo",GVT_U,GSem(GSem_RecordID,"eos"),"Eo");

		GELEM_VARVECTOR(i_math::matrix43f,matsLS);GELEM_UID(4)
			GELEM_EDITVAR("位点(局部空间)",GVT_Fx12,GSem(GSem_Unknown,"MatSetLS"),"位点(局部空间)");
			GELEM_BVR();

		GELEM_VAR_INIT(RecordID,idPathRes,RecordID_Invalid);GELEM_UID(2)
			GELEM_EDITVAR("路径资源",GVT_U,GSem(GSem_RecordID,"resources"),"路径资源");
			GELEM_BVR();

		GELEM_OBJVAR(LoAgentRef,refAgentPortal);GELEM_UID(5)
			GELEM_EDITOBJ_EX("石门引用","石门引用",GSem_Unknown);
			GELEM_BVR();

		GELEM_BEHAVIORMEM_OBJID(nmVar,"Eo保存变量","Eo保存变量");GELEM_UID(3)
    END_GOBJ();    

public: //当作protected

	RecordID idEo;
	DEFINE_BVR(RecordID,idPathRes);
	StringID nmVar;
	DEFINE_BVR(LoAgentRef,refAgentPortal);
	DEFINE_BVR(std::vector<i_math::matrix43f>,matsLS);

};


class CBgnGA_LanternRoute_Spawn:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_LanternRoute_Spawn);

	CBgnGA_LanternRoute_Spawn()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};


class CBgpGA_LanternRoute_Stop:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_LanternRoute_Stop);

	virtual const char *GetTypeName()	{		return "灯塔路线_Stop";	}
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
		s="n/a";
		if (nmVar!=StringID_Invalid)
		{
			FormatString(s,"停止路线对象(%s)",assist->GetStr(nmVar));
		}

	}

	BEGIN_GOBJ_PURE_UID2(CBgpGA_LanternRoute_Stop,457,1);

	GELEM_BEHAVIORMEM_OBJID(nmVar,"Eo变量","Eo变量");GELEM_UID(1)
		END_GOBJ();    

public: //当作protected

	StringID nmVar;

};


class CBgnGA_LanternRoute_Stop:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_LanternRoute_Stop);

	CBgnGA_LanternRoute_Stop()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};



class CBgpGA_LanternRoute_Check:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_LanternRoute_Check);

	virtual const char *GetTypeName()	{		return "灯塔路线_检测状态";	}
	virtual DWORD GetStubCount()
	{
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"是");
			STUB_OUT(2,"否");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Misc;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if (nmVar!=StringID_Invalid)
		{
			FormatString(s,"检测路线(%s)的石门是否开启",assist->GetStr(nmVar));
		}

	}

	BEGIN_GOBJ_PURE_UID2(CBgpGA_LanternRoute_Check,458,1);

	GELEM_BEHAVIORMEM_OBJID(nmVar,"Eo变量","Eo变量");GELEM_UID(1)
		END_GOBJ();    

public: //当作protected

	StringID nmVar;

};


class CBgnGA_LanternRoute_Check:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_LanternRoute_Check);

	CBgnGA_LanternRoute_Check()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};