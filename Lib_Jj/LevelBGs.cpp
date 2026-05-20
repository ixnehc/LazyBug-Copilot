/********************************************************************
	created:	2012/11/21 
	author:		cxi
	
	purpose:	Level BehaviorGraph
*********************************************************************/
#include "stdh.h"
#include "LevelBGs.h"


#include "behaviorgraph/BehaviorGraphPads.h"

#include "commondefines/general_stl.h"

#include "resdata/ResDataDefines.h"

#include <fstream>

#include "log/LogDump.h"

#include "behaviorgraph/Behavior.h"
#include "behaviorgraph/BgnInclude.h"



////////////////////////////////////////////////////////////////////////
//CLevelBGs

// static void CollectFiles(const char *pathFull,std::vector<std::string>&vecFiles)
// {
// 	std::string pathToFind,pathToCollect;
// 	pathToFind=pathFull;
// 	pathToFind+="\\*.*";
// 	HANDLE hFindFile;
// 	WIN32_FIND_DATAA fd;
// 	hFindFile=FindFirstFileA(pathToFind.c_str(),&fd);
// 	if (hFindFile)
// 	{
// 		do
// 		{
// 			if ((fd.cFileName[0] == '.')&&(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
// 			{
// 				if (fd.cFileName[1] =='\0'||(fd.cFileName[1]=='.'&&fd.cFileName[2] == '\0'))
// 					continue;//ignore the dots
// 			}
// 			if (fd.dwFileAttributes&FILE_ATTRIBUTE_HIDDEN)
// 				continue;//忽略隐藏文件
// 			std::string pathSub;
// 			pathSub=pathFull;
// 			pathSub=pathSub+"\\"+fd.cFileName;
// 			if (fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
// 			{
// 				CollectFiles(pathSub.c_str(),vecFiles);
// 				continue;
// 			}
// 			extern BOOL CheckFileSuffix(const char *fn,const char *suffix);
// 			if (CheckFileSuffix(pathSub.c_str(),"bgr"))
// 				vecFiles.push_back(pathSub);
// 		}while(FindNextFileA(hFindFile,&fd));
// 		::FindClose(hFindFile);
// 	}
// }
// 
// struct ResFileHeader2
// {
// 	ResType type:8;
// 	DWORD off:24;//ResData完整数据的偏移(距离文件起始位置)
// };
// 
// 
// struct BgpClasses;
// extern BgpClasses *BgpClasses_GetSingleton();
// 
// BOOL CLevelBGs::_LoadBGPads(const char *path,CBehaviorGraphPads &pads)
// {
// 	std::ifstream ifs;
// 	ifs.open(path,std::ios_base::in|std::ios_base::binary);
// 	if (!ifs.is_open())
// 		return FALSE;
// 
// 	ResFileHeader2 header;
// 	ifs.read((char *)&header,sizeof(header));
// 
// 	if (header.type!=Res_BehaviorGraph)
// 	{
// 		ifs.close();
// 		return FALSE;
// 	}
// 
// 	ifs.seekg(header.off);
// 
// 	DWORD sz;
// 	ifs.read((char *)&sz,sizeof(sz));
// 	_bufTemp.resize(sz);
// 	ifs.read((char *)_bufTemp.data(),sz);
// 
// 	ifs.close();
// 
// 	return _LoadBGPadsFromData(_bufTemp.data(),pads,(LinkPadClasses *)BgpClasses_GetSingleton());
// }


void CLevelBGs::Init(CBehaviorGraphUtil &util)
{
	DWORD nNames;
	StringID *nms=util.EnumNames(nNames);

	for (int i=0;i<nNames;i++)
	{
		CBehaviorGraphPads *pads=Class_New2(CBehaviorGraphPads);
		
		if (!util.LoadBGPads(nms[i],*pads))
		{
			Safe_Class_Delete(pads);
			continue;
		}

		if (!util.ResolveBGPads(*pads))
		{
			Safe_Class_Delete(pads);
			continue;
		}

		BgpFamily family=pads->GetFamily();
		if ((family!=BgpFamily_Common)&&(family!=BgpFamily_Level)
			&&(family!=BgpFamily_MagicBoard))
		{
			pads->Clear();
			Safe_Class_Delete(pads);
			continue;
		}

		_CompileBG(pads);
	}

	//Resolve includes
	if (TRUE)
	{
		std::set<CBehaviorGraph *>stack;
		std::set<CBehaviorGraph *>resolved;
		for (int i=0;i<_bgs.size();i++)
		{
			CBehaviorGraph *bg=_bgs[i];
			if (!bg)
				continue;

			_ResolveIncludes(bg,stack,resolved);
		}
	}
}

void CLevelBGs::Clear()
{
	__super::Clear();
	_bufTemp.clear();
}

BOOL CLevelBGs::_ResolveIncludes(CBehaviorGraph *bg,std::set<CBehaviorGraph *>&stack,std::set<CBehaviorGraph *>&resolved)
{
	if (stack.find(bg)!=stack.end())
	{
		LOG_DUMP_1P("CLevelBGs",Log_Error,"行为图(%s)中发现有循环内联现象!",StrLib_GetStr(bg->GetName()));
		return FALSE;
	}
	if (resolved.find(bg)!=resolved.end())
		return TRUE;
	//Push
	stack.insert(bg);

	BOOL bResolved=TRUE;
	if (bg->GetPads())
	{
		DWORD n=bg->GetPads()->GetPadCount();
		for (int i=0;i<n;i++)
		{
			CLinkPad *pad=bg->GetPads()->GetPad(i);
			if (!pad)
				continue;
			if (pad->GetClass()->CheckName("CBgp_Include"))
			{
				CBgp_Include*padInclude=(CBgp_Include*)pad;
				if (padInclude->_nm!=StringID_Invalid)
				{
					CBehaviorGraph *bgSub=FindBG(padInclude->_nm);
					if (!bgSub)
					{
						LOG_DUMP_2P("CLevelBGs",Log_Error,"行为图(%s)中发现有无效的内联(%s)!",
							StrLib_GetStr(bg->GetName()), StrLib_GetStr(padInclude->_nm));
						continue;
					}

					if (FALSE==_ResolveIncludes(bgSub,stack,resolved))
					{
						bResolved=FALSE;
						break;
					}
					bg->ResolveInclude(pad->GetID(),bgSub);
				}
			}
		}

	}


	//Pop
	if (TRUE)
	{
		std::set<CBehaviorGraph *>::iterator it=stack.find(bg);
		if (it!=stack.end())
			stack.erase(it);
	}

	if (bResolved)
		resolved.insert(bg);
	return bResolved;
}
