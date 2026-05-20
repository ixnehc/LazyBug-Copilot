#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"



class CBgpThreat_UpdateCtrl:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpThreat_UpdateCtrl);

	virtual const char *GetTypeName()	{		return "Threat更新控制";	};
	virtual DWORD GetStubCount()	
	{		
		return 2;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB()
			STUB_IN(0,"入口");
			STUB_OUT(1,"出口");
		END_STUB()
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Threat;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

	BEGIN_GOBJ_PURE_UID(CBgpThreat_UpdateCtrl,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(BOOL,bActive,TRUE);
			GELEM_EDITVAR("激活更新",GVT_S,GSem_Boolean,"是否激活更新");

	END_GOBJ();    

public: //当作protected

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		if (bActive)
			s="更新Threat";
		else
			s="不更新Threat";
	}

	BOOL bActive;

};

class CBgnThreat_UpdateCtrl:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnThreat_UpdateCtrl);

	CBgnThreat_UpdateCtrl()
	{
		_bFinalized=FALSE;
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void RewindOk(WORD keyRewind,BGNOutputs &outputs);//因为执行成功导致的Rewind
	virtual void RewindFail(WORD keyRewind,BGNOutputs &outputs);//因为执行失败导致的Rewind
	virtual void Break(BGNOutputs &outputs);
protected:

	BOOL _Finalize(BGNOutputs &outputs);//返回有没有启动一个thread

	BOOL _bFinalized;

};

