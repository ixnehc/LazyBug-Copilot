

#include "stdh.h"
#include "GuiAgent_ValueSet.h"
#include "GuiData_ValueSet.h"
#include "GuiView_ValueSet.h"
#include "AgentCmdID.h"

#include "RichGridValueSetItem.h"
#include "valueset/valueset.h"
#include "RichGrid.h"

#include "ColorAlphaDialog.h"

#include "graphicsgraph.h"

#include "Log/LogDump.h"

#define SNAP_KEY_TIME(t) (t)=(t)/(ANIMTICK_PER_SECOND/200)*(ANIMTICK_PER_SECOND/200);

//////////////////////////////////////////////////////////////////////////
//CGuiAgent_ValueSet2DTransform

void CGuiAgent_ValueSet2DTransform::OnUpdateTransform( const i_math::pos2df & pos, const i_math::pos2df &scale )
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


BOOL CGuiAgent_ValueSet2DTransform::OnRButtonClick( int x, int y, DWORD flag )
{
	_AddMenu( "调整合适比例", ID_AGENT_ValueSet_AutoFit);

	return TRUE;
}


BOOL CGuiAgent_ValueSet2DTransform::Fit(const char *path)
{
	CGuiData_ValueSet *data=(CGuiData_ValueSet *)FindData("ValueSet");
	if (!data)
		return FALSE;
	ValueSetGroup *pGrp=data->GetSelGroup();
	if (pGrp)
	{
		i_math::rectf rcSrc,rcSrcClose;
		if (!path[0])
		{
			rcSrc=pGrp->CalcBound(data->_selentry.c_str(),FALSE,FALSE);
			rcSrcClose=pGrp->CalcBound(data->_selentry.c_str(),FALSE,TRUE);
		}
		else
		{
			rcSrc=pGrp->CalcBound(path,TRUE,FALSE);
			rcSrcClose=pGrp->CalcBound(path,TRUE,TRUE);
		}
		if (rcSrc.isValid())
		{
			i_math::recti rc,rc2;
			_GetClientRect(rc);
			rc.cutout(0,RULERY_THICK,rc2);
			rc.cutout(3,RULERX_THICK,rc2);

			if (rc.isValid())
			{
				i_math::rectf rcDest;
				rcDest=rc.convert<float>();
				Swap(rcDest.Top(),rcDest.Bottom());

				i_math::pos2df offOrg,scaleOrg;
				_GetTransformGG(offOrg,scaleOrg);

				i_math::pos2df off,scale,off2,scale2;
				ResolveTransform(rcSrc,rcDest,off,scale);
				scale.y=-scale.y;
				ResolveTransform(rcSrcClose,rcDest,off2,scale2);
				scale2.y=-scale2.y;

				if ((off==offOrg)&&(scale==scaleOrg))
				{
					off=off2;
					scale=scale2;
				}

				_SetTransformGG(off,scale);
				_Redraw(TRUE);
				return TRUE;
			}
		}
	}
	return FALSE;
}


BOOL CGuiAgent_ValueSet2DTransform::OnCommand(DWORD idCmd)
{
	if (idCmd==ID_AGENT_ValueSet_AutoFit)
	{
		Fit("");

		return FALSE;
	}

	return TRUE;

}




//////////////////////////////////////////////////////////////////////////
//CGuiAgent_ValueSetCommand

BOOL CGuiAgent_ValueSetCommand::OnRButtonClick( int x, int y, DWORD flag )
{
	_AddMenu( "隐藏所有", ID_AGENT_ValueSet_HideAll);

	return TRUE;
}

BOOL CGuiAgent_ValueSetCommand::OnCommand(DWORD idCmd)
{
	CGuiData_ValueSet *data=(CGuiData_ValueSet *)FindData("ValueSet");

	if (idCmd==ID_AGENT_ValueSet_HideAll)
	{
		CRichGrid *grid=data->GetGrid();

		ValueSetGroup *grp=data->GetSelGroup();

		std::vector<ValueSet *>hides;
		for (int i=0;i<grp->entries.size();i++)
		{
			Ref *ref=grp->entries[i].ref;
			CRichGrid_ValueSetItem*item=(CRichGrid_ValueSetItem*)ref->GetStuff();
			if (!item)
				continue;
			ValueSet *vs=item->GetBind();
			if (!vs)
				continue;
			if (vs->_bVisible)
				hides.push_back(vs);
		}
		if (hides.size()>0)
		{
			grid->OnBeginItemChange(NULL);
			for (int i=0;i<hides.size();i++)
				hides[i]->_bVisible=FALSE;
			grid->OnItemChange(NULL);
			grid->OnEndItemChange(NULL);
		}
		_Redraw(FALSE);
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//CGuiAgent_ValueSetEdit

CGuiAgent_ValueSetEdit::CGuiAgent_ValueSetEdit()
{
}


BOOL CGuiAgent_ValueSetEdit::OnRButtonClick( int x, int y, DWORD flag )
{
	CGuiData_ValueSet *data = (CGuiData_ValueSet*)FindData( "ValueSet" );

	_AddMenuSep();
	
	int iSel=data->GetSelKey();
	if (iSel!=-1)
	{
		if (iSel>0)
			_AddMenu( "删除控制点", ID_AGENT_ValueSetDragger_Delete );
		_AddMenuSep();
		_AddMenu( "复制控制点值", ID_AGENT_ValueSetDragger_CopyValue );
		if ( _cb.bContent)
		{
			ValueSetEntry *entry=data->GetSelEntry();
			ValueSet *vs=entry->GetValueSet(NULL);
			if (_cb.kt==vs->GetKeyType())
				_AddMenu( "粘贴控制点值", ID_AGENT_ValueSetDragger_PasteValue );
		}
	}
	_AddMenuSep();
	if (iSel!=-1)
		_AddMenu( "设为循环开始", ID_AGENT_ValueSetDragger_SetLoopStart );
	if (data->GetSelEntry())
		_AddMenu( "取消循环", ID_AGENT_ValueSetDragger_RemoveLoop );

	_AddMenuSep();

	if (data->GetSelEntry())
		_AddMenu( "重置", ID_AGENT_ValueSet_Reset);


	_AddMenuSep();
	return TRUE;
}

//x,y是屏幕空间的值
int CGuiAgent_ValueSetEdit::_HitTest(int x,int y)
{
	CGuiData_ValueSet *data = (CGuiData_ValueSet*)FindData( "ValueSet" );

	ValueSetEntry *entry=data->GetSelEntry();
	if (!entry)
		return -1;
	ValueSet *vs=entry->GetValueSet(NULL);

	if (vs->GetKeyType()==KT_Float)
	{
		for (int i=vs->GetKeyCount()-1;i>=0;i--)
		{
			Key_f *k=(Key_f *)vs->GetKey(i);
			i_math::pos2df pt;
			pt.x=ANIMTICK_TO_SECOND(k->t);
			pt.y=k->v;
			_GGToScreen_f(pt.x,pt.y);

			i_math::recti rc;
			rc.set((int)pt.x,(int)pt.y,(int)pt.x,(int)pt.y);
			rc.inflate(KEY_RADIUS,KEY_RADIUS,KEY_RADIUS,KEY_RADIUS);
			if (rc.isPointInside(x,y))
				return i;
		}
	}
	if (vs->GetKeyType()==KT_Color)
	{
		ValueSetGroup *grp=data->GetSelGroup();
		int idx=-1;
		for (int i=0;i<grp->entries.size();i++)
		{
			ValueSetEntry *e=&grp->entries[i];
			if (e==entry)
			{
				idx++;
				break;
			}
			ValueSet *vs=e->GetValueSet(NULL);
			if (vs)
			{
				if (vs->GetKeyType()==KT_Color)
				if (vs->_bVisible)
					idx++;
			}
		}
		assert(idx>=0);

		std::vector<i_math::recti>rcs;
		if (TRUE)
		{
			extern void CalcColorBandVer(int idx,int &top,int &bottom,BOOL bHilight);
			int top,bottom;
			CalcColorBandVer(idx,top,bottom,TRUE);

			void BuildColorKeysRect(CRuler &ruler,ValueSet *vs,i_math::recti &rcKeys,std::vector<i_math::recti>&rcs);

			i_math::pos2df off;
			i_math::pos2df scale;
			_GetTransformGG(off,scale);
			CRuler ruler;

			ruler.SetLength(100);//随便填一个值,
			ruler.SetOff(off.x);
			ruler.SetScale(scale.x);
			i_math::recti rcKeys;
			rcKeys.Top()=top;
			rcKeys.Bottom()=bottom;

			BuildColorKeysRect(ruler,vs,rcKeys,rcs);
		}

		for (int i=rcs.size()-1;i>=0;i--)
		{
			if (rcs[i].isPointInside(x,y))
				return i;
		}
	}
	return -1;
}



BOOL CGuiAgent_ValueSetEdit::OnLButtonDown( int x,int y,DWORD flag )
{
	CGuiData_ValueSet *data = (CGuiData_ValueSet*)FindData( "ValueSet" );
	assert( data != NULL );

	int idx=_HitTest(x,y);
//	if (idx==-1)
//		return TRUE;//没有选中任何东西

	data->SetSelKey(idx);

	if (idx!=-1)
	{
		ValueSetEntry *entry=data->GetSelEntry();
		if (entry)
		{
			ValueSet *vs=entry->GetValueSet(NULL);
			if (vs)
			{
				if (vs->GetKeyType()==KT_Color)
				{
					Key_col *k=(Key_col *)vs->GetKey(idx);
					data->_colpage->SetCurColor(k->color);
				}
			}
		}
	}

	_Redraw(FALSE);

	return CGuiAgent_Dragger<DRAG_BUTTON_LEFT,0>::OnLButtonDown( x, y, flag );
}


BOOL CGuiAgent_ValueSetEdit::OnBeginDrag( int x, int y,DWORD flag )
{
	CGuiData_ValueSet *data = (CGuiData_ValueSet*)FindData( "ValueSet" );
	CRichGrid *grid=data->GetGrid();
	if (!grid)
		return FALSE;

	int idx=data->GetSelKey();
	if ( -1 != idx)
	{
		_ptDrag.set(x,y);
		_bDragLock=TRUE;


		_Redraw(FALSE);

		return TRUE;
	}
	return FALSE;
}

static void Limit(ValueSetEntry *entry,AnimTick &t,float &v)
{
	CRichGrid_ValueSetItem *item=entry->GetItem();
	if (!item)
		return;
	i_math::rectf rcLimit=item->GetLimitRect();
	if (t<ANIMTICK_FROM_SECOND(rcLimit.Left()))
		t=ANIMTICK_FROM_SECOND(rcLimit.Left());
	if (t>ANIMTICK_FROM_SECOND(rcLimit.Right()))
		t=ANIMTICK_FROM_SECOND(rcLimit.Right());
	if (v<rcLimit.Top())
		v=rcLimit.Top();
	if (v>rcLimit.Bottom())
		v=rcLimit.Bottom();
}


void CGuiAgent_ValueSetEdit::OnDrag( int x, int y, DWORD flag )
{
	CGuiData_ValueSet *data = (CGuiData_ValueSet*)FindData( "ValueSet" );
	CRichGrid *grid=data->GetGrid();
	if (!grid)
		return;

	ValueSetEntry *entry=data->GetSelEntry();
	if (!entry)
		return;

	if (_bDragLock)
	{
		if (abs(x-_ptDrag.x)+abs(y-_ptDrag.y)>4)
		{
			_bDragLock=FALSE;//Break the lock
			grid->OnBeginItemChange(entry->GetItem());
		}
	}
	if (_bDragLock)
		return;

	int iSel=data->GetSelKey();
	if (iSel==-1)
		return;

	ValueSet *vs=entry->GetValueSet(NULL);
	if (!vs)
		return;

	i_math::pos2df pt;
	pt.x=(float)x;
	pt.y=(float)y;

	_ScreenToGG_f(pt.x,pt.y);

	AnimTick t=ANIMTICK_FROM_SECOND(pt.x);
	if (pt.x<0)
		t=0;
	SNAP_KEY_TIME(t);

	Limit(entry,t,pt.y);

	if (iSel==0)
		t=0;
	else	// 不允许当前移动控制点的 x值小于前一个点
	{
		if ( t< vs->GetKey(iSel-1)->t)
			t= vs->GetKey(iSel-1)->t;
	}
	// 不允许当前移动控制点的 x值大于后一个点
	if ( iSel< ((int)vs->GetKeyCount()) - 1 )
	{
		if ( t>vs->GetKey(iSel+1)->t)
			t=vs->GetKey(iSel+1)->t;
	}


	if (vs->GetKeyType()==KT_Float)
	{
		Key_f *k=(Key_f *)vs->GetKey(iSel);
		k->t=t;
		k->v=pt.y;
	}
	if (vs->GetKeyType()==KT_Color)
	{
		Key_col *k=(Key_col*)vs->GetKey(iSel);
		k->t=t;
	}



	grid->OnItemChange(entry->GetItem());

	_Redraw( TRUE );
}



void CGuiAgent_ValueSetEdit::OnEndDrag( int x, int y, DWORD flag )
{
	OnDrag(x,y,flag);
	if (_bDragLock)
		return;//没有开始过

	CGuiData_ValueSet *data = (CGuiData_ValueSet*)FindData( "ValueSet" );
	CRichGrid *grid=data->GetGrid();
	if (!grid)
		return;
	ValueSetEntry *entry=data->GetSelEntry();
	if (!entry)
		return;

	grid->OnEndItemChange(entry->GetItem());
	_Redraw( TRUE );
}

BOOL CGuiAgent_ValueSetEdit::OnMouseMove(int x,int y,DWORD flag)
{

	int iSel=_HitTest(x,y);
	if (iSel!=-1)
		_SetCursor(IDC_AIM);

	return CGuiAgent_Dragger<DRAG_BUTTON_LEFT,0>::OnMouseMove(x,y,flag);

}


BOOL CGuiAgent_ValueSetEdit::OnSetCursor(int x,int y,DWORD flag)
{
	int iSel=_HitTest(x,y);
	if (iSel!=-1)
		_SetCursor(IDC_AIM);
	return FALSE;
}


BOOL CGuiAgent_ValueSetEdit::OnLButtonDblClk( int x,int y,DWORD flag )
{
	CGuiData_ValueSet *data = (CGuiData_ValueSet*)FindData( "ValueSet" );

	CRichGrid *grid=data->GetGrid();
	if (!grid)
		return FALSE;

	ValueSetEntry *entry=data->GetSelEntry();
	if (!entry)
		return FALSE;

	ValueSet *vs=entry->GetValueSet(NULL);
	if (!vs)
		return FALSE;

	i_math::pos2df pt;
	pt.x=(float)x;
	pt.y=(float)y;

	_ScreenToGG_f(pt.x,pt.y);

	AnimTick t=ANIMTICK_FROM_SECOND(pt.x);
	if (pt.x<0)
		t=0;
	SNAP_KEY_TIME(t);
	if (vs->GetKeyCount()==0)
		t=0;

	Limit(entry,t,pt.y);

	int idx;
	if (TRUE)
	{
		DWORD iKey1,iKey2;
		float r;
		if (FALSE==vs->FindKeys(t,iKey1,iKey2,r))
			idx=0;
		else
			idx=iKey1+1;
	}

	grid->OnBeginItemChange(entry->GetItem());
	if (vs->GetKeyType()==KT_Float)
	{
		Key_f k;
		k.t=t;
		k.v=pt.y;
		vs->InsertKey(idx,k);
	}

	if (vs->GetKeyType()==KT_Color)
	{
		Key_col k;
		k.t=t;
		k.color=data->_colpage->GetCurColor();
		vs->InsertKey(idx,k);
	}

	if (idx>=0)
	if (idx<=vs->GetLoopIndex())
		vs->SetLoopIndex(vs->GetLoopIndex()+1);

	grid->OnItemChange(entry->GetItem());
	grid->OnEndItemChange(entry->GetItem());

	data->SetSelKey(idx);
	_Redraw(TRUE);

	return FALSE;
}

BOOL CGuiAgent_ValueSetEdit::OnCommand( DWORD idCmd )
{
	CGuiData_ValueSet *data = (CGuiData_ValueSet*)FindData( "ValueSet" );
	CRichGrid *grid=data->GetGrid();
	if (!grid)
		return FALSE;
	ValueSetEntry *entry=data->GetSelEntry();
	ValueSet *vs=NULL;
	if (entry)
		vs=entry->GetValueSet(NULL);
	int iSel=data->GetSelKey();

	switch ( idCmd )
	{
		case ID_AGENT_ValueSetDragger_Delete:
		{
			if (vs&&(iSel>0))
			{
				grid->OnBeginItemChange(NULL);
				if (vs->GetLoopIndex()!=-1)
				{
					if (iSel<vs->GetLoopIndex())
					{
						if (!vs->SetLoopIndex(vs->GetLoopIndex()-1))
							vs->SetLoopIndex(-1);
					}
					else
					{
						if (iSel==vs->GetLoopIndex())
							vs->SetLoopIndex(-1);
					}
				}

				vs->RemoveKey(iSel);
				grid->OnItemChange(NULL);
				grid->OnEndItemChange(NULL);
				_Redraw(FALSE);
			}
			break;
		}
		case ID_AGENT_ValueSetDragger_SetLoopStart:
		{
			if (vs&&(iSel!=-1))
			{
				grid->OnBeginItemChange(NULL);
				vs->SetLoopIndex(iSel);
				grid->OnItemChange(NULL);
				grid->OnEndItemChange(NULL);
				_Redraw(FALSE);
			}
			break;
		}
		case ID_AGENT_ValueSetDragger_RemoveLoop:
		{
			if (vs)
			{
				grid->OnBeginItemChange(NULL);
				vs->SetLoopIndex(-1);
				grid->OnItemChange(NULL);
				grid->OnEndItemChange(NULL);
				_Redraw(FALSE);
			}
			break;
		}
		case ID_AGENT_ValueSetDragger_CopyValue:
		{
			if (vs&&(iSel>=0))
			{
				_cb.bContent=TRUE;
				_cb.kt=vs->GetKeyType();
				if (vs->GetKeyType()==KT_Float)
					_cb.v=((Key_f*)vs->GetKey(iSel))->v;
				if (vs->GetKeyType()==KT_Color)
					_cb.col=((Key_col*)vs->GetKey(iSel))->color;
			}
			break;
		}
		case ID_AGENT_ValueSetDragger_PasteValue:
		{
			if (vs&&(iSel>=0)&&_cb.bContent)
			{
				if (vs->GetKeyType()==_cb.kt)
				{
					grid->OnBeginItemChange(NULL);
					if (_cb.kt==KT_Float)
						((Key_f*)vs->GetKey(iSel))->v=_cb.v;
					if (_cb.kt==KT_Color)
						((Key_col*)vs->GetKey(iSel))->color=_cb.col;
					grid->OnItemChange(NULL);
					grid->OnEndItemChange(NULL);
					_Redraw(FALSE);
				}
			}
			break;
		}
		case ID_AGENT_ValueSet_Reset:
		{
			if (vs)
			{
				grid->OnBeginItemChange(NULL);

				CRichGrid_ValueSetItem *item=entry->GetItem();
				i_math::rectf rcLimit=item->GetLimitRect();
				if (vs->GetKeyType()==KT_Float)
				{
					if (vs->GetKeyCount()>0)
						vs->ResetFloat(vs->GetFloat(0));
					else
						vs->ResetFloat(rcLimit.Bottom());
				}
				if (vs->GetKeyType()==KT_Color)
				{
					if (vs->GetKeyCount()>0)
						vs->ResetColor(vs->GetColor(0));
					else
						vs->ResetColor(0xffffffff);
				}

				grid->OnItemChange(NULL);
				grid->OnEndItemChange(NULL);
				_Redraw(FALSE);
			}
			break;
		}

		default:
			return TRUE;
	}
	return FALSE;
}

