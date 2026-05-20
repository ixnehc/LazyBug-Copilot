#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"


class CBgpRtnu_GetCount:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpRtnu_GetCount);

	virtual const char *GetTypeName()	{		return "得到随从个数";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Rtnu;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if((idUnit!=RecordID_Invalid)&&(nmVar!=StringID_Invalid)&&(nmPlayerVar!=StringID_Invalid))
		{
			FormatString(s,"得到[%s]的随从[%s]的数量,保存到变量[%s]中",assist->GetStr(nmPlayerVar),assist->GetUnitName(idUnit),assist->GetStr(nmVar));
		}
	}

    BEGIN_GOBJ_PURE_UID2(CBgpRtnu_GetCount,449,1);
		GELEM_BGP_BASE();

		GELEM_BEHAVIORMEM_OBJID(nmPlayerVar,"Player变量名称","得到哪个player的随从数量")
		GELEM_VAR_INIT(RecordID,idUnit,RecordID_Invalid);
			GELEM_EDITVAR("随从单位ID",GVT_U,GSem(GSem_RecordID,"units"),"随从单位ID");
		GELEM_BEHAVIORMEM_INTERGER(nmVar,"保存变量","得到的数量值保存在哪个变量中")
    END_GOBJ();    

public: //当作protected
	RecordID idUnit;
	StringID nmVar;
	StringID nmPlayerVar;

};

struct AttrNodeBase;
class CBgnRtnu_GetCount:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnRtnu_GetCount);

	CBgnRtnu_GetCount()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);


protected:



};
