
#pragma once


class CClass;
class IAssetSystem;
struct ProfilerMgr;
struct LogHandler;
class CStrLib;
struct GStack;
class IAssetPackage
{
public:
	virtual const char *GetName()=0;
	virtual const char **GetClassNames(DWORD &c)=0;
	virtual CClass *GetClass(const char *name)=0;


	virtual void SetNext(IAssetPackage *al)=0;
	virtual IAssetPackage *GetNext()=0;

	virtual ProfilerMgr *GetProfilerMgr()=0;
	virtual void RegisterLogHandler(LogHandler &handler)=0;
	virtual void SetStrLib(CStrLib *strlib)=0;
	virtual void AttachGStack(GStack *stack)=0;
};

#define BeginPackageService()													\
	void CAssetPackage::LoadServices(IAssetSystem *pAS)		\
	{

#define EndPackageService()													\
	}

#define DefinePackageService(serviceclss,serviceid)				\
	pAS->RegisterService(serviceid,new serviceclss);
