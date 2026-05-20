/********************************************************************
	created:	2012/11/20 
	author:		cxi
	
	purpose:	BehaviorGraph Edit Panel
*********************************************************************/
#include "stdh.h"

#include "WMGuiLib.h"

#include "RenderSystem/IMesh.h"
#include "RenderSystem/IMtrl.h"
#include "RenderSystem/IAnim.h"
#include "RenderSystem/IAnimTree.h"
#include "RenderSystem/IFont.h"
#include "RenderSystem/IBehaviorGraph.h"
#include "RenderSystem/IUtilRS.h"


#include "WorldSystem/IWorldSystem.h"


#include "RenderPortBase.h"
#include "../Common/behaviorgraph/BehaviorGraphPads.h"

#include "behaviorgraph/Behavior.h"
#include "behaviorgraph/BgnState.h"
#include "behaviorgraph/BgnHelper.h"
#include "behaviorgraph/BgpFunc.h"

#include "behaviorgraph/BehaviorDebug_RegistryBase.h"


#include "WndBase.h"
#include "GuiEditor_res.h"
#include "GuiAgent_general.h"
#include "matrixedit_base.h"

#include "BehaviorGraphEditPanel.h"

#include "EditPopup.h"

#include "SscUID.h"

PadID GenUniquePadID()
{
	extern SscUID SscUID_SafeGen();
	return (PadID)SscUID_SafeGen();
}

#define EquipGenUniquePadID(pads) ((CBehaviorGraphPads*)pads)->SetPadIDGenCallBack(GenUniquePadID);


CBehaviorDebugClient_RegistryBase *GetBehaviorDebugClient()
{
	static BOOL bInit=FALSE;
	static CBehaviorDebugClient_RegistryBase dbgclient;

	if (!bInit)
	{
		dbgclient.Init("IxSoftware","IxEngine");
		bInit=TRUE;
	}

	return &dbgclient;

}

//////////////////////////////////////////////////////////////////////////
//Reps_BehaviorGraph
void Reps_BehaviorGraph::Copy(ResEditPanelState &src)
{
	panel=src.panel;
	ResData_Delete(resdata);
	resdata=(ResData*)src.resdata->GetClass()->New();

	BehaviorGraphData *dataBG=(BehaviorGraphData *)resdata;
	dataBG->pads.SetClasses(g_ssGuiLib.pRS->GetBehaviorGraphMgr()->GetClasses());
	resdata->Copy(*src.resdata);

}

void Reps_BehaviorGraph::SetData(ResData *data)
{
	ResData_Delete(resdata);
	resdata=(ResData*)data->GetClass()->New();
	BehaviorGraphData *dataBG=(BehaviorGraphData *)resdata;
	dataBG->pads.SetClasses(g_ssGuiLib.pRS->GetBehaviorGraphMgr()->GetClasses());
	resdata->Copy(*data);
}

CGraphBgPads *Reps_BehaviorGraph::GetGraph()
{
	if (panel)
		return ((CBehaviorGraphEditPanel*)panel)->GetGraph();
	return NULL;
}





//////////////////////////////////////////////////////////////////////////
//CDataSrc_GraphBgPads
CGraphPads *CDataSrc_GraphBgPads::GetGraph(CGuiAgent *agent)
{
	CGuiData_Res *data=(CGuiData_Res *)agent->FindData("resource");
	if (data)
	{
		Reps_BehaviorGraph *state=(Reps_BehaviorGraph *)data->GetState();
		if (state)
			return state->GetGraph();
	}
	return NULL;
}

std::vector<PadID>*CDataSrc_GraphBgPads::GetSelBuf(CGuiAgent *agent)
{
	CGuiData_Res *data=(CGuiData_Res *)agent->FindData("resource");
	if (data)
	{
		Reps_BehaviorGraph *state=(Reps_BehaviorGraph *)data->GetState();
		if (state)
			return &state->sels;
	}
	return NULL;
}

void CDataSrc_GraphBgPads::NotifyChange(CGuiAgent *agent,BOOL bSave)
{
	CGuiData_Res *data=(CGuiData_Res *)agent->FindData("resource");
	if (data)
	{
		Reps_BehaviorGraph *state=(Reps_BehaviorGraph *)data->GetState();
		state->panel->RefreshStateMod(bSave);
	}
}

//////////////////////////////////////////////////////////////////////////
//CGuiAgent_NewBgPads
CGuiAgent_NewBgPads::CGuiAgent_NewBgPads()
{
	LinkPadClasses *clsses=g_ssGuiLib.pRS->GetBehaviorGraphMgr()->GetClasses();
	clsses->CollectNames(_classes);
}

BOOL CGuiAgent_NewBgPads::OnRButtonClick(int x,int y,DWORD flag)
{
	BgpFamily family=BgpFamily_Common;
	CGuiData_Res *data=(CGuiData_Res *)FindData("resource");
	Reps_BehaviorGraph *state=(Reps_BehaviorGraph *)data->GetState();
	if (state)
	{
		CGraphBgPads *graph=state->GetGraph();

		CBehaviorGraphPads *pads=(CBehaviorGraphPads *)graph->GetPads();
		if (pads)
		{
			DWORD c=pads->GetPadCount();
			for (int i=0;i<c;i++)
			{
				CBehaviorGraphPad *pad=(CBehaviorGraphPad *)pads->GetPad(i);

				if (pad->GetClass()->CheckName("CBgp_Graph"))
				{
					if (((CBgp_Graph*)pad)->_nm!=StringID_Invalid)
						family=((CBgp_Graph*)pad)->_family;
				}
			}
		}
	}

	std::string s;
	_PushMenu("新增"); 

	LinkPadClasses *clsses=g_ssGuiLib.pRS->GetBehaviorGraphMgr()->GetClasses();

	std::vector<int>categories[BgpCtgr_Max];
	for (int i=0;i<_classes.size();i++)
	{
		CBehaviorGraphPad *p=(CBehaviorGraphPad *)clsses->New(_classes[i].c_str());
		if (!p)
			continue;
		categories[p->GetCategory()].push_back(i);
		Class_Delete(p);
	}

	if (TRUE)
	{
		std::map<std::string,UINT> temp;

		for (int i=1;i<BgpCtgr_Max;i++)
		{
			temp.clear();

			_PushMenu(GetBgpCategoryName((BgpCategory)i));
			for (int k=0;k<categories[i].size();k++)
			{
				int idx=categories[i][k];
				CBehaviorGraphPad *p=(CBehaviorGraphPad *)clsses->New(_classes[idx].c_str());
				if (!p)
					continue;

				if (p->GetFamily()!=BgpFamily_Common)
				{
					if (p->GetFamily()!=family)
					{
						Class_Delete(p);
						continue;
					}
				}
				s=p->GetTypeName();
				Class_Delete(p);

				temp[s]=ID_AGENT_NEW_BGPAD_BEGIN+idx;
			}

			std::map<std::string,UINT>::iterator it;
			for (it=temp.begin();it!=temp.end();it++)
			{
				_AddMenu((*it).first.c_str(),(*it).second);
			}

			_PopMenu();
		}
	}

	_PopMenu();
	_AddMenuSep();
	_ScreenToGG(x,y);
	_pt.set(x,y);
	return TRUE;
}

BOOL CGuiAgent_NewBgPads::OnCommand(DWORD idCmd)
{
	if ((idCmd>=ID_AGENT_NEW_BGPAD_BEGIN)&&(idCmd<ID_AGENT_NEW_BGPAD_END))
	{
		CGuiData_Res *data=(CGuiData_Res *)FindData("resource");
		Reps_BehaviorGraph *state=(Reps_BehaviorGraph *)data->GetState();
		if (state)
		{
			CGraphBgPads *graph=state->GetGraph();

			CBehaviorGraphPads *pads=(CBehaviorGraphPads *)graph->GetPads();
			if (pads)
			{
				EquipGenUniquePadID(pads);
				PadID id=pads->NewPad(_classes[idCmd-ID_AGENT_NEW_BGPAD_BEGIN].c_str(),_pt);

				state->panel->RefreshStateMod();
			}
		}
		_Redraw(FALSE);
		return FALSE;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//CGuiAgent_BgPadsCommand

CGuiAgent_BgPadsCommand::CGuiAgent_BgPadsCommand(CDataSrc_GraphPads *src)
{
	_src=src;
	_sel=PadID_Null;
	_bp=PadID_Null;
	_iStub=-1;

	_util.Init(g_ssGuiLib.pRS->GetPath(Path_BehaviorGraph),(BgpClasses*)g_ssGuiLib.pRS->GetBehaviorGraphMgr()->GetClasses());
}


void CGuiAgent_BgPadsCommand::_TransformGGtoPads(CLinkPads *pads)
{
	i_math::pos2df off,scale;
	_GetTransformGG(off,scale);
	pads->SetFolderXfm(pads->GetCurFolder(),off,scale);
}

void CGuiAgent_BgPadsCommand::_TransformGGfromPads(CLinkPads *pads)
{
	i_math::pos2df off,scale;
	if (pads->GetFolderXfm(pads->GetCurFolder(),off,scale))
		_SetTransformGG(off,scale);
}

extern StringID GetGraphName(CLinkPads *pads);

CBehaviorGraphPads *CGuiAgent_BgPadsCommand::_GetPads()
{
	CGuiData_Res *data=(CGuiData_Res *)FindData("resource");
	Reps_BehaviorGraph *state=(Reps_BehaviorGraph *)data->GetState();
	if (state)
	{
		CGraphBgPads *graph=state->GetGraph();

		return (CBehaviorGraphPads *)graph->GetPads();
	}
	return NULL;
}


void CGuiAgent_BgPadsCommand::_AddImportMenu()
{
	CBehaviorGraphPads *pads=_GetPads();

	if (pads)
	{
		DWORD c;
		StringID *nms=_util.EnumNames(c);

		_nmsToImport.clear();

		StringID nmMe=pads->GetName();

		for (int i=0;i<c;i++)
		{
			StringID nm=nms[i];
			if (nm==nmMe)
				continue;
			if (pads->IsBase(nm))
				continue;

			if (!_util.VerifyNewBase(*pads,nm))
				continue;

			_nmsToImport.push_back(nm);
		}

		if (_nmsToImport.size()>0)
		{
			_PushMenu("导入...");
			for (int i=0;i<_nmsToImport.size();i++)
				_AddMenu(StrLib_GetStr(_nmsToImport[i]),ID_AGENT_IMPORT_BGPADS_START+i,MF_ENABLED|MF_STRING);
			_PopMenu();

			_AddMenuSep();
		}
	}
}

void CGuiAgent_BgPadsCommand::_AddBasePadsMenu()
{
	CBehaviorGraphPads *pads=_GetPads();
	if (pads)
	{
		_includes.clear();
		_nmsToRemove.clear();

		StringID *nmsBase;
		DWORD c;
		nmsBase=pads->GetBases(c);

		std::vector<PadID>ids;
		std::string s;

		if (c>0)
		{
			CBehaviorGraphPads padsBase;
			for (int i=0;i<c;i++)
			{
				ids.clear();
				StringID nmBase=nmsBase[i];
				_nmsToRemove.push_back(nmBase);
				_PushMenu(StrLib_GetStr(nmBase));

				_AddMenu("删除",ID_AGENT_REMOVE_BGPADS_START+i,MF_ENABLED|MF_STRING);

				padsBase.Clear();
				if (TRUE==_util.LoadBGPads(nmBase,padsBase))
				{
					if (_util.ResolveBGPads(padsBase))
					{
						for (int k=0;k<2;k++)
						{
							ids.clear();
							if (k==0)
								padsBase.EnumTopStates(ids);
							else
								padsBase.EnumFuncs(ids);

							if (ids.size()>0)
								_AddMenuSep();
							for (int i=0;i<ids.size();i++)
							{
								Include include;
								include.idPad=ids[i];
								include.nmBase=nmBase;
								include.bIncluded=pads->IsPadIncluded(padsBase,ids[i]);

								s=padsBase.GetPadName(ids[i]);
								if (!include.bIncluded)
									_AddMenu(s.c_str(),ID_AGENT_INCLUDE_BGPAD_START+_includes.size(),MF_ENABLED|MF_STRING);
								else
									_AddMenu(s.c_str(),ID_AGENT_INCLUDE_BGPAD_START+_includes.size(),MF_ENABLED|MF_STRING|MF_CHECKED);

								_includes.push_back(include);
							}
						}
					}
				}

				_PopMenu();
			}
			_AddMenuSep();
		}

	}
}



BOOL CGuiAgent_BgPadsCommand::OnRButtonClick(int x,int y,DWORD flag)
{
	std::vector<PadID>*sels=_GetSelBuf();
	CGraphPads *graph=_GetGraph();


//	_AddMenu("刷新Unique PadID",ID_AGENT_BGPAD_GEN_UNIQUE_PADID,MF_ENABLED|MF_STRING);

	if (TRUE)
	{
		CBehaviorDebugClient_RegistryBase *debug=GetBehaviorDebugClient();

		StringID nmBg=GetGraphName(graph->GetPads());

		debug->GetObjsByName(nmBg,_objsToDebug);
		DWORD c=_objsToDebug.size();
		DWORD objSel=debug->GetSelObj();
		if (c>0)
		{
			std::string s;
			_PushMenu("调试对象");
			if (objSel==0)
				_AddMenu("任意对象",ID_AGENT_SEL_DEBUG_OBJ_START,MF_ENABLED|MF_STRING|MF_CHECKED);
			else
				_AddMenu("任意对象",ID_AGENT_SEL_DEBUG_OBJ_START,MF_ENABLED|MF_STRING);
			for (int i=0;i<c;i++)
			{
				StringID nm=nmBg;
				if (nm!=StringID_Invalid)
					FormatString(s,"%s%08x",StrLib_GetStr(nm),_objsToDebug[i]);
				else
					s="<未知>";
				if (objSel==_objsToDebug[i])
					_AddMenu(s.c_str(),ID_AGENT_SEL_DEBUG_OBJ_START+1+i,MF_ENABLED|MF_STRING|MF_CHECKED);
				else
					_AddMenu(s.c_str(),ID_AGENT_SEL_DEBUG_OBJ_START+1+i,MF_ENABLED|MF_STRING);
			}

			_PopMenu();
		}
	}

	_AddMenu("清除所有断点",ID_AGENT_CLEAR_ALL_BREAKPOINT,MF_ENABLED|MF_STRING);

	if (TRUE)
	{
		if (GetBehaviorDebugClient()->GetState()->bpCur.IsEmpty())
		{
			_AddMenu("显示当前断点",ID_AGENT_GOTO_CUR_BREAKPOINT,MF_DISABLED|MF_GRAYED|MF_STRING);
		}
		else
		{
			StringID nmBg=GetBehaviorDebugClient()->GetState()->bpCur.nmBG;
			if (nmBg==GetGraphName(graph->GetPads()))
				_AddMenu("显示当前断点",ID_AGENT_GOTO_CUR_BREAKPOINT,MF_ENABLED|MF_STRING);
			else
			{
				std::string s;
				FormatString(s,"断点位于:%s",StrLib_GetStr(nmBg));
				_AddMenu(s.c_str(),ID_AGENT_GOTO_CUR_BREAKPOINT,MF_ENABLED|MF_STRING);
			}
		}
	}

	_AddMenuSep();

	if (flag&CtrlOpFlag_ShiftDown)
		_AddImportMenu();

	_AddBasePadsMenu();

	if (sels->size()!=1)
		return TRUE;

	_pt.x=x;
	_pt.y=y;
	GetWnd()->ClientToScreen(&_pt);

	_ScreenToGG(x,y);

	GraphPadHit hit;
	BOOL bHit=graph->HitTest(x,y,hit);
	if (bHit)
	{
		CBehaviorGraphPad*pad=(CBehaviorGraphPad*)(graph->GetPads()->FindPad(hit.id));
		_sel=hit.id;

		std::string s;
		FormatString(s,"复制\"%s\"",pad->GetClass()->GetName());
		_AddMenu(s.c_str(),ID_AGENT_COPY_BGPAD_CLASSNAME,MF_ENABLED|MF_STRING);

	}

	_AddMenuSep();

	return TRUE;
}

BOOL CGuiAgent_BgPadsCommand::OnCommand(DWORD idCmd)
{
	CGraphPads *graph=_GetGraph();

	if ((idCmd>=ID_AGENT_SEL_DEBUG_OBJ_START)&&(idCmd<ID_AGENT_SEL_DEBUG_OBJ_END))
	{
		DWORD objToSel=0;
		if (idCmd>ID_AGENT_SEL_DEBUG_OBJ_START)
		{
			if (idCmd-ID_AGENT_SEL_DEBUG_OBJ_START-1<_objsToDebug.size())
			{
				objToSel=_objsToDebug[idCmd-ID_AGENT_SEL_DEBUG_OBJ_START-1];
			}
		}

		BehaviorDebugCmd cmd;
		cmd.tp = BehaviorDebugCmd::SelectObj;
		cmd.obj = objToSel;
		GetBehaviorDebugClient()->SendCommand(cmd);
		_Redraw(FALSE);
	}



	if ((idCmd>=ID_AGENT_REMOVE_BGPADS_START)&&(idCmd<ID_AGENT_REMOVE_BGPADS_END))
	{
		int idx=idCmd-ID_AGENT_REMOVE_BGPADS_START;
		if (idx<_nmsToRemove.size())
		{
			StringID nmBase=_nmsToRemove[idx];

			CBehaviorGraphPads *pads=_GetPads();
			pads->RemoveBase(nmBase);
			_NotifyChange(TRUE);
		}

	}


	if ((idCmd>=ID_AGENT_INCLUDE_BGPAD_START)&&(idCmd<ID_AGENT_INCLUDE_BGPAD_END))
	{
		int idx=idCmd-ID_AGENT_INCLUDE_BGPAD_START;
		if (idx<_includes.size())
		{
			Include &include=_includes[idx];
			if (!include.bIncluded)
			{
				CBehaviorGraphPads *pads=_GetPads();
				if (pads)
				{
					if (_util.UnresolveBGPads(*pads))
					{
						CBehaviorGraphPads::Mod_IncludeFolder mod;
						mod.idPad=include.idPad;
						pads->_foldersInclude.push_back(mod);
						_NotifyChange(TRUE);
					}
				}
			}
		}
	}

	if ((idCmd>=ID_AGENT_IMPORT_BGPADS_START)&&(idCmd<ID_AGENT_IMPORT_BGPADS_END))
	{
		int idx=idCmd-ID_AGENT_IMPORT_BGPADS_START;
		if (idx<_nmsToImport.size())
		{
			CBehaviorGraphPads *pads=_GetPads();
			if (pads)
			{
				StringID nmBase=_nmsToImport[idx];
				if (_util.VerifyNewBase(*pads,nmBase))
				{
					pads->AddBase(nmBase);
					_NotifyChange(TRUE);
				}
			}
		}
	}

	if (idCmd==ID_AGENT_BGPAD_GEN_UNIQUE_PADID)
	{
		CGraphPads *graph=_GetGraph();
		CLinkPads *pads=graph->GetPads();
		EquipGenUniquePadID(pads);

		pads->RefreshPadIDs();
		_NotifyChange(TRUE);
	}

	if (idCmd==ID_AGENT_PASTE_PADS)
	{
		CGraphPads *graph=_GetGraph();
		CLinkPads *pads=graph->GetPads();
		EquipGenUniquePadID(pads);
	}


	if (idCmd==ID_AGENT_CLEAR_ALL_BREAKPOINT)
	{
		BehaviorDebugCmd cmd;
		cmd.tp=BehaviorDebugCmd::ClearBreakPoint;
		GetBehaviorDebugClient()->SendCommand(cmd);
	}

	if (idCmd==ID_AGENT_COPY_BGPAD_CLASSNAME)
	{
		CBehaviorGraphPad*pad=(CBehaviorGraphPad*)(graph->GetPads()->FindPad(_sel));
		extern void CopyToClipboard(CWnd *wnd,const char *str);
		CopyToClipboard(GetWnd(),pad->GetClass()->GetName());
	}

	if (idCmd==ID_AGENT_GOTO_CUR_BREAKPOINT)
	{
		if (!GetBehaviorDebugClient()->GetState()->bpCur.IsEmpty())
		{
			BehaviorDebugStep bpCur=GetBehaviorDebugClient()->GetState()->bpCur;
			StringID nmBg=GetGraphName(graph->GetPads());
			if (nmBg!=StringID_Invalid)
			{
				if (nmBg==bpCur.nmBG)
					_EnsureVisible(bpCur.idPad);
			}
		}

	}


	return TRUE;
}

CBehaviorGraphPad *FindStatePad(CLinkPads *pads,StringID nm)
{
	DWORD c=pads->GetPadCount();
	for (int i=0;i<c;i++)
	{
		CLinkPad *pad=pads->GetPad(i);
		if (pad->GetClass()->CheckName("CBgp_State"))
		{
			CBgp_State* padState=(CBgp_State*)pad;
			if (padState->_nm==nm)
				return padState;
		}
	}

	return NULL;
}

StringID GetGraphName(CLinkPads *pads)
{
	DWORD c=pads->GetPadCount();
	for (int i=0;i<c;i++)
	{
		CLinkPad *pad=pads->GetPad(i);
		if (pad->GetClass()->CheckName("CBgp_Graph"))
		{
			CBgp_Graph* padNm=(CBgp_Graph*)pad;
			return padNm->_nm;
		}
	}
	return StringID_Invalid;
}



BOOL CGuiAgent_BgPadsCommand::OnLButtonDblClk(int x,int y,DWORD flag)
{
	CGraphPads *graph=_GetGraph();
	CLinkPads *pads=graph->GetPads();

	_TransformGGtoPads(pads);

	_ScreenToGG(x,y);

	BOOL bHandled=FALSE;

	GraphPadHit hit;
	BOOL bHit=graph->HitTest(x,y,hit);
	if (bHit)
	{
		CBehaviorGraphPad*pad=(CBehaviorGraphPad*)(pads->FindPad(hit.id));

		if (flag&CtrlOpFlag_CtrlDown)
		{
			if (pad->_nmBase!=StringID_Invalid)
			{
				std::string path=_util.FindBGPadsPath(pad->_nmBase);
				if (!path.empty())
				{
					AfxGetMainWnd()->SendMessage(GLM_ResTree_DblClick,(WPARAM)path.c_str());

					extern CCurrentUserRegistry g_reg;
					PadID idPad=pad->GetID();
					g_reg.WriteVar("[EnsureVisiblePad]","PadID",idPad);
					g_reg.SendEvent("[EnsureVisiblePad]");
				}
			}
		}
		else
		{
			if (pad->GetClass()->CheckName("CBgp_SwitchState"))
			{
				CBgp_SwitchState* padSwitchState=(CBgp_SwitchState*)pad;
				if (padSwitchState->_nm!=StringID_Invalid)
				{
					CBehaviorGraphPad*padState=FindStatePad(pads,padSwitchState->_nm);
					if (padState)
					{
						if (padState->IsFolder())
						{
							pads->ClearFolderStack();
							std::vector<PadID> stacks;
							stacks.push_back(padState->GetID());
							while(padState)
							{
								PadID idFolder=padState->GetFolder();
								if (idFolder==PadID_Null)
									break;
								stacks.push_back(idFolder);
								padState=(CBehaviorGraphPad*)pads->FindPad(idFolder);
							}
							for (int i=stacks.size()-1;i>=0;i--)
								pads->PushFolder(stacks[i]);

							bHandled=TRUE;
						}
					}
				}
			}

			if (pad->GetClass()->CheckName("CBgp_Call"))
			{
				CBgp_Call* padCall=(CBgp_Call*)pad;
				if (padCall->_nm!=StringID_Invalid)
				{
					CBehaviorGraphPad*padFunc=((CBehaviorGraphPads*)pads)->FindFunc(padCall->_nm);
					if (padFunc)
					{
						std::vector<PadID> stacks;
						if (padFunc->IsFolder())
						{
							pads->ClearFolderStack();

							stacks.push_back(padFunc->GetID());

							while(padFunc)
							{
								PadID idFolder=padFunc->GetFolder();
								if (idFolder==PadID_Null)
									break;
								stacks.push_back(idFolder);
								padFunc=(CBehaviorGraphPad*)pads->FindPad(idFolder);
							}

							for (int i=stacks.size()-1;i>=0;i--)
								pads->PushFolder(stacks[i]);

							bHandled=TRUE;
						}
					}
				}
			}

		}

	}

	if (bHandled)
	{
		//更新Transform GG
		_TransformGGfromPads(pads);
		_TransformGGtoPads(pads);

		_NotifyChange(TRUE);
		_Redraw(FALSE);
		return FALSE;
	}

	return TRUE;
}

void CGuiAgent_BgPadsCommand::_EnsureVisible(PadID idPad)
{
	CGraphPads *graph=_GetGraph();
	CLinkPads *pads=graph->GetPads();

	CLinkPad *pad=pads->FindPad(idPad);
	if (pad)
	{
		pads->ClearFolderStack();
		std::vector<PadID> stacks;
		if (pad->IsFolder())
			stacks.push_back(pad->GetID());
		while(pad)
		{
			PadID idFolder=pad->GetFolder();
			if (idFolder==PadID_Null)
				break;
			stacks.push_back(idFolder);
			pad=(CBehaviorGraphPad*)pads->FindPad(idFolder);
		}
		for (int i=stacks.size()-1;i>=0;i--)
			pads->PushFolder(stacks[i]);

		_TransformGGfromPads(pads);
		_TransformGGtoPads(pads);

		i_math::pos2di posPad=graph->GetPadPos(idPad);
		i_math::recti rc;
		_GetClientRect(rc);
		int x,y;
		x=rc.getWidth()/2;
		y=rc.getHeight()/2;
		_ScreenToGG(x,y);

		i_math::pos2df offGG,scaleGG;
		_GetTransformGG(offGG,scaleGG);

		offGG.x+=scaleGG.x*(float)(x-posPad.x);
		offGG.y+=scaleGG.y*(float)(y-posPad.y);
		_SetTransformGG(offGG,scaleGG);


		_NotifyChange(TRUE);
		_Redraw(FALSE);
	}

}


BOOL CGuiAgent_BgPadsCommand::OnTimer(int dt,DWORD flag)
{
	GetBehaviorDebugClient()->Update();

	if (GetBehaviorDebugClient()->ExistsSelFrameData())
		_Redraw(FALSE);

	if (TRUE)
	{
		extern CCurrentUserRegistry g_reg;
		if (g_reg.PeekEvent("[EnsureVisiblePad]"))
		{
			PadID idPad;
			g_reg.ReadVar("[EnsureVisiblePad]","PadID",idPad);
			CBehaviorGraphPads *pads=_GetPads();
			if (pads->FindPad(idPad))
			{
				g_reg.FetchEvent("[EnsureVisiblePad]");
				_EnsureVisible(idPad);
				std::vector<PadID>*sels=_src->GetSelBuf(this);
				if (sels)
				{
					sels->clear();
					(*sels).push_back(idPad);
				}

			}
		}
	}

	PadID bpCur=GetBehaviorDebugClient()->GetState()->bpCur.idPad;
	if (_bp!=bpCur)
	{
		_bp=bpCur;
		if (_bp!=PadID_Null)
			_EnsureVisible(_bp);
	}

	return TRUE;
}

BOOL CGuiAgent_BgPadsCommand::OnKeyDown(char c,DWORD flag)
{
	CGraphPads *graph=_GetGraph();
	CLinkPads *pads=graph->GetPads();
	StringID nmBg=GetGraphName(pads);

	if (c==120)
	{//F9
		if (nmBg!=StringID_Invalid)
		{
			if (_src)
			{
				std::vector<PadID>*sels=_src->GetSelBuf(this);
				if (sels)
				{
					BehaviorDebugCmd cmd;
					cmd.tp=BehaviorDebugCmd::ToggleBreakPoint;
					cmd.bp.nmBG=nmBg;
					if (sels->size()==1)
					{
						cmd.bp.idPad=(*sels)[0];
						GetBehaviorDebugClient()->SendCommand(cmd);
					}
				}
			}
		}
		return FALSE;
	}

	if (c==119)
	{//F8
		if (nmBg!=StringID_Invalid)
		{
			BehaviorDebugState *state=GetBehaviorDebugClient()->GetState();
			if (state->IsBreaking())
			{
				BehaviorDebugCmd cmd;
				cmd.tp=BehaviorDebugCmd::StepForward;
				GetBehaviorDebugClient()->SendCommand(cmd);
			}
		}
	}
	if (c==116)
	{//F5
		if (nmBg!=StringID_Invalid)
		{
			BehaviorDebugState *state=GetBehaviorDebugClient()->GetState();
			if (state->IsBreaking())
			{
				BehaviorDebugCmd cmd;
				cmd.tp=BehaviorDebugCmd::Continue;
				GetBehaviorDebugClient()->SendCommand(cmd);
			}
		}
	}

	return TRUE;
}



//////////////////////////////////////////////////////////////////////////

CBehaviorGraphEditPanel::CBehaviorGraphEditPanel()
{
	_anchor.SetResType(Res_BehaviorGraph);
	_anchor.SetLabel("BehaviorGraph");

	_dataLast=NULL;

	_verStrLib=0xffffffff;

}

CBehaviorGraphEditPanel::~CBehaviorGraphEditPanel(void)
{
	ResData_Delete(_dataLast);

}

void CBehaviorGraphEditPanel::Init3d()
{
}
void CBehaviorGraphEditPanel::Clear3d()
{
}

BEGIN_MESSAGE_MAP(CBehaviorGraphEditPanel,CResEditPanel)
	ON_WM_SIZE()
	ON_WM_TIMER()

END_MESSAGE_MAP()

void CBehaviorGraphEditPanel::DoDataExchange(CDataExchange* pDX)
{	
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX,IDC_BGANCHOR,_anchor);
}

BOOL CBehaviorGraphEditPanel::OnInitDialog()
{
	CResEditPanel::OnInitDialog();

	//grid
	if (TRUE)
	{
		CRect rc;
		GET_CONTROL_RECT(this,IDC_BGGRID,rc);
		HIDE_CONTROL(this,IDC_BGGRID);
		_grid.Create(rc,this,IDC_BGGRID);
		_grid.SetOwner(this);
	}

	AddCtrl(dynamic_cast<CResEditCtrl*>(&_grid));

	_util.Init(g_ssGuiLib.pRS->GetPath(Path_BehaviorGraph),(BgpClasses*)g_ssGuiLib.pRS->GetBehaviorGraphMgr()->GetClasses());

	return TRUE;  // 除非设置了控件的焦点，否则返回 TRUE
}



void CBehaviorGraphEditPanel::OnResDataChange(ResData *dataNew)
{
	_stateToMod->SetData(dataNew);
	if(!dataNew)  return;

	//初始化GG Transform
	if (TRUE)
	{
		BehaviorGraphData *data=(BehaviorGraphData *)_stateToMod->resdata;
		i_math::pos2df off,scale;
		if (data->pads.GetFolderXfm(data->pads.GetCurFolder(),off,scale))
			_view2->SetTransformGG(off,scale);
	}



}

void CBehaviorGraphEditPanel::Draw(IRenderPort *rp)
{
}


void CBehaviorGraphEditPanel::Draw(GraphicsGraph*gg)
{	
	Reps_BehaviorGraph *state=(Reps_BehaviorGraph *)_stateToMod;
	if (!state)
		return;
	BehaviorGraphData *data=(BehaviorGraphData *)state->resdata;

	state->GetGraph()->Draw(gg,&state->sels[0],state->sels.size());

}

//返回值0: 没有变化,1:有轻微的变化,2:有巨大的变化
static int CheckChange(BehaviorGraphData *data1,BehaviorGraphData *data2)
{
	if ((!data1)&&(data2))
		return 2;//巨大变化
	if ((!data2)&&(data1))
		return 2;//巨大变化

	if (data1->Equal(*data2))
		return 0;//没有变化

	//检查是轻微变化还是巨大变化
	BehaviorGraphData *dataNew=(BehaviorGraphData *)data1->Clone();
	BehaviorGraphData *dataLast=(BehaviorGraphData *)data2->Clone();
	DWORD n,n2;
	CLinkPad **buf=dataLast->pads.GetPads(n);
	CLinkPad **buf2=dataNew->pads.GetPads(n2);

	BOOL bChanged=TRUE;
	if (n==n2)
	{
		//把位置改成同样的
		for (int i=0;i<n;i++)
			buf[i]->SetPos(buf2[i]->GetPos());

		//FolderXfm全部清除
		dataLast->pads.ClearFolderXfm();
		dataNew->pads.ClearFolderXfm();

		if (dataLast->Equal(*dataNew))
			bChanged=FALSE;
	}
	ResData_Delete(dataNew);
	ResData_Delete(dataLast);

	return bChanged?2:1;
}

void CBehaviorGraphEditPanel::_Copy(ResData *dest,ResData *src)
{
	if ((!src)||(!dest))
		return;

	((BehaviorGraphData *)dest)->pads.SetClasses(g_ssGuiLib.pRS->GetBehaviorGraphMgr()->GetClasses());
	dest->Copy(*src);
}

BOOL CBehaviorGraphEditPanel::RepairState(ResEditPanelState *state)
{
	CBehaviorGraphPads *pads=&((BehaviorGraphData *)((Reps_BehaviorGraph*)_stateToMod->resdata))->pads;

	StringID nm=pads->GetName();
	pads->Clear();

	_util.LoadBGPads(nm,*pads);

	return TRUE;
}


void CBehaviorGraphEditPanel::RefreshStateMod(BOOL bSave)
{
	_util.UnresolveBGPads(((BehaviorGraphData *)((Reps_BehaviorGraph*)_stateToMod->resdata))->pads);
	__super::RefreshStateMod(bSave);
}


ResEditPanelState *CBehaviorGraphEditPanel::_GetStateToSave()
{
	Reps_BehaviorGraph *stateToSave=new Reps_BehaviorGraph;
	if (_stateToMod->resdata)
	{
		stateToSave->Copy(*_stateToMod);
		_util.UnresolveBGPads(((BehaviorGraphData *)((Reps_BehaviorGraph*)stateToSave->resdata))->pads);
	}
	return stateToSave;
}



BOOL CBehaviorGraphEditPanel::StateToControl(ResEditPanelState *state0)//Update the controls in the panel to reflect the state
{	
	_util.ResolveBGPads(((BehaviorGraphData *)((Reps_BehaviorGraph*)state0->resdata))->pads);
	FillDescAssist_GuiLib assist;
	assist.SetGG(_view2->GetGG());
	assist.SetBehaviorGraphPads(&((BehaviorGraphData *)((Reps_BehaviorGraph*)state0->resdata))->pads);
	_util.Repos(((BehaviorGraphData *)((Reps_BehaviorGraph*)state0->resdata))->pads,&assist);


	if (!CResEditPanel::StateToControl(state0))
		return FALSE;

	Reps_BehaviorGraph *state=(Reps_BehaviorGraph *)state0;
	BehaviorGraphData *data=(BehaviorGraphData *)state->resdata;

	if (data)
		g_ssGuiLib.pUtilRS->RepairResData(data);

	DWORD verStrLib=StrLib_Get()->GetModifyVer();

	//Unresolve _dataLast
	if (_dataLast)
		assert(_dataLast->pads.IsResolved());

	if ((CheckChange(_dataLast,data)>0)||(_verStrLib!=verStrLib))
	{//有一点点变化

		_graph.Clear();
		_graph.Load(&data->pads);
		_graph.RecalcLayout(_view2->GetGG());

		if (!_dataLast)
			_dataLast=(BehaviorGraphData *)data->GetClass()->New();

		_Copy(_dataLast,data);
	}

	_verStrLib=verStrLib;

	return TRUE;
}


ResEditPanelState *CBehaviorGraphEditPanel::_NewState()
{
	 Reps_BehaviorGraph * state =  new Reps_BehaviorGraph;
	 return state;
}

void CBehaviorGraphEditPanel::_UpdateView2Agent()
{
	static PadsCB cb;
	ClearAgent_View2();
	if (_bEnable)
	{
		_view2->AddAgent(0,new CGuiAgent_GraphPadScroll(&_src),AGENTPRIORITY_STANDARD+1);	
		_view2->AddAgent(0,new CGuiAgent_GraphPadSel(&_src,FALSE),AGENTPRIORITY_STANDARD+12);	
		_view2->AddAgent(0,new CGuiAgent_GraphPadRectSel(&_src),AGENTPRIORITY_STANDARD+4);	
		_view2->AddAgent(0,new CGuiAgent_GraphPadConnect(&_src),AGENTPRIORITY_STANDARD+6);	
		_view2->AddAgent(0,new CGuiAgent_GraphPadCommand(&_src,&cb),AGENTPRIORITY_STANDARD+6);	
		_view2->AddAgent(0,new CGuiAgent_NewBgPads(),AGENTPRIORITY_STANDARD+6);	
		_view2->AddAgent(0,new CGuiAgent_BgPadsCommand(&_src),AGENTPRIORITY_STANDARD+10);	
	}
	else
	{
		_view2->AddAgent(0,new CGuiAgent_GraphPadScroll(&_src),AGENTPRIORITY_STANDARD+1);	
		_view2->AddAgent(0,new CGuiAgent_GraphPadSel(&_src,TRUE),AGENTPRIORITY_STANDARD+12);	
		_view2->AddAgent(0,new CGuiAgent_GraphPadRectSel(&_src),AGENTPRIORITY_STANDARD+4);	
	}
}


void CBehaviorGraphEditPanel::OnSelect()
{
	ClearAgent();

	_AddCameraController();

	_UpdateView2Agent();
}

void CBehaviorGraphEditPanel::EnablePanel(BOOL bEnable)
{
	BOOL bChange=(bEnable!=_bEnable);

	CResEditPanel::EnablePanel(bEnable);

	if (bChange)
		_UpdateView2Agent();
}

void CBehaviorGraphEditPanel::OnSize(UINT nType, int cx, int cy)
{
	CResEditPanel::OnSize(nType, cx, cy);

	if (_grid.GetSafeHwnd())
	{
		CRect rc;
		_grid.GetWindowRect(&rc);
		ScreenToClient(&rc);
		rc.bottom=rc.top+cy-34;
		rc.right=rc.left+cx-20;
		if (rc.bottom<rc.top+4)
			rc.bottom=rc.top+4;
		if (rc.right<rc.left+4)
			rc.right=rc.left+4;
		_grid.MoveWindow(&rc);
	}
}


BOOL CBehaviorGraphEditPanel::GetAIName(StringID &nmAI)
{
	if (!_dataLast)
		return FALSE;

	DWORD c=_dataLast->pads.GetPadCount();
	for (int i=0;i<c;i++)
	{
		CLinkPad *pad=_dataLast->pads.GetPad(i);
		if (pad)
		{
			if (pad->GetClass()->CheckName("CBgp_Graph"))
			{
				CBgp_Graph* p=(CBgp_Graph* )pad;
				nmAI=p->_nm;
				return TRUE;
			}
		}
	}
	return FALSE;
}



BOOL CBehaviorGraphEditPanel::GetMapInfo(RecordID &idMap,std::string &nmMap)
{
	nmMap="";
	if (!_dataLast)
		return FALSE;

	DWORD c=_dataLast->pads.GetPadCount();
	for (int i=0;i<c;i++)
	{
		CLinkPad *pad=_dataLast->pads.GetPad(i);
		if (pad)
		{
			if (pad->GetClass()->CheckName("CBgp_Graph"))
			{
				CBgp_Graph* p=(CBgp_Graph* )pad;

				extern RecordID SeekMapRecordIDFromLevelAI(StringID nmAI,std::string &nmMap);
				idMap=SeekMapRecordIDFromLevelAI(p->_nm,nmMap);
				if (idMap!=RecordID_Invalid)
					return TRUE;
			}
		}
	}
	return FALSE;
}

void CBehaviorGraphEditPanel::UpdateUI()
{
    CBehaviorGraphPads *pads = &((BehaviorGraphData *)((Reps_BehaviorGraph*)_stateToMod->resdata))->pads;

    StringID nm = pads->GetName();

//     BehaviorDebugCmd cmd;
//     cmd.tp = BehaviorDebugCmd::SelectObj;
//     cmd.nmBg = nm;
//     GetBehaviorDebugClient()->SendCommand(cmd);

}
