#pragma once

#include "behaviorgraph/BehaviorGraphPads.h"
#include "behaviorgraph/Behavior.h"
#include "behaviorgraph/BehaviorCustomConst.h"

#include "behaviorgraph/BehaviorValue.h"

#include "LevelBehavior.h"


class CBgp_CalcFace:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_CalcFace);

	virtual const char *GetTypeName()	{		return "计算朝向";	};
	virtual DWORD GetStubCount()	
	{		
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB()
			STUB_IN(0,"开始");
			STUB_OUT(1,"成功");
			STUB_OUT(2,"失败");
		END_STUB()
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Math;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

    BEGIN_GOBJ_PURE_UID(CBgp_CalcFace,1);
		GELEM_BGP_BASE();

		GELEM_BEHAVIORMEM_POS(varSrcPos,"起始位置变量","使用那个变量里的位置"); GELEM_UID(1);
		GELEM_BEHAVIORMEM_POS(varTargetPos,"终止位置变量","使用那个变量里的位置"); GELEM_UID(2);
		GELEM_BEHAVIORMEM_NUMBER(result,"朝向结果","行为图内存变量名称");GELEM_UID(3);

	END_GOBJ();    

public: //当作protected

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if (result!=StringID_Invalid)
		{
			if ((varSrcPos!=StringID_Invalid)&&(varTargetPos!=StringID_Invalid))
			{
				FormatString(s,"计算从[%s]到[%s]的朝向,结果保存在[%s]中",StrLib_GetStr(varSrcPos),StrLib_GetStr(varTargetPos),StrLib_GetStr(result));
			}
		}
	}

	StringID varSrcPos;
	StringID varTargetPos;

	StringID result;

};

class CBgn_CalcFace:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_CalcFace);

	CBgn_CalcFace()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};
