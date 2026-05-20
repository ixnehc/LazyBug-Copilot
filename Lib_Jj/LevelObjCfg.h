#pragma once

#include "class/class.h"

#include "LevelDefines.h"


class CLevelObjSrc;
inline void los_verify(CLevelObjSrc*c) {}

#define DEFINE_LEVELOBJSRC_CLASS(clss,uid)													\
	_DEFINE_CLASS_BEGIN(CClass,CClassPool,clss,void)								\
		instance._flag|=ClassF_LevelSrc;															\
		instance._uid=uid;																					\
		{clss *p=NULL;los_verify(p);}																	\
	_DEFINE_CLASS_END(clss)																			\
	typedef clss ThisType;


class CLevelObj;
struct GObjBase;

class CLevelObjParam
{
public:
	CLevelObjParam()
	{
	}

	virtual CClass *GetClass()=0;
	virtual GObjBase *GetGObj()=0;
protected:

	friend class CLevelSources;

};

class CLevelObjSrc
{
public:
	CLevelObjSrc()
	{
		_idCache=LevelObjSrcID_Invalid;
	}

	virtual CClass *GetClass()=0;
	virtual GObjBase *GetGObj()=0;
	i_math::matrix43f&GetMat()	{		return _mat;	}
	LevelObjSrcID GetID()	{		return _idCache;	}
	void CacheID();
protected:
	i_math::matrix43f _mat;

	LevelObjSrcID _idCache;

	friend class CLevelSources;

};



class CDataPacket;
class CLevelObjSrc;
class CLevelSources
{
public:
	void Clear();

	BOOL Save(CDataPacket &dp);
	BOOL Load(CDataPacket &dp);

	void Add(CLevelObjSrc *src)
	{
		_buf.push_back(src);
	}
protected:
	std::vector<CLevelObjSrc*> _buf;

	friend class CLevel;
};

