#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"


class CBgpGA_FillFlusks:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_FillFlusks);

	virtual const char *GetTypeName()	{		return "填充宝瓶";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_GA;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		FormatString(s,"装满TalkPlayer的宝瓶");
	}

    BEGIN_GOBJ_PURE_UID2(CBgpGA_FillFlusks,423,1);
	END_GOBJ();    

public: //当作protected

};


class CBgnGA_FillFlusks:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_FillFlusks);

	CBgnGA_FillFlusks()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};


class CBgpGA_CheckFilledFlusks:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_CheckFilledFlusks);

	virtual const char *GetTypeName()	{		return "检测已填充宝瓶";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_GA;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		FormatString(s,"TalkPlayer是否有已填充宝瓶");
	}

	BEGIN_GOBJ_PURE_UID2(CBgpGA_CheckFilledFlusks,466,1);
	END_GOBJ();    

public: //当作protected

};


class CBgnGA_CheckFilledFlusks:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_CheckFilledFlusks);

	CBgnGA_CheckFilledFlusks()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};




class CBgpGA_DecFilledFlusks:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_DecFilledFlusks);

	virtual const char *GetTypeName()	{		return "消耗已填充宝瓶";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_GA;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		FormatString(s,"消耗一个TalkPlayer的已填充宝瓶");
	}

	BEGIN_GOBJ_PURE_UID2(CBgpGA_DecFilledFlusks,467,1);
	END_GOBJ();    

public: //当作protected

};


class CBgnGA_DecFilledFlusks:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_DecFilledFlusks);

	CBgnGA_DecFilledFlusks()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};
