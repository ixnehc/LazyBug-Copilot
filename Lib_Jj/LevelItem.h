#pragma once

#include "class/class.h"

#include "LevelDefines.h"

class CLevelItem;
inline void li_verify(CLevelItem*c) {}


#define DEFINE_LEVELITEM_CLASS(clss,uid)													\
	_DEFINE_CLASS_BEGIN(CClass,CClassPool,clss,void)								\
	instance._flag|=ClassF_LevelItem;															\
	instance._uid=uid;																					\
{clss *p=NULL;li_verify(p);}																	\
	_DEFINE_CLASS_END(clss)																			\
	typedef clss ThisType;


struct LevelRecordItem;
class CLevelObjSrc;
class CLevelMsgBuf;
class CLevel;
class CBitPacket;
class CLevelItem
{
public:
	IMPLEMENT_REFCOUNT_OVERRIDE;
	void OnRelease();

	DEFINE_LEVELITEM_CLASS(CLevelItem,1);


	CLevelItem()
	{
		_level=NULL;
		_bits=0;
		_id=LevelItemID_Invalid;
		_rec=NULL;
	}


	BOOL IsAlive()	{		return _bAlive==1?TRUE:FALSE;	}
	BOOL Create();//加引用计数
	void Destroy();//减引用计数

	LevelItemID GetID()	{		return _id;	}

	LevelRecordItem *GetRec()	{		return _rec;	}
	void SetRec(LevelRecordItem *rec)	{		_rec=rec;	}

	virtual BOOL OnCreate()	{		return TRUE;	}
	virtual void OnDestroy(){	}

protected:

	void _Destroy();

	union
	{
		struct
		{
			WORD _bAlive:1;
			WORD _bEnum:1;
		};
		WORD _bits;
	};
	LevelItemID _id;
	CLevel *_level;

	LevelRecordItem *_rec;

	friend class CLevel;
	friend class CLevelIDs;

};
