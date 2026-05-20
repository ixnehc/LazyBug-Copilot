

#include "stdh.h"
#include "GuiAgent_Changelists.h"
#include "GuiData_Changelists.h"
#include "GuiView_Changelists.h"
#include "AgentCmdID.h"

#include "graphicsgraph.h"

#include "Log/LogDump.h"

#define SNAP_KEY_TIME(t) (t)=(t)/(ANIMTICK_PER_SECOND/200)*(ANIMTICK_PER_SECOND/200);

#define ID_AGENT_Changelists_RefreshCurChangelist 1
#define ID_AGENT_Changelists_CommitCurChangelist 2
#define ID_AGENT_Changelists_RemoveCurChangelist 3

//////////////////////////////////////////////////////////////////////////
//CGuiAgent_Changelists2DTransform

void CGuiAgent_Changelists2DTransform::OnUpdateTransform( const i_math::pos2df & pos, const i_math::pos2df &scale )
{
	CGuiAgent_2DTransform::OnUpdateTransform( pos, scale );


	_Redraw(TRUE);
}

void Resolve(float x1,float x2,float x1_t,float x2_t,float &xo,float &xs)
{
	xs=(x1_t-x2_t)/(x1-x2);
	xo=(x1_t*(x1-x2)-x1*(x1_t-x2_t))/(x1-x2);
}

void ResolveTransform(i_math::rectf &rcSrc,i_math::rectf &rcDest,i_math::pos2df &off,i_math::pos2df &scale)
{
	Resolve(rcSrc.Left(),rcSrc.Right(),rcDest.Left(),rcDest.Right(),off.x,scale.x);
	Resolve(rcSrc.Top(),rcSrc.Bottom(),rcDest.Top(),rcDest.Bottom(),off.y,scale.y);
}


BOOL CGuiAgent_Changelists2DTransform::OnRButtonClick( int x, int y, DWORD flag )
{
// 	_AddMenu( "调整合适比例", ID_AGENT_Changelists_AutoFit);

	return TRUE;
}


BOOL CGuiAgent_Changelists2DTransform::Fit(const char *path)
{
// 	CGuiData_Changelists *data=(CGuiData_Changelists *)FindData("Changelists");
// 	if (!data)
// 		return FALSE;
// 	ChangelistsGroup *pGrp=data->GetSelGroup();
// 	if (pGrp)
// 	{
// 		i_math::rectf rcSrc,rcSrcClose;
// 		if (!path[0])
// 		{
// 			rcSrc=pGrp->CalcBound(data->_selentry.c_str(),FALSE,FALSE);
// 			rcSrcClose=pGrp->CalcBound(data->_selentry.c_str(),FALSE,TRUE);
// 		}
// 		else
// 		{
// 			rcSrc=pGrp->CalcBound(path,TRUE,FALSE);
// 			rcSrcClose=pGrp->CalcBound(path,TRUE,TRUE);
// 		}
// 		if (rcSrc.isValid())
// 		{
// 			i_math::recti rc,rc2;
// 			_GetClientRect(rc);
// 			rc.cutout(0,RULERY_THICK,rc2);
// 			rc.cutout(3,RULERX_THICK,rc2);
// 
// 			if (rc.isValid())
// 			{
// 				i_math::rectf rcDest;
// 				rcDest=rc.convert<float>();
// 				Swap(rcDest.Top(),rcDest.Bottom());
// 
// 				i_math::pos2df offOrg,scaleOrg;
// 				_GetTransformGG(offOrg,scaleOrg);
// 
// 				i_math::pos2df off,scale,off2,scale2;
// 				ResolveTransform(rcSrc,rcDest,off,scale);
// 				scale.y=-scale.y;
// 				ResolveTransform(rcSrcClose,rcDest,off2,scale2);
// 				scale2.y=-scale2.y;
// 
// 				if ((off==offOrg)&&(scale==scaleOrg))
// 				{
// 					off=off2;
// 					scale=scale2;
// 				}
// 
// 				_SetTransformGG(off,scale);
// 				_Redraw(TRUE);
// 				return TRUE;
// 			}
// 		}
// 	}
	return FALSE;
}


BOOL CGuiAgent_Changelists2DTransform::OnCommand(DWORD idCmd)
{
// 	if (idCmd==ID_AGENT_Changelists_AutoFit)
// 	{
// 		Fit("");
// 
// 		return FALSE;
// 	}

	return TRUE;

}




//////////////////////////////////////////////////////////////////////////
//CGuiAgent_ChangelistsCommand

BOOL CGuiAgent_ChangelistsCommand::OnRButtonClick( int x, int y, DWORD flag )
{
// 	_AddMenu( "隐藏所有", ID_AGENT_Changelists_HideAll);

	return TRUE;
}

BOOL CGuiAgent_ChangelistsCommand::OnCommand(DWORD idCmd)
{
	CGuiData_Changelists *data=(CGuiData_Changelists *)FindData("Changelists");

// 	if (idCmd==ID_AGENT_Changelists_HideAll)
// 	{
// 		CRichGrid *grid=data->GetGrid();
// 
// 		ChangelistsGroup *grp=data->GetSelGroup();
// 
// 		std::vector<Changelists *>hides;
// 		for (int i=0;i<grp->entries.size();i++)
// 		{
// 			Ref *ref=grp->entries[i].ref;
// 			CRichGrid_ChangelistsItem*item=(CRichGrid_ChangelistsItem*)ref->GetStuff();
// 			if (!item)
// 				continue;
// 			Changelists *vs=item->GetBind();
// 			if (!vs)
// 				continue;
// 			if (vs->_bVisible)
// 				hides.push_back(vs);
// 		}
// 		if (hides.size()>0)
// 		{
// 			grid->OnBeginItemChange(NULL);
// 			for (int i=0;i<hides.size();i++)
// 				hides[i]->_bVisible=FALSE;
// 			grid->OnItemChange(NULL);
// 			grid->OnEndItemChange(NULL);
// 		}
// 		_Redraw(FALSE);
// 	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//CGuiAgent_ChangelistsEdit

CGuiAgent_ChangelistsEdit::CGuiAgent_ChangelistsEdit()
{
}

BOOL CGuiAgent_ChangelistsEdit::OnRButtonDown(int x, int y, DWORD flag)
{
	CGuiData_Changelists* data = (CGuiData_Changelists*)FindData("Changelists");
	assert(data != NULL);

	_ScreenToGG(x, y);

	data->SetSel(data->_snapshot.HitTest((float)x, (float)y));

	_Redraw(FALSE);

	return TRUE;
}


BOOL CGuiAgent_ChangelistsEdit::OnRButtonClick( int x, int y, DWORD flag )
{
	CGuiData_Changelists *data = (CGuiData_Changelists*)FindData( "Changelists" );


	_AddMenu( "Refresh", ID_AGENT_Changelists_RefreshCurChangelist);

	if (data->_sels.size() == 1)
	{
		ChangelistsSnapshot::Node *node = data->_snapshot.FindNode(data->_sels[0]);
		if (node)
		{
			if (node->_isCur)
			{
				_AddMenu("Commit", ID_AGENT_Changelists_CommitCurChangelist);
			}
		}
		if (data->_changelists)
		{
			if (!data->_changelists->HasChild(data->_sels[0]))
				_AddMenu("Delete", ID_AGENT_Changelists_RemoveCurChangelist);
		}
	}

//	_AddMenuSep();

	return TRUE;
}


BOOL CGuiAgent_ChangelistsEdit::OnLButtonDown( int x,int y,DWORD flag )
{
	CGuiData_Changelists* data = (CGuiData_Changelists*)FindData("Changelists");

	_ScreenToGG(x, y);

	int hilightIndex = -1;
	FileChangeListUID hilightUid = FileChangeListUID_Invalid;
	if (ChangelistsSnapshot::Node* node = data->_snapshot.NodeHitTest((float)x, (float)y, hilightIndex))
	{
		hilightUid = node->_uid;
	}

	data->_snapshot.SetSelected(hilightUid, hilightIndex);
	data->SetSel(hilightUid);

	if ((hilightUid != FileChangeListUID_Invalid) && (hilightIndex >= 0))
		data->RequestOpenSel();

	_Redraw(FALSE);

	return FALSE;
	
}


BOOL CGuiAgent_ChangelistsEdit::OnBeginDrag( int x, int y,DWORD flag )
{
// 	CGuiData_Changelists *data = (CGuiData_Changelists*)FindData( "Changelists" );
// 	CRichGrid *grid=data->GetGrid();
// 	if (!grid)
// 		return FALSE;
// 
// 	int idx=data->GetSelKey();
// 	if ( -1 != idx)
// 	{
// 		_ptDrag.set(x,y);
// 		_bDragLock=TRUE;
// 
// 
// 		_Redraw(FALSE);
// 
// 		return TRUE;
// 	}
	return FALSE;
}

void CGuiAgent_ChangelistsEdit::OnDrag( int x, int y, DWORD flag )
{
// 	CGuiData_Changelists *data = (CGuiData_Changelists*)FindData( "Changelists" );
// 	CRichGrid *grid=data->GetGrid();
// 	if (!grid)
// 		return;
// 
// 	ChangelistsEntry *entry=data->GetSelEntry();
// 	if (!entry)
// 		return;
// 
// 	if (_bDragLock)
// 	{
// 		if (abs(x-_ptDrag.x)+abs(y-_ptDrag.y)>4)
// 		{
// 			_bDragLock=FALSE;//Break the lock
// 			grid->OnBeginItemChange(entry->GetItem());
// 		}
// 	}
// 	if (_bDragLock)
// 		return;
// 
// 	int iSel=data->GetSelKey();
// 	if (iSel==-1)
// 		return;
// 
// 	Changelists *vs=entry->GetChangelists(NULL);
// 	if (!vs)
// 		return;
// 
// 	i_math::pos2df pt;
// 	pt.x=(float)x;
// 	pt.y=(float)y;
// 
// 	_ScreenToGG_f(pt.x,pt.y);
// 
// 	AnimTick t=ANIMTICK_FROM_SECOND(pt.x);
// 	if (pt.x<0)
// 		t=0;
// 	SNAP_KEY_TIME(t);
// 
// 	Limit(entry,t,pt.y);
// 
// 	if (iSel==0)
// 		t=0;
// 	else	// 不允许当前移动控制点的 x值小于前一个点
// 	{
// 		if ( t< vs->GetKey(iSel-1)->t)
// 			t= vs->GetKey(iSel-1)->t;
// 	}
// 	// 不允许当前移动控制点的 x值大于后一个点
// 	if ( iSel< ((int)vs->GetKeyCount()) - 1 )
// 	{
// 		if ( t>vs->GetKey(iSel+1)->t)
// 			t=vs->GetKey(iSel+1)->t;
// 	}
// 
// 
// 	if (vs->GetKeyType()==KT_Float)
// 	{
// 		Key_f *k=(Key_f *)vs->GetKey(iSel);
// 		k->t=t;
// 		k->v=pt.y;
// 	}
// 	if (vs->GetKeyType()==KT_Color)
// 	{
// 		Key_col *k=(Key_col*)vs->GetKey(iSel);
// 		k->t=t;
// 	}
// 
// 
// 
// 	grid->OnItemChange(entry->GetItem());

	_Redraw( TRUE );
}



void CGuiAgent_ChangelistsEdit::OnEndDrag( int x, int y, DWORD flag )
{
// 	OnDrag(x,y,flag);
// 	if (_bDragLock)
// 		return;//没有开始过
// 
// 	CGuiData_Changelists *data = (CGuiData_Changelists*)FindData( "Changelists" );
// 	CRichGrid *grid=data->GetGrid();
// 	if (!grid)
// 		return;
// 	ChangelistsEntry *entry=data->GetSelEntry();
// 	if (!entry)
// 		return;
// 
// 	grid->OnEndItemChange(entry->GetItem());
	_Redraw( TRUE );
}

BOOL CGuiAgent_ChangelistsEdit::OnMouseMove(int x,int y,DWORD flag)
{
	CGuiData_Changelists* data = (CGuiData_Changelists*)FindData("Changelists");

	_ScreenToGG(x, y);

	int hilightIndex=-1;
	FileChangeListUID hilightUid = FileChangeListUID_Invalid;
	if (ChangelistsSnapshot::Node* node = data->_snapshot.NodeHitTest((float)x, (float)y,hilightIndex))
	{
		hilightUid = node->_uid;
	}

	if (data->_snapshot.SetHilite(hilightUid, hilightIndex))
	{
		_Redraw(FALSE);
	}

	return CGuiAgent_Dragger<DRAG_BUTTON_LEFT,0>::OnMouseMove(x,y,flag);

}


BOOL CGuiAgent_ChangelistsEdit::OnSetCursor(int x,int y,DWORD flag)
{
// 	int iSel=_HitTest(x,y);
// 	if (iSel!=-1)
// 		_SetCursor(IDC_AIM);
	return FALSE;
}


BOOL CGuiAgent_ChangelistsEdit::OnLButtonDblClk( int x,int y,DWORD flag )
{
	CGuiData_Changelists* data = (CGuiData_Changelists*)FindData("Changelists");
	assert(data != NULL);

	if (data->_sels.size() == 1)
	{
		CChangelists* changelists = data->_changelists;
		if (changelists)
		{
			changelists->SwitchTo(data->_sels[0]);
		}
		_Redraw(FALSE);
	}

	return FALSE;
}

BOOL CGuiAgent_ChangelistsEdit::OnCommand( DWORD idCmd )
{
 	CGuiData_Changelists *data = (CGuiData_Changelists*)FindData( "Changelists" );

	switch (idCmd)
	{
		case ID_AGENT_Changelists_RefreshCurChangelist:
		{
			CChangelists* changelists = data->_changelists;
			if (changelists)
			{
				changelists->RefreshCur();
			}
			_Redraw(FALSE);
			break;
		}
		case ID_AGENT_Changelists_CommitCurChangelist:
		{
			CChangelists* changelists = data->_changelists;
			if (changelists)
			{
				changelists->CommitCur();
			}
			_Redraw(FALSE);
			break;
		}
		case ID_AGENT_Changelists_RemoveCurChangelist:
		{
			CChangelists* changelists = data->_changelists;
			if (changelists)
			{
				if (data->_sels.size() == 1)
				{
					changelists->Remove(data->_sels[0]);
					_Redraw(FALSE);
				}
			}
			break;
		}

	}
	return FALSE;
}

BOOL CGuiAgent_ChangelistsEdit::OnMouseWheel(int delta, DWORD flag)
{
	i_math::pos2di pt;
	_GetCursorPos(pt);

	CGuiData_Changelists* data = (CGuiData_Changelists*)FindData("Changelists");

	_ScreenToGG(pt.x, pt.y);

	int hilightIndex = -1;
	FileChangeListUID hilightUid = FileChangeListUID_Invalid;
	if (ChangelistsSnapshot::Node* node = data->_snapshot.NodeHitTest((float)pt.x, (float)pt.y, hilightIndex))
	{
		if (delta > 0)
			node->Scroll(-1);
		else
			node->Scroll(1);

		hilightIndex = -1;
		hilightUid = FileChangeListUID_Invalid;
		if (node = data->_snapshot.NodeHitTest((float)pt.x, (float)pt.y, hilightIndex))
			hilightUid = node->_uid;

		data->_snapshot.SetHilite(hilightUid, hilightIndex);

		_Redraw(FALSE);
		return FALSE;
	}
	return TRUE;
}
