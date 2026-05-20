/********************************************************************
	created:	2010/4/14   14:05
	file path:	d:\IxEngine\Proj_GuiLib
	author:		chenxi
	
	purpose:	所有CLinkPads 的graph的基类
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"
#include ".\GuiLib.h"

#include "GraphPads.h"

#include "stringparser/stringparser.h" 

#include "graphicsgraph.h"

#define MAX_ITEM_WIDTH 200

#define GRID_UNIT 12



//////////////////////////////////////////////////////////////////////////
//GraphPadItem

void GraphPadItem::UpdateSize(GraphicsGraph *gg)
{
	sz=gg->MessureText(show.c_str());
	if (sz.w>MAX_ITEM_WIDTH)
		sz.w=MAX_ITEM_WIDTH;
}


i_math::pos2di GraphPadItem::GetConnectSpot(BOOL bConnecting)
{
	i_math::pos2di pt;
	if (!bConnecting)
		pt.set(rcFocus.Right()+2,rcFocus.getCenter().y);
	else
		pt.set(rcFocus.Left()-2,rcFocus.getCenter().y);

	return pt;
}

BOOL GraphPadItem::IsFocus()
{
	if (!graph)
		return FALSE;
	return graph->GetFocus()==this;
}

PadStubType GraphPadItem::GetStubType()
{
	if (!graph)
		return PadStub_None;

	CLinkPads *pads=graph->GetPads();
	CLinkPad *pad=pads->FindPad(id);
	if (!pad)
		return PadStub_None;
	if (iStub<0)
		return PadStub_None;
	PadStub stb=pad->GetStub(iStub);
	return stb.type;
}



//////////////////////////////////////////////////////////////////////////
//CGraphPad
void CGraphPad::SetPos(i_math::pos2di &pt)
{		
	_pt=pt;	
// 	_pt.scale_signed(GRID_UNIT);
// 	_pt*=GRID_UNIT;
	_bLayoutDirty=TRUE;
}


//////////////////////////////////////////////////////////////////////////
//CGraphPads

void CGraphPads::Zero()
{
	_ver=0;
	_focus=NULL;
	_pads=NULL;
}


void CGraphPads::Clear()
{
	for (int i=0;i<_buf.size();i++)
		Class_Delete(_buf[i]);
	_buf.clear();

	_connects.clear();

	Zero();
}


int CGraphPads::_FindPad(PadID id)
{
	int idx;
	PVEC_FIND_BY_ELEMENT(_buf,_id,id,idx);
	return idx;
}

CGraphPad *CGraphPads::FindPad(PadID id)
{
	int idx=_FindPad(id);
	if (idx==-1)
		return NULL;
	return _buf[idx];
}

GraphPadItem *CGraphPads::FindItem(PadID id,const char *name)
{
	CGraphPad *p=FindPad(id);
	if (!p)
		return NULL;
	return p->FindItem(name);
}


i_math::pos2di CGraphPads::GetPadPos(PadID id)
{
	i_math::pos2di pos;
	CLinkPad *pad=_pads->FindPad(id);
	if (pad)
	{
		PadID idFolder=_pads->GetCurFolder();
		if (id==idFolder)
			return pad->GetFolderPos();
		return pad->GetPos();
	}
	return i_math::pos2di(0,0);
}

void CGraphPads::SetPadPos(PadID id,i_math::pos2di &pos)
{
	CLinkPad *pad=_pads->FindPad(id);
	if (pad)
	{
		PadID idFolder=_pads->GetCurFolder();
		if (id==idFolder)
			pad->SetFolderPos(pos);
		else
			pad->SetPos(pos);
	}
}

void CGraphPads::_FillGraphPad(CGraphPad *gpad,CLinkPad *pad)
{
	gpad->_id=pad->GetID();
	gpad->_pt=GetPadPos(pad->GetID());
	gpad->_bFolder=pad->IsFolder();
	gpad->_bCurFolder=(_pads->GetCurFolder()==pad->GetID());
}


void CGraphPads::Load(CLinkPads *pads)
{
	Clear();

	_pads=pads;

	//载入 pads
	if (TRUE)
	{
		PadID idFolder=pads->GetCurFolder();
		DWORD count;
		PadID *ids=pads->GetInFolders(idFolder,count);

		if (idFolder!=PadID_Null)
			_buf.push_back(_LoadPad(pads->FindPad(idFolder)));
		for (int i=0;i<count;i++)
		{
			CGraphPad *pad=_LoadPad(_pads->FindPad(ids[i]));
			_buf.push_back(pad);
		}
	}

	_SortPads();//给子类一个机会重新调整pad的顺序

	//载入links
	DWORD nPads;
	CLinkPad **buf=pads->GetPads(nPads);
	DWORD nLinks;
	CLinkPads::Link *links=pads->GetLinks(nLinks);

	DWORD nConns=0;
	_connects.resize(nLinks);

	CLinkPad *pad;
	PadStub stb;
	for (int i=0;i<nLinks;i++)
	{
		CLinkPads::Link *link=&links[i];
		_Connect *p=&_connects[nConns];

		for (int j=0;j<2;j++)
		{
			pad=buf[link->iPad[j]];
			stb=pad->GetStub(link->iStub[j]);
			p->item[j]=FindItem(pad->GetID(),stb.name);
		}

		if (p->item[0]&&p->item[1])
			nConns++;
	}

	_connects.resize(nConns);
}

void CGraphPads::_DrawDynConnect(GraphicsGraph *gg,ConnectDyn &conn)
{
	if (!conn.IsEmpty())
	{
		i_math::pos2di pt1,pt2;
		pt2=conn.pt;
		for (int i=0;i<conn.items.size();i++)
		{
			switch(conn.type)
			{
			case ConnectDyn::Connecting:
				pt1=conn.items[i]->GetConnectSpot(TRUE);
				gg->DrawConnectH(pt1,pt2,RGB(0,0,0),DRAWCONNECT_INV);
				break;
			case ConnectDyn::Connected:
			case ConnectDyn::Void:
				pt1=conn.items[i]->GetConnectSpot(FALSE);
				gg->DrawConnectH(pt2,pt1,RGB(0,0,0),DRAWCONNECT_INV);
				break;
			}
		}
	}
}

void CGraphPads::_DrawPermConnect(GraphicsGraph *gg)
{
	//draw the permanent connects
	for (int i=0;i<_connects.size();i++)
	{
		_Connect *p=&_connects[i];
		if (p->item[0]&&p->item[1])
		{
			gg->DrawConnectH(p->item[0]->GetConnectSpot(TRUE),
				p->item[1]->GetConnectSpot(FALSE),RGB(0,0,0),DRAWCONNECT_INV);
		}
	}
}

void CGraphPads::RecalcLayout(GraphicsGraph *gg)
{
	if (!gg)
		return;
	for (int i=0;i<_buf.size();i++)
	{
		if (_buf[i]->_bLayoutDirty)
		{
			_buf[i]->RecalcLayout(gg);
			_buf[i]->_bLayoutDirty=FALSE;
		}
	}
}


void CGraphPads::Draw(GraphicsGraph *gg,PadID *sels,DWORD c)
{
	for (int i=0;i<_buf.size();i++)
	{
		BOOL bHilight=FALSE;
		for (int j=0;j<c;j++)
		{
			if (sels[j]==_buf[i]->_id)
			{
				bHilight=TRUE;
				break;
			}
		}
		if (_buf[i]->_bLayoutDirty)
		{
			_buf[i]->RecalcLayout(gg);
			_buf[i]->_bLayoutDirty=FALSE;
		}
		_buf[i]->Draw(gg,bHilight);
	}

	//draw the dynamic connect
	_DrawDynConnect(gg,_connectDyn);
	_DrawPermConnect(gg);
}

BOOL CGraphPads::HitTest(int x,int y,GraphPadHit &hit)
{
	for(int i=_buf.size()-1;i>=0;i--)
	{
		if (_buf[i]->HitTest(x,y,hit))
			return TRUE;
	}
	return FALSE;
}

PadID *CGraphPads::RectHitTest(i_math::recti &rc,DWORD &c)
{
	_temp2.clear();
	for(int i=_buf.size()-1;i>=0;i--)
	{
		if (rc.isRectCollided(_buf[i]->_rc))
			_temp2.push_back(_buf[i]->_id);
	}

	c=_temp2.size();
	return _temp2.data();
}


BOOL CGraphPads::SetFocusItem(GraphPadItem *item)
{
	if (item==_focus)
		return FALSE;
	_focus=item;
	return TRUE;
}

void CGraphPads::ClearFocusConnect()
{
	for (int i=0;i<_connectDyn.items.size();i++)
		Class_Delete(_connectDyn.items[i]);
	_connectDyn.items.clear();
}


GraphPadItem **CGraphPads::GetConnects(GraphPadItem *item,BOOL bConnecting,DWORD &c)
{
	_temp.clear();
	DWORD idx=bConnecting?0:1;
	for (int i=0;i<_connects.size();i++)
	{
		_Connect *p=&_connects[i];
		if (p->item[idx]==item)
			_temp.push_back(p->item[1-idx]);
	}
	c=_temp.size();
	return _temp.data();
}
