#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorParam.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "bitset/bitset.h"

#include "LevelAttrs_Weak.h"



#define Repeater_OutputStart (1)
class CBgp_Repeater_Obsolete:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_Repeater_Obsolete);

	enum Mode
	{
		StepByStep,
		TryUntilTrue,
		AbandonIfFalse,
		EnsureTrue,
	};

	virtual const char *GetTypeName()	{		return "重复执行(Obsolete)";	};
	virtual DWORD GetStubCount()	
	{		
		return _nSteps>20?(Repeater_OutputStart+20):(Repeater_OutputStart+_nSteps);
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB()
			STUB_IN(0,"开始");
			STUB_OUT(1,"步骤01");
			STUB_OUT(2,"步骤02");
			STUB_OUT(3,"步骤03");
			STUB_OUT(4,"步骤04");
			STUB_OUT(5,"步骤05");
			STUB_OUT(6,"步骤06");
			STUB_OUT(7,"步骤07");
			STUB_OUT(8,"步骤08");
			STUB_OUT(9,"步骤09");
			STUB_OUT(10,"步骤10");
			STUB_OUT(11,"步骤11");
			STUB_OUT(12,"步骤12");
			STUB_OUT(13,"步骤13");
			STUB_OUT(14,"步骤14");
			STUB_OUT(15,"步骤15");
			STUB_OUT(16,"步骤16");
			STUB_OUT(17,"步骤17");
			STUB_OUT(18,"步骤18");
			STUB_OUT(19,"步骤19");
			STUB_OUT(20,"步骤20");
		END_STUB()
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Controller;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

    BEGIN_GOBJ_PURE_UID(CBgp_Repeater_Obsolete,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(int,_mode,0);
			GELEM_EDITVAR("运行模式",GVT_S,GSem(GSem_Interger,"正常模式,尝试到成功为止,遇失败停止,确保成功"),"重复模式");
		GELEM_VAR_INIT(int,_nSteps,1);
			GELEM_EDITVAR("步骤个数",GVT_S,GSem(GSem_Interger,"1:1,2:2,3:3,4:4,5:5,6:6,7:7,8:8,9:9,10:10,11:11,12:12,13:13,14:14,15:15,16:16,17:17,18:18,19:19,20:20"),"有几个步骤");
		GELEM_VAR_INIT(int,_nLoop,0);
			GELEM_EDITVAR("重复次数",GVT_S,GSem_Interger,"重复几次,0表示重复无数次");
	END_GOBJ();    

public: //当作protected

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		switch(_mode)
		{
			case StepByStep:
			{
				if (_nLoop>0)
					FormatString(s,"重复执行,%d次",_nLoop);
				else
					FormatString(s,"重复执行,永久");
				break;
			}
			case TryUntilTrue:
			{
				if (_nLoop>0)
					FormatString(s,"依次尝试每个步骤,直到成功为止,共%d个循环",_nLoop);
				else
					FormatString(s,"依次尝试每个步骤,直到成功为止,永久循环");
				break;
			}
			case AbandonIfFalse:
			{
				if (_nLoop>0)
					FormatString(s,"依次尝试每个步骤,遇到失败就停止,共%d个循环",_nLoop);
				else
					FormatString(s,"依次尝试每个步骤,遇到失败就停止,永久执行");
				break;
			}
			case EnsureTrue:
			{
				if (_nLoop>0)
					FormatString(s,"确保每个步骤执行成功后再执行下一步骤,共%d个循环",_nLoop);
				else
					FormatString(s,"确保每个步骤执行成功后再执行下一步骤,永久执行");
				break;
			}
		}
	}

	Mode _mode;
	DWORD _nSteps;
	DWORD _nLoop;

};

class CBgn_Repeater_Obsolete:public CBehaviorGraphNode
{
public:
	DEFINE_CLASS(CBgn_Repeater_Obsolete);

	CBgn_Repeater_Obsolete()
	{
		_iCurStep=0;
		_iCurLoop=0;
		_bAnyOk=FALSE;
		_bNeedFire=FALSE;
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);
	virtual void RewindOk(WORD keyRewind,BGNOutputs &outputs);//因为执行成功导致的Rewind
	virtual void RewindFail(WORD keyRewind,BGNOutputs &outputs);//因为执行失败导致的Rewind
	virtual void Break(BGNOutputs &outputs);

protected:
	void _Fire(BGNOutputs &outputs);
	int _FindFirstLink(int iStart);
	int _iCurStep;
	int _iCurLoop;
	BOOL _bNeedFire;
	BOOL _bAnyOk;
};


#define Sqeuence_OutputStart (1)
class CBgp_Sequence:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_Sequence);

	virtual const char *GetTypeName()	{		return "顺序执行";	};
	virtual DWORD GetStubCount()	
	{		
		return _nSteps>20?(Sqeuence_OutputStart+20):(Sqeuence_OutputStart+_nSteps);
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB()
			STUB_IN(0,"开始");
			STUB_OUT(1,"步骤01");
			STUB_OUT(2,"步骤02");
			STUB_OUT(3,"步骤03");
			STUB_OUT(4,"步骤04");
			STUB_OUT(5,"步骤05");
			STUB_OUT(6,"步骤06");
			STUB_OUT(7,"步骤07");
			STUB_OUT(8,"步骤08");
			STUB_OUT(9,"步骤09");
			STUB_OUT(10,"步骤10");
			STUB_OUT(11,"步骤11");
			STUB_OUT(12,"步骤12");
			STUB_OUT(13,"步骤13");
			STUB_OUT(14,"步骤14");
			STUB_OUT(15,"步骤15");
			STUB_OUT(16,"步骤16");
			STUB_OUT(17,"步骤17");
			STUB_OUT(18,"步骤18");
			STUB_OUT(19,"步骤19");
			STUB_OUT(20,"步骤20");
		END_STUB()
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Controller;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

    BEGIN_GOBJ_PURE_UID(CBgp_Sequence,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(int,_nSteps,1);
			GELEM_EDITVAR("步骤个数",GVT_S,GSem(GSem_Interger,"1:1,2:2,3:3,4:4,5:5,6:6,7:7,8:8,9:9,10:10,11:11,12:12,13:13,14:14,15:15,16:16,17:17,18:18,19:19,20:20"),"有几个步骤");
	END_GOBJ();    

public: //当作protected

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="顺序执行,每个步骤成功后执行下个步骤";
	}

	DWORD _nSteps;
};

class CBgn_Sequence:public CBehaviorGraphNode
{
public:
	DEFINE_CLASS(CBgn_Sequence);

	CBgn_Sequence()
	{
		_iCurStep=0;
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void RewindOk(WORD keyRewind,BGNOutputs &outputs);//因为执行成功导致的Rewind
	virtual void RewindFail(WORD keyRewind,BGNOutputs &outputs);//因为执行失败导致的Rewind
	virtual void Break(BGNOutputs &outputs);

protected:
	void _Fire(BGNOutputs &outputs);
	int _FindFirstLink(int iStart);
	int _iCurStep;
};

#define Selector_OutputStart (1)
class CBgp_Selector:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_Selector);

	virtual const char *GetTypeName()	{		return "选择执行";	};
	virtual DWORD GetStubCount()	
	{		
		return _nSteps>20?(Selector_OutputStart+20):(Selector_OutputStart+_nSteps);
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB()
			STUB_IN(0,"开始");
			STUB_OUT(1,"步骤01");
			STUB_OUT(2,"步骤02");
			STUB_OUT(3,"步骤03");
			STUB_OUT(4,"步骤04");
			STUB_OUT(5,"步骤05");
			STUB_OUT(6,"步骤06");
			STUB_OUT(7,"步骤07");
			STUB_OUT(8,"步骤08");
			STUB_OUT(9,"步骤09");
			STUB_OUT(10,"步骤10");
			STUB_OUT(11,"步骤11");
			STUB_OUT(12,"步骤12");
			STUB_OUT(13,"步骤13");
			STUB_OUT(14,"步骤14");
			STUB_OUT(15,"步骤15");
			STUB_OUT(16,"步骤16");
			STUB_OUT(17,"步骤17");
			STUB_OUT(18,"步骤18");
			STUB_OUT(19,"步骤19");
			STUB_OUT(20,"步骤20");
		END_STUB()
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Controller;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

    BEGIN_GOBJ_PURE_UID(CBgp_Selector,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(int,_nSteps,1);
			GELEM_EDITVAR("步骤个数",GVT_S,GSem(GSem_Interger,"1:1,2:2,3:3,4:4,5:5,6:6,7:7,8:8,9:9,10:10,11:11,12:12,13:13,14:14,15:15,16:16,17:17,18:18,19:19,20:20"),"有几个步骤");
	END_GOBJ();    

public: //当作protected

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="选择执行,每个步骤失败后执行下个步骤";
	}

	DWORD _nSteps;
};

class CBgn_Selector:public CBehaviorGraphNode
{
public:
	DEFINE_CLASS(CBgn_Selector);

	CBgn_Selector()
	{
		_iCurStep=0;
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void RewindOk(WORD keyRewind,BGNOutputs &outputs);//因为执行成功导致的Rewind
	virtual void RewindFail(WORD keyRewind,BGNOutputs &outputs);//因为执行失败导致的Rewind
	virtual void Break(BGNOutputs &outputs);

protected:
	void _Fire(BGNOutputs &outputs);
	int _FindFirstLink(int iStart);
	int _iCurStep;
};


class CBgp_Repeater:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_Repeater);

	enum Mode
	{
		Forever,
		UntilSuccess,
		UntilFailure,
	};

	virtual const char *GetTypeName()	{		return "重复执行";	};
	virtual DWORD GetStubCount()	
	{		
		return 2;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB()
			STUB_IN(0,"开始");
			STUB_OUT(1,"步骤");
		END_STUB()
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Controller;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

	BEGIN_GOBJ_PURE_UID(CBgp_Repeater,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(int,_mode,0);
			GELEM_EDITVAR("运行模式",GVT_S,GSem(GSem_Interger,"永远重复,直到成功,直到失败"),"重复模式");

	END_GOBJ();    

public: //当作protected

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		static std::string ss;

		switch(_mode)
		{
		case Forever:
				s="永久重复";
				break;
		case UntilSuccess:
				s="直到成功";
				break;
		case UntilFailure:
				s="直到失败";
				break;
		}
	}
	Mode _mode;
};

class CBgn_Repeater:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_Repeater);

	CBgn_Repeater()
	{
		_bNeedFire=FALSE;
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);
	virtual void RewindOk(WORD keyRewind,BGNOutputs &outputs);//因为执行成功导致的Rewind
	virtual void RewindFail(WORD keyRewind,BGNOutputs &outputs);//因为执行失败导致的Rewind
	virtual void Break(BGNOutputs &outputs);

protected:
	void _Fire(BGNOutputs &outputs);
	BOOL _bNeedFire;
};

class CBgp_Ensure:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_Ensure);

	virtual const char *GetTypeName()	{		return "确保成功";	};
	virtual DWORD GetStubCount()	
	{		
		return 2;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB()
			STUB_IN(0,"开始");
			STUB_OUT(1,"步骤");
		END_STUB()
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Controller;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

	BEGIN_GOBJ_PURE_UID(CBgp_Ensure,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(int,_nMaxRepeat,0);
			GELEM_EDITVAR("最大重复次数",GVT_S,GSem_Interger,"0表示无限次");
		GELEM_VAR_INIT(int,_nMaxRepeatVary,0);
			GELEM_EDITVAR("最大重复次数浮动",GVT_S,GSem_Interger,"最大重复次数浮动");
	END_GOBJ();    

public: //当作protected

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		static std::string ss;

		s="重复执行直到成功";
		if (_nMaxRepeat>0)
		{
			if (_nMaxRepeatVary<=0)
				AppendFmtString(s,",最多重复%d次",_nMaxRepeat);
			else
				AppendFmtString(s,",最多重复%d(+/-%d)次",_nMaxRepeat,_nMaxRepeatVary);
		}
	}
	int _nMaxRepeat;
	int _nMaxRepeatVary;
};

class CBgn_Ensure:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_Ensure);

	CBgn_Ensure()
	{
		_bNeedFire=FALSE;
		_nToRepeat=0;
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);
	virtual void RewindOk(WORD keyRewind,BGNOutputs &outputs);//因为执行成功导致的Rewind
	virtual void RewindFail(WORD keyRewind,BGNOutputs &outputs);//因为执行失败导致的Rewind
	virtual void Break(BGNOutputs &outputs);

protected:
	void _Fire(BGNOutputs &outputs);
	BOOL _bNeedFire;
	DWORD _nToRepeat;
};


class CBgp_Loop:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_Loop);

	virtual const char *GetTypeName()	{		return "循环执行";	};
	virtual DWORD GetStubCount()	
	{		
		return 2;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB()
			STUB_IN(0,"开始");
			STUB_OUT(1,"步骤");
		END_STUB()
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Controller;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

	BEGIN_GOBJ_PURE_UID(CBgp_Loop,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(int,_nLoop,1);
			GELEM_EDITVAR("循环次数",GVT_S,GSem_Interger,"循环次数");
			GELEM_BVR();
	END_GOBJ();    

public: //当作protected

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		static std::string ss;

		FormatString(s,"循环次数:%s",GetBVRDesc_Int(BVR_ARG(_nLoop),assist));
	}

	DEFINE_BVR(int,_nLoop);
};

class CBgn_Loop:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_Loop);

	CBgn_Loop()
	{
		_nLoop=0;
		_bNeedFire=FALSE;
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);
	virtual void RewindOk(WORD keyRewind,BGNOutputs &outputs);//因为执行成功导致的Rewind
	virtual void RewindFail(WORD keyRewind,BGNOutputs &outputs);//因为执行失败导致的Rewind
	virtual void Break(BGNOutputs &outputs);

protected:
	void _Fire(BGNOutputs &outputs);
	DWORD _nLoop;
	BOOL _bNeedFire;
};




#define Parallel_OutputStart (1)
class CBgp_Parallel:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_Parallel);

	enum Mode
	{
		AllReturn,
		FirstReturn,

		ForceDword=0xffffffff,
	};

	virtual const char *GetTypeName()	{		return "同时执行";	};
	virtual DWORD GetStubCount()	
	{		
		return _nSteps>20?(Parallel_OutputStart+20):(Parallel_OutputStart+_nSteps);
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB()
			STUB_IN(0,"开始");
			STUB_OUT(1,"步骤01");
			STUB_OUT(2,"步骤02");
			STUB_OUT(3,"步骤03");
			STUB_OUT(4,"步骤04");
			STUB_OUT(5,"步骤05");
			STUB_OUT(6,"步骤06");
			STUB_OUT(7,"步骤07");
			STUB_OUT(8,"步骤08");
			STUB_OUT(9,"步骤09");
			STUB_OUT(10,"步骤10");
			STUB_OUT(11,"步骤11");
			STUB_OUT(12,"步骤12");
			STUB_OUT(13,"步骤13");
			STUB_OUT(14,"步骤14");
			STUB_OUT(15,"步骤15");
			STUB_OUT(16,"步骤16");
			STUB_OUT(17,"步骤17");
			STUB_OUT(18,"步骤18");
			STUB_OUT(19,"步骤19");
			STUB_OUT(20,"步骤20");
		END_STUB()
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Controller;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

    BEGIN_GOBJ_PURE_UID(CBgp_Parallel,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(Mode,_mode,AllReturn);
			GELEM_EDITVAR("模式",GVT_S,GSem(GSem_Interger,"全部返回,首先返回"),"执行模式");

		GELEM_VAR_INIT(int,_nSteps,1);
			GELEM_EDITVAR("步骤个数",GVT_S,GSem(GSem_Interger,"1:1,2:2,3:3,4:4,5:5,6:6,7:7,8:8,9:9,10:10,11:11,12:12,13:13,14:14,15:15,16:16,17:17,18:18,19:19,20:20"),"有几个步骤");
	END_GOBJ();    

public: //当作protected

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		if (_mode==AllReturn)
			s="全部返回";
		if (_mode==FirstReturn)
			s="首先返回";
	}

	Mode _mode;

	DWORD _nSteps;

};

class CBgn_Parallel:public CBehaviorGraphNode
{
public:
	DEFINE_CLASS(CBgn_Parallel);

	CBgn_Parallel()
	{
	}

	~CBgn_Parallel()
	{

	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void RewindOk(WORD keyRewind,BGNOutputs &outputs);//因为执行成功导致的Rewind
	virtual void RewindFail(WORD keyRewind,BGNOutputs &outputs);//因为执行失败导致的Rewind
	virtual void Break(BGNOutputs &outputs);

protected:

	void _Break(BGNOutputs &outputs);
	Bitset<1> _flags;
};


#define Random_OutputStart (1)
class CBgp_Random:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_Random);

	virtual const char *GetTypeName()	{		return "随机执行";	};
	virtual DWORD GetStubCount()	
	{		
		return _nSteps>20?(Random_OutputStart+20):(Random_OutputStart+_nSteps);
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB()
			STUB_IN(0,"开始");
			STUB_OUT(1,"步骤01");
			STUB_OUT(2,"步骤02");
			STUB_OUT(3,"步骤03");
			STUB_OUT(4,"步骤04");
			STUB_OUT(5,"步骤05");
			STUB_OUT(6,"步骤06");
			STUB_OUT(7,"步骤07");
			STUB_OUT(8,"步骤08");
			STUB_OUT(9,"步骤09");
			STUB_OUT(10,"步骤10");
			STUB_OUT(11,"步骤11");
			STUB_OUT(12,"步骤12");
			STUB_OUT(13,"步骤13");
			STUB_OUT(14,"步骤14");
			STUB_OUT(15,"步骤15");
			STUB_OUT(16,"步骤16");
			STUB_OUT(17,"步骤17");
			STUB_OUT(18,"步骤18");
			STUB_OUT(19,"步骤19");
			STUB_OUT(20,"步骤20");
		END_STUB()
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Controller;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

    BEGIN_GOBJ_PURE_UID(CBgp_Random,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(int,_nSteps,1);
			GELEM_EDITVAR("步骤个数",GVT_S,GSem(GSem_Interger,"1:1,2:2,3:3,4:4,5:5,6:6,7:7,8:8,9:9,10:10,11:11,12:12,13:13,14:14,15:15,16:16,17:17,18:18,19:19,20:20"),"有几个步骤");
	END_GOBJ();    

public: //当作protected

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="随机选择运行";
	}

	DWORD _nSteps;

};

class CBgn_Random:public CBehaviorGraphNode
{
public:
	DEFINE_CLASS(CBgn_Random);

	CBgn_Random()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

class CBgp_Attempt_Obsolete:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_Attempt_Obsolete);

	virtual const char *GetTypeName()	{		return "尝试执行";	};
	virtual DWORD GetStubCount()	
	{		
		return 4;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB()
			STUB_IN(0,"开始");
			STUB_OUT(1,"尝试");
			STUB_OUT(2,"成功");
			STUB_OUT(3,"失败");
		END_STUB()
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Controller;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

    BEGIN_GOBJ_PURE_UID(CBgp_Attempt_Obsolete,1);
		GELEM_BGP_BASE();

	END_GOBJ();    

public: //当作protected

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="";
	}
};

class CBgn_Attempt_Obsolete:public CBehaviorGraphNode
{
public:
	DEFINE_CLASS(CBgn_Attempt_Obsolete);

	CBgn_Attempt_Obsolete()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void RewindOk(WORD keyRewind,BGNOutputs &outputs);//因为执行成功导致的Rewind
	virtual void RewindFail(WORD keyRewind,BGNOutputs &outputs);//因为执行失败导致的Rewind
	virtual void Break(BGNOutputs &outputs);
protected:

};


class CBgp_Xto1:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_Xto1);

	virtual const char *GetTypeName()	{		return "多转1";	};
	virtual DWORD GetStubCount()	
	{		
		return 1+_nIn;;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB()
			STUB_OUT(0,"出口");
			STUB_IN(1,"入口01");
			STUB_IN(2,"入口02");
			STUB_IN(3,"入口03");
			STUB_IN(4,"入口04");
			STUB_IN(5,"入口05");
			STUB_IN(6,"入口06");
			STUB_IN(7,"入口07");
			STUB_IN(8,"入口08");
			STUB_IN(9,"入口09");
			STUB_IN(10,"入口10");
			STUB_IN(11,"入口11");
			STUB_IN(12,"入口12");
			STUB_IN(13,"入口13");
			STUB_IN(14,"入口14");
			STUB_IN(15,"入口15");
			STUB_IN(16,"入口16");
			STUB_IN(17,"入口17");
			STUB_IN(18,"入口18");
			STUB_IN(19,"入口19");
			STUB_IN(20,"入口20");
		END_STUB()
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Controller;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

    BEGIN_GOBJ_PURE_UID(CBgp_Xto1,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(int,_nIn,2);
			GELEM_EDITVAR("入口个数",GVT_S,GSem(GSem_Interger,"1:1,2:2,3:3,4:4,5:5,6:6,7:7,8:8,9:9,10:10,11:11,12:12,13:13,14:14,15:15,16:16,17:17,18:18,19:19,20:20"),"有几个入口");

	END_GOBJ();    

public: //当作protected

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="";
	}
	DWORD _nIn;
};

class CBgn_Xto1:public CBehaviorGraphNode
{
public:
	DEFINE_CLASS(CBgn_Xto1);

	CBgn_Xto1()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
protected:

};

class CBgp_Fail:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_Fail);

	virtual const char *GetTypeName()	{		return "失败";	};
	virtual DWORD GetStubCount()	
	{		
		return 1;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB()
			STUB_IN(0,"失败");
		END_STUB()
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Controller;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

    BEGIN_GOBJ_PURE_UID(CBgp_Fail,1);
		GELEM_BGP_BASE();

	END_GOBJ();    

public: //当作protected
	virtual void FillDesc(std::string &s,FillDescAssist *assist)	{		s="";	}
};

class CBgn_Fail:public CBehaviorGraphNode
{
public:
	DEFINE_CLASS(CBgn_Fail);

	CBgn_Fail()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

class CBgp_Succeed:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_Succeed);

	virtual const char *GetTypeName()	{		return "成功";	};
	virtual DWORD GetStubCount()	
	{		
		return 1;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB()
			STUB_IN(0,"成功");
		END_STUB()
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Controller;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

    BEGIN_GOBJ_PURE_UID(CBgp_Succeed,1);
		GELEM_BGP_BASE();

	END_GOBJ();    

public: //当作protected
	virtual void FillDesc(std::string &s,FillDescAssist *assist)	{		s="";	}
};

class CBgn_Succeed:public CBehaviorGraphNode
{
public:
	DEFINE_CLASS(CBgn_Succeed);

	CBgn_Succeed()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};




class CBgp_Rate:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_Rate);

	virtual const char *GetTypeName()	{		return "几率执行";	};
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Controller;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

    BEGIN_GOBJ_PURE_UID(CBgp_Rate,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(float,rate,0.5f);
			GELEM_EDITVAR("成功几率",GVT_F,GSem(GSem_Float,"0,1,0.01"),"成功几率");
			GELEM_BVR()
	END_GOBJ();    

public: //当作protected

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		if (_BVR(rate)==StringID_BhvValInvalidRef)
			FormatString(s,"成功几率:%.2f%%",rate*100.0f);
		else
			FormatString(s,"成功几率:%s",assist->GetStr(_BVR(rate)));
	}

	DEFINE_BVR(float,rate);

};

class CBgn_Rate:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_Rate);

	CBgn_Rate()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};


class CBgp_Delay_Obsolete:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_Delay_Obsolete);

	virtual const char *GetTypeName()	{		return "延迟(Obsolete)";	}
	virtual DWORD GetStubCount()
	{
		return 2;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"时间到");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Controller;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		FormatString(s,"延迟%.2f秒",ANIMTICK_TO_SECOND(_dur));
	}

    BEGIN_GOBJ_PURE_UID(CBgp_Delay_Obsolete,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(AnimTick,_dur,ANIMTICK_FROM_SECOND(1.0f));
			GELEM_EDITVAR("延迟时间",GVT_U,GSem(GSem_AnimTick,"0,600000,0.1"),"延迟时间,单位为秒");
			GELEM_BVR();

    END_GOBJ();    

public: //当作protected

	DEFINE_BVR(AnimTick,_dur);

};


class CBgn_Delay_Obsolete:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_Delay_Obsolete);

	CBgn_Delay_Obsolete()
	{
		_tStart=0;
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);

protected:
	AnimTick _tStart;

};


class CBgp_Delay:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_Delay);

	virtual const char *GetTypeName()	{		return "延迟";	}
	virtual DWORD GetStubCount()
	{
		return 2;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"时间到");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Controller;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		FormatString(s,"延迟%s秒",GetBVRDesc_Float(BVR_ARG(_dur),assist));
		if (_varyDur>0.0f)
			AppendFmtString(s,",(+/-)%.2f秒",_varyDur);
	}

    BEGIN_GOBJ_PURE_UID(CBgp_Delay,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(float,_dur,1.0f);
			GELEM_EDITVAR("延迟时间",GVT_F,GSem(GSem_Float,"0,600000,0.1"),"延迟时间,单位为秒");
			GELEM_BVR();

		GELEM_VAR_INIT(float,_varyDur,0.0f);
			GELEM_EDITVAR("延迟时间浮动",GVT_F,GSem(GSem_Float,"0.0,10.0,0.05"),"延迟时间浮动");

    END_GOBJ();    

public: //当作protected

	DEFINE_BVR(float,_dur);
	float _varyDur;

};


class CBgn_Delay:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_Delay);

	CBgn_Delay()
	{
		_tEnd=0;
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);

protected:
	AnimTick _tEnd;

};



class CBgp_Interrupt:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_Interrupt);

	virtual const char *GetTypeName()	{		return "中断";	};
	virtual DWORD GetStubCount()	
	{		
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB()
			STUB_IN(0,"开始");
			STUB_OUT(1,"执行");
			STUB_OUT(2,"中断/结束");
		END_STUB()
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Controller;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

	BEGIN_GOBJ_PURE_UID(CBgp_Interrupt,1);
		GELEM_BGP_BASE();

	END_GOBJ();    

public: //当作protected

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="";
	}
};

class CBgn_Interrupt:public CBehaviorGraphNode
{
public:
	DEFINE_CLASS(CBgn_Interrupt);

	CBgn_Interrupt()
	{
		_bFinalized=FALSE;
		_bOk=FALSE;
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void RewindOk(WORD keyRewind,BGNOutputs &outputs);//因为执行成功导致的Rewind
	virtual void RewindFail(WORD keyRewind,BGNOutputs &outputs);//因为执行失败导致的Rewind
	virtual void Break(BGNOutputs &outputs);
protected:

	BOOL _Finalize(BGNOutputs &outputs);//返回有没有启动一个thread

	BOOL _bOk;
	BOOL _bFinalized;

};


#define Strategy_OutputStart (1)
class CBgp_Strategy:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_Strategy);

	virtual const char *GetTypeName()	{		return "抢占分支";	};
	virtual DWORD GetStubCount()	
	{		
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB()
			STUB_IN(0,"开始");
			STUB_OUT(1,"条件");
			STUB_OUT(2,"执行");
		END_STUB()
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Controller;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

    BEGIN_GOBJ_PURE_UID(CBgp_Strategy,1);
		GELEM_BGP_BASE();

	END_GOBJ();    

public: //当作protected

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
	}

};

class CBgn_Decision;
class CBgn_Strategy:public CBehaviorGraphNode
{
public:
	DEFINE_CLASS(CBgn_Strategy);

	CBgn_Strategy()
	{
	}

	~CBgn_Strategy()
	{

	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);
	virtual void RewindOk(WORD keyRewind,BGNOutputs &outputs);//因为执行成功导致的Rewind
	virtual void RewindFail(WORD keyRewind,BGNOutputs &outputs);//因为执行失败导致的Rewind

protected:

	CBgn_Decision *_GetParent();

};


#define Decision_OutputStart (1)
class CBgp_Decision:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_Decision);

	enum Mode
	{
		AllReturn,
		FirstReturn,

		ForceDword=0xffffffff,
	};

	virtual const char *GetTypeName()	{		return "抢占执行";	};
	virtual DWORD GetStubCount()	
	{		
		return _nBranches>20?(Decision_OutputStart+20):(Decision_OutputStart+_nBranches);
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB()
			STUB_IN(0,"开始");
			STUB_OUT(1,"分支01");
			STUB_OUT(2,"分支02");
			STUB_OUT(3,"分支03");
			STUB_OUT(4,"分支04");
			STUB_OUT(5,"分支05");
			STUB_OUT(6,"分支06");
			STUB_OUT(7,"分支07");
			STUB_OUT(8,"分支08");
			STUB_OUT(9,"分支09");
			STUB_OUT(10,"分支10");
			STUB_OUT(11,"分支11");
			STUB_OUT(12,"分支12");
			STUB_OUT(13,"分支13");
			STUB_OUT(14,"分支14");
			STUB_OUT(15,"分支15");
			STUB_OUT(16,"分支16");
			STUB_OUT(17,"分支17");
			STUB_OUT(18,"分支18");
			STUB_OUT(19,"分支19");
			STUB_OUT(20,"分支20");
		END_STUB()
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Controller;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

    BEGIN_GOBJ_PURE_UID(CBgp_Decision,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(int,_nBranches,1);
			GELEM_EDITVAR("分支个数",GVT_S,GSem(GSem_Interger,"1:1,2:2,3:3,4:4,5:5,6:6,7:7,8:8,9:9,10:10,11:11,12:12,13:13,14:14,15:15,16:16,17:17,18:18,19:19,20:20"),"有几个步骤");
	END_GOBJ();    

public: //当作protected

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
	}

	DWORD _nBranches;

};

class CBgn_Decision:public CBehaviorGraphNode
{
public:
	DEFINE_CLASS(CBgn_Decision);

	CBgn_Decision()
	{
		_iCondition=-1;
		_iExecute=-1;
		_iConditionToStart=-1;
	}

	~CBgn_Decision()
	{

	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);
	virtual void Break(BGNOutputs &outputs);

	void RegisterStrategy(DWORD idx,BgnThread &thrdCond,BgnThread &thrdExec);
protected:

	struct StrategyThreads
	{
		BgnThread thrdCond;
		BgnThread thrdExec;
	};

	void _Break(BGNOutputs &outputs);

	std::vector<StrategyThreads> _thrdsStrategy;

	void _FireCondition(BGNOutputs &outputs);
	void _FireExecute(BGNOutputs &outputs);
	int _iCondition;
	int _iExecute;

	int _iConditionToStart;

	friend class CBgn_Strategy;
};



class CBgp_KeepCheck:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_KeepCheck);

	enum Mode
	{
		CheckSuccess,
		CheckFailure,
	};

	virtual const char *GetTypeName()	{		return "持续检测";	};
	virtual DWORD GetStubCount()	
	{		
		return 2;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB()
			STUB_IN(0,"开始");
			STUB_OUT(1,"步骤");
		END_STUB()
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Controller;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

	BEGIN_GOBJ_PURE_UID(CBgp_KeepCheck,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(int,_mode,0);
			GELEM_EDITVAR("运行模式",GVT_S,GSem(GSem_Interger,"检测成功,检测失败"),"持续检测模式");
		GELEM_VAR_INIT(AnimTick,_dur,ANIMTICK_FROM_SECOND(1.0f));
			GELEM_EDITVAR("持续时间",GVT_U,GSem(GSem_AnimTick,"0.1,100,0.1"),"持续时间");
	END_GOBJ();    

public: //当作protected

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		switch(_mode)
		{
		case CheckSuccess:
			FormatString(s,"过去%.2f秒皆为成功",ANIMTICK_TO_SECOND(_dur));
			break;
		case CheckFailure:
			FormatString(s,"过去%.2f秒皆为失败",ANIMTICK_TO_SECOND(_dur));
			break;
		}
	}
	Mode _mode;
	AnimTick _dur;
};

class CBgn_KeepCheck:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_KeepCheck);

	CBgn_KeepCheck()
	{
		_tStart=ANIMTICK_INFINITE;
		_bNeedFire=FALSE;
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);
	virtual void RewindOk(WORD keyRewind,BGNOutputs &outputs);//因为执行成功导致的Rewind
	virtual void RewindFail(WORD keyRewind,BGNOutputs &outputs);//因为执行失败导致的Rewind
	virtual void Break(BGNOutputs &outputs);

protected:
	void _Fire(BGNOutputs &outputs);
	BOOL _bNeedFire;
	AnimTick _tStart;
};


class CBgp_Always:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_Always);

	enum Mode
	{
		CheckSuccess,
		CheckFailure,
	};

	virtual const char *GetTypeName()	{		return "一直";	};
	virtual DWORD GetStubCount()	
	{		
		return 2;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB()
			STUB_IN(0,"开始");
			STUB_OUT(1,"步骤");
		END_STUB()
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Controller;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

	BEGIN_GOBJ_PURE_UID(CBgp_Always,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(int,_mode,0);
			GELEM_EDITVAR("运行模式",GVT_S,GSem(GSem_Interger,"检测成功,检测失败"),"持续检测模式");
		GELEM_VAR_INIT(AnimTick,_dur,ANIMTICK_FROM_SECOND(1.0f));
			GELEM_EDITVAR("持续时间",GVT_U,GSem(GSem_AnimTick,"0.1,100,0.1"),"持续时间");
	END_GOBJ();    

public: //当作protected

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		switch(_mode)
		{
		case CheckSuccess:
			FormatString(s,"过去%.2f秒一直成功",ANIMTICK_TO_SECOND(_dur));
			break;
		case CheckFailure:
			FormatString(s,"过去%.2f秒一直失败",ANIMTICK_TO_SECOND(_dur));
			break;
		}
	}
	Mode _mode;
	AnimTick _dur;
};

class CBgn_Always:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_Always);

	CBgn_Always()
	{
		_tStart=0;
		_bThreading=FALSE;
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void RewindOk(WORD keyRewind,BGNOutputs &outputs);//因为执行成功导致的Rewind
	virtual void RewindFail(WORD keyRewind,BGNOutputs &outputs);//因为执行失败导致的Rewind
	virtual void Break(BGNOutputs &outputs);

protected:
	void _SetStartTime(AnimTick t);
	AnimTick _tStart;
	BOOL _bThreading;
};


class CBgp_WeaksMod:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_WeaksMod);

	enum Mode
	{
		Mode_None,
		Mode_Set,
		Mode_Modify,
		Mode_Filter,

		Mode_ForceDword=0xffffffff,
	};

	virtual const char *GetTypeName()	{		return "弱点修改";	};
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Controller;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

	BEGIN_GOBJ_PURE_UID(CBgp_WeaksMod,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(BOOL,bBackup,TRUE);
			GELEM_EDITVAR("备份当前值",GVT_S,GSem_Boolean,"备份当前弱点后再修改");
		GELEM_VAR_INIT(Mode,mode,Mode_None);
			GELEM_EDITVAR("模式",GVT_S,GSem(GSem_Interger,
					"n/a:0"		"|弱点&增加弱点&减少弱点&弱点Mask,"
					"设置弱点:1"		"|增加弱点&减少弱点&弱点Mask,"
					"修改弱点:2"		"|弱点&弱点Mask,"
					"过滤弱点:3"		"|弱点&减少弱点"
					),"奖励类型");
		GELEM_OBJ(WeaksEx,weaksOverride);
			GELEM_EDITOBJ("弱点","弱点");
		GELEM_OBJ(WeaksEx,weaksFilter);
			GELEM_EDITOBJ("弱点Mask","弱点Mask");
		GELEM_OBJ(WeaksEx,weaksAdd);
			GELEM_EDITOBJ("增加弱点","增加弱点");
		GELEM_OBJ(WeaksEx,weaksRemove);
			GELEM_EDITOBJ("减少弱点","减少弱点");

	END_GOBJ();    

public: //当作protected

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="不修改";
		switch(mode)
		{
			case Mode_Set:
				s="设置模式";
				break;
			case Mode_Modify:
				s="修改模式";
				break;
			case Mode_Filter:
				s="过滤模式";
				break;
		}
		if (mode==Mode_None)
		{
			if (bBackup)	
				s="备份当前值但不修改";
			else
				s="不做任何事";
		}
		else
		{
			if (bBackup)
				s+=",备份当前值并修改";
			else
				s+=",直接在当前值上修改";
		}
	}

	Mode mode;
	BOOL bBackup;

	WeaksEx weaksOverride;
	WeaksEx weaksAdd;
	WeaksEx weaksRemove;
	WeaksEx weaksFilter;
};

class CBgn_WeaksMod:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_WeaksMod);

	CBgn_WeaksMod()
	{
		_bOverriden=FALSE;
		_bFinalized=FALSE;
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void RewindOk(WORD keyRewind,BGNOutputs &outputs);//因为执行成功导致的Rewind
	virtual void RewindFail(WORD keyRewind,BGNOutputs &outputs);//因为执行失败导致的Rewind
	virtual void Break(BGNOutputs &outputs);
protected:

	BOOL _Finalize(BGNOutputs &outputs);//返回有没有启动一个thread

	BOOL _bOverriden;
	BOOL _bFinalized;

};

