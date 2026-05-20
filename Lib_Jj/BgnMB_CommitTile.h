#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"


class CBgpMB_CommitTile:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpMB_CommitTile);

	virtual const char *GetTypeName()	{		return "释放格子";	}
	virtual DWORD GetStubCount()
	{
		return 2;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"成功");
			STUB_OUT(2,"失败");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Action;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_MagicBoard;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if (idTile!=RecordID_Invalid)
			s=assist->GetMagicTileName(idTile);
	}

    BEGIN_GOBJ_PURE(CBgpMB_CommitTile,1);
		GELEM_VAR_INIT(RecordID,idTile,RecordID_Invalid);
			GELEM_EDITVAR("格子的类型",GVT_U,GSem(GSem_RecordID,"magictiles"),"要施放什么格子");
    END_GOBJ();    

public: //当作protected
	RecordID idTile;

};


class CBgnMB_CommitTile:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnMB_CommitTile);


	CBgnMB_CommitTile()
	{
	}


	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);
	virtual void Break(BGNOutputs &outputs);
	virtual void Destroy();

protected:

	void _Update(MagicBoardAIContext *ctx,BGNOutputs &outputs);
};

