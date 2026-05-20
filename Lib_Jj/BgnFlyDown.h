#pragma once

#include "BgnFlyingHover.h"


struct CBgp_FlyDown:public CBgp_FlyingHover
{
	DEFINE_CLASS(CBgp_FlyDown);

	virtual const char *GetTypeName()	{		return "盘旋着陆";	}
	virtual DWORD GetStubCount()
	{
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"成功着陆");
			STUB_OUT(2,"无法着陆");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Action;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if (_idGesture!=RecordID_Invalid)
		{
			if (_dur>0)
				FormatString(s,"尝试着陆,使用[%s]\n尝试时间:%.2f(+/-)%.2f秒",assist->GetGestureName(_idGesture),
							ANIMTICK_TO_SECOND(_dur),ANIMTICK_TO_SECOND(_durVary));
			else
				FormatString(s,"尝试着陆,使用[%s]",assist->GetGestureName(_idGesture));
		}
	}

	BEGIN_GOBJ_PURE_UID(CBgp_FlyDown,1); DERIVE_GOBJ(CBgp_FlyDown,CBgp_FlyingHover)
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(RecordID,_idGesture,RecordID_Invalid);
			GELEM_EDITVAR("使用的Gesture",GVT_U,GSem(GSem_RecordID,"gestures"),"使用哪个gesture来落地");

	END_GOBJ();

	RecordID _idGesture;

};



class Gesture_FlyDown;
class CBgn_FlyDown:public CBgn_FlyingHover
{
public:
	DEFINE_CLASS(CBgn_FlyDown);

	enum Stage
	{
		Hovering,
		Prepare,
		Descend,
	};

	CBgn_FlyDown()
	{
		_owner=NULL;
		_stage=Hovering;
		_ges=NULL;
		_bSwitchGround=FALSE;
	}

	virtual void Destroy();

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);
	virtual void Break(BGNOutputs &outputs);

protected:

	void _SwitchGround();

	void _FireFail(BGNOutputs &outputs);

	CLevelObj *_owner;

	Gesture_FlyDown*_ges;

	AResult _result;

	BOOL _CanDescend();
	BOOL _Descend();
	int _stage;
	LevelPos3D _posStart;//开始下落点
	LevelPos3D _posEnd;//着地点

	BOOL _bSwitchGround;

};


