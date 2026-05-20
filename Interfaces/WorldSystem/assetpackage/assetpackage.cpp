/********************************************************************
	created:	2008/1/14   13:35
	filename: 	e:\IxEngine\Interfaces\WorldSystem\assetbase\assetlib.cpp
	author:		cxi
	
	purpose:	functions for export asset package information
*********************************************************************/
#include "stdh.h"

#include <assert.h>

#include "assetpackage.h"

#include "interface/interface.h"

#include "../assetcore/asset.h"

#include "class/class.h"

#include "timer/Profiler.h"
#include "Log/logdump.h"
#include "gds/GStack.h"
#include "strlib/strlib.h"


EXPOSE_SINGLE_INTERFACE(CAssetPackage,IAssetPackage,"AssetPackage01")


const char *CAssetPackage::GetName()
{
	extern char g_libname[];
	return g_libname;
}

const char **CAssetPackage::GetClassNames(DWORD &c)
{
	static std::vector<const char*> buf;
	static std::vector<std::string>buildin;
	CClass::Enum(0,ClassF_Asset,buildin);
	c=buildin.size();
	if (c<=0)
		return NULL;
	buf.resize(c);
	for (int i=0;i<c;i++)
		buf[i]=buildin[i].c_str();
	return buf.data();
}

CClass*CAssetPackage::GetClass(const char *name)
{
	return CClass::Find(name);
}



ProfilerMgr *CAssetPackage::GetProfilerMgr()
{
	extern ProfilerMgr *GetProfilerMgr();

	::GetProfilerMgr()->SetName(GetName());
	return ::GetProfilerMgr();
}

void CAssetPackage::RegisterLogHandler(LogHandler &handler)
{
	extern void ::RegisterLogHandler(LogHandler &handler);
	::RegisterLogHandler(handler);
}

void CAssetPackage::SetStrLib(CStrLib *strlib)
{
	::StrLib_Set(strlib);
}

void CAssetPackage::AttachGStack(GStack *stack)
{
	GStack::stack()=stack;
}


