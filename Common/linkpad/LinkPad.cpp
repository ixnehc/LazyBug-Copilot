/********************************************************************
	created:	12:4:2010   14:23
	file path:	d:\IxEngine\Common\linkpad
	author:		chenxi
	
	purpose:	linkable pad
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"
#include "LinkPad.h"

#include "../datapacket/DataPacket.h"

#include "../class/class.h"
#include "../gds/GObj.h"

#include "../Log/LogDump.h"


//////////////////////////////////////////////////////////////////////////
//CLinkPad
void CLinkPad::Clear()
{
	_ClearIdxCache();
}

void CLinkPad::_BuildIdxCache()
{
	if (_idxmap.size()>0)
		return;//already built

	DWORD c=GetStubCount();

	PadStub t;
	for (int i=0;i<c;i++)
	{
		t=GetStub(i);
		_idxmap[std::string(t.name)]=i;
	}
}

void CLinkPad::_ClearIdxCache()
{
	_idxmap.clear();
}


//////////////////////////////////////////////////////////////////////////
//CLinkPads
std::vector<CLinkPads::LinkPersist>CLinkPads::_links2;

void CLinkPads::Clear()
{
	for (int i=0;i<_pads.size();i++)
	{
		Safe_Class_Delete(_pads[i]);
	}
	_pads.clear();
	_links.clear();
	_ClearIdxCache();
}


void CLinkPads::_BuildIdxCache()
{
	if (_idxmap.size()>0)
		return;//already built

	for (int i=0;i<_pads.size();i++)
		_idxmap[_pads[i]->_id]=i;
}

void CLinkPads::_ClearIdxCache()
{
	_idxmap.clear();
}

int CLinkPads::FindPadIdx(PadID id)
{
	_BuildIdxCache();
	std::unordered_map<PadID,short>::iterator it=_idxmap.find(id);
	if (it==_idxmap.end())
		return -1;

	return (*it).second;
}

CLinkPad *CLinkPads::FindPad(PadID id)
{
	int idx=FindPadIdx(id);
	if (idx==-1)
		return NULL;
	assert(_pads[idx]->GetID()==id);
	return _pads[idx];
}

CLinkPad *CLinkPads::GetPad(int idxPad)
{
	if ((DWORD)idxPad<_pads.size())
		return _pads[idxPad];
	return NULL;
}



int CLinkPads::FindStubIdx(int iPad,const char *stub)
{
	if (((DWORD)iPad)>=_pads.size())
		return -1;
	CLinkPad *pad=_pads[iPad];
	pad->_BuildIdxCache();
	std::unordered_map<std::string,short>::iterator it=pad->_idxmap.find(std::string(stub));
	if (it==pad->_idxmap.end())
		return -1;
	return (*it).second;
}

#define CUR_VER 4
void CLinkPads::Save(CDataPacket &dp)
{
	dp.Data_NextByte()=CUR_VER;
	dp.Data_NextByte()=_CalcClassCode();

	DP_WriteVar(dp,_idSeed);

	//先保存pads
	dp.Data_NextWord()=_pads.size();
	for (int i=0;i<_pads.size();i++)
	{
		CLinkPad *pad=_pads[i];
		DP_PreSafeSave(dp);
		WORD uid=0;
		if (_clsses)
			uid=_clsses->UIDFromClassName(pad->GetClass()->GetName());
		if (uid<GOBJ_UID_START)
			dp.Data_WriteString(pad->GetClass()->GetName());
		else
			dp.Data_NextDword()=(DWORD)uid;
		DP_WriteVar(dp,pad->_id);
		DP_WriteVar(dp,pad->_pt);
		DP_WriteVar(dp,pad->_idFolder);
		DP_WriteVar(dp,pad->_ptFolder);
		DP_WriteVar(dp,pad->_bFolder);
		dp.Data_WriteString(pad->_nameFolder.c_str());
		GObjBase *gobj=pad->GetGObj();
		SaveGObj(dp,gobj);
		DP_PostSafeSave();
	}

	//然后保存links
	_ToPersist();
	DP_WriteVector(dp,_links2);

	DP_WriteVector(dp,_stackFolder);

	DP_WriteVector(dp,_xfmsFolder);
}

PadID CLinkPads::_ReadShortPadID(CDataPacket &dp)
{
	PadID_Short id;
	DP_ReadVar(dp,id);
	return (PadID)id;
}


void CLinkPads::Load(CDataPacket &dp,BOOL &bLongPadID)
{
	Clear();

	BYTE ver=dp.Data_NextByte();

	bLongPadID=TRUE;
	if (ver<4)
		bLongPadID=FALSE;

	BYTE cc=dp.Data_NextByte();

	if (ver>=3)
	{
		if (bLongPadID)
		{
			DP_ReadVar(dp,_idSeed);
		}
		else
			_idSeed=_ReadShortPadID(dp);
	}


	BOOL bSomethingWrong=FALSE;
	//先载入pads
	if (TRUE)
	{
		DWORD c=0;
		std::string s;
		_pads.resize(dp.Data_NextWord());
		for (int i=0;i<_pads.size();i++)
		{
			CLinkPad *pad=NULL;

			DP_PreSafeLoad(dp);

			DWORD uid=0;
			s="";
			DWORD *p=(DWORD*)dp.GetCurBufferPointer();
			if (*p>=GOBJ_UID_START)
				uid=dp.Data_NextDword();
			else
				dp.Data_ReadString(s);

			CLinkPad *pad=NULL;
			if (_clsses)
			{
				if (uid>=GOBJ_UID_START)
					pad=_clsses->New((WORD)uid);
				else
					pad=_clsses->New(s.c_str());
			}
			else
				pad=(CLinkPad *)CClass::New(s.c_str());
			if (pad)
			{
				_pads[c]=pad;
				c++;

				if (bLongPadID)
				{
					DP_ReadVar(dp,pad->_id);
				}
				else
					pad->_id=_ReadShortPadID(dp);

				DP_ReadVar(dp,pad->_pt);
				if (ver<=1)
				{
					pad->_idFolder=PadID_Null;
					pad->_ptFolder=pad->_pt;
					pad->_bFolder=0;
				}
				else
				{
					if (bLongPadID)
					{
						DP_ReadVar(dp,pad->_idFolder);
					}
					else
						pad->_idFolder=_ReadShortPadID(dp);

					DP_ReadVar(dp,pad->_ptFolder);
					DP_ReadVar(dp,pad->_bFolder);
					dp.Data_ReadString(pad->_nameFolder);
				}
				GObjBase *gobj=pad->GetGObj();
				LoadGObj(dp,gobj,NULL);
			}
			else
				bSomethingWrong=TRUE;
				
			DP_PostSafeLoad();
		}
		_pads.resize(c);
	}

	//再载入links
	if (bLongPadID)
	{
		DP_ReadVector(dp,_links2);
	}
	else
	{
		//Read from Short Links
		std::vector<LinkPersist_Short> links2_Short;
		DP_ReadVector(dp,links2_Short);

		_links2.resize(links2_Short.size());
		for (int i=0;i<_links2.size();i++)
		{
			_links2[i].From(links2_Short[i]);
		}
	}
	_FromPersist();

	//folder栈
	if (ver>1)
	{
		if (bLongPadID)
		{
			DP_ReadVector(dp,_stackFolder);
			DP_ReadVector(dp,_xfmsFolder);
		}
		else
		{
			std::vector<PadID_Short> stackFolder_Short;
			DP_ReadVector(dp,stackFolder_Short);
			_stackFolder.resize(stackFolder_Short.size());
			for (int i=0;i<_stackFolder.size();i++)
				_stackFolder[i]=(PadID)stackFolder_Short[i];

			std::vector<FolderXfm_Short> xfmsFolder_Short;
			DP_ReadVector(dp,xfmsFolder_Short);
		}
	}

	if (bSomethingWrong)
		_ValidateFolders();

	if (ver<3)
	{
		_idSeed=1;
		for (int i=0;i<_pads.size();i++)
		{
			if (_pads[i]->GetID()>_idSeed)
				_idSeed=_pads[i]->GetID();
		}
	}
}

CLinkPad **CLinkPads::GetPads(DWORD &n)
{
	n=_pads.size();
	return _pads.data();
}

CLinkPads::Link *CLinkPads::GetLinks(DWORD &n)
{
	n=_links.size();
	return _links.data();
}

CLinkPads::LinkPersist*CLinkPads::GetPersistLinks(DWORD &n)
{
	_ToPersist();
	n=_links2.size();
	return _links2.data();
}

PadID CLinkPads::GenPadID()
{
	std::unordered_map<PadID,int> t;
	for (int i=0;i<_pads.size();i++)
		t[_pads[i]->_id]=1;

	PadID id=_idSeed;
	_idSeed++;
	while(1)
	{
		if (t.find(id)==t.end())
			return id;
		id++;
	}
	return id;//到不了这

}

void CLinkPads::RemoveLink(PadID id,const char *stub)
{
	int iPad=FindPadIdx(id);
	int iStub=FindStubIdx(iPad,stub);
	if (iStub==-1)
		return;

	DWORD c=0;
	for (int i=0;i<_links.size();i++)
	{
		Link *link=&_links[i];
		if ((link->iPad[0]==iPad)&&(link->iStub[0]==iStub))
			continue;
		if ((link->iPad[1]==iPad)&&(link->iStub[1]==iStub))
			continue;
		_links[c]=*link;
		c++;
	}
	_links.resize(c);

}

BOOL CLinkPads::AddLink(PadID idSrc,const char *stubSrc,PadID idTarget,const char *stubTarget)
{
	Link link;
	link.iPad[0]=FindPadIdx(idSrc);
	link.iStub[0]=FindStubIdx(link.iPad[0],stubSrc);
	link.iPad[1]=FindPadIdx(idTarget);
	link.iStub[1]=FindStubIdx(link.iPad[1],stubTarget);
	if ((link.iStub[0]==-1)||(link.iStub[1]==-1))
		return FALSE;

	//检查single link
	for (int i=0;i<2;i++)
	{
		PadStub stb=_pads[link.iPad[i]]->GetStub(link.iStub[i]);
		if (stb.bSingleLink)
		{
			if (i==0)
				RemoveLink(idSrc,stubSrc);
			else
				RemoveLink(idTarget,stubTarget);
		}
	}

	UNIQUE_VEC_ADD(_links,link);
	return TRUE;
}

void CLinkPads::_ToPersist()
{
	_links2.resize(_links.size());
	for (int i=0;i<_links.size();i++)
	{
		Link *link=&_links[i];
		LinkPersist *link2=&_links2[i];
		for (int j=0;j<2;j++)
		{
			link2->idPad[j]=_pads[link->iPad[j]]->_id;
			const char *nm=_pads[link->iPad[j]]->GetStub(link->iStub[j]).name;
			if (strlen(nm)>=MAX_PADSTUB_NAME)
			{
				LOG_DUMP_2P("CLinkPads",Log_Error,"Stub名称长度(\"%s\")超过%d!",nm,MAX_PADSTUB_NAME-1);
				continue;
			}
			strcpy(link2->nameStub[j],nm);
		}
	}
}

void CLinkPads::_FromPersist()
{
	_links.resize(_links2.size());
	DWORD c=0;

	for (int i=0;i<_links.size();i++)
	{
		Link *link=&_links[i];
		LinkPersist *link2=&_links2[i];
		for (int j=0;j<2;j++)
		{
			link->iPad[j]=FindPadIdx(link2->idPad[j]);
			link->iStub[j]=FindStubIdx(link->iPad[j],link2->nameStub[j]);
		}
		if ((link->iStub[0]==-1)||(link->iStub[1]==-1))
			continue;
		_links[c]=*link;
		c++;
	}
	_links.resize(c);
}


void CLinkPads::RemovePads(PadID *ids,DWORD count,BOOL bRemoveSubs)
{
	_ToPersist();

	std::vector<PadID> removes;
	VEC_APPEND_BUFFER(removes,ids,count);

	//搜集subs
	if (bRemoveSubs)
	{
		std::vector<PadID>subs;
		for (int i=0;i<count;i++)
		{
			CLinkPad *pad=FindPad(ids[i]);
			if (pad)
			{
				if ((pad->IsFolder())&&(pad->GetID()!=GetCurFolder()))
				{
					_GetFolderSubs(ids[i],subs);
					for (int j=0;j<subs.size();j++)
						UNIQUE_VEC_ADD(removes,subs[j]);
				}
			}
		}
	}

	//把要删的全UnFold
	for (int i=0;i<count;i++)
	{
		if (IsFolder(ids[i]))
			UnFold(ids[i]);
	}

	DWORD c=0;
	for (int i=0;i<_pads.size();i++)
	{
		PadID id=_pads[i]->GetID();
		for (int j=0;j<removes.size();j++)
		{
			if (removes[j]==id)
			{
				Safe_Class_Delete(_pads[i]);
			}
		}
		if (_pads[i])
		{
			_pads[c]=_pads[i];
			c++;
		}
	}
	_pads.resize(c);
	_ClearIdxCache();

	_FromPersist();

	_ValidateFolders();
}

PadID CLinkPads::_NewPad(CLinkPad *pad,i_math::pos2di &pt)
{
	pad->_id=GenPadID();
	pad->_pt.set((short)pt.x,(short)pt.y);
	pad->_ptFolder=pad->_pt;
	pad->_idFolder=GetCurFolder();

	_pads.push_back(pad);
	_ClearIdxCache();
	return pad->_id;
}


PadID CLinkPads::NewPad(const char *clssname,i_math::pos2di &pt)
{
	CLinkPad *pad=NULL;
	if (_clsses)
		pad=_clsses->New(clssname);
	else
		pad=(CLinkPad *)CClass::New(clssname);
	if (!pad)
		return PadID_Null;
	return _NewPad(pad,pt);
}

void CLinkPads::PreModify(PadID id)	
{		
	_ToPersist();

	CLinkPad *pad=FindPad(id);
	if (pad)
		pad->_ClearIdxCache();

	_ClearIdxCache();	
}



void CLinkPads::PushFolder(PadID id)
{
	CLinkPad *pad=FindPad(id);
	if (pad)
	{
		if (pad->IsFolder())
			_stackFolder.push_back(id);
	}
}

void CLinkPads::PopFolder()
{
	if (_stackFolder.size()>0)
		_stackFolder.pop_back();
}

void CLinkPads::PopToFolder(PadID id)
{
	for (int i=0;i<_stackFolder.size();i++)
	{
		if (_stackFolder[i]==id)
		{
			_stackFolder.resize(i+1);
			return;
		}
	}
}


PadID CLinkPads::GetCurFolder()
{
	if (_stackFolder.size()<=0)
		return PadID_Null;
	return _stackFolder[_stackFolder.size()-1];
}

void CLinkPads::_ValidateFolders()
{
	for (int i=0;i<_pads.size();i++)
	{
		CLinkPad *pad=_pads[i];
		if (pad->IsFolder())
		{
			if (!CanFold(pad->GetID()))
				UnFold(pad->GetID());
		}
	}

	//检查各个pad的idFolder是否有效
	for (int i=0;i<_pads.size();i++)
	{
		CLinkPad *pad=_pads[i];
		if (pad->_idFolder!=PadID_Null)
		{
			if (FindPadIdx(pad->_idFolder)==-1)
				pad->_idFolder=PadID_Null;
		}
	}

	//从栈中清除已经不是folder的pad
	int c=0;
	for (int i=0;i<_stackFolder.size();i++)
	{
		CLinkPad *pad=FindPad(_stackFolder[i]);
		if (pad)
		{
			if (pad->IsFolder())
			{
				_stackFolder[c]=_stackFolder[i];
				c++;
			}
		}
	}
	_stackFolder.resize(c);

	c=0;
	for (int i=0;i<_xfmsFolder.size();i++)
	{
		if (_xfmsFolder[i].id!=PadID_Null)
		{
			CLinkPad *pad=FindPad(_xfmsFolder[i].id);
			if (pad)
			{
				if (pad->IsFolder())
				{
					_xfmsFolder[c]=_xfmsFolder[i];
					c++;
				}
			}
		}
		else
		{
			_xfmsFolder[c]=_xfmsFolder[i];
			c++;
		}
	}
	_xfmsFolder.resize(c);
}

BOOL CLinkPads::_GetLinkSubs(PadID id,std::unordered_set<int>&subs,std::unordered_set<int> *links)
{
	subs.clear();
	if (links)
		links->clear();
	int idx=FindPadIdx(id);
	if (idx==-1)
		return FALSE;

	subs.insert(idx);
	while(1)
	{
		BOOL bNewAdded=FALSE;
		for (int i=0;i<_links.size();i++)
		{
			int idxSrc=_links[i].iPad[0];
			int idxTarget=_links[i].iPad[1];
			if (_FoldLinkSrc())
				Swap(idxSrc,idxTarget);

			if (subs.find(idxSrc)!=subs.end())
			{
				if (links)
					links->insert(i);//加入link
				if (subs.find(idxTarget)==subs.end())
				{//原来不存在,新增一个
					subs.insert(idxTarget);
					bNewAdded=TRUE;
				}
			}
		}
		if (!bNewAdded)
			break;
	}

	return TRUE;	
}

BOOL CLinkPads::_IsFolderSub(PadID id,PadID idSub)
{
	if (id==idSub)
		return FALSE;
	CLinkPad *pad;
	while(idSub!=PadID_Null)
	{
		pad=FindPad(idSub);
		if (!pad)
			return FALSE;
		idSub=pad->_idFolder;
		if (idSub==id)
			return TRUE;
	}
	return FALSE;
}

BOOL CLinkPads::_GetFolderSubs(PadID id,std::vector<PadID> &subs)
{
	subs.clear();
	for (int i=0;i<_pads.size();i++)
	{
		if (_IsFolderSub(id,_pads[i]->GetID()))
			subs.push_back(_pads[i]->GetID());
	}
	return TRUE;
}



BOOL CLinkPads::CanFold(PadID id)
{
	std::unordered_set<int> subs;
	std::unordered_set<int> links;

	if (FALSE==_GetLinkSubs(id,subs,&links))
		return FALSE;

	int idx=FindPadIdx(id);

	//检查连到这些subs的link是不是都在已经枚举到的links中
	for (int i=0;i<_links.size();i++)
	{
		int idxSrc=_links[i].iPad[0];
		int idxTarget=_links[i].iPad[1];
		if (_FoldLinkSrc())
			Swap(idxSrc,idxTarget);

		if (idxTarget==idx)
			continue;
		if (subs.find(idxTarget)!=subs.end())
		{
			if (links.find(i)==links.end())
				return FALSE;//这个link不在枚举到的link中,
		}
	}
	return TRUE;
}

BOOL CLinkPads::Fold(PadID id)
{
	CLinkPad *pad=FindPad(id);
	if (!pad)
		return FALSE;
	if (pad->IsFolder())
		return TRUE;

	std::unordered_set<int> subs;
	if (FALSE==_GetLinkSubs(id,subs,NULL))
		return FALSE;

	std::unordered_set<int>::iterator it;
	for (it=subs.begin();it!=subs.end();it++)
	{
		CLinkPad *p=_pads[*it];
		if (p!=pad)
		{
			if (p->_idFolder==pad->_idFolder)//只fold同一级的
				p->_idFolder=id;
		}
	}
	pad->_ptFolder=pad->_pt;
	pad->_bFolder=1;

	return TRUE;
}

BOOL CLinkPads::UnFold(PadID id)
{
	CLinkPad *pad=FindPad(id);
	if (!pad)
		return FALSE;
	if (!pad->IsFolder())
		return TRUE;

	for (int i=0;i<_pads.size();i++)
	{
		CLinkPad *p=_pads[i];
		if (p!=pad)
		{
			if (p->_idFolder==id)
			{
				p->_idFolder=pad->_idFolder;
				p->_pt=p->_pt-pad->_ptFolder+pad->_pt;
			}
		}
	}

	pad->_bFolder=0;

	_ValidateFolders();
	return TRUE;
}


PadID *CLinkPads::GetInFolders(PadID idFolder,DWORD &count)
{
	count=0;
	_temp.clear();

	for (int i=0;i<_pads.size();i++)
	{
		CLinkPad *p=_pads[i];
		if (p->_id!=idFolder)
		{
			if (p->_idFolder==idFolder)
				_temp.push_back(p->_id);
		}
	}
	count=_temp.size();
	return _temp.data();
}

BOOL CLinkPads::IsFolder(PadID id)
{
	CLinkPad *pad=FindPad(id);
	if (pad)
		return pad->IsFolder();
	return FALSE;
}


BOOL CLinkPads::GetFolderXfm(PadID id,i_math::pos2df &off,i_math::pos2df &scale)
{
	off.set(0,0);
	scale.set(1,1);
	int idx;
	VEC_FIND_BY_ELEMENT(_xfmsFolder,id,id,idx);
	if (idx==-1)
		return FALSE;
	off=_xfmsFolder[idx].off;
	scale=_xfmsFolder[idx].scale;
	return TRUE;
}

BOOL CLinkPads::SetFolderXfm(PadID id,i_math::pos2df &off,i_math::pos2df &scale)
{
	int idx;
	VEC_FIND_BY_ELEMENT(_xfmsFolder,id,id,idx);
	if (idx==-1)
	{
		if (id!=PadID_Null)
		{
			CLinkPad *pad=FindPad(id);
			if (!pad)
				return FALSE;
			if (!pad->IsFolder())
				return FALSE;
		}

		FolderXfm t;
		t.id=id;
		_xfmsFolder.push_back(t);
		idx=_xfmsFolder.size()-1;
	}

	_xfmsFolder[idx].off=off;
	_xfmsFolder[idx].scale=scale;

	return TRUE;
}

PadID *CLinkPads::GetFolderSubs(PadID id,DWORD &count)
{
	_GetFolderSubs(id,_temp);
	count=_temp.size();
	return _temp.data();
}

void CLinkPads::ClearFolderXfm()
{
	_xfmsFolder.clear();
}

void CLinkPads::RefreshPadIDs()
{
	_ClearIdxCache();
	std::unordered_map<PadID,PadID> remap;
	for (int i=0;i<_pads.size();i++)
	{
		PadID idOld=_pads[i]->_id;
		_pads[i]->_id=GenPadID();
		remap[idOld]=_pads[i]->_id;
	}

	for (int i=0;i<_pads.size();i++)
	{
		PadID id=_pads[i]->_idFolder;
		std::unordered_map<PadID,PadID>::iterator it=remap.find(id);
		if (it!=remap.end())
		{
			_pads[i]->_idFolder=(*it).second;
		}
		else
			_pads[i]->_idFolder=PadID_Null;
	}


	for (int i=0;i<_stackFolder.size();i++)
	{
		PadID id=_stackFolder[i];
		std::unordered_map<PadID,PadID>::iterator it=remap.find(id);
		if (it!=remap.end())
			_stackFolder[i]=(*it).second;
		else
		{
			_stackFolder.clear();//invalid stack,clear it
			break;
		}
	}


	if (TRUE)
	{
		int c=0;
		for (int i=0;i<_xfmsFolder.size();i++)
		{
			PadID id=_xfmsFolder[i].id;
			std::unordered_map<PadID,PadID>::iterator it=remap.find(id);
			if (it!=remap.end())
			{
				_xfmsFolder[i].id=(*it).second;
				_xfmsFolder[c]=_xfmsFolder[i];
				c++;
			}
		}
		_xfmsFolder.resize(c);
	}

}

BOOL CLinkPads::IsInFolder(PadID idPad,PadID idFolder)
{
	return GetInFolderDepth(idPad,idFolder)>=0;
}

int CLinkPads::GetInFolderDepth(PadID idPad,PadID idFolder)
{
	int depth=0;
	while(idPad!=PadID_Null)
	{
		if (idPad==idFolder)
			return depth;
		CLinkPad *pad=FindPad(idPad);
		if (!pad)
			return -1;
		idPad=pad->GetFolder();
		depth++;
	}
	if (idFolder==PadID_Null)
		return depth;
	return -1;
}

