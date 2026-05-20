/********************************************************************
	created:	2010/4/14   15:09
	file path:	d:\IxEngine\Proj_GuiLib
	author:		chenxi
	
	purpose:	用来编辑GraphPads的Agent
*********************************************************************/

#include "stdh.h"

#include "WorldSystem/IWorldSystem.h"

#include "GraphPads.h"

#include "GuiAgent_GraphPads.h"

#include <vector>
#include <string>

#include "stringparser/stringparser.h"
 

#include "commondefines/general_stl.h"

#include "AgentCmdID.h"

#include "graphicsgraph.h"

#include "EditPopup.h"


//////////////////////////////////////////////////////////////////////////
//CGuiAgent_GraphPadScroll
void CGuiAgent_GraphPadScroll::OnUpdateTransform(const i_math::pos2df & pos,const i_math::pos2df &scale)
{
	CGraphPads *graph=_GetGraph();
	CLinkPads *pads=graph->GetPads();

	pads->SetFolderXfm(pads->GetCurFolder(),(i_math::pos2df&)pos,(i_math::pos2df&)scale);
}



//////////////////////////////////////////////////////////////////////////
//CGuiAgent_GraphPadSel
void CGuiAgent_GraphPadSel::_SelectPad(PadID id)
{
	std::vector<PadID>*sels=_GetSelBuf();
	if (!sels)
		return;
	if (id==PadID_Null)
		sels->clear();
	else
	{
		int idx;
		VEC_FIND(*sels,id,idx);
		if (idx==-1)
		{
			sels->resize(1);
			(*sels)[0]=id;
			_Redraw(FALSE);
		}
	}
}


BOOL CGuiAgent_GraphPadSel::OnBeginDrag(int x,int y,DWORD flag)
{
	_ScreenToGG(x,y);

	std::vector<PadID>*sels=_GetSelBuf();
	CGraphPads *graph=_GetGraph();
	CLinkPads *pads=graph->GetPads();

	_sels.clear();
	_starts.clear();

	GraphPadHit hit;
	BOOL bHit=graph->HitTest(x,y,hit);
	_SelectPad(hit.id);
	_Redraw(FALSE);

	if (_bReadOnly)
		return FALSE;
	if (bHit)
	{
		if (hit.id!=PadID_Null)
		{
			if (hit.part==GraphPadHit::Blank)
			{
				_sels=*sels;
				_ModDrags(_sels);
				_starts.resize(_sels.size());
				for (int i=0;i<_sels.size();i++)
					_starts[i]=graph->GetPadPos(_sels[i]);
				_pt.set(x,y);
				return TRUE;
			}
		}
	}

	_NotifyChange(FALSE);

	return FALSE;
}

void CGuiAgent_GraphPadSel::OnDrag(int x,int y,DWORD flag)
{
	CGraphPads *graph=_GetGraph();
	CLinkPads *pads=graph->GetPads();

	_ScreenToGG(x,y);

	i_math::pos2di off(x,y);
	off-=_pt;

	if (_sels.size())
	{
		for (int i=0;i<_sels.size();i++)
		{
			graph->SetPadPos(_sels[i],_starts[i]+off);

			CGraphPad *gpad=graph->FindPad(_sels[i]);
			if (gpad)
				gpad->SetPos(_starts[i]+off);
		}
	}

	_Redraw(FALSE);

}


void CGuiAgent_GraphPadSel::OnEndDrag(int x,int y,DWORD flag)
{
	_NotifyChange(TRUE);
	_Redraw(FALSE);
}

BOOL CGuiAgent_GraphPadSel::OnRButtonDown(int x,int y,DWORD flag)
{
	std::vector<PadID>*sels=_GetSelBuf();

	if (sels->size()>0)
		return TRUE;//对于右键来说,如果原来有选中的东西,我们什么也不能改变

	return OnRButtonClick(x,y,flag);
}

BOOL CGuiAgent_GraphPadSel::OnRButtonClick(int x,int y,DWORD flag)
{
	CGraphPads *graph=_GetGraph();

	_ScreenToGG(x,y);

	GraphPadHit hit;
	graph->HitTest(x,y,hit);
	_SelectPad(hit.id);
	_Redraw(FALSE);

	return TRUE;

}


BOOL CGuiAgent_GraphPadSel::OnTimer(int dt,DWORD flag)
{
	CGraphPads *graph=_GetGraph();

	i_math::pos2di ptCursor;
	_GetCursorPos(ptCursor);

	_ScreenToGG(ptCursor.x,ptCursor.y);

	if (_IsInMenu())
		return TRUE;

	GraphPadHit hit;
	BOOL bChanged;
	if (graph->HitTest(ptCursor.x,ptCursor.y,hit))
		bChanged=graph->SetFocusItem(hit.item);
	else
		bChanged=graph->SetFocusItem(NULL);

	if (bChanged)
		_Redraw(FALSE);

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//CGuiAgent_GraphPadRectSel

void CGuiAgent_GraphPadRectSel::_Sel(PadID *inrects,DWORD c)
{
	std::vector<PadID>*sels=_GetSelBuf();

	*sels=_initials;

	for (int i=0;i<c;i++)
	{
		PadID id=inrects[i];

		int idx;
		VEC_FIND(*sels,id,idx);
		if (idx==-1)
		{//原来没有,我们添加
			sels->push_back(id);
		}
		else
		{//原来有的,我们删除
			sels->erase(sels->begin()+idx);
		}
	}
}


BOOL CGuiAgent_GraphPadRectSel::OnBeginDrag(int x,int y,DWORD flag)
{
	std::vector<PadID>*sels=_GetSelBuf();
	CGraphPads *graph=_GetGraph();

	_ScreenToGG(x,y);

	if (TRUE)
	{
		GraphPadHit hit;
		BOOL bHit=graph->HitTest(x,y,hit);

		if (bHit)
			return FALSE;
	}

	if (flag&CtrlOpFlag_CtrlDown)
		_initials=*sels;
	else
		_initials.clear();

	_start.set(x,y);
	_rcDraw.set(x,y,x,y);

	return TRUE;
}

void CGuiAgent_GraphPadRectSel::OnDrag(int x,int y,DWORD flag)
{
	CGraphPads *graph=_GetGraph();
	_ScreenToGG(x,y);

	i_math::recti rc;
	rc.set(_start.x,_start.y,x,y);
	rc.repair();

	DWORD c;
	PadID *ids=graph->RectHitTest(rc,c);

	_Sel(ids,c);

	_rcDraw=rc;

	_Redraw(FALSE);
}


void CGuiAgent_GraphPadRectSel::OnEndDrag(int x,int y,DWORD flag)
{
	OnDrag(x,y,flag);

	_NotifyChange(FALSE);
	_Redraw(FALSE);

}

BOOL CGuiAgent_GraphPadRectSel::OnDraw()
{
	if (!_bInDrag)
		return TRUE;
	GraphicsGraph *gg=GetGG();

	gg->DrawFrameRect(_rcDraw,0x00ff00,1,128);
	return TRUE;
}



//////////////////////////////////////////////////////////////////////////
//CGuiAgent_GraphPadConnect
BOOL CGuiAgent_GraphPadConnect::OnBeginDrag(int x,int y,DWORD flag)
{
	CGraphPads *graph=_GetGraph();
	CLinkPads *pads=graph->GetPads();

	_ScreenToGG(x,y);

	ConnectDyn conn;
	GraphPadHit hit;
	if (graph->HitTest(x,y,hit))
	{
		switch(hit.part)
		{
			case GraphPadHit::Out:
			case GraphPadHit::COut:
			{
				conn.type=(hit.part==GraphPadHit::Out)?ConnectDyn::Connecting:ConnectDyn::ConnectingC;
				conn.AddItem(hit.item);
				conn.pt.set(x,y);
				break;
			}
			case GraphPadHit::In:
			case GraphPadHit::CIn:
			{
				DWORD n;
				GraphPadItem **items=graph->GetConnects(hit.item,FALSE,n);
				if (n>0)
				{
					pads->RemoveLink(hit.id,hit.item->name.c_str());
					for (int i=0;i<n;i++)
						conn.AddItem(items[i]);
					conn.pt=hit.item->GetConnectSpot(FALSE);
					conn.type=(hit.part==GraphPadHit::In)?ConnectDyn::Connecting:ConnectDyn::ConnectingC;
				}
				else
				{
					conn.AddItem(hit.item);
					conn.pt.set(x,y);
					conn.type=(hit.part==GraphPadHit::In)?ConnectDyn::Connected:ConnectDyn::ConnectedC;
				}
				break;
			}
			default:
				break;
		}
	}

	if (conn.IsEmpty())
		return FALSE;

	_conn=conn;
	graph->SetFocusConnect(_conn);

	_NotifyChange(FALSE);

	_Redraw(FALSE);

	return TRUE;
}
 

void CGuiAgent_GraphPadConnect::OnDrag(int x,int y,DWORD flag)
{
	CGraphPads *graph=_GetGraph();
	_ScreenToGG(x,y);

	_conn.pt.set(x,y);
	GraphPadHit hit;
	if (graph->HitTest(x,y,hit))
	{
		if (hit.item)
		{
			if (_conn.type==ConnectDyn::Connecting)
			{
				if (hit.part==GraphPadHit::In)
					_conn.pt=hit.item->GetConnectSpot(FALSE);
			}
			if (_conn.type==ConnectDyn::Connected)
			{
				if (hit.part==GraphPadHit::Out)
					_conn.pt=hit.item->GetConnectSpot(TRUE);
			}
			if (_conn.type==ConnectDyn::ConnectingC)
			{
				if (hit.part==GraphPadHit::CIn)
					_conn.pt=hit.item->GetConnectSpot(FALSE);
			}
			if (_conn.type==ConnectDyn::ConnectedC)
			{
				if (hit.part==GraphPadHit::COut)
					_conn.pt=hit.item->GetConnectSpot(TRUE);
			}
		}
	}

	graph->SetFocusConnect(_conn);


	_Redraw(FALSE);
}

void CGuiAgent_GraphPadConnect::OnEndDrag(int x,int y,DWORD flag)
{
	CGraphPads *graph=_GetGraph();
	CLinkPads *pads=graph->GetPads();

	_ScreenToGG(x,y);

	GraphPadItem *item=NULL;
	GraphPadHit hit;
	if (graph->HitTest(x,y,hit))
	{
		if (hit.item)
		{
			if (_conn.type==ConnectDyn::Connecting)
			{
				if (hit.part==GraphPadHit::In)
					item=hit.item;
			}
			if (_conn.type==ConnectDyn::Connected)
			{
				if (hit.part==GraphPadHit::Out)
					item=hit.item;
			}
			if (_conn.type==ConnectDyn::ConnectingC)
			{
				if (hit.part==GraphPadHit::CIn)
					item=hit.item;
			}
			if (_conn.type==ConnectDyn::ConnectedC)
			{
				if (hit.part==GraphPadHit::COut)
					item=hit.item;
			}
		}
	}

	if ((item)&&(!_conn.IsEmpty()))
	{
		assert((_conn.type==ConnectDyn::Connecting)||(_conn.type==ConnectDyn::Connected)||
			(_conn.type==ConnectDyn::ConnectingC)||(_conn.type==ConnectDyn::ConnectedC)			);
		for(int i=0;i<_conn.items.size();i++)
		{
			if ((_conn.type==ConnectDyn::Connecting)||(_conn.type==ConnectDyn::ConnectingC))
				pads->AddLink(_conn.items[i]->id,_conn.items[i]->name.c_str(),
											item->id,item->name.c_str());
			else
				pads->AddLink(item->id,item->name.c_str(),
											_conn.items[i]->id,_conn.items[i]->name.c_str());
		}
	}

	graph->ClearFocusConnect();

	_NotifyChange(TRUE);

	_Redraw(FALSE);
}

//////////////////////////////////////////////////////////////////////////
//PadsCB

int PadsCB::Find(PadID id)
{
	int idx;
	VEC_FIND_BY_ELEMENT(pads,id,id,idx);
	return idx;
}


BOOL PadsCB::Exist(PadID id)
{
	return Find(id)!=-1;
}


//////////////////////////////////////////////////////////////////////////
//CGuiAgent_GraphPadCommand

void CGuiAgent_GraphPadCommand::_TransformGGtoPads(CLinkPads *pads)
{
	i_math::pos2df off,scale;
	_GetTransformGG(off,scale);
	pads->SetFolderXfm(pads->GetCurFolder(),off,scale);
}

void CGuiAgent_GraphPadCommand::_TransformGGfromPads(CLinkPads *pads)
{
	i_math::pos2df off,scale;
	if (pads->GetFolderXfm(pads->GetCurFolder(),off,scale))
		_SetTransformGG(off,scale);
}

BOOL CGuiAgent_GraphPadCommand::OnMouseMove(int x,int y,DWORD flag)
{
	_iSelFolderLabel=-1;
	for (int i=0;i<_labelsFolder.size();i++)
	{
		if (_labelsFolder[i].rc.isPointInside(x,y))
		{
			_iSelFolderLabel=i;
			_labelSelFolder=_labelsFolder[i];
		}
	}

	if (_iSelFolderLabel!=-1)
		return FALSE;
	return __super::OnMouseMove(x,y,flag);
}

BOOL CGuiAgent_GraphPadCommand::OnLButtonClick(int x,int y,DWORD flag)
{
	CGraphPads *graph=_GetGraph();
	CLinkPads *pads=graph->GetPads();

	_TransformGGtoPads(pads);

	for (int i=0;i<_labelsFolder.size();i++)
	{
		if (_labelsFolder[i].rc.isPointInside(x,y))
		{
			pads->PopToFolder(_labelsFolder[i].idPad);

			//更新Transform GG
 			_TransformGGfromPads(pads);
			_TransformGGtoPads(pads);

			_NotifyChange(TRUE);
			_Redraw(FALSE);
			return FALSE;
		}
	}

	return __super::OnLButtonClick(x,y,flag);
}



BOOL CGuiAgent_GraphPadCommand::OnDraw()
{
	CGraphPads *graph=_GetGraph();
	CLinkPads *pads=graph->GetPads();

	DWORD count;
	PadID *ids=pads->GetFolderStack(count);

	_labelsFolder.clear();
	_labelsFolder.reserve(count);

	std::vector<std::string> buf;
	for (int i=0;i<count;i++)
	{
		CLinkPad *pad=pads->FindPad(ids[i]);
		if (pad)
		{
			if (pad->IsFolder())
			{
				const char *name=pad->GetFolderShowName();
				if (!name[0])
					buf.push_back(std::string("<No Name>"));
				else
					buf.push_back(std::string(name));
				FolderLabel label;
				label.idPad=ids[i];
				_labelsFolder.push_back(label);
			}
		}
	}

	//更新labels
	if (TRUE)
	{
		GraphicsGraph *gg=GetGG();
		i_math::pos2df off,scale;
		gg->GetTranform(off,scale);
		gg->ResetTransform();

		const int extend=8;
		const int gap=4;
		int x=0;
		for (int i=0;i<buf.size();i++)
		{
			i_math::size2di sz;
			sz=gg->MessureText(buf[i].c_str(),1000);
			sz.w+=extend;
			i_math::recti rc;
			rc.set(i_math::pos2di(x,0),sz);
			x+=sz.w;
			x+=gap;
			_labelsFolder[i].rc=rc;

			BOOL bSel=FALSE;
			if (_iSelFolderLabel==i)
			{
				if (_labelsFolder[i].idPad==_labelSelFolder.idPad)
					bSel=TRUE;
			}

			if (!bSel)
				gg->DrawRoundCornerRect(rc,4,0x00ff00,0x007f00);
			else
				gg->DrawRoundCornerRect(rc,4,0xffffff,0x00ff00);

			gg->DrawText(buf[i].c_str(),rc,DT_CENTER,FALSE,0x000000);
		}

		gg->Transform(off,scale);
	}

	if (TRUE)
	{
		BOOL bNeedResetSel=TRUE;
		if (_iSelFolderLabel>=0)
		{
			if (_iSelFolderLabel<_labelsFolder.size())
			{
				if (_labelsFolder[_iSelFolderLabel].idPad==_labelSelFolder.idPad)
					bNeedResetSel=FALSE;
			}
		}
		if (bNeedResetSel)
		{
			_iSelFolderLabel=-1;
		}
	}

	return TRUE;
}


BOOL CGuiAgent_GraphPadCommand::OnRButtonClick(int x,int y,DWORD flag)
{
	CGraphPads *graph=_GetGraph();
	CLinkPads *pads=graph->GetPads();
	_ScreenToGG(x,y);

	_pt.set(x,y);

	GraphPadHit hit;
	if (graph->HitTest(x,y,hit))
	{
		if (hit.id!=PadID_Null)
		{
			_AddMenu("删除",ID_AGENT_REMOVE_PADS);
			_AddMenu("复制",ID_AGENT_COPY_PADS);

			_AddMenuSep();
			if (TRUE)
			{
				std::vector<PadID>*sels=_GetSelBuf();
				if (sels->size()==1)
				{
					PadID idSel=(*sels)[0];
					CLinkPad *pad=pads->FindPad(idSel);
					if (pad->IsFolder())
					{
						_AddMenu("UnFold",ID_AGENT_UNFOLD_PAD);
						_AddMenu("改名",ID_AGENT_RENAME_PADFOLDER);
					}
					else
					{
						if (pads->CanFold(idSel))
							_AddMenu("Fold",ID_AGENT_FOLD_PAD);
					}
				}
			}
			_AddMenuSep();


			return FALSE;
		}
	}

	if (_cb)
	if (!_cb->IsEmpty())
		_AddMenu("粘帖",ID_AGENT_PASTE_PADS);
	
	return FALSE;
}

void CGuiAgent_GraphPadCommand::_Copy(std::vector<PadID>*sels,CLinkPads *pads)
{
	if (!_cb)
		return;
	_cb->Clear();

	i_math::recti rc;
	std::vector<CLinkPad *>buf;
	for (int i=0;i<sels->size();i++)
	{
		PadID id=(*sels)[i];
		CLinkPad *pad=pads->FindPad(id);
		if (pad)
		{
			rc.merge(pad->GetPos());
			buf.push_back(pad);
		}
	}

	DWORD nSel=buf.size();//选中的pad的个数

	for (int i=0;i<sels->size();i++)
	{
		PadID id=(*sels)[i];
		CLinkPad *pad=pads->FindPad(id);
		if (pad)
		{
			if (pad->IsFolder()&&(id!=pads->GetCurFolder()))
			{
				DWORD c;
				PadID *subs=pads->GetFolderSubs(id,c);
				for (int j=0;j<c;j++)
				{
					CLinkPad *pad=pads->FindPad(subs[j]);
					if (pad)
						UNIQUE_VEC_ADD(buf,pad);
				}
			}
		}
	}

	_cb->pads.resize(buf.size());

	for (int i=0;i<buf.size();i++)
	{
		CLinkPad *pad=buf[i];
		PadsCB::Pad *padCB=&_cb->pads[i];

		padCB->classname=pad->GetClass()->GetName();
		padCB->id=pad->GetID();
		padCB->name=pad->GetName();
		padCB->bSub=(i>=nSel);
		padCB->ptOff=pad->GetPos()-rc.UpperLeftCorner;
		padCB->nameFolder=pad->GetFolderName();
		padCB->bFolder=pad->IsFolder()&&(pad->GetID()!=pads->GetCurFolder());
		padCB->ptFolder=pad->GetFolderPos();
		padCB->idFolder=pad->GetFolder();
		padCB->pt=pad->GetPos();

		CDataPacket dp;
		DP_BeginSave(dp,padCB->data);
			SaveGObj(dp,pad->GetGObj());
		DP_EndSave();
	}

	DWORD nLinks;
	CLinkPads::LinkPersist *links=pads->GetPersistLinks(nLinks);

	for (int i=0;i<nLinks;i++)
	{
		CLinkPads::LinkPersist *link=&links[i];
		if (_cb->Exist(link->idPad[0])&&_cb->Exist(link->idPad[1]))
			_cb->links.push_back(*link);
	}
}

void CGuiAgent_GraphPadCommand::_Paste(CLinkPads *pads)
{
	if (!_cb)
		return;

	std::unordered_map<PadID,PadID> lookup;
	for (int i=0;i<_cb->pads.size();i++)
	{
		PadsCB::Pad *padCB=&_cb->pads[i];

		i_math::pos2di pt;
		if(!padCB->bSub)
			pt=_pt+padCB->ptOff;
		else
			pt=padCB->pt;
		PadID id=pads->NewPad(padCB->classname.c_str(),pt);
		assert(id!=PadID_Null);

		lookup[padCB->id]=id;

		CLinkPad *pad=pads->FindPad(id);

		pad->SetName(padCB->name);

		if (padCB->bFolder)
			pads->Fold(id);

		pad->SetFolderName(padCB->nameFolder.c_str());
		pad->SetFolderPos(padCB->ptFolder);

		CDataPacket dp;
		dp.SetDataBufferPointer(&padCB->data[0]);
		LoadGObj(dp,pad->GetGObj(),NULL);
	}

	PadID idCurFolder=pads->GetCurFolder();

	for (int i=0;i<_cb->pads.size();i++)
	{
		PadsCB::Pad *padCB=&_cb->pads[i];
		PadID id=lookup[padCB->id];
		CLinkPad *pad=pads->FindPad(id);

		if(!padCB->bSub)
			pad->SetFolder(idCurFolder);
		else
			pad->SetFolder(lookup[padCB->idFolder]);
	}

	for (int i=0;i<_cb->links.size();i++)
	{
		CLinkPads::LinkPersist *link=&_cb->links[i];

		PadID id[2];
		id[0]=lookup[link->idPad[0]];
		id[1]=lookup[link->idPad[1]];

		pads->AddLink(id[0],link->nameStub[0],id[1],link->nameStub[1]);
	}
}


BOOL CGuiAgent_GraphPadCommand::OnCommand(DWORD idCmd)
{
	std::vector<PadID>*sels=_GetSelBuf();
	CGraphPads *graph=_GetGraph();
	CLinkPads *pads=graph->GetPads();

	if (idCmd==ID_AGENT_REMOVE_PADS)
	{
		pads->RemovePads(&(*sels)[0],sels->size());
		_NotifyChange(TRUE);
		_Redraw(FALSE);
		return FALSE;
	}
	if (idCmd==ID_AGENT_FOLD_PAD)
	{
		if (sels->size()==1)
		{
			pads->Fold((*sels)[0]);
			_NotifyChange(TRUE);
			_Redraw(FALSE);

			CLinkPad *pad=pads->FindPad((*sels)[0]);
			if (pad)
			{
				if (!pad->GetFolderName()[0])
					OnCommand(ID_AGENT_RENAME_PADFOLDER);
			}
		}
		return FALSE;
	}
	if (idCmd==ID_AGENT_UNFOLD_PAD)
	{
		if (sels->size()==1)
		{
			pads->UnFold((*sels)[0]);
			_NotifyChange(TRUE);
			_Redraw(FALSE);
		}
		return FALSE;
	}
	if (idCmd==ID_AGENT_RENAME_PADFOLDER)
	{
		_Redraw(TRUE);
		if (sels->size()==1)
		{
			CLinkPad *pad=pads->FindPad((*sels)[0]);
			if (pad)
			{
				i_math::pos2di pt=graph->GetPadPos((*sels)[0]);
				_GGToScreen(pt.x,pt.y);
				GetWnd()->ClientToScreen((CPoint*)&pt);

				CEditPopup popup;
				std::string name=popup.Popup(pt.x,pt.y,pad->GetFolderName());
				if (name!=pad->GetFolderName())
				{
					pad->SetFolderName(name.c_str());

					_NotifyChange(TRUE);
					_Redraw(FALSE);
				}
			}
		}
	}
	if (idCmd==ID_AGENT_COPY_PADS)
	{
		_Copy(sels,pads);
		return FALSE;
	}

	if (idCmd==ID_AGENT_PASTE_PADS)
	{
		_Paste(pads);
		_NotifyChange(TRUE);
		_Redraw(FALSE);
		return FALSE;
	}

	return TRUE;
}

BOOL CGuiAgent_GraphPadCommand::OnLButtonDblClk(int x,int y,DWORD flag)
{
	CGraphPads *graph=_GetGraph();
	CLinkPads *pads=graph->GetPads();

	_TransformGGtoPads(pads);

	_ScreenToGG(x,y);

	_pt.set(x,y);

	BOOL bHandled=FALSE;

	GraphPadHit hit;
	if (graph->HitTest(x,y,hit))
	{
		if (hit.id!=PadID_Null)
		{
			CLinkPad *pad=pads->FindPad(hit.id);
			if (pad)
			{
				if (pad->IsFolder()&&(pad->GetID()!=pads->GetCurFolder()))
				{
					pads->PushFolder(hit.id);
					bHandled=TRUE;
				}
			}
		}
	}
	else
	{
		pads->PopFolder();
		bHandled=TRUE;
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
