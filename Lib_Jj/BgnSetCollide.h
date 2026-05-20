#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "LevelDetectTargetFlags.h"

class CBgp_SetCollide:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_SetCollide);

	enum Op
	{
		None,
		SetGhost,
		ClearGhost,

		ForceDword=0xffffffff,
	};

	virtual const char *GetTypeName()	{		return "设置碰撞";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Action;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if (_op==SetGhost)
			s="设为Ghost模式";
		if (_op==ClearGhost)
			s="清除Ghost模式";
	}

    BEGIN_GOBJ_PURE_UID(CBgp_SetCollide,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(Op,_op,SetGhost);
			GELEM_EDITVAR("操作",GVT_S,GSem(GSem_Interger,"设置Ghost:1,清除Ghost:2"),"设置碰撞的操作");
	END_GOBJ();    

public: //当作protected

	Op _op;

};


class CBgn_SetCollide:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_SetCollide);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};
