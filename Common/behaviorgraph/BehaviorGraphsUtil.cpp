/********************************************************************
	created:	2015/12/26 
	author:		cxi
	
	purpose:	BehaviorGraphUtil
*********************************************************************/
#include "stdh.h"
#include "BehaviorGraphs.h"
#include "BehaviorGraphsUtil.h"


#include "BehaviorGraphPads.h"
#include "BgnRelay.h"
//#include "BgnController.h"
#include "BgnHelper.h"
#include "BgnState.h"
#include "BgnInclude.h"

#include "../commondefines/general_stl.h"

#include "../log/LogDump.h"

#include "../resdata/ResDataDefines.h"
#include <fstream>

#include "Behavior.h"

#include "BehaviorDebug.h"



//////////////////////////////////////////////////////////////////////////
//BgpClasses
struct BgpClasses;
extern BgpClasses *BgpClasses_GetSingleton();

//////////////////////////////////////////////////////////////////////////
//CBehaviorGraphUtil
struct ResFileHeader2
{
	ResType type:8;
	DWORD off:24;//ResData完整数据的偏移(距离文件起始位置)
};

static StringID LoadBgName(const char *path)
{
	std::ifstream ifs;
	ifs.open(path,std::ios_base::in|std::ios_base::binary);
	if (!ifs.is_open())
		return FALSE;

	ResFileHeader2 header;
	ifs.read((char *)&header,sizeof(header));

	if (header.type!=Res_BehaviorGraph)
	{
		ifs.close();
		return StringID_Invalid;
	}

	ifs.seekg(header.off);

	DWORD sz;
	ifs.read((char *)&sz,sizeof(sz));

	WORD ver;
	ifs.read((char *)&ver,sizeof(ver));

	StringID nm;
	ifs.read((char *)&nm,sizeof(nm));

	ifs.close();

	return nm;
}

static void CollectBgNames(const char *pathFull,std::vector<StringID>&nms)
{
	std::string pathToFind,pathToCollect;
	pathToFind=pathFull;
	pathToFind+="\\*.*";
	HANDLE hFindFile;
	WIN32_FIND_DATAA fd;
	hFindFile=FindFirstFileA(pathToFind.c_str(),&fd);
	if (hFindFile)
	{
		do
		{
			if ((fd.cFileName[0] == '.')&&(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
			{
				if (fd.cFileName[1] =='\0'||(fd.cFileName[1]=='.'&&fd.cFileName[2] == '\0'))
					continue;//ignore the dots
			}
			if (fd.dwFileAttributes&FILE_ATTRIBUTE_HIDDEN)
				continue;//忽略隐藏文件
			std::string pathSub;
			pathSub=pathFull;
			pathSub=pathSub+"\\"+fd.cFileName;
			if (fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
			{
				CollectBgNames(pathSub.c_str(),nms);
				continue;
			}
			extern BOOL CheckFileSuffix(const char *fn,const char *suffix);
			if (CheckFileSuffix(pathSub.c_str(),"bgr"))
			{
				StringID nm=LoadBgName(pathSub.c_str());
				if (nm!=StringID_Invalid)
					nms.push_back(nm);
			}
		}while(FindNextFileA(hFindFile,&fd));
		::FindClose(hFindFile);
	}
}

StringID *CBehaviorGraphUtil::EnumNames(DWORD &count)
{
	_temp2.clear();
	CollectBgNames(_pathRoot.c_str(),_temp2);
	count=_temp2.size();
	return _temp2.data();
}



struct BgpClasses;
extern BgpClasses *BgpClasses_GetSingleton();

BOOL CBehaviorGraphUtil::_LoadBGPads(const char *path,StringID nm0,CBehaviorGraphPads &pads)
{
	std::ifstream ifs;
	ifs.open(path,std::ios_base::in|std::ios_base::binary);
	if (!ifs.is_open())
		return FALSE;

	ResFileHeader2 header;
	ifs.read((char *)&header,sizeof(header));

	if (header.type!=Res_BehaviorGraph)
	{
		ifs.close();
		return StringID_Invalid;
	}

	ifs.seekg(header.off);

	DWORD sz;
	ifs.read((char *)&sz,sizeof(sz));

	BOOL bRet=FALSE;

	if (sz>sizeof(WORD)+sizeof(StringID))
	{
		WORD ver;
		ifs.read((char *)&ver,sizeof(ver));

		StringID nm;
		ifs.read((char *)&nm,sizeof(nm));

		if (nm==nm0)
		{
			sz-=sizeof(WORD)+sizeof(StringID);

			_temp.resize(sz);
			ifs.read((char *)_temp.data(),sz);

			CDataPacket dp;
			dp.SetDataBufferPointer(_temp.data());

			pads.SetClasses(_clsses);
			pads.Load(dp);

			assert(pads.GetMinPadID()>2000);

			bRet=TRUE;
		}
	}

	ifs.close();

	return bRet;
}

BOOL CBehaviorGraphUtil::_FindBGPads(const char *pathRoot,StringID nm,std::string &path)
{
	BOOL bRet=FALSE;
	std::string pathToFind,pathToCollect;
	pathToFind=pathRoot;
	pathToFind+="\\*.*";
	HANDLE hFindFile;
	WIN32_FIND_DATAA fd;
	hFindFile=FindFirstFileA(pathToFind.c_str(),&fd);
	if (hFindFile)
	{
		do
		{
			if ((fd.cFileName[0] == '.')&&(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
			{
				if (fd.cFileName[1] =='\0'||(fd.cFileName[1]=='.'&&fd.cFileName[2] == '\0'))
					continue;//ignore the dots
			}
			if (fd.dwFileAttributes&FILE_ATTRIBUTE_HIDDEN)
				continue;//忽略隐藏文件
			std::string pathSub;
			pathSub=pathRoot;
			pathSub=pathSub+"\\"+fd.cFileName;
			if (fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
			{
				if (_FindBGPads(pathSub.c_str(),nm,path))
				{
					bRet=TRUE;
					break;
				}
				continue;
			}
			extern BOOL CheckFileSuffix(const char *fn,const char *suffix);
			if (CheckFileSuffix(pathSub.c_str(),"bgr"))
			{
				StringID nmT=LoadBgName(pathSub.c_str());
				if (nmT==nm)
				{
					path=pathSub;
					bRet=TRUE;
					break;
				}
			}
		}while(FindNextFileA(hFindFile,&fd));
		::FindClose(hFindFile);
	}

	return bRet;
}


BOOL CBehaviorGraphUtil::_FindAndLoadBGPads(const char *pathRoot,StringID nm,CBehaviorGraphPads &pads)
{
	BOOL bRet=FALSE;
	std::string pathToFind,pathToCollect;
	pathToFind=pathRoot;
	pathToFind+="\\*.*";
	HANDLE hFindFile;
	WIN32_FIND_DATAA fd;
	hFindFile=FindFirstFileA(pathToFind.c_str(),&fd);
	if (hFindFile)
	{
		do
		{
			if ((fd.cFileName[0] == '.')&&(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
			{
				if (fd.cFileName[1] =='\0'||(fd.cFileName[1]=='.'&&fd.cFileName[2] == '\0'))
					continue;//ignore the dots
			}
			if (fd.dwFileAttributes&FILE_ATTRIBUTE_HIDDEN)
				continue;//忽略隐藏文件
			std::string pathSub;
			pathSub=pathRoot;
			pathSub=pathSub+"\\"+fd.cFileName;
			if (fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
			{
				if (_FindAndLoadBGPads(pathSub.c_str(),nm,pads))
				{
					bRet=TRUE;
					break;
				}
				continue;
			}
			extern BOOL CheckFileSuffix(const char *fn,const char *suffix);
			if (CheckFileSuffix(pathSub.c_str(),"bgr"))
			{
				if (_LoadBGPads(pathSub.c_str(),nm,pads))
				{
					bRet=TRUE;
					break;
				}
			}
		}while(FindNextFileA(hFindFile,&fd));
		::FindClose(hFindFile);
	}

	return bRet;
}

BOOL CBehaviorGraphUtil::LoadBGPads(StringID nm,CBehaviorGraphPads &pads)
{
	return _FindAndLoadBGPads(_pathRoot.c_str(),nm,pads);
}

const char *CBehaviorGraphUtil::FindBGPadsPath(StringID nm)
{
	static std::string path;
	path="";
	if (TRUE==_FindBGPads(_pathRoot.c_str(),nm,path))
		return path.c_str();
	return "";
}

BOOL CBehaviorGraphUtil::_VerifyBases(CBehaviorGraphPads &pads,std::set<StringID> &existing)
{
	for (int i=0;i<pads._bases.size();i++)
	{
		if (existing.find(pads._bases[i])!=existing.end())
			return FALSE;
		existing.insert(pads._bases[i]);
	}

	CBehaviorGraphPads padsWorking;
	for (int i=0;i<pads._bases.size();i++)
	{
		padsWorking.Clear();
		if (FALSE==LoadBGPads(pads._bases[i],padsWorking))
			continue;

		if (!_VerifyBases(padsWorking,existing))
		{
			return FALSE;
		}
	}

	return TRUE;
}


BOOL CBehaviorGraphUtil::VerifyBases(CBehaviorGraphPads &pads)
{
	std::set<StringID> existing;
	existing.insert(pads.GetName());

	return _VerifyBases(pads,existing);
}

BOOL CBehaviorGraphUtil::VerifyNewBase(CBehaviorGraphPads &pads,StringID nmNewBase)
{
	std::set<StringID> existing;
	existing.insert(pads.GetName());
	existing.insert(nmNewBase);

	return _VerifyBases(pads,existing);
}


BOOL CBehaviorGraphUtil::_MergeBGPads(CBehaviorGraphPads &pads,CBehaviorGraphPads &padsBase)
{
	StringID nmBase=padsBase.GetName();

	std::vector<CLinkPads::LinkPersist> links;
	if (TRUE)
	{
		padsBase._ToPersist();
		links=padsBase._links2;
	}

	//从padsBase里导入pad
	if (TRUE)
	{
		for (int j=0;j<padsBase._pads.size();j++)
		{
			CLinkPad *pad=padsBase._pads[j];
			if (pads.IsPadIncluded(padsBase,pad->GetID()))
			{
				((CBehaviorGraphPad*)pad)->_nmBase=padsBase.GetName();
				pads._pads.push_back(pad);//加进来
				continue;
			}
		}


		pads._ClearIdxCache();

		int c=0;
		for (int j=0;j<padsBase._pads.size();j++)
		{
			CLinkPad *pad=padsBase._pads[j];
			if (pads.FindPad(pad->GetID()))
				continue;//被加入到pads中了
			
			padsBase._pads[c]=pad;
			c++;
		}
		padsBase._pads.resize(c);

		padsBase._ClearIdxCache();
	}



	//重载数据
	if (TRUE)
	{
		CDataPacket dp;
		for (int i=0;i<pads._padsOverride.size();i++)
		{
			CBehaviorGraphPads::Mod_OverridePad *mod=&pads._padsOverride[i];

			CBehaviorGraphPad *pad=(CBehaviorGraphPad *)pads.FindPad(mod->idPad);
			if (pad)
			{
				if (pad->_nmBase!=nmBase)
					continue;
				dp.SetDataBufferPointer(&mod->data[0]);

				std::vector<void*> *ptrsDelta=&pads._lookupDeltePtrs[pad->GetID()];
				pad->GetGObj()->LoadDelta(dp,ptrsDelta);
			}
		}
	}

	//位置
	if (TRUE)
	{
		for (int i=0;i<pads._padsMove.size();i++)
		{
			CBehaviorGraphPads::Mod_MovePad *mod=&pads._padsMove[i];

			CBehaviorGraphPad *pad=(CBehaviorGraphPad *)pads.FindPad(mod->idPad);
			if (pad)
			{
				if (pad->_nmBase!=nmBase)
					continue;
				pad->SetPos(i_math::pos2di(mod->pos.x,mod->pos.y));
			}
		}
	}


	//导入 links
	if (TRUE)
	{
		pads._ToPersist();
		for (int i=0;i<links.size();i++)
		{
			CLinkPads::LinkPersist *link=&links[i];
			if (pads.FindPad(link->idPad[0])&&pads.FindPad(link->idPad[1]))
			{
				pads._links2.push_back(*link);
			}
		}
		for (int i=0;i<pads._linksAdd.size();i++)
		{
			CBehaviorGraphPads::Mod_AddLink &mod=pads._linksAdd[i];

			CBehaviorGraphPad *pad0=(CBehaviorGraphPad *)pads.FindPad(mod.link.idPad[0]);
			CBehaviorGraphPad *pad1=(CBehaviorGraphPad *)pads.FindPad(mod.link.idPad[1]);
			if (pad0&&pad1)
			{
				if ((nmBase==pad0->_nmBase)||(nmBase==pad1->_nmBase))
					pads._links2.push_back(mod.link);
			}
		}

		pads._FromPersist();
	}


	return TRUE;
}

void CBehaviorGraphUtil::_CullBGPads(CBehaviorGraphPads &pads,CBehaviorGraphPads &padsBase)
{
	//先搜集include 的folder
	for(int i=0;i<padsBase._pads.size();i++)
	{
		CBehaviorGraphPad *padBase=(CBehaviorGraphPad*)padsBase._pads[i];

		CBehaviorGraphPad *pad=(CBehaviorGraphPad*)pads.FindPad(padBase->GetID());

		if (pad)
		{
			if (padBase->IsFolder())
			{
				if (padBase->GetFolder()==PadID_Null)
				{
					//顶级folder
					CBehaviorGraphPads::Mod_IncludeFolder mod;
					mod.idPad=padBase->GetID();
					pads._foldersInclude.push_back(mod);
				}
			}
		}
	}

	//搜集exclude的folder
	for(int i=0;i<padsBase._pads.size();i++)
	{
		CBehaviorGraphPad *padBase=(CBehaviorGraphPad*)padsBase._pads[i];

		CBehaviorGraphPad *pad=(CBehaviorGraphPad*)pads.FindPad(padBase->GetID());

		if (!pad)
		{
			//不存在,看看是否自己属于的folder被include了
			for(int j=0;j<pads._foldersInclude.size();j++)
			{
				if (padsBase.IsInFolder(padBase->GetID(),pads._foldersInclude[j].idPad))
				{
					CBehaviorGraphPads::Mod_ExcludePad mod;
					mod.idPad=padBase->GetID();
					pads._folderExclude.push_back(mod);
				}
			}
		}
	}

	//搜集位置移动
	for(int i=0;i<padsBase._pads.size();i++)
	{
		CBehaviorGraphPad *padBase=(CBehaviorGraphPad*)padsBase._pads[i];

		CBehaviorGraphPad *pad=(CBehaviorGraphPad*)pads.FindPad(padBase->GetID());

		if (pad)
		{
			if (pad->_pt!=padBase->_pt)
			{
				CBehaviorGraphPads::Mod_MovePad mod;
				mod.pos=pad->_pt;
				mod.idPad=pad->GetID();
				pads._padsMove.push_back(mod);
			}
		}
	}

	//搜集数据重载
	for(int i=0;i<padsBase._pads.size();i++)
	{
		CBehaviorGraphPad *padBase=(CBehaviorGraphPad*)padsBase._pads[i];

		CBehaviorGraphPad *pad=(CBehaviorGraphPad*)pads.FindPad(padBase->GetID());

		if (pad)
		{
			if (pad->GetClass()->IsSameWith(padBase->GetClass()))
			{
				if (!pad->GetGObj()->Equals(padBase->GetGObj()))
				{
					pads._padsOverride.resize(pads._padsOverride.size()+1);
					CBehaviorGraphPads::Mod_OverridePad *mod=&pads._padsOverride[pads._padsOverride.size()-1];
					DP_BeginSave(dp,mod->data);
					pad->GetGObj()->SaveDelta(dp,padBase->GetGObj());
					DP_EndSave();

					mod->idPad=pad->GetID();
				}
			}
		}
	}

	pads._ToPersist();

	//清除属于Base的link,搜集连接Base的link
	if (TRUE)
	{
		int c=0;
		for (int i=0;i<pads._links2.size();i++)
		{
			BOOL bExist0=padsBase.FindPad(pads._links2[i].idPad[0])!=NULL;
			BOOL bExist1=padsBase.FindPad(pads._links2[i].idPad[1])!=NULL;
			if (bExist0&&bExist1)
				continue;//discard it
			if (bExist0||bExist1)
			{
				CBehaviorGraphPads::Mod_AddLink mod;
				mod.link=pads._links2[i];
				pads._linksAdd.push_back(mod);
				continue;
			}
			pads._links2[c]=pads._links2[i];
			c++;
		}

		pads._links2.resize(c);
	}

	//清除属于Base的pad
	if (TRUE)
	{
		int c=0;
		for (int i=0;i<pads._pads.size();i++)
		{
			if (padsBase.FindPad(pads._pads[i]->GetID()))
			{
				pads._pads[i]->Clear();
				Safe_Class_Delete(pads._pads[i]);
				continue;//discard it
			}
			pads._pads[c]=pads._pads[i];
			c++;
		}

		pads._pads.resize(c);
		pads._ClearIdxCache();
	}

	pads._FromPersist();

}


BOOL CBehaviorGraphUtil::_ResolveBGPads(CBehaviorGraphPads &pads)
{

	CBehaviorGraphPads padsWorking;
	for (int i=0;i<pads._bases.size();i++)
	{
		padsWorking.Clear();
		if (FALSE==LoadBGPads(pads._bases[i],padsWorking))
			continue;

		if (!_ResolveBGPads(padsWorking))
			return FALSE;

		_MergeBGPads(pads,padsWorking);
	}

	pads._bResolved=TRUE;

	return TRUE;
}



BOOL CBehaviorGraphUtil::ResolveBGPads(CBehaviorGraphPads &pads)
{
	if (pads.IsResolved())
		return TRUE;

	if (!VerifyBases(pads))
		return FALSE;

	pads._lookupDeltePtrs.clear();

	BOOL bRet=_ResolveBGPads(pads);
	pads.ValidateFolders();

	if (TRUE)
	{
		for (int i=0;i<pads._padsOverride.size();i++)
		{
			CBehaviorGraphPad *pad=(CBehaviorGraphPad *)pads.FindPad(pads._padsOverride[i].idPad);
			if (pad)
				pad->_bOverriden=1;
		}
	}


	return bRet;
}

BOOL CBehaviorGraphUtil::UnresolveBGPads(CBehaviorGraphPads &pads)
{
	if (!pads.IsResolved())
		return TRUE;

	if (!VerifyBases(pads))
		return FALSE;

	std::vector<CBehaviorGraphPads *>workings;
	for (int i=0;i<pads._bases.size();i++)
	{
		CBehaviorGraphPads *padsWorking=Class_New2(CBehaviorGraphPads);
		if (FALSE==LoadBGPads(pads._bases[i],*padsWorking))
			break;

		if (!_ResolveBGPads(*padsWorking))
			break;

		workings.push_back(padsWorking);
	}

	BOOL bRet=FALSE;
	if (workings.size()>=pads._bases.size())
	{
		//Base全部载入并成功Resolve了
		bRet=TRUE;
		pads.ClearMods();//清除所有的mods
		for (int i=0;i<workings.size();i++)
		{
			CBehaviorGraphPads *padsWorking=workings[i];

			_CullBGPads(pads,*padsWorking);
		}

		pads._bResolved=FALSE;
	}

	for (int i=0;i<workings.size();i++)
	{
		Safe_Class_Delete(workings[i]);
	}

	workings.clear();

	return bRet;
}

BOOL CBehaviorGraphUtil::Node::IsRoot()
{
	if (candidatesParent.size()<=0)
		return TRUE;
	if (pad)
	{
		if (pad->IsFolder())
			return TRUE;
	}
	return FALSE;
}

void CBehaviorGraphUtil::_DispatchDepth(Node *node,int &depthMax)
{
	int depth=node->depth;
	depth++;

	for (int i=0;i<node->childs.size();i++)
	{
		Node *nodeChild=node->childs[i];
		if (nodeChild->bLockRecursive)
			continue;

		if (depth>nodeChild->depth)
		{
			nodeChild->depth=depth;
			nodeChild->parent=node;

			if (depth>depthMax)
				depthMax=depth;
		}

		node->bLockRecursive=TRUE;
		if (!nodeChild->IsRoot())
			_DispatchDepth(nodeChild,depthMax);
		node->bLockRecursive=FALSE;
	}
}

void CBehaviorGraphUtil::_Deposit(Node *node,std::vector<std::vector<Node*> >&nodesDepth)
{
	for (int i=0;i<node->childs.size();i++)
	{
		Node *nodeChild=node->childs[i];

		if (nodeChild->parent!=node)
			continue;
		if (nodeChild->bLockRecursive)
			continue;

		if (nodeChild->depth<nodesDepth.size())
			nodesDepth[nodeChild->depth].push_back(nodeChild);

		node->bLockRecursive=TRUE;
		if (!nodeChild->IsRoot())
			_Deposit(nodeChild,nodesDepth);
		node->bLockRecursive=FALSE;
	}

}

float CBehaviorGraphUtil::_CalcPadHeight(CBehaviorGraphPad *pad,FillDescAssist *assist)
{
	static std::string s;
	s="";
	pad->FillDesc(s,assist);

	int nLines=1;
	int nIn=0,nOut=0;
	for (int i=0;i<pad->GetStubCount();i++)
	{
		PadStub stb=pad->GetStub(i);
		if (stb.type==PadStub_In)
			nIn++;
		if (stb.type==PadStub_Out)
			nOut++;
	}

	if (!pad->IsFolder())
	{
		if (nIn>nOut)
			nLines+=nIn;
		else
			nLines+=nOut;
	}
	else
		nLines+=nIn;

	nLines++;//title

	return 20.0f*(float)(nLines)+assist->CalcHeight(s.c_str());
}

#define REPOS_LINEGAP 40.0f
#define REPOS_COLUMNGAP 300.0f

void CBehaviorGraphUtil::_LocateParent(std::vector<Node*>&nodesChild)
{
	if (nodesChild.size()<=0)
		return;
	Node *parent=nodesChild[0]->parent;
	float yStart=nodesChild[0]->yAbs;
	float height=0.0f;
	for (int i=0;i<nodesChild.size();i++)
	{
		if (i>0)
			height+=REPOS_LINEGAP;
		height+=nodesChild[i]->height;
	}

	parent->yAbs=yStart+height/2.0f-parent->height/2.0f;

	for (int i=0;i<nodesChild.size();i++)
	{
		nodesChild[i]->yLocal=nodesChild[i]->yAbs-parent->yAbs;
		nodesChild[i]->yAbs=-1.0f;
	}
}

void CBehaviorGraphUtil::_LocateChilds(Node *node)
{
	i_math::pos2di pos=node->pad->GetPos();
	if (node->pad->IsFolder())
		pos=node->pad->GetFolderPos();

	for (int i=0;i<node->childs.size();i++)
	{
		Node *child=node->childs[i];
		if (child->parent!=node)
			continue;
		if (child->bLockRecursive)
			continue;

		i_math::pos2di posChild;
		posChild.x=pos.x+(int)REPOS_COLUMNGAP;
		posChild.y=pos.y+(int)child->yLocal;

		child->pad->SetPos(posChild);

		node->bLockRecursive=TRUE;
		if (!child->IsRoot())
			_LocateChilds(child);
		node->bLockRecursive=FALSE;



	}
}



void CBehaviorGraphUtil::Repos(CBehaviorGraphPads &pads,FillDescAssist *assist)
{
	std::unordered_map<PadID,Node*> lookup;
	if (TRUE)
	{
		for (int i=0;i<pads._pads.size();i++)
		{
			CBehaviorGraphPad *pad=(CBehaviorGraphPad *)pads._pads[i];
			if (pad)
			{
				Node *node=_poolNode.Alloc();
				node->pad=pad;
				_nodes.push_back(node);
				lookup[pad->GetID()]=node;

				node->height=_CalcPadHeight(pad,assist);
			}
		}
	}

	//建立parent-child连接
	for (int i=0;i<pads._links.size();i++)
	{
		CLinkPads::Link &link=pads._links[i];

		CBehaviorGraphPad *parent=NULL;
		CBehaviorGraphPad *child=NULL;
		int idxChild=-1;

		if (TRUE)
		{
			CBehaviorGraphPad *pad0=(CBehaviorGraphPad *)pads._pads[link.iPad[0]];
			CBehaviorGraphPad *pad1=(CBehaviorGraphPad *)pads._pads[link.iPad[1]];

			PadStub stb0=pad0->GetStub(link.iStub[0]);
			PadStub stb1=pad1->GetStub(link.iStub[1]);


			if ((stb0.type==PadStub_Out)&&(stb1.type==PadStub_In))
			{
				parent=pad0;
				child=pad1;
				idxChild=link.iStub[0];
			}
			if ((stb1.type==PadStub_Out)&&(stb0.type==PadStub_In))
			{
				parent=pad1;
				child=pad0;
				idxChild=link.iStub[1];
			}
		}

		if ((parent==NULL)||(child==NULL))
			continue;

		assert(lookup.find(parent->GetID())!=lookup.end());
		assert(lookup.find(child->GetID())!=lookup.end());

		Node *nodeParent=lookup[parent->GetID()];
		Node *nodeChild=lookup[child->GetID()];

		//不能重复加入child
		if (TRUE)
		{
			int i;
			for (i=0;i<nodeParent->childs.size();i++)
			{
				if (nodeChild==nodeParent->childs[i])
					break;
			}
			if (i<nodeParent->childs.size())
				continue;
		}

		//插入child
		if (TRUE)
		{
			BOOL bInserted=FALSE;
			for (int i=0;i<nodeParent->indicesChilds.size();i++)
			{
				if (idxChild<nodeParent->indicesChilds[i])
				{
					nodeParent->indicesChilds.insert(nodeParent->indicesChilds.begin()+i,idxChild);
					nodeParent->childs.insert(nodeParent->childs.begin()+i,nodeChild);
					bInserted=TRUE;
					break;
				}
			}

			if (!bInserted)
			{
				nodeParent->indicesChilds.push_back(idxChild);
				nodeParent->childs.push_back(nodeChild);
			}
		}

		nodeChild->candidatesParent.push_back(nodeParent);
	}

	int depthMax=0;
	std::vector< std::vector<Node*> > nodesDepth;
	std::vector< Node*> nodesChilds;
	for (int i=0;i<_nodes.size();i++)
	{
		Node *node=_nodes[i];
		if (node->IsRoot())
		{
			//更新各个node的depth
			_DispatchDepth(node,depthMax);

			//将node放置到各个depth中去
			nodesDepth.resize(depthMax+1);
			_Deposit(node,nodesDepth);

			for (int j=nodesDepth.size()-1;j>=0;j--)
			{
				//排列这个depth上的node
				std::vector<Node*> &nodes=nodesDepth[j];

				float yAbs=0.0f;

				for (int k=0;k<nodes.size();k++)
				{
					Node *node=nodes[k];

					if (node->yAbs>yAbs)
						yAbs=node->yAbs;
					else
					{
						float delta=yAbs-node->yAbs;
						if (!(node->yAbs>=0.0f))
							delta=0.0f;
						node->yAbs=yAbs;
						if (delta>0.0f)
						{
							for (int ii=k+1;ii<nodes.size();ii++)
							{
								if (nodes[ii]->yAbs>=0.0f)
									nodes[ii]->yAbs+=delta;
							}
						}
					}

					yAbs+=node->height;
					yAbs+=REPOS_LINEGAP;
				}

				if (j>0)
				{
					Node *nodeParent=NULL;
					float yStart=0.0f;
					for (int k=0;k<nodes.size();k++)
					{
						Node *node=nodes[k];

						assert(node->parent);

						if ((nodeParent)&&(node->parent!=nodeParent))
						{
							_LocateParent(nodesChilds);
							nodesChilds.clear();
							nodeParent=NULL;
						}

						if (nodeParent==NULL)
						{
							nodeParent=node->parent;
							nodesChilds.push_back(node);
							continue;
						}
						if (nodeParent==node->parent)
						{
							nodesChilds.push_back(node);
							continue;
						}
					}
					_LocateParent(nodesChilds);
					nodesChilds.clear();
					nodeParent=NULL;
				}
			}

			nodesDepth.clear();

			_LocateChilds(node);
		}
	}

	_poolNode.Reset(FALSE);
	_nodes.clear();
	lookup.clear();
}
