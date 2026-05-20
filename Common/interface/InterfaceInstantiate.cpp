/********************************************************************
	created:	30:7:2006   11:23
	filename: 	d:\IxEngine\Common\interface\InterfaceInstantiate.cpp
	file path:	d:\IxEngine\Common\interface
	file base:	InterfaceInstantiate
	author:		ixnehc
	
	purpose:	functions for easily instantiate interfaces
*********************************************************************/
#include "stdh.h"

#include <DbgHelp.h>
#include "InterfaceInstantiate.h"

#include "../stringparser/stringparser.h"

#include <string>
#include <map>

#include <tchar.h>

std::string g_pathDllDir;
const char *g_pathDlls[]=
{
	"\\stlport_vc7146.dll",
	"\\lzma._dll",
	"\\ijl15.dll",
// 	"\\msvcm80.dll",
// 	"\\msvcp80.dll",
// 	"\\msvcr80.dll"
};
const char *g_pathDllsD[]=
{
	"\\stlport_vc7146.dll",
	"\\lzma._dll",
	"\\ijl15.dll",

	//vc8µÄÔËÐÐ¿â
// 	"\\msvcm80d.dll",
// 	"\\msvcp80d.dll",
// 	"\\msvcr80d.dll"


};
HMODULE g_hDlls[ARRAY_SIZE(g_pathDlls)];

std::map<std::string,HMODULE>g_mapModules;

HANDLE g_hCurProc=NULL;

//copied from 
static bool IsNT()
{
	return true;
}

static HANDLE GetCurProcHandle()
{
	if (IsNT())
		return GetCurrentProcess();
	else
		return (HANDLE)((char*)0+GetCurrentProcessId());
}


void II_Init(HMODULE hMod)
{
#if defined(UNICODE) || defined(_UNICODE)
	wchar_t buffer[512];
#else
	char buffer[512];
#endif
	GetModuleFileName(hMod,buffer,500);
	g_pathDllDir = toMBCS(buffer);
	int l;
	l=(int)g_pathDllDir.rfind('\\');
	g_pathDllDir=((g_pathDllDir).substr(0,l));

	//supporting dll
#ifdef _DEBUG
	const char **pathDlls=g_pathDllsD;
#else
	const char **pathDlls=g_pathDlls;
#endif

	for (int i=0;i<ARRAY_SIZE(g_pathDlls);i++)
		g_hDlls[i] = LoadLibrary(fromMBCS((g_pathDllDir + pathDlls[i]).c_str()));

	g_hCurProc=GetCurProcHandle();
}

void II_UnInit()
{
	std::map<std::string,HMODULE>::iterator it;
	for (it=g_mapModules.begin();it!=g_mapModules.end();it++)
	{
		if ((*it).second)
			FreeLibrary((*it).second);
	}
	g_mapModules.clear();

	for (int i=0;i<ARRAY_SIZE(g_pathDlls);i++)
		FreeLibrary(g_hDlls[i]);
};



HMODULE II_GetModule(std::string &path)
{
	std::map<std::string,HMODULE>::iterator it;
	it=g_mapModules.find(path);
	HMODULE hMod;
	if (it==g_mapModules.end())
	{
		hMod = LoadLibrary(fromMBCS(path.c_str()));
		if (!hMod)
		{
			DWORD err=GetLastError();
			std::string s;
			s="Failed to load module \"";
			s+=path.c_str();
			s+="\"!";
			MessageBox(NULL, fromMBCS(s.c_str()), _T("Error"),MB_OK);
			return NULL;
		}
		g_mapModules[path]=hMod;

// 		if (TRUE)//Load the symbol
// 		{
// 			std::string pathPDB=path;
// // 			RemoveFileSuffix(pathPDB);
// // 			MakeFileSuffix(pathPDB,"pdb");
// 
// 			HANDLE hFile = CreateFileA(pathPDB.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
// 			if (hFile)
// 			{
// // 				DWORD hi,sz;
// // 				sz=GetFileSize(hFile,&hi);
// 				DWORD ret=SymLoadModule(g_hCurProc,hFile,(PSTR)pathPDB.c_str(), 0, (DWORD)((char*)hMod -(char*)0), 0);
// 			}
// 
// 		}
	}
	else
		hMod=(*it).second;

	return hMod;
}

void *II_AcuireXS(const char *path,const char *name)
{
	std::string s;
	s=g_pathDllDir+"\\"+path;
	HMODULE hMod;
	hMod=II_GetModule(s);
	if (hMod)
		return CreateInterfaceInModule(hMod,name);
	return NULL;
}
