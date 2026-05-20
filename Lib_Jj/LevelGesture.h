#pragma once

#include "class/class.h"
#include "gds/GObj.h"

#include "records/recordsdefine.h"

#include "LevelDefines.h"

#include "unitmgr/Unit3DMgr.h"

class CLevelGesture;
inline void gesture_verify(CLevelGesture*c) {}

#define DEFINE_GESTURE_CLASS(clss,uid)															\
	_DEFINE_CLASS_BEGIN(CClass,CClassPool,clss,void)								\
	instance._flag|=ClassF_LevelGesture;																	\
	instance._uid=uid;																					\
		{clss *p=NULL;gesture_verify(p);}																\
		_DEFINE_CLASS_END(clss)																			\
		typedef clss ThisType;


#define DEFINE_GESTUREPARAM_CLASS(clss)											\
	DEFINE_CLASS(clss);																	\
	virtual CClass*GetGestureClass()																				\
	{																																\
		extern CClass *GetGestureClass_##clss();																				\
		return GetGestureClass_##clss();																				\
	}

#define BIND_GESTUREPARAM(clssGesture,clssGestureParam)													\
	CClass *GetGestureClass_##clssGestureParam()																				\
	{																													\
		return Class_Ptr2(clssGesture);																\
	}																													\
	CClass *GetClass_##clssGestureParam()														\
	{																													\
		return Class_Ptr2(clssGestureParam);															\
	}

struct LevelGestureParam
{
	virtual CClass *GetClass()=0;
	virtual GObjBase*GetGObj()=0;
	virtual CClass*GetGestureClass()=0;
};

struct LevelRecordGesture;
struct LevelGestureCore
{
	LevelGestureCore()
	{
		Zero();
	}
	void Zero()
	{
		param=NULL;
		idRec=RecordID_Invalid;
		rec=NULL;
		faceInitial=0.0f;
		radiusOwner=0.0f;
		radiusTarget=0.0f;
		t=0;
	}
	LevelGestureParam *param;
	LevelRecordGesture *rec;
	RecordID idRec;
	LevelSkillTarget target;
	LevelPos3D pos3DInitial;
	LevelFace faceInitial;
	float radiusOwner;
	float radiusTarget;
	AnimTick t;
};

struct LevelGestureEvent
{
	BEGIN_GOBJ_PURE(LevelGestureEvent,1);
		GELEM_VAR_INIT(StringID,nm,RecordID_Invalid);
			GELEM_EDITVAR("事件名称",GVT_U,GSem(GSem_StringID,"动画事件"),"事件名称");
		GELEM_VAR_INIT(AnimTick,t,ANIMTICK_FROM_SECOND(0.0f));
			GELEM_EDITVAR("发送时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"发送事件的时间");
	END_GOBJ();


	StringID nm;
	AnimTick t;
};


class CLevelObj;
struct LevelGestureParam;
class CLevelGesture:public CUnitGesture
{
public:
	IMPLEMENT_REFCOUNT_C;
	CLevelGesture()
	{
		Zero();
	}
	~CLevelGesture()
	{
		_Destroy();
	}

	struct EventEntry
	{
		StringID nm;
		i_math::xformf xfm;
	};

	void Zero()
	{
		_owner=NULL;
		_core.Zero();
		_tEvent=0;
		_bFinished=FALSE;
	}

	BOOL IsAlive()
	{
		return _owner!=NULL;
	}


	virtual BOOL Create(CLevelObj *owner,RecordID idRec,LevelGestureParam *param,LevelSkillTarget &target);
	virtual void Destroy();

	virtual UnitGestureUID GetUID()	{		return _core.idRec;	}

	virtual void UpdateEvent(AnimTick dt);
	virtual StringID FetchEvent(i_math::xformf &xfm);

	virtual void WriteFirstSync(CBitPacket *bp)	{}
	virtual BOOL WriteSync(CBitPacket *bp)	{		return FALSE;	}

	virtual void Finish()	{		_bFinished=TRUE;	}
	virtual BOOL IsFinished()	{		return _bFinished;	}

protected:
	void _Destroy();

	void _ApplySpeedRate(AnimTick &dt);
	void _ApplySpeedRate(float &dt);

	virtual void _OnCreate(){}
	virtual void _OnDestroy(){}

	CLevelObj *_owner;
	LevelGestureCore _core;

	AnimTick _tEvent;
	std::deque<EventEntry> _events;

	BOOL _bFinished;
	
};

#define UnitGestureUID_BuildIn (0xffffffff) //用于所有由程序实现的Gesture(无表格项关联的Gesture)

//这个类Gesture的类由各种技能使用,它返回的UID是一个固定的非表格项ID
class CLevelGesture_BuildIn:public CUnitGesture
{
public:

	virtual UnitGestureUID GetUID()	{		return UnitGestureUID_BuildIn;	}


};

