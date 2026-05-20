
#include "stdh.h"
#include "engine.h"

#include "stringparser/stringparser.h"

#include "../interface/InterfaceInstantiate.h"

#include "FileSystem/IFileSystem.h"

#include "Log/LogFile.h"

#include <assert.h>


//////////////////////////////////////////////////////////////////////////
//CEngine

CEngine g_Engine;

extern void InitStrLib(CStrLib &strlib,const char *pathEngineRoot);
extern BOOL LoadStrLib(CStrLib &strlib,IFileSystem *pFS);


BOOL CEngine::Init(EngineParam &param)
{
	BOOL bRet=FALSE;
	II_Init(NULL);

	II_AccuireFS01(_pFS);
	if (!_pFS)
	{
		LogFile::Prompt("Fail to instantiate IFileSystem(01) interfaces");
		goto _final;
	}

	//初始化StrLib
	if (TRUE)
	{
		std::string path,s;
		path=GetModuleFolderPath(NULL);
		CutTailSubPath(path,s);
#ifdef _DEBUG
		CutTailSubPath(path,s);
#endif

		InitStrLib(_strlib,path.c_str());
		LoadStrLib(_strlib,_pFS);

	}


	bRet=TRUE;

_final:

	return bRet;

}

void CEngine::UnInit()
{
	_strlib.Clear();

	II_UnInit();
}


//////////////////////////////////////////////////////////////////////////
//CStrLib相关

void InitStrLib(CStrLib &strlib,const char *pathEngineRoot)
{
	std::string pathStrLib= std::string(pathEngineRoot)+"\\data\\stringlib\\default.strlib";
	strlib.Init();
	strlib.SetPath(pathStrLib.c_str());
}

BOOL LoadStrLib(CStrLib &strlib,IFileSystem *pFS)
{
	const char *pathStrLib=strlib.GetPath();
	IFile *fl=pFS->OpenFileAbs(pathStrLib,FileAccessMode_Read);
	if (fl)
	{
		std::vector<BYTE>buf;
		IFile_ReadVector(fl,buf);
		fl->Close();
		CDataPacket dp;
		dp.SetDataBufferPointer(buf.data());
		strlib.Load(dp);
	}
	else
	{
		if (pFS->ExistFileAbs(pathStrLib))
			LogFile::Prompt("无法打开%s!",pathStrLib);
		else
		{
			LogFile::Prompt("%s 不存在,创建一个新的!",pathStrLib);
			CDataPacket dp;
			std::vector<BYTE>buf;
			DP_BeginSave(dp,buf);
			strlib.Save(dp);
			DP_EndSave();
			fl=pFS->OpenFileAbs(pathStrLib,FileAccessMode_Write);
			if (fl)
			{
				IFile_WriteVector(fl,buf);
				fl->Close();
			}
		}
	}

	return TRUE;
}

