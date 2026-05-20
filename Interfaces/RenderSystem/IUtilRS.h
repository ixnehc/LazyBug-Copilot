#pragma once

#include "IRenderSystemDefines.h"

#include <string>
#include <vector>
#include <map>

class IRenderSystem;
class IFileSystem;

struct ResData;
class ResDataGroup;
class ResDataList;
class ResDataRename;

class CShaderLib;


struct TexInfo;
struct TexData;
struct RecordsData;
struct BehaviorGraphData;
struct LinkPadClasses;
class CClass;
class IUtilRS
{
public:
	//working environment
	virtual void SetRS(IRenderSystem *pRS)=0;
	virtual void SetFS(IFileSystem *pFS)=0;
	virtual IRenderSystem *GetRS()=0;
	virtual IFileSystem *GetFS()=0;

	//utility functions
	virtual BOOL LockRefDevice()=0;
	virtual void UnlockRefDevice()=0;


	virtual BOOL LoadTexData(const char *path,TexData *data)=0;
	virtual TexData *LoadTexData(const char *path)=0;
	virtual BOOL LoadTexInfo(TexInfo *ti,TexData *data)=0;
	virtual BOOL CullTexData(TexData *dest,TexData *src,i_math::recti &rc)=0;

	virtual BOOL SaveTexData(const char *path,TexData *data)=0;

	//Resource File
	virtual BOOL RepairResData(ResData *resdata)=0;//check whether the res data is in its proper format,if not ,try to repair it
																						//返回是否有修改

	virtual ResData *LoadRes(const char *path,BOOL bHeader)=0;
	virtual RecordsData *LoadRes_Records(const char *path,CClass *clss)=0;
	virtual BehaviorGraphData*LoadRes_BehaviorGraph(const char *path,LinkPadClasses *clsses)=0;
	virtual BOOL SaveRes(const char *path,ResData *data)=0;

	//for effect
	virtual void*CompileEffect(const char *sFX,DWORD lenFX,std::string &error)=0;
	//check to see whether sFX could pass the compiler,return FALSE if fail
	//if bAssemblyInfo is TRUE, the assembly codes will be returned in ErrorOrAssem,if passing the compiler
	virtual BOOL CheckCompileEffect(const char *sFX,DWORD lenFX,BOOL bAssemblyInfo,std::string &ErrorOrAssem)=0;

	//Shader Lib
	virtual BOOL SaveShaderLib(CShaderLib &slib,const char *path)=0;
	virtual BOOL LoadShaderLib(CShaderLib &slib,const char *path)=0;


};

