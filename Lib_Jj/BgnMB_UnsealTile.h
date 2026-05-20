#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"


class CBgpMB_UnsealTile:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpMB_UnsealTile);

	virtual const char *GetTypeName()	{		return "翻开格子";	}
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
	virtual BgpFamily GetFamily()	{		return BgpFamily_MagicBoard;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
	}

    BEGIN_GOBJ_PURE(CBgpMB_UnsealTile,1);
    END_GOBJ();    

public: //当作protected

};


class CBgnMB_UnsealTile:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnMB_UnsealTile);


	CBgnMB_UnsealTile()
	{
	}


	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);
	virtual void Break(BGNOutputs &outputs);
	virtual void Destroy();

protected:

	void _Update(MagicBoardAIContext *ctx,BGNOutputs &outputs);
};

