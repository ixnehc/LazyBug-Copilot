#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

class CBgpSetupSlatesA_SetType:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpSetupSlatesA_SetType);

	virtual const char *GetTypeName()	{		return "设置类型";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_SlatesA;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if (grp!=RecordID_Invalid)
			FormatString(s,"将石板组(%s)内石板设为 %s 类型",StrLib_GetStr(grp),LevelSlateA_NameFromType(tp));
		else
			FormatString(s,"将石板组内所有石板设为 %s 类型",LevelSlateA_NameFromType(tp));
		if (result!=StringID_Invalid)
			AppendFmtString(s,"\n结果保存在变量{%s}中",assist->GetStr(result));
	}

    BEGIN_GOBJ_PURE_UID(CBgpSetupSlatesA_SetType,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT( StringID,grp,StringID_Invalid);	
			GELEM_EDITVAR( "石板组名", GVT_U, GSem(GSem_StringID,"石板组名称"), "石板组名称" );
		GELEM_VAR_INIT(LevelSlateType,tp,LevelSlateTypeA_Blank);
			GELEM_EDITVAR("类型",GVT_S,GSem(GSem_Interger,GSemConstraint_LevelSlateTypeA),"类型");
		GELEM_VAR_INIT( StringID,result,StringID_Invalid);
			GELEM_EDITVAR( "产生石板保存变量", GVT_U, GSem(GSem_StringID,"行为图内存变量名称:Obj"), "产生的石板保存在哪个变量里");
    END_GOBJ();    

public: //当作protected

	StringID grp;
	LevelSlateType tp;
	StringID result;
};


class CBgnSetupSlatesA_SetType:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnSetupSlatesA_SetType);

	CBgnSetupSlatesA_SetType()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};
