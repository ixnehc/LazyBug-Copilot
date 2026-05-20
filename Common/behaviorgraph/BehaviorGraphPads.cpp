/********************************************************************
	created:	14:4:2010   12:19
	file path:	d:\IxEngine\Common\resdata
	author:		chenxi
	
	purpose:	所有的BehaviorGraph Pad
*********************************************************************/
#include "stdh.h"
#include "../commondefines/general_stl.h"

#include "Behavior.h"

#include "BehaviorGraphPads.h"
#include "BgnHelper.h"
#include "BgnState.h"
#include "BgpFunc.h"
// 

#include "../stringparser/stringparser.h"

#include <assert.h> 

GElemBase *GetBVRElem(GElemBase *elem)
{
	if (elem)
	{
		if (elem->next)
		{
			GElemBase *elemNext=elem->next;
			std::string s="__bvr_";
			s+=elem->elemname;
			if (elemNext->elemname==s)
				return elemNext;
		}
	}
	return NULL;
}


void CBehaviorGraphPads::SetPadIDGenCallBack(UniquePadIDGenFunc func)
{
	_callbackGenPadID=func;
}


BYTE CBehaviorGraphPads::_CalcClassCode()
{
	BYTE *p=NULL;
	return FORCE_TYPE(BYTE,p);

}


BgpFamily CBehaviorGraphPads::GetFamily()
{
	for (int i=0;i<_pads.size();i++)
	{
		CBehaviorGraphPad *pad=(CBehaviorGraphPad *)_pads[i];
		if (!pad)
			continue;
		if (pad->GetFamily()!=BgpFamily_Common)
			return pad->GetFamily();
	}
	return BgpFamily_Common;
}

StringID CBehaviorGraphPads::GetName()
{
	for (int i=0;i<_pads.size();i++)
	{
		CBehaviorGraphPad *pad=(CBehaviorGraphPad *)_pads[i];
		if (!pad)
			continue;
		if (!pad->GetClass()->CheckName("CBgp_Graph"))
			continue;
		CBgp_Graph *padGraph=(CBgp_Graph*)pad;
		return padGraph->_nm;
	}
	return StringID_Invalid;
}

const char *CBehaviorGraphPads::GetPadName(PadID idPad)
{
	CBehaviorGraphPad *pad=(CBehaviorGraphPad *)FindPad(idPad);
	if (!pad)
		return "";

	if (pad->IsFolder())
	{
		if (!pad->_nameFolder.empty())
			return pad->_nameFolder.c_str();
	}

	if (pad->GetClass()->CheckName("CBgp_State"))
	{
		CBgp_State *padState=(CBgp_State*)pad;
		if (padState->_nm!=StringID_Invalid)
		{
			return StrLib_GetStr(padState->_nm);
		}
	}
	if (pad->GetClass()->CheckName("CBgp_Func"))
	{
		CBgp_Func *padFunc=(CBgp_Func*)pad;
		if (padFunc->_nm!=StringID_Invalid)
		{
			return StrLib_GetStr(padFunc->_nm);
		}
	}

	static std::string s;
	FormatString(s,"%s[%08X]",pad->GetClass()->GetName(),pad->GetID());
	return s.c_str();
}



void CBehaviorGraphPads::EnumConstsDeclare(std::vector<BhvConstDeclare*>&entries,BOOL bSource,BOOL bParam)
{
	entries.clear();
	for (int i=0;i<_pads.size();i++)
	{
		CBehaviorGraphPad *pad=(CBehaviorGraphPad *)_pads[i];
		if (!pad)
			continue;
		if (!pad->GetClass()->CheckName("CBgp_Consts"))
			continue;
		CBgp_Consts *padConsts=(CBgp_Consts *)pad;
		for (int i=0;i<padConsts->_declares2.size();i++)
		{
			BhvConstDeclare *e=&padConsts->_declares2[i];
			if (e->nm!=StringID_Invalid)
			{
				if (bSource&&(e->tpShow!=0)&&(e->tpShow!=1))
					continue;
				if (bParam&&(e->tpShow!=0)&&(e->tpShow!=2))
					continue;
				entries.push_back(e);
			}
		}
	}
}

BhvConstDeclare*CBehaviorGraphPads::FindConstDeclare(StringID nm)
{
	if (!_bLookUpConsts)
	{
		std::vector<BhvConstDeclare*>entries;
		EnumConstsDeclare(entries,FALSE,FALSE);
		for (int i=0;i<entries.size();i++)
			_lookupConsts[entries[i]->nm]=entries[i];
		_bLookUpConsts=TRUE;
	}

	std::unordered_map<StringID,BhvConstDeclare*>::iterator it=_lookupConsts.find(nm);
	if (it==_lookupConsts.end())
		return NULL;
	return (*it).second;
}


void CBehaviorGraphPads::Save(CDataPacket &dp)
{
	CLinkPads::Save(dp);

	WORD ver=3;
	dp.Data_WriteSimple(ver);

	dp.Data_WriteSimple(_bResolved);

	DP_WriteVector(dp,_bases);

	DP_WriteVector(dp,_foldersInclude);
	DP_WriteVector(dp,_folderExclude);
	DP_WriteVector(dp,_padsMove);
	DP_WriteVector(dp,_linksAdd);

	dp.Data_NextDword()=_padsOverride.size();
	for (int i=0;i<_padsOverride.size();i++)
		_padsOverride[i].Save(dp);
}
void CBehaviorGraphPads::Load(CDataPacket &dp)
{
	Clear();

	BOOL bLongPadID;
	CLinkPads::Load(dp,bLongPadID);

	WORD ver;
	dp.Data_ReadSimple(ver);

	dp.Data_ReadSimple(_bResolved);

	DP_ReadVector(dp,_bases);

	DP_ReadVector(dp,_foldersInclude);
	DP_ReadVector(dp,_folderExclude);
	DP_ReadVector(dp,_padsMove);
	if (ver>=3)
		DP_ReadVector(dp,_linksAdd);

	_padsOverride.resize(dp.Data_NextDword());
	for (int i=0;i<_padsOverride.size();i++)
		_padsOverride[i].Load(dp);

}

BOOL CBehaviorGraphPads::IsPadIncluded(CBehaviorGraphPads &padsBase,PadID idPad)
{
	if (TRUE)
	{
		int i;
		for (i=0;i<_foldersInclude.size();i++)
		{
			PadID idFolder=_foldersInclude[i].idPad;
			if (padsBase.IsInFolder(idPad,idFolder))
				break;
		}

		if (i>=_foldersInclude.size())
			return FALSE;//不在任何一个folder里
	}

	for (int i=0;i<_folderExclude.size();i++)
	{
		PadID idFolder=_folderExclude[i].idPad;
		if (padsBase.IsInFolder(idPad,idFolder))
			return FALSE;//被exclude
	}

	return TRUE;
}

void CBehaviorGraphPads::AddBase(StringID nm)
{
	UNIQUE_VEC_ADD(_bases,nm);
}


BOOL CBehaviorGraphPads::IsBase(StringID nm)
{
	int idx;
	VEC_FIND(_bases,nm,idx);
	if (idx!=-1)
		return TRUE;
	return FALSE;
}

StringID *CBehaviorGraphPads::GetBases(DWORD &c)
{
	c=_bases.size();
	return _bases.data();
}

void CBehaviorGraphPads::RemoveBase(StringID nm)
{
	if (!_bResolved)
		return;//目前不支持没有Resolve的pads

	CBehaviorGraphPad *pad;

	if (TRUE)
	{
		DWORD c=0;
		for (int i=0;i<_foldersInclude.size();i++)
		{
			pad=(CBehaviorGraphPad *)FindPad(_foldersInclude[i].idPad);
			if (pad)
			{
				if (pad->_nmBase==nm)
					continue;
			}
			_foldersInclude[c]=_foldersInclude[i];
			c++;
		}
		_foldersInclude.resize(c);
	}

	if (TRUE)
	{
		DWORD c=0;
		for (int i=0;i<_folderExclude.size();i++)
		{
			pad=(CBehaviorGraphPad *)FindPad(_folderExclude[i].idPad);
			if (pad)
			{
				if (pad->_nmBase==nm)
					continue;
			}
			_folderExclude[c]=_folderExclude[i];
			c++;
		}
		_folderExclude.resize(c);
	}

	if (TRUE)
	{
		DWORD c=0;
		for (int i=0;i<_padsMove.size();i++)
		{
			pad=(CBehaviorGraphPad *)FindPad(_padsMove[i].idPad);
			if (pad)
			{
				if (pad->_nmBase==nm)
					continue;
			}
			_padsMove[c]=_padsMove[i];
			c++;
		}
		_padsMove.resize(c);
	}

	if (TRUE)
	{
		DWORD c=0;
		for (int i=0;i<_padsOverride.size();i++)
		{
			pad=(CBehaviorGraphPad *)FindPad(_padsOverride[i].idPad);
			if (pad)
			{
				if (pad->_nmBase==nm)
					continue;
			}
			_padsOverride[c]=_padsOverride[i];
			c++;
		}
		_padsOverride.resize(c);
	}

	if (TRUE)
	{
		DWORD c=0;
		for (int i=0;i<_linksAdd.size();i++)
		{
			pad=(CBehaviorGraphPad *)FindPad(_linksAdd[i].link.idPad[0]);
			if (pad)
			{
				if (pad->_nmBase==nm)
					continue;
			}
			pad=(CBehaviorGraphPad *)FindPad(_linksAdd[i].link.idPad[1]);
			if (pad)
			{
				if (pad->_nmBase==nm)
					continue;
			}
			_linksAdd[c]=_linksAdd[i];
			c++;
		}
		_linksAdd.resize(c);
	}


	if (TRUE)
	{
		std::vector<PadID> removes;
		for (int i=0;i<_pads.size();i++)
		{
			if (_pads[i])
			{
				if (((CBehaviorGraphPad*)_pads[i])->_nmBase==nm)
					removes.push_back(_pads[i]->GetID());
			}
		}
		if (removes.size()>0)
			RemovePads(removes.data(),removes.size());
	}

	VEC_REMOVE(_bases,nm);

}



void CBehaviorGraphPads::EnumTopStates(std::vector<PadID> &ids)
{
	for (int i=0;i<_pads.size();i++)
	{
		CBehaviorGraphPad *pad=(CBehaviorGraphPad *)_pads[i];
		if (!pad)
			continue;
		if (!pad->GetClass()->CheckName("CBgp_State"))
			continue;
		if (!pad->IsFolder())
			continue;
		if (pad->GetFolder()!=PadID_Null)
			continue;//Not root
		CBgp_State *padState=(CBgp_State*)pad;
		UNIQUE_VEC_ADD(ids,padState->GetID());
	}
}

void CBehaviorGraphPads::EnumFuncs(std::vector<PadID> &ids)
{
	for (int i=0;i<_pads.size();i++)
	{
		CBehaviorGraphPad *pad=(CBehaviorGraphPad *)_pads[i];
		if (!pad)
			continue;
		if (!pad->GetClass()->CheckName("CBgp_Func"))
			continue;
		CBgp_Func *padFunc=(CBgp_Func*)pad;
		UNIQUE_VEC_ADD(ids,padFunc->GetID());
	}
}


std::vector<void*> *CBehaviorGraphPads::FindDeltaPtrs(PadID idPad)
{
	std::unordered_map<PadID,std::vector<void*> >::iterator it=_lookupDeltePtrs.find(idPad);
	if (it!=_lookupDeltePtrs.end())
		return &(*it).second;
	return NULL;
}

CBgp_Func *CBehaviorGraphPads::FindFunc(StringID nm)
{
	for (int i=0;i<_pads.size();i++)
	{
		CBehaviorGraphPad *pad=(CBehaviorGraphPad *)_pads[i];
		if (!pad)
			continue;
		if (!pad->GetClass()->CheckName("CBgp_Func"))
			continue;

		CBgp_Func *padFunc=(CBgp_Func*)pad;
		if (padFunc->_nm==nm)
			return padFunc;
	}

	return NULL;
}

CBgp_Func *CBehaviorGraphPads::FindOwnerFunc(CBehaviorGraphPad *pad)
{
	while(pad)
	{
		if (pad->GetClass()->CheckName("CBgp_Func"))
			return (CBgp_Func *)pad;

		pad=(CBehaviorGraphPad *)FindPad(pad->GetFolder());
	}
	return NULL;

}

CBgp_Consts*CBehaviorGraphPads::FindConsts(CBehaviorGraphPad *padFrom)
{
	CBgp_Consts*result=NULL;
	int depthMin=10000;
	for (int i=0;i<_pads.size();i++)
	{
		CBehaviorGraphPad *pad=(CBehaviorGraphPad *)_pads[i];
		if (!pad)
			continue;
		if (pad->GetClass()->CheckName("CBgp_Consts"))
		{
			PadID idFolder=pad->GetFolder();//consts所在的Folder
			int depth=GetInFolderDepth(padFrom->GetID(),idFolder);
			if (depth>0)
			{
				if (depth<depthMin)
				{
					depthMin=depth;
					result=(CBgp_Consts*)pad;
				}
			}
		}
	}
	return result;
}

CBgp_Vars*CBehaviorGraphPads::FindVars()
{
	for (int i=0;i<_pads.size();i++)
	{
		CBehaviorGraphPad *pad=(CBehaviorGraphPad *)_pads[i];
		if (!pad)
			continue;
		if (pad->GetClass()->CheckName("CBgp_Vars"))
			return (CBgp_Vars*)pad;
	}
	return NULL;
}

BehaviorMemType CBehaviorGraphPads::GetVarMemType(StringID nm)
{
	for (int i=0;i<_pads.size();i++)
	{
		CBehaviorGraphPad *pad=(CBehaviorGraphPad *)_pads[i];
		if (!pad)
			continue;
		if (pad->GetClass()->CheckName("CBgp_Vars"))
		{
			CBgp_Vars *padVars=(CBgp_Vars*)pad;
			for (int i=0;i<padVars->_declares2.size();i++)
			{
				BhvVarDeclare *declare=&padVars->_declares2[i];
				if (declare->nm==nm)
					return declare->tp;
			}
		}
	}
	return BehaviorMemType_None;
}


BOOL CBehaviorGraphPads::ResolveBhvValType(BhvValType &tp,CClass *&clss,GElemBase *&elem_)
{
	LinkPadClasses *clsses=_clsses;

	if (!clsses)
		return FALSE;

	static std::vector<CClass*> buf;
	buf.clear();
	clsses->CollectPadClasses(buf);

	for (int i=0;i<buf.size();i++)
	{
		if (!buf[i])
			continue;
		CBehaviorGraphPad *pad=(CBehaviorGraphPad *)buf[i]->New();
		if (pad)
		{
			GObjBase *gobj=pad->GetGObj();
			if (gobj)
			{
				GElemBase *elem=gobj->GetElems();
				while(elem)
				{
					if (tp.IsCompatible(elem))
					{
						Safe_Class_Delete(pad);
						clss=buf[i];
						elem_=elem;
						return TRUE;
					}
					elem=elem->next;
				}
			}
		}
	}
	assert(FALSE);
	return FALSE;
}
