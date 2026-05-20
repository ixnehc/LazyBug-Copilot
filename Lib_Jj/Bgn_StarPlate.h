#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"


class CBgp_StarPlateOp:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_StarPlateOp);

	virtual const char *GetTypeName()	{		return "StarPlateOp";	}
	virtual DWORD GetStubCount()
	{
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"成功");
			STUB_OUT(2,"失败");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Misc;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		if (mode==1)
			s="检查能否激活Site";
		if (mode==0)
			s="激活Site";
		if (mode==2)
			s="检查Site是否已激活";
		if (mode==3)
			s="检查Stand是否已激活(Obsolete)";
		if (mode==4)
			s="检查是否所有Troop都已死亡(Obsolete)";
		if (mode==5)
			s="检查是否需要SpawnEnemy";
		if (mode==6)
			s="通知Enemy已经Spawn";
		if (mode==7)
			s="检测是否有Site被激活";
		if (mode==8)
			s="检测是否所有Site均被激活";
	}

    BEGIN_GOBJ_PURE_UID2(CBgp_StarPlateOp,493,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(int,mode,1); GELEM_UID(1);
			GELEM_EDITVAR("模式",GVT_U,GSem(GSem_Interger,
				"激活Site模式"
				",检查能否激活Site"
				",检测Site是否激活"
				",检测Stand是否激活"
				",检测所有Troop都已死亡"
				",检查是否需要SpawnEnemy"
				",通知Enemy已经Spawn"
				",检测是否有Site被激活"
				",检测是否所有Site均被激活"
				),"工作模式");
    END_GOBJ();    

public: //当作protected
	int mode;

};


class CBgn_StarPlateOp:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_StarPlateOp);

	CBgn_StarPlateOp()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs) override;

protected:
};
