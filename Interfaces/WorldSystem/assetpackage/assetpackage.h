
#pragma once

#include "IAssetPackage.h"


class CAssetPackage:public IAssetPackage
{
public:
	CAssetPackage()
	{
		_next=NULL;
	}
	virtual const char *GetName();
	virtual const char **GetClassNames(DWORD &c);
	virtual CClass *GetClass(const char *name);
	virtual void LoadServices(IAssetSystem *pAS);

	virtual void SetNext(IAssetPackage *al)	{		_next=al;	}
	virtual IAssetPackage *GetNext()	{		return _next;	}

	virtual ProfilerMgr *GetProfilerMgr();
	virtual void RegisterLogHandler(LogHandler &handler);
	virtual void SetStrLib(CStrLib *strlib);
	virtual void AttachGStack(GStack *stack);


protected:
	IAssetPackage *_next;

};

#define EXPOSE_ASSET_PACKAGE(name) char g_libname[]=name