#pragma once

#include "ActBase.h"

#include "Act_Hover.h"



struct ActParam_FlyDown:public ActParam_Hover
{
	DEFINE_CLASS(ActParam_FlyDown);

	BEGIN_GOBJ_PURE(ActParam_FlyDown,1); DERIVE_GOBJ(ActParam_FlyDown,ActParam_Hover)

		GELEM_VAR_INIT(RecordID,idGesture,RecordID_Invalid);
			GELEM_EDITVAR("使用的Gesture",GVT_U,GSem(GSem_RecordID,"gestures"),"使用哪个gesture来落地");

	END_GOBJ();

	RecordID idGesture;

};



class Gesture_FlyDown;
class Act_FlyDown:public Act_Hover
{
public:
	DEFINE_ACT_CLASS(Act_FlyDown);

	enum Stage
	{
		Hovering,
		Prepare,
		Descend,
	};

	Act_FlyDown()
	{
		_stage=Hovering;
		_result=A_Pending;
		_ges=NULL;
		_bSwitchGround=FALSE;
	}

	void Start(AnimTick t);
	virtual void Finish();
	void Update(AnimTick t);

	AResult GetResult()	{		return _result;	}

protected:

	void _SwitchGround();

	Gesture_FlyDown*_ges;

	AResult _result;

	BOOL _CanDescend();
	void _Descend();
	int _stage;
	LevelPos3D _posStart;//开始下落点
	LevelPos3D _posEnd;//着地点

	BOOL _bSwitchGround;

};


