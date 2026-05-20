
#pragma once

class IRenderSystem;
class IResource
{
public:
	virtual IRenderSystem *GetRS()=0;//得到这个资源属于的RenderSystem
	virtual int AddRef()=0;//return the ref-count after adding
	virtual int Release()=0;//return the ref-count after release
	virtual const char *GetPath()=0;//return the path,if nameless,return ""
	virtual AResult Touch()=0;//return whether the resource is ready for use
	virtual BOOL ForceTouch()=0;//return whether the resource is ready to touch
	virtual DWORD GetVer()=0;//返回版本号,当这个资源被重新载入的时候,这个版本号会被增加
};

#define SafeTouch(pRes) (((pRes)?(pRes)->Touch():A_Fail))
#define SafeForceTouch(pRes) (((pRes)?(pRes)->ForceTouch():FALSE))

class IResourceMgr
{
public:
	virtual IRenderSystem *GetRS()=0;
	virtual IResource *ObtainRes(const char *pathRes)=0;//如果pathRes为空,返回NULL,否则返回一个非空的指针,
																							//用户可以通过返回的指针来查询(使用Touch()/ForceTouch())资源是否正确载入
	virtual void Update()=0;

	virtual BOOL CheckResLeak()=0;//Check whether there is any resource left not released in the mgr

};
