#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"


class CBgp_TalkAddChoice:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_TalkAddChoice);

	virtual const char *GetTypeName()	{		return "添加选择";	};
	virtual DWORD GetStubCount()
	{
		return 2;	
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB()
			STUB_IN(0,"开始");
			STUB_OUT(1,"结束");
		END_STUB()
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Talk;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

    BEGIN_GOBJ_PURE_UID(CBgp_TalkAddChoice,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT( StringID,_choice,StringID_Invalid);	
			GELEM_EDITVAR( "选择项", GVT_U, GSem(GSem_StringID,"谈话文字"), "选择项" );

    END_GOBJ();    

public: //当作protected

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if (_choice!=StringID_Invalid)
			FormatString(s,"%s",StrLib_GetStr(_choice));
	}

	StringID _choice;
};

class CBgn_TalkAddChoice:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_TalkAddChoice);

	CBgn_TalkAddChoice()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:
};

class CBgp_TalkSentence:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_TalkSentence);

	virtual const char *GetTypeName()	{		return "设置句子";	};
	virtual DWORD GetStubCount()
	{
		return 2;	
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB()
			STUB_IN(0,"开始");
		STUB_OUT(1,"结束");
		END_STUB()
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Talk;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	BEGIN_GOBJ_PURE_UID(CBgp_TalkSentence,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT( StringID,_snt,StringID_Invalid);	
			GELEM_EDITVAR( "句子", GVT_U, GSem(GSem_StringID,"谈话文字"), "句子" );

	END_GOBJ();    

public: //当作protected

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if (_snt!=StringID_Invalid)
			FormatString(s,"%s",StrLib_GetStr(_snt));
	}

	StringID _snt;
};

class CBgn_TalkSentence:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_TalkSentence);

	CBgn_TalkSentence()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:
};


class CBgp_TalkSpeak:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_TalkSpeak);

	virtual const char *GetTypeName()	{		return "说话";	};
	virtual DWORD GetStubCount()	{		return 2;	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB()
			STUB_IN(0,"开始");
			STUB_OUT(1,"结束");
		END_STUB()
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Talk;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

    BEGIN_GOBJ_PURE_UID(CBgp_TalkSpeak,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT( StringID,_content,StringID_Invalid);	
			GELEM_EDITVAR( "说什么", GVT_U, GSem(GSem_StringID,"谈话内容"), "说什么内容" );

		GELEM_VAR_INIT(BOOL,_bPlayerSpeak,0);
			GELEM_EDITVAR("主角说话 ",GVT_S,GSem_Boolean,"是否是主角说的话");

    END_GOBJ();    

public: //当作protected

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		if (_content==StringID_Invalid)
			s="<没有内容>";
		else
		{
			s=StrLib_GetStr(_content);
			if (_bPlayerSpeak)
				s=std::string("主角说:\n")+s;
		}
	}

	StringID _content;
	BOOL _bPlayerSpeak;
};

class CBgn_TalkSpeak:public CLevelBgn
{
public:
	CBgn_TalkSpeak()
	{
		_idPlayer=LevelPlayerID_Invalid;
	}

	DEFINE_CLASS(CBgn_TalkSpeak);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);
	virtual void Break(BGNOutputs &outputs);
	virtual void Destroy();

	LevelPlayerID _idPlayer;
};

class CBgp_TalkDialog:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_TalkDialog);

	virtual const char *GetTypeName()	{		return "弹出对话框";	};
	virtual DWORD GetStubCount()	
	{	
		DWORD c=_cmds.size();
		if (c>9)
			c=9;
		return 1+c;	
	}
	virtual PadStub GetStub(DWORD idx)
	{
		if (idx==0)
			return PadStub("开始",PadStub_In,TRUE);

		idx-=1;

		static std::string str;

		if (idx<_cmds.size())
		{
			StringID s=_cmds[idx];
			if (s==StringID_Invalid)
				return PadStub("n/a",PadStub_Out,TRUE);
			FormatString(str,"!!%d",s);
			return PadStub(str.c_str(),PadStub_Out,TRUE);
		}

		return PadStub();


	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Talk;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

    BEGIN_GOBJ_PURE_UID(CBgp_TalkDialog,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT( StringID,_dlg,StringID_Invalid);	
			GELEM_EDITVAR( "对话框名称", GVT_U, GSem(GSem_StringID,"对话框名称"), "对话框名称" );
			GELEM_BVR();
		GELEM_VARVECTOR_INIT(StringID,_cmds,StringID_Invalid);
			GELEM_EDITVAR("处理命令",GVT_U,GSem(GSem_StringID,"行为图对话框命令"),"处理的各种对话框命令");

    END_GOBJ();    

public: //当作protected

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if (_dlg!=StringID_Invalid)
		{
			FormatString(s,"弹出对话框:%s",BVRDESC_StringID(_dlg));

		}
	}

	DEFINE_BVR(StringID,_dlg);
	std::vector<StringID> _cmds;

};

class CBgn_TalkDialog:public CLevelBgn
{
public:
	CBgn_TalkDialog()
	{
		_idPlayer=LevelPlayerID_Invalid;
		_bHandling=FALSE;
	}

	DEFINE_CLASS(CBgn_TalkDialog);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);
	virtual void Break(BGNOutputs &outputs);
	virtual void Destroy();

	virtual void RewindOk(WORD keyRewind,BGNOutputs &outputs);//因为执行成功导致的Rewind
	virtual void RewindFail(WORD keyRewind,BGNOutputs &outputs);//因为执行失败导致的Rewind

	LevelPlayerID _idPlayer;

	BOOL _bHandling;
};

class CBgp_GetTalkDialogParam:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_GetTalkDialogParam);

	virtual const char *GetTypeName()	{		return "对话框提交参数";	};
	virtual DWORD GetStubCount()	
	{	
		return 2;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB()
			STUB_IN(0,"开始");
			STUB_OUT(1,"结束");
		END_STUB()
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Talk;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

    BEGIN_GOBJ_PURE_UID(CBgp_GetTalkDialogParam,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(int,_idx,0);
			GELEM_EDITVAR("哪个参数",GVT_S,GSem(GSem_Interger,"提交参数#0,提交参数#1"),"哪个参数");
		GELEM_BEHAVIORMEM_NUMBER(_var,"保存变量","对话框提交的参数保存在哪个变量里")
    END_GOBJ();    

public: //当作protected

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if (_var!=StringID_Invalid)
			FormatString(s,"提交参数%d保存在变量[%s]中",_idx,assist->GetStr(_var));
	}

	int _idx;
	StringID _var;
};

class CBgn_GetTalkDialogParam:public CLevelBgn
{
public:
	CBgn_GetTalkDialogParam()
	{
	}

	DEFINE_CLASS(CBgn_GetTalkDialogParam);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
};



class CBgp_TalkPopup:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_TalkPopup);


	virtual const char *GetTypeName()	{		return "弹出选择";	};

	virtual DWORD GetStubCount()	
	{		
		DWORD c=_choices.size();
		if (c>9)
			c=9;
		return 2+c;	
	}
	virtual PadStub GetStub(DWORD idx)
	{
		if (idx==0)
			return PadStub("开始",PadStub_In,TRUE);
		if (idx==1)
			return PadStub("[准备]",PadStub_Out,TRUE);

		idx-=2;

		static std::string str;

		if (idx<_choices.size())
		{
			StringID s=_choices[idx];
			if (s==StringID_Invalid)
				return PadStub("n/a",PadStub_Out,TRUE);
			FormatString(str,"!!%d",s);
			return PadStub(str.c_str(),PadStub_Out,TRUE);
		}

		return PadStub();
	}

	virtual BgpCategory GetCategory()	{		return BgpCtgr_Talk;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

    BEGIN_GOBJ_PURE_UID(CBgp_TalkPopup,1);
		GELEM_BGP_BASE();

		GELEM_BEHAVIORMEM_NUMBER(_varStop,"中止变量","当这个寄存器的值非0时,中止弹出,如果不设,则只弹出一次")

		GELEM_VAR_INIT(LevelTalkState,_statePopup,LevelTalk_Topic);
			GELEM_EDITVAR("风格",GVT_S,GSem(GSem_Interger,"主题:3,提示:5"),"选择的类型");
		GELEM_VARVECTOR_INIT(StringID,_choices,StringID_Invalid);
			GELEM_EDITVAR("选择内容",GVT_U,GSem(GSem_StringID,"谈话文字"),"选择内容");
    END_GOBJ();    

public: //当作protected

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if (_choices.size()>0)
		{
			s="";
			if (_statePopup==LevelTalk_Topic)
				s="弹出主题菜单\n";
			if (_statePopup==LevelTalk_Prompt)
				s="弹出提示\n";
// 			for (int i=0;i<_choices.size();i++)
// 			{
// 				if (_choices[i])
// 					AppendFmtString(s,"(%d).%s\n",i+1,StrLib_GetStr(_choices[i]));
// 				else
// 					AppendFmtString(s,"(%d). n/a \n",i+1);
// 			}
			if (_varStop!=StringID_Invalid)
				AppendFmtString(s,"重复弹出,直到中止变量[%s]设为1为止",assist->GetStr(_varStop));
		}
	}

	LevelTalkState _statePopup;
	StringID _varStop;
	std::vector<StringID> _choices;
};

class CBgn_TalkPopup:public CLevelBgn
{
public:
	CBgn_TalkPopup()
	{
		_idPlayer=LevelPlayerID_Invalid;
		_stage=None;
	}

	enum State
	{
		None=0,
		Preparing,
		Waiting,
		Handling,
	};

	DEFINE_CLASS(CBgn_TalkPopup);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);
	virtual void RewindOk(WORD keyRewind,BGNOutputs &outputs);//因为执行成功导致的Rewind
	virtual void RewindFail(WORD keyRewind,BGNOutputs &outputs);//因为执行失败导致的Rewind

	virtual void Break(BGNOutputs &outputs);
	virtual void Destroy();

protected:
	BOOL _NeedRepeat();
	void _Prepare(CLevelTalks *talks,BGNOutputs &outputs);
	void _TryRepeat(BGNOutputs &outputs);

	LevelPlayerID _idPlayer;
	BYTE _stage;

};


class CBgp_BreakTalk:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_BreakTalk);

	virtual const char *GetTypeName()	{		return "中止对话";	};
	virtual DWORD GetStubCount()	{		return 2;	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB()
			STUB_IN(0,"中止对话");
			STUB_OUT(1,"结束");
		END_STUB()
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Talk;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

    BEGIN_GOBJ_PURE_UID(CBgp_BreakTalk,1);
		GELEM_BGP_BASE();


    END_GOBJ();    

public: //当作protected

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
	}
};

class CBgn_BreakTalk:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_BreakTalk);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};


class CBgp_WaitTalk:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_WaitTalk);

	virtual const char *GetTypeName()	{		return "等待对话";	};
	virtual DWORD GetStubCount()	{		return 2;	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB()
			STUB_IN(0,"开始");
			STUB_OUT(1,"结束");
		END_STUB()
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Talk;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

    BEGIN_GOBJ_PURE_UID(CBgp_WaitTalk,1);
		GELEM_BGP_BASE();


    END_GOBJ();    

public: //当作protected

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
	}

};

class CBgn_WaitTalk:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_WaitTalk);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);

protected:

};



class CBgp_RecordTalkPlayer:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_RecordTalkPlayer);

	virtual const char *GetTypeName()	{		return "记录交谈对象";	};
	virtual DWORD GetStubCount()	{		return 2;	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB()
			STUB_IN(0,"开始");
			STUB_OUT(1,"结束");
		END_STUB()
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Talk;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	BEGIN_GOBJ_PURE_UID(CBgp_RecordTalkPlayer,1);
		GELEM_BGP_BASE();
		GELEM_BEHAVIORMEM_OBJID(_nmVar,"变量名称","将当前谈话对象记录到哪个变量里")
	END_GOBJ();    

public: //当作protected

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		if (_nmVar==StringID_Invalid)
			s="n/a";
		else
			FormatString(s,"当前谈话对象记录到变量[%s]里",assist->GetStr(_nmVar));
	}

	StringID _nmVar;


};

class CBgn_RecordTalkPlayer:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_RecordTalkPlayer);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};


class CBgp_CheckTalk:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_CheckTalk);

	virtual const char *GetTypeName()	{		return "检测对话";	};
	virtual DWORD GetStubCount()	{		return 3;	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB()
			STUB_IN(0,"开始");
			STUB_OUT(1,"是");
			STUB_OUT(2,"否");
		END_STUB()
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Talk;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	BEGIN_GOBJ_PURE_UID2(CBgp_CheckTalk,427,1);
	GELEM_BGP_BASE();


	END_GOBJ();    

public: //当作protected

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="检测当前是否在发生对话";
	}

};

class CBgn_CheckTalk:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_CheckTalk);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};
