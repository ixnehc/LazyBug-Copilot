#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"


class CBgpMB_CheckCommitCount:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpMB_CheckCommitCount);

	virtual const char *GetTypeName()	{		return "检查释放格子数量";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Condition;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_MagicBoard;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if (idTile!=RecordID_Invalid)
			FormatString(s,"格子(%s)的数量是否达到%d个",assist->GetMagicTileName(idTile),cMin);
	}

    BEGIN_GOBJ_PURE(CBgpMB_CheckCommitCount,1);
		GELEM_VAR_INIT(RecordID,idTile,RecordID_Invalid);
			GELEM_EDITVAR("格子的类型",GVT_U,GSem(GSem_RecordID,"magictiles"),"要检查什么格子");
		GELEM_VAR_INIT(int,cMin,1);
			GELEM_EDITVAR("最小数量",GVT_S,GSem(GSem_Interger,"1:1,2:2,3:3,4:4,5:5,6:6,7:7,8:8,9:9,10:10"),"最小格子数量");

    END_GOBJ();    

public: //当作protected
	RecordID idTile;
	int cMin;

};


class CBgnMB_CheckCommitCount:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnMB_CheckCommitCount);


	CBgnMB_CheckCommitCount()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

