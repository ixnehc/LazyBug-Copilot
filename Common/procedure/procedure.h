
#pragma once

#define NEXTLOC_START ((int*)0x7fffffff)
#define NEXTLOC_END ((int*)0x7ffffffe)

class CClass;

class CProcedure
{
public:
	CProcedure()
	{
		_bEnd=TRUE;
		_cur=NULL;
		_locNext=NEXTLOC_START;
		_tStart=0;
	}
	virtual ~CProcedure()
	{
		Safe_Class_Delete(_cur);
	}

	virtual CClass *GetClass()=0;

	void Start();
	void Update();
	void Destroy();

	BOOL IsEnd()	{		return _bEnd;	}

protected:
	void _Start();
	void _End();

	virtual void _Main(){}
	virtual void _OnDestroy(){}
	virtual void _OnUpdate(){}
	virtual void _OnStart(){}
	virtual void _OnEnd(){}
	virtual DWORD _GetCurTime()	{		return 0;	}
	virtual DWORD _GetGuardDur()	{		return 0;	}//如果一个Proc持续的时间超过了指定的GuardDuration,则会发警告消息,并终止自己

	BOOL _bEnd;

	CProcedure *_cur;

	int *_locNext;

	DWORD _tStart;

};


#define Procedure_Begin()														\
if (_locNext==NEXTLOC_START)												\
	goto __Start;																			\
if (_locNext==NEXTLOC_END)													\
	return;

#define Procedure_Label(label)													\
static int label;																			\
	if (_locNext==&label)															\
		goto _##label;


#define Procedure_End()															\
__Start:

#define Procedure_Start(name,clss)											\
	_cur=Class_New2(clss);															\
	((clss*)_cur)->Start();																\
	_locNext=name;																		\
	return;																						\
_##name:


#define Procedure_Pend(label)												\
_locNext=&label;																	\
if (!_cur->IsEnd())																	\
	return;																					\
_cur->Destroy();																		\
Safe_Class_Delete(_cur);															\
_##label:

#define _EndAndReturn() {_End();return;}