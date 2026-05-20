
#pragma once

#include "IResource.h"
class CRecords;
class IRecords :public IResource
{
public: 
	virtual CRecords *GetRecords()=0;

};

class CClass;
class IRecordsMgr :public IResourceMgr
{
public:
	virtual void BindRecordClass(const char *nameRes,CClass *clssRecord)=0;//将资源文件名和一个class绑定起来
	virtual CClass *FindRecordClass(const char *nameRes)=0;

};
