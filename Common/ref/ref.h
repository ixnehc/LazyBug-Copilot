
#pragma once

#include "../class/class.h"

struct Ref
{
	DEFINE_CLASS(Ref);
	Ref()
	{
		refcount=0;
		stuff=NULL;
	}
	int AddRef()	{		refcount++;return refcount;	}
	int Release()
	{
		refcount--;
		int t=refcount;
		if (refcount<=0)
			Class_Delete(this);
		return t;
	}

	void *GetStuff()	{		return stuff;	}

	int refcount;
	void *stuff;
};

#define DEFINE_REF()					\
struct __Ref										\
{														\
	__Ref()	{		ptr=NULL;	}			\
	Ref *ptr;										\
};														\
__Ref __ref;										\
Ref *GetRef()									\
{														\
	if (!__ref.ptr)									\
	{													\
		__ref.ptr=Class_New2(Ref);	\
		__ref.ptr->stuff=this;				\
		__ref.ptr->AddRef();				\
	}													\
	return __ref.ptr;							\
}														\
Ref *ObtainRef()								\
{														\
	Ref *ret=GetRef();						\
	ret->AddRef();							\
	return ret;									\
}														\
void BreakRef()								\
{														\
	if (__ref.ptr)									\
		__ref.ptr->stuff=NULL;			\
	SAFE_RELEASE(__ref.ptr);			\
}


