#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"


class CBgpMB_CheckResCount:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpMB_CheckResCount);

	virtual const char *GetTypeName()	{		return "检查资源数量";	}
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
		BOOL bAny=FALSE;
		for (int i=1;i<MBRes_ActualMax;i++)
		{
			if (cMin[i]>0)
				bAny=TRUE;
		}
		if (bAny)
		{
			s="";
			if (cMin[MBRes_Mana]>0)
				AppendFmtString(s,"法力达到%d",cMin[MBRes_Mana]);
			if (cMin[MBRes_Gold]>0)
			{
				if(s!="")
					s+=",并且";
				AppendFmtString(s,"黄金数达到%d",cMin[MBRes_Gold]);
			}
			if (cMin[MBRes_Crystal]>0)
			{
				if(s!="")
					s+=",并且";
				AppendFmtString(s,"水晶数达到%d",cMin[MBRes_Crystal]);
			}
		}
	}

    BEGIN_GOBJ_PURE(CBgpMB_CheckResCount,1);
		GELEM_VAR_INIT(RecordID,idTile,RecordID_Invalid);
			GELEM_EDITVAR("格子的类型",GVT_U,GSem(GSem_RecordID,"magictiles"),"要检查什么格子");

		GELEM_VARARRAY_INIT(int ,cMin,0); 
			GELEM_EDITVAR("各种资源的最小数量",GVT_S,GSem(GSem_Interger,"$Lable{//n/a,法力,金币,水晶}"),"各种资源的最小数量");

    END_GOBJ();    

public: //当作protected
	RecordID idTile;
	int cMin[MBRes_ActualMax];

};


class CBgnMB_CheckResCount:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnMB_CheckResCount);


	CBgnMB_CheckResCount()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

