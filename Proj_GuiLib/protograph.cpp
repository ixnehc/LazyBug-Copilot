/********************************************************************
created:	2008/5/22   14:00
file path:	d:\IxEngine\Proj_GuiLib
author:		cxi

purpose:	proto graphic elements
*********************************************************************/

#include "stdh.h"
#include "commondefines/general_stl.h"
#include ".\GuiLib.h"

#include "WorldSystem/IEntitySystem.h"

#include "protograph.h"

#include "GuiView_protologic.h"

#include "RenderPortBase.h"

#include "gds/GProp.h"

#include "stringparser/stringparser.h" 

#include "MultiTree/NodeTree.h"

#include "graphicsgraph.h"





#define MAX_ITEM_WIDTH 200

#define TITLE_HEIGHT 18
#define TITLE_MARGIN 24
#define DEFERRED_MARGIN 20

#define ITEM_HEIGHT 20

#define STUB_ARROW_WIDTH 18

#define ITEM_SPACE 48
#define MOREBUTTON_SIZE (i_math::size2di(10,8))

#define SEP_HEIGHT 4

#define SHRINK_LENGTH 14

#define EXPOSED_HEIGHT 20
#define EXPOSED_MARGIN 4

#define GRID_UNIT 12



//////////////////////////////////////////////////////////////////////////
//GraphItem


IMPLEMENT_CLASS(GraphItem);

void GraphItem::SetShow(const char *s,GraphicsGraph *gg)
{
	show=s;
	sz=gg->MessureText(show.c_str());

	if (sz.w>MAX_ITEM_WIDTH)
		sz.w=MAX_ITEM_WIDTH;

}

i_math::pos2di GraphItem::GetConnectSpot(BOOL bConnecting)
{
	i_math::pos2di pt;
	if (bConnecting)
		pt.set(rcFocus.Right()+2,rcFocus.getCenter().y);
	else
		pt.set(rcFocus.Left()-2,rcFocus.getCenter().y);

	return pt;
}

BOOL GraphItem::IsFocus()
{
	if (!graph)
		return FALSE;
	return graph->GetFocus()==this;
}





//////////////////////////////////////////////////////////////////////////
//CGraphStandard
IMPLEMENT_CLASS(CGraphStandard);

void CGraphStandard::Zero()
{
	_id=ProtoNodeID_Null;
	_typePN=PN_Asset;
	_bDynamic=FALSE;
	_bVirtual=FALSE;
	_defergrp=PNDeferGrp_None;
	_bLab=FALSE;
	_bEditHelper=FALSE;
	_ver=0;
}

void CGraphStandard::Clear()
{
	for (int i=0;i<_props.size();i++)
		Class_Delete(_props[i]);
	for (int i=0;i<_signals.size();i++)
		Class_Delete(_signals[i]);
	for (int i=0;i<_slots.size();i++)
		Class_Delete(_slots[i]);
	for (int i=0;i<_calls.size();i++)
		Class_Delete(_calls[i]);

	_props.clear();//connectable props
	_signals.clear();
	_slots.clear();
	_calls.clear();

	Zero();
}

BOOL CGraphStandard::_LocateMoreBtn(GraphItem*item,int x,int y)
{
	if (item->bShow)
	{
		item->rc.set(i_math::pos2di(x,y),MOREBUTTON_SIZE);
		item->rcFocus=item->rc;
		item->rcFocus.inflate(-1,-1,-1,-1);
		return TRUE;
	}
	return FALSE;
}

BOOL CGraphStandard::_ExistPropSeg()
{
	if ((_props.size()<=0)&&(!_moreProp.bShow)&&(!_createProp.bShow))
		return FALSE;
	return TRUE;
}

BOOL CGraphStandard::_ExistSignalSlotSeg()
{
	if ((_signals.size()<=0)&&(_slots.size()<=0)&&
		(!_moreSignal.bShow)&&(!_moreSlot.bShow)&&
		(!_createSignal.bShow)&&(!_createSlot.bShow))
		return FALSE;
	return TRUE;
}

BOOL CGraphStandard::_ExistCallSeg()
{
	if ((_calls.size()<=0)&&(!_moreCall.bShow)&&(!_createCall.bShow))
		return FALSE;
	return TRUE;
}


BOOL CGraphStandard::_NeedSep()
{
	return (_ExistPropSeg()&&
		(_ExistSignalSlotSeg()||_ExistCallSeg()));
}

BOOL CGraphStandard::_NeedSep2()
{
	return (_ExistSignalSlotSeg()&&_ExistCallSeg());
}


void CGraphStandard::_LocateProp(GraphItem *item,i_math::recti &rcItem)
{
	item->rc=rcItem;
	item->rcFocus=rcItem;
	item->rcFocus.inflate(-1,-1,-1,-1);
	rcItem+=i_math::pos2di(0,ITEM_HEIGHT);
}

void CGraphStandard::_LocateSignal(GraphItem *item,i_math::recti &rcItem)
{
	item->rc=rcItem;
	item->rcFocus=rcItem;
	item->rcFocus.Left()=item->rcFocus.Right()-item->sz.w-STUB_ARROW_WIDTH-4;
	item->rcFocus.inflate(-1,-1,-1,-1);
	rcItem+=i_math::pos2di(0,ITEM_HEIGHT);
}

void CGraphStandard::_LocateSlot(GraphItem *item,i_math::recti &rcItem)
{
	item->rc=rcItem;
	item->rcFocus=rcItem;
	item->rcFocus.Right()=item->rcFocus.Left()+item->sz.w+STUB_ARROW_WIDTH+4;
	item->rcFocus.inflate(-1,-1,-1,-1);
	rcItem+=i_math::pos2di(0,ITEM_HEIGHT);
}

void CGraphStandard::_LocateCall(GraphItem *item,i_math::recti &rcItem)
{
	item->rc=rcItem;
	item->rcFocus=rcItem;
	item->rcFocus.inflate(-1,-1,-1,-1);
	rcItem+=i_math::pos2di(0,ITEM_HEIGHT);
}


void CGraphStandard::RecalcLayout(GraphicsGraph *gg)
{
	DWORD w,h;

	if (TRUE)
	{
		h=TITLE_HEIGHT;
		h+=_props.size()*ITEM_HEIGHT;
		if (_signals.size()>_slots.size())
			h+=_signals.size()*ITEM_HEIGHT;
		else
			h+=_slots.size()*ITEM_HEIGHT;
		h+=_calls.size()*ITEM_HEIGHT;
	}

	//precalculate the w x h
	if (TRUE)
	{
		w=_title.sz.w+TITLE_MARGIN;
		if ((_defergrp!=PNDeferGrp_None)&&(_defergrp!=PNDeferGrp_Dyn))
			w+=DEFERRED_MARGIN;
		DWORD ww;

		for (int i=0;i<_props.size();i++)
		{
			ww=_props[i]->sz.w+STUB_ARROW_WIDTH*2+ITEM_SPACE;
			if (ww>w)
				w=ww;
		}

		if (_moreProp.bShow)
			h+=MOREBUTTON_SIZE.h;

		for (int i=0;i<_calls.size();i++)
		{
			ww=_calls[i]->sz.w+STUB_ARROW_WIDTH*2+ITEM_SPACE;
			if (ww>w)
				w=ww;
		}

		if (_moreCall.bShow)
			h+=MOREBUTTON_SIZE.h;

		for (int i=0;i<_signals.size();i++)
		{
			if (i>=_slots.size())
				ww=_signals[i]->sz.w+STUB_ARROW_WIDTH+ITEM_SPACE;
			else
				ww=_signals[i]->sz.w+STUB_ARROW_WIDTH+
				ITEM_SPACE+
				_slots[i]->sz.w+STUB_ARROW_WIDTH;
			if (ww>w)
				w=ww;
		}

		for (int i=0;i<_slots.size();i++)
		{
			ww=_slots[i]->sz.w+STUB_ARROW_WIDTH+ITEM_SPACE;
			if (ww>w)
				w=ww;
		}

		if (_createProp.bShow)
		{
			h+=ITEM_HEIGHT;
			ww=_createProp.sz.w+STUB_ARROW_WIDTH+ITEM_SPACE;
			if (ww>w)
				w=ww;
		}
		if (_createCall.bShow)
		{
			h+=ITEM_HEIGHT;
			ww=_createCall.sz.w+STUB_ARROW_WIDTH+ITEM_SPACE;
			if (ww>w)
				w=ww;
		}
		if (_createSignal.bShow&&_createSlot.bShow)
		{
			h+=ITEM_HEIGHT;
			ww=_createSignal.sz.w+STUB_ARROW_WIDTH+
				ITEM_SPACE+
				_createSlot.sz.w+STUB_ARROW_WIDTH;
			if (ww>w)
				w=ww;
		}
		else
		{
			if (_createSignal.bShow)
			{
				h+=ITEM_HEIGHT;
				ww=_createSignal.sz.w+STUB_ARROW_WIDTH+ITEM_SPACE;
				if (ww>w)
					w=ww;
			}
			if (_createSlot.bShow)
			{
				h+=ITEM_HEIGHT;
				ww=_createSlot.sz.w+STUB_ARROW_WIDTH+ITEM_SPACE;
				if (ww>w)
					w=ww;
			}
		}

	}

	if (_NeedSep())
		h+=SEP_HEIGHT;

	if (_NeedSep2())
		h+=SEP_HEIGHT;

	if (_signals.size()==_slots.size())
	{
		if (_moreSignal.bShow||_moreSlot.bShow)
			h+=MOREBUTTON_SIZE.h;
	}
	if ((_signals.size()>_slots.size())&&_moreSignal.bShow)
		h+=MOREBUTTON_SIZE.h;
	if ((_signals.size()<_slots.size())&&_moreSlot.bShow)
		h+=MOREBUTTON_SIZE.h;

	h+=2;//add a bottom margin
	if (h<=32)
		h=32;

	_rc.set(0,0,w,h);
	_rc+=_pt;

	i_math::recti rcItem;
	rcItem.set(0,0,w,ITEM_HEIGHT);

	rcItem+=_pt;

	_title.rc=rcItem;
	_title.rc.Bottom()=_title.rc.Top()+TITLE_HEIGHT;

	_shrink.rc.set(0,0,SHRINK_LENGTH,SHRINK_LENGTH);
	_shrink.rc+=i_math::pos2di(rcItem.Right()-SHRINK_LENGTH-2,rcItem.Top()+4);
	_shrink.rcFocus=_shrink.rc;

	rcItem+=i_math::pos2di(0,TITLE_HEIGHT);

	for (int i=0;i<_props.size();i++)
		_LocateProp(_props[i],rcItem);

	if (_createProp.bShow)
		_LocateProp(&_createProp,rcItem);

	if (_LocateMoreBtn(&_moreProp,rcItem.Left()+8,rcItem.Top()+1))
		rcItem+=i_math::pos2di(0,MOREBUTTON_SIZE.h);

	if (_NeedSep())
	{
		_ySep=rcItem.Top();
		rcItem+=i_math::pos2di(0,SEP_HEIGHT);
	}
	else
		_ySep=-10000;

	if (TRUE)
	{
		i_math::recti rcItemSignal,rcItemSlot;
		rcItemSignal=rcItem;
		rcItemSlot=rcItem;
		for (int i=0;i<_signals.size();i++)
			_LocateSignal(_signals[i],rcItemSignal);

		if (_createSignal.bShow)
			_LocateSignal(&_createSignal,rcItemSignal);

		if (_LocateMoreBtn(&_moreSignal,rcItemSignal.Right()-8-MOREBUTTON_SIZE.w,
			rcItemSignal.Top()+1))
			rcItemSignal+=i_math::pos2di(0,MOREBUTTON_SIZE.h);

		for (int i=0;i<_slots.size();i++)
			_LocateSlot(_slots[i],rcItemSlot);

		if (_createSlot.bShow)
			_LocateSlot(&_createSlot,rcItemSlot);

		if (_LocateMoreBtn(&_moreSlot,rcItemSlot.Left()+8,rcItemSlot.Top()+1))
			rcItemSlot+=i_math::pos2di(0,MOREBUTTON_SIZE.h);
		if (rcItemSignal.Top()>rcItemSlot.Top())
			rcItem=rcItemSignal;
		else
			rcItem=rcItemSlot;
	}

	if (_NeedSep2())
	{
		_ySep2=rcItem.Top();
		rcItem+=i_math::pos2di(0,SEP_HEIGHT);
	}
	else
		_ySep2=-10000;

	for (int i=0;i<_calls.size();i++)
		_LocateCall(_calls[i],rcItem);

	if (_createCall.bShow)
		_LocateCall(&_createCall,rcItem);

	if (_LocateMoreBtn(&_moreCall,rcItem.Left()+8,rcItem.Top()+1))
		rcItem+=i_math::pos2di(0,MOREBUTTON_SIZE.h);

}

void CGraphStandard::_DrawTitle(GraphItem*item,BOOL bHiLight,GraphicsGraph *gg)
{
	switch(_typePN)
	{
	case PN_LuaObj:
		{
			if (!bHiLight)
				gg->GradientV(item->rc,RGB(218,218,218),RGB(55,196,196));
			else
				gg->GradientV(item->rc,RGB(233,233,233),RGB(85,209,255));
			break;
		}
	case PN_Asset:
		{
			if (!bHiLight)
				gg->GradientV(item->rc,RGB(218,218,218),RGB(240,138,32));
			else
				gg->GradientV(item->rc,RGB(233,233,233),RGB(255,132,0));
			break;
		}
	case PN_Entity:
		{
			if (!bHiLight)
				gg->GradientV(item->rc,RGB(218,218,218),RGB(46,195,98));
			else
				gg->GradientV(item->rc,RGB(233,233,233),RGB(112,252,146));
			break;
		}
	}

	//title 字符串
	if (TRUE)
	{
		i_math::recti rc=item->rc;
		if ((_defergrp!=PNDeferGrp_None)&&(_defergrp!=PNDeferGrp_Dyn))
			rc.inflate(-DEFERRED_MARGIN,0,0,0);
		gg->DrawText(item->show.c_str(),rc);
	}

	//Deferred标志
	if ((_defergrp!=PNDeferGrp_None)&&(_defergrp!=PNDeferGrp_Dyn))
	{
		i_math::recti rcPie=item->rc;
		rcPie.Right()=rcPie.Left()+item->rc.getHeight();
		rcPie.inflate(-2,-2,-2,-2);
		//		gg->

		//		rcPie.inflate(-2,-2,-2,-2);

		extern DWORD GetPNDeferGrpColor(PNDeferGrp grp);
		DWORD col=GetPNDeferGrpColor(_defergrp);
		DWORD col2=RGB(GetRValue(col)/2,GetGValue(col)/2,GetBValue(col)/2);

		gg->DrawRoundCornerRect(rcPie,2,col,col2);



	}
}

void CGraphStandard::_DrawProp(GraphItem*item,GraphicsGraph *gg)
{
	if (item->IsFocus())
		gg->FillSolidRect(item->rcFocus,RGB(120,120,255));

	i_math::recti rc=item->rc;
	if (item->bConnectable)
	{
		i_math::pos2di pt=rc.UpperLeftCorner;
		pt.y+=2;
		pt.x+=2;
		gg->GradientArrow(pt,RGB(0,128,0),RGB(0,245,0));
		pt.x=rc.Right()-STUB_ARROW_WIDTH;
		gg->GradientArrow(pt,RGB(0,128,0),RGB(0,245,0));
	}
	else
	{
		i_math::recti rcPie=rc;
		rcPie+=i_math::pos2di(2,2);
		rcPie.Right()=rcPie.Left()+STUB_ARROW_WIDTH-4;
		rcPie.Bottom()=rcPie.Top()+STUB_ARROW_WIDTH-4;

		rcPie.inflate(-1,-1,-1,-1);

		gg->GradientPie(rcPie,RGB(0,245,0),RGB(0,128,0));
	}
	rc+=i_math::pos2di(STUB_ARROW_WIDTH,0);
	gg->DrawText(item->show.c_str(),rc);
}

void CGraphStandard::_DrawSignal(GraphItem*item,GraphicsGraph *gg)
{
	if (item->IsFocus())
		gg->FillSolidRect(item->rcFocus,RGB(120,120,255));

	i_math::recti rc=item->rc;
	i_math::pos2di pt=rc.UpperLeftCorner;
	pt.y+=2;
	pt.x=rc.Right()-STUB_ARROW_WIDTH;
	if (!item->bPassive)
		gg->GradientArrow(pt,RGB(139,69,0),RGB(255,126,0));
	else
		gg->GradientArrow(pt,RGB(0,64,160),RGB(0,102,255));

	rc-=i_math::pos2di(STUB_ARROW_WIDTH+2,0);
	gg->DrawText(item->show.c_str(),rc,DT_RIGHT);
}

void CGraphStandard::_DrawSlot(GraphItem*item,GraphicsGraph *gg)
{
	if (item->IsFocus())
		gg->FillSolidRect(item->rcFocus,RGB(120,120,255));

	i_math::recti rc=item->rc;
	i_math::pos2di pt=rc.UpperLeftCorner;
	pt.y+=2;
	pt.x+=2;
	if (item->bPassive)
		gg->GradientArrow(pt,RGB(0,64,160),RGB(0,102,255));
	else
		gg->GradientArrow(pt,RGB(139,69,0),RGB(255,126,0));

	rc+=i_math::pos2di(STUB_ARROW_WIDTH,0);
	gg->DrawText(item->show.c_str(),rc);
}

void CGraphStandard::_DrawCall(GraphItem*item,GraphicsGraph *gg)
{
	if (item->IsFocus())
		gg->FillSolidRect(item->rcFocus,RGB(120,120,255));

	i_math::recti rc=item->rc;
	i_math::pos2di pt=rc.UpperLeftCorner;
	pt.y+=2;
	pt.x+=2;
	if (item->bPassive)
		gg->GradientArrow(pt,RGB(0,64,160),RGB(0,102,255));
	else
		gg->GradientArrow(pt,RGB(139,69,0),RGB(255,126,0));

	rc+=i_math::pos2di(STUB_ARROW_WIDTH,0);
	gg->DrawText(item->show.c_str(),rc);
}


void CGraphStandard::_DrawMore(GraphItem*item,GraphicsGraph *gg)
{
	if (!item->bShow)
		return;

	gg->DrawMore(item->rc.UpperLeftCorner);
}


void CGraphStandard::_DrawSep(GraphicsGraph *gg,int ySep)
{
	if (ySep==-10000)
		return;
	i_math::recti rc=_rc;
	rc.Top()=ySep+1;
	rc.Bottom()=ySep+2;

	gg->FillSolidRect(rc,RGB(128,128,128));

}

void CGraphStandard::_DrawShrink(GraphItem*item,GraphicsGraph *gg)
{
	gg->DrawShrink(item->rc.UpperLeftCorner);
}

void CGraphStandard::_DrawCreate(GraphItem*item,BOOL bLeft,GraphicsGraph *gg)
{
	if (!item->bShow)
		return;
	if (item->IsFocus())
		gg->FillSolidRect(item->rcFocus,RGB(120,120,255));

	i_math::recti rc=item->rc;
	i_math::pos2di pt=rc.UpperLeftCorner;
	pt.y+=2;
	if (!bLeft)
	{
		pt.x=rc.Right()-STUB_ARROW_WIDTH;
		gg->GradientFlash(pt,RGB(252,255,0),RGB(255,180,0));

		rc-=i_math::pos2di(STUB_ARROW_WIDTH+2,0);
		gg->DrawText(item->show.c_str(),rc,DT_RIGHT);
	}
	else
	{
		pt.x+=2;
		gg->GradientFlash(pt,RGB(252,255,0),RGB(255,180,0));

		rc+=i_math::pos2di(STUB_ARROW_WIDTH,0);
		gg->DrawText(item->show.c_str(),rc);
	}
}

void CGraphStandard::_DrawDesc(GraphicsGraph *gg)
{
	if (_desc=="")
		return;
	i_math::size2di sz=gg->MessureText(_desc.c_str());
	i_math::recti rc;
	rc.set(_pt.x,_pt.y-sz.h,_pt.x+sz.w+6,_pt.y);
	gg->DrawRoundCornerRect(rc,3,RGB(184,184,184),RGB(149,149,149));
	gg->DrawText(_desc.c_str(),rc,DT_LEFT,FALSE,RGB(0,0,0));
}



void CGraphStandard::Draw(GraphicsGraph *gg,BOOL bHilight)
{
	if (TRUE)//the bg
	{
		DWORD alpha=255;

		if (_bVirtual)
			alpha=64;

		DWORD col1;
		if (!_bLab)
			col1=bHilight?RGB(245,245,245):RGB(210,210,210);
		else
			col1=bHilight?RGB(255,223,146):RGB(189,170,125);

		if (_bEditHelper)
		{
			i_math::recti rc=_rc;
			rc.inflate(4,4,4,4);

			gg->FrameRoundCornerRect(rc,2,RGB(128,192,255),3,255,TRUE);

		}


		if (_bDynamic)
		{
			i_math::recti rc=_rc;
			rc+=i_math::pos2di(8,8);
			gg->FillSolidRect(rc,col1,alpha<255?alpha/2:alpha);
			gg->DrawFrameRect(rc,RGB(0,0,0),1,alpha<255?alpha/2:alpha);
			rc-=i_math::pos2di(4,4);
			gg->FillSolidRect(rc,col1,alpha<255?alpha/4:alpha);
			gg->DrawFrameRect(rc,RGB(0,0,0),1,alpha<255?alpha/4:alpha);
			gg->FillSolidRect(_rc,col1,alpha<255?alpha/4:alpha);
		}
		else
			gg->FillSolidRect(_rc,col1,alpha);


		gg->FillSolidRect(_rc,col1,alpha);

		if (_bDynamic)
			gg->DrawFrameRect(_rc,RGB(128,128,128),1,alpha);

	}

	_DrawTitle(&_title,bHilight,gg);

	for (int i=0;i<_props.size();i++)
		_DrawProp(_props[i],gg);
	for (int i=0;i<_signals.size();i++)
		_DrawSignal(_signals[i],gg);
	for (int i=0;i<_slots.size();i++)
		_DrawSlot(_slots[i],gg);
	for (int i=0;i<_calls.size();i++)
		_DrawCall(_calls[i],gg);

	_DrawCreate(&_createProp,TRUE,gg);
	_DrawCreate(&_createSignal,FALSE,gg);
	_DrawCreate(&_createSlot,TRUE,gg);
	_DrawCreate(&_createCall,TRUE,gg);

	_DrawSep(gg,_ySep);
	_DrawSep(gg,_ySep2);

	_DrawMore(&_moreProp,gg);
	_DrawMore(&_moreSignal,gg);
	_DrawMore(&_moreSlot,gg);
	_DrawMore(&_moreCall,gg);

	_DrawShrink(&_shrink,gg);

	_DrawDesc(gg);

}


BOOL CGraphStandard::_ItemHitTest(int x,int y,GraphItem*item,int part,GraphHit &hit)
{
	if (item->rcFocus.isPointInside(x,y))
	{
		hit.part=(GraphHit::Part)part;
		hit.item=item;
		return TRUE;
	}
	return FALSE;
}

BOOL CGraphStandard::_ItemsHitTest(int x,int y,GraphItem**items,DWORD c,int part,GraphHit &hit)
{
	for (int i=0;i<c;i++)
	{
		if (_ItemHitTest(x,y,items[i],part,hit))
			return TRUE;
	}

	return FALSE;
}

GraphItem *CGraphStandard::FindItem(const char *name)
{
	int idx;
	PVEC_FIND_BY_ELEMENT(_props,name,name,idx);
	if (idx!=-1)
		return _props[idx];
	PVEC_FIND_BY_ELEMENT(_signals,name,name,idx);
	if (idx!=-1)
		return _signals[idx];
	PVEC_FIND_BY_ELEMENT(_slots,name,name,idx);
	if (idx!=-1)
		return _slots[idx];
	PVEC_FIND_BY_ELEMENT(_calls,name,name,idx);
	if (idx!=-1)
		return _calls[idx];
	return NULL;
}



BOOL CGraphStandard::HitTest(int x,int y,GraphHit &hit)
{
	if (!_rc.isPointInside(x,y))
		return FALSE;

	hit.id=_id;

	if (_ItemsHitTest(x,y,_props.data(),_props.size(),GraphHit::PropIn,hit))
	{
		if (hit.item->rcFocus.getCenter().x<x)
			hit.part=GraphHit::PropOut;
		return TRUE;
	}
	if (_ItemsHitTest(x,y,_signals.data(),_signals.size(),GraphHit::Signal,hit))
		return TRUE;
	if (_ItemsHitTest(x,y,_slots.data(),_slots.size(),GraphHit::Slot,hit))
		return TRUE;
	if (_ItemsHitTest(x,y,_calls.data(),_calls.size(),GraphHit::Call,hit))
		return TRUE;

	if (_ItemHitTest(x,y,&_createProp,GraphHit::CreateProp,hit))
		return TRUE;
	if (_ItemHitTest(x,y,&_createSlot,GraphHit::CreateSlot,hit))
		return TRUE;
	if (_ItemHitTest(x,y,&_createSignal,GraphHit::CreateSignal,hit))
		return TRUE;
	if (_ItemHitTest(x,y,&_createCall,GraphHit::CreateCall,hit))
		return TRUE;

	if (_ItemHitTest(x,y,&_moreProp,GraphHit::MoreProp,hit))
		return TRUE;
	if (_ItemHitTest(x,y,&_moreSignal,GraphHit::MoreSignal,hit))
		return TRUE;
	if (_ItemHitTest(x,y,&_moreSlot,GraphHit::MoreSlot,hit))
		return TRUE;
	if (_ItemHitTest(x,y,&_moreCall,GraphHit::MoreCall,hit))
		return TRUE;
	if (_ItemHitTest(x,y,&_shrink,GraphHit::Shrink,hit))
		return TRUE;

	hit.part=GraphHit::Blank;
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//CGraphExposed
IMPLEMENT_CLASS(CGraphExposed);

void CGraphExposed::RecalcLayout(GraphicsGraph *gg)
{
	DWORD w,h;
	w=_title.sz.w;
	h=EXPOSED_HEIGHT;
	_title.rc.set(0,0,w+20,h);
	_title.rc+=_pt;
	_title.rc+=i_math::pos2di(EXPOSED_MARGIN,EXPOSED_MARGIN);
	_rc=_title.rc;
	_rc.inflate(EXPOSED_MARGIN,EXPOSED_MARGIN,EXPOSED_MARGIN,EXPOSED_MARGIN);
}

void CGraphExposed::_DrawTitle(GraphItem*item,GraphicsGraph *gg)
{
	gg->DrawText(item->show.c_str(),item->rc,DT_CENTER);
}


void CGraphExposed::Draw(GraphicsGraph *gg,BOOL bHilight)
{
	switch(_stubtype)
	{
	case GStub_Call:
	case GStub_Slot:
		{
			gg->DrawRoundRect(_rc,RGB(0,102,255),RGB(0,64,160));
			gg->DrawConnectH(i_math::pos2di(_rc.Right()+_rc.getHeight()/2,_rc.getCenter().y),
				_ptDest,RGB(0,102,255),DRAWCONNECT_DASH);
			break;
		}
	case GStub_Signal:
		gg->DrawRoundRect(_rc,RGB(255,126,0),RGB(139,69,0));
		gg->DrawConnectH(_ptDest,
			i_math::pos2di(_rc.Left()-_rc.getHeight()/2,_rc.getCenter().y),RGB(255,126,0),
			DRAWCONNECT_DASH);
		break;
	case GStub_Pump:
		gg->DrawRoundRect(_rc,RGB(255,126,0),RGB(139,69,0));
		gg->DrawConnectH(i_math::pos2di(_rc.Right()+_rc.getHeight()/2,_rc.getCenter().y),
			_ptDest,RGB(255,126,0),DRAWCONNECT_DASH);
		break;
	case GStub_Origin:
		gg->DrawRoundRect(_rc,RGB(0,102,255),RGB(0,64,160));
		gg->DrawConnectH(_ptDest,
			i_math::pos2di(_rc.Left()-_rc.getHeight()/2,_rc.getCenter().y),RGB(0,102,255),
			DRAWCONNECT_DASH);
		break;

	case GStub_Property:
		gg->DrawRoundRect(_rc,RGB(0,245,0),RGB(0,128,0));
		gg->DrawConnectH(i_math::pos2di(_rc.Right()+_rc.getHeight()/2,_rc.getCenter().y),
			_ptDest,RGB(0,245,0),
			DRAWCONNECT_DASH|DRAWCONNECT_BIDIRECTIONAL);
		break;
	}

	_DrawTitle(&_title,gg);

}

BOOL CGraphExposed::HitTest(int x,int y,GraphHit &hit)
{
	if (_rc.isPointInside(x,y))
	{
		hit.nameExpose=_title.name;
		return TRUE;
	}
	return FALSE;
}




//////////////////////////////////////////////////////////////////////////
//CProtoGraph

void CProtoGraph::Zero()
{
	_ver=0;
	_bEditMode=TRUE;
}

void CProtoGraph::_ClearExposes()
{
	for (int i=0;i<_exposes.size();i++)
		Class_Delete(_exposes[i]);
	_exposes.clear();
}


void CProtoGraph::Clear()
{
	for (int i=0;i<_standards.size();i++)
		Class_Delete(_standards[i]);
	_standards.clear();

	_ClearExposes();

	_connects.clear();

	Zero();
}

void CProtoGraph::_MakeCreateItem(GraphItem *item,ProtoNodeID id,
								  const char *name,GraphicsGraph *gg)
{
	item->bShow=TRUE;
	item->graph=this;
	item->id=id;
	item->SetShow(name,gg);
	item->SetName(name);
	item->bConnectable=TRUE;
}


CGraphStandard* CProtoGraph::_LoadStandard(GraphNodeDescs &descs,IProtoNode *node,GraphicsGraph *gg)
{
	CGraphStandard *p=Class_New(CGraphStandard);

	p->_title.SetShow(node->GetShowName(node->GetName(),FALSE),gg);
	p->_pt=node->GetGraphPos();
	p->_pt.scale_signed(GRID_UNIT);
	p->_pt*=GRID_UNIT;

	p->_id=node->GetID();
	p->_typePN=node->GetType();
	p->_bDynamic=node->IsDynamic();
	p->_bVirtual=node->IsVirtual();
	p->_defergrp=node->GetDeferGrp();
	p->_bLab=node->IsLab();
	p->_bEditHelper=node->IsEditHelper();

	std::vector<std::string>hides;
	if (TRUE)
	{
		std::string s=node->GetGraphStubHides();
		SplitStringBy(",",s,&hides);
	}

	std::string sShow;
	DWORD cStubs=node->GetStubCount();
	for (int j=0;j<cStubs;j++)
	{
		const char *name;
		GStubBase *stb=node->GetStub(j,name);
		if (!stb)
			continue;

		if (strcmp(name,"Xform")==0)//忽略Xform属性
			continue;

		GraphItem *item=Class_New(GraphItem);
		item->graph=this;
		item->id=p->_id;

		item->SetName(name);
		if (TRUE)
		{
			sShow=name;
			std::unordered_map<std::string,std::string>::iterator it=descs.descStbs.find(std::string(name));
			if (it!=descs.descStbs.end())
			{
				if ((*it).second!="")
					sShow=(*it).second;
			}
			item->SetShow(sShow.c_str(),gg);
		}

		item->bConnectable=stb->IsConnectable();

		if (TRUE)
		{
			int idx;
			VEC_FIND(hides,name,idx);
			if (idx==-1)
				item->bShow=1;
		}

		switch(stb->type)
		{
		case GStub_Property:
		case GStub_Origin:
		case GStub_Slot:
		case GStub_Call:
			item->bPassive=TRUE;
		}


		if (stb->type==GStub_Property)
		{
			if (item->bShow)
				p->_props.push_back(item);
			p->_moreProp.bShow=p->_moreProp.bShow||(!item->bShow);
		}
		if ((stb->type==GStub_Signal)||(stb->type==GStub_Origin))
		{
			if (item->bShow)
				p->_signals.push_back(item);
			p->_moreSignal.bShow=p->_moreSignal.bShow||(!item->bShow);
		}
		if ((stb->type==GStub_Slot)||(stb->type==GStub_Pump))
		{
			if (item->bShow)
				p->_slots.push_back(item);
			p->_moreSlot.bShow=p->_moreSlot.bShow||(!item->bShow);
		}
		if (stb->type==GStub_Call)
		{
			if (item->bShow)
				p->_calls.push_back(item);
			p->_moreCall.bShow=p->_moreCall.bShow||(!item->bShow);
		}

		if(!item->bShow)
			Class_Delete(item);

	}

	if (node->GetType()==PN_LuaObj)
	{
		_MakeCreateItem(&p->_createProp,p->_id," ... ",gg);
		_MakeCreateItem(&p->_createSignal,p->_id," ... ",gg);
		_MakeCreateItem(&p->_createSlot,p->_id," ... ",gg);
		_MakeCreateItem(&p->_createCall,p->_id," ... ",gg);
	}

	p->_shrink.bShow=TRUE;

	p->_ver=node->GetVer();

	p->_desc=descs.desc;

	p->RecalcLayout(gg);

	return p;
}

void CProtoGraph::_LoadExposes(IProto *proto,GraphicsGraph *gg)
{
	_ClearExposes();

	DWORD c=proto->GetStubCount();

	_exposes.resize(c);

	for (int i=0;i<c;i++)
	{
		ProtoStubInfo info;
		proto->GetStubInfo(i,info);

		CGraphExposed *p=Class_New(CGraphExposed);

		const char *name;
		GStubBase *stb=proto->GetStub(i,name);

		p->_title.SetShow(info.name,gg);
		p->_title.SetName(info.name);

		p->_pt=info.pos;
		p->_pt.scale_signed(GRID_UNIT);
		p->_pt*=GRID_UNIT;
		p->_idInner=info.idInner;
		p->_nameInner=info.nameInner;

		p->_stubtype=stb->type;

		if (TRUE)
		{
			GraphItem *item=FindItem(info.idInner,info.nameInner);
			switch(p->_stubtype)
			{
			case GStub_Property:
			case GStub_Call:
			case GStub_Slot:
			case GStub_Pump:
				{
					p->_ptDest=item->GetConnectSpot(FALSE);
					break;
				}
			case GStub_Signal:
			case GStub_Origin:
				{
					p->_ptDest=item->GetConnectSpot(TRUE);
					break;
				}
			}
		}

		p->RecalcLayout(gg);

		_exposes[i]=p;
	}

}

void CProtoGraph::_LoadConnects(IProto *proto)
{
	DWORD c=proto->GetConnectCount();
	_connects.resize(c);

	IProtoNode *nodes[2];
	GStubBase *stbs[2];

	for (int i=0;i<c;i++)
	{
		PNConnect t;
		proto->GetConnect(i,t);

		_Connect *p=&_connects[i];
		p->bToDyn=FALSE;
		p->bToDefer=FALSE;
		p->bErr=FALSE;
		memset(nodes,0,sizeof(nodes));
		memset(stbs,0,sizeof(stbs));
		for (int i=0;i<2;i++)
		{
			p->item[i]=FindItem(t.id[i],t.name[i]);
			nodes[i]=proto->GetNode(t.id[i]);
			if (nodes[i])
				stbs[i]=nodes[i]->FindStub(t.name[i]);
		}
		if (nodes[1])
		{
			if (nodes[1]->IsDynamic())
			{
				p->bToDyn=TRUE;
			}
			else
			{
				if (nodes[1]->GetDeferGrp()!=PNDeferGrp_None)
					p->bToDefer=TRUE;
			}
		}
		if (stbs[0]&&stbs[1])
			p->bErr=!CheckStubDataCompatible(stbs[0],stbs[1]);
	}
}

int CProtoGraph::_FindStandard(ProtoNodeID id)
{
	int idx;
	PVEC_FIND_BY_ELEMENT(_standards,_id,id,idx);
	return idx;
}

GraphItem *CProtoGraph::FindItem(ProtoNodeID id,const char *name)
{
	int idx=_FindStandard(id);
	if (idx==-1)
		return NULL;
	CGraphStandard *p=_standards[idx];

	return p->FindItem(name);
}

static void GetNodeDescs(GraphNodeDescs &descs,IProto *proto,IProtoNode *node,IEntity *en)
{
	descs.Clear();
	if (!en)
		return;
	if (node->GetType()==PN_Asset)
	{
		descs.desc=proto->GetDesc(node->GetID(),"",en);
		DWORD c=node->GetStubCount();
		const char *name;
		for (int i=0;i<c;i++)
		{
			node->GetStub(i,name);
			if (name)
			{
				if (name[0])
				{
					const char *desc=proto->GetDesc(node->GetID(),name,en);
					if (desc[0])
						descs.descStbs[std::string(name)]=desc;
				}
			}
		}
	}
}


void CProtoGraph::Load(IProto *proto,GraphicsGraph *gg,IEntitySystem *pES)
{
	BOOL bEditMode=pES->IsEditMode();
	if ((_ver==proto->GetVer())&&(bEditMode==_bEditMode))
		return;//no change

	//创建一个entity用来得到desc
	IEntity *en=NULL;
	if (bEditMode)
		en=pES->CreateEntity(i_math::matrix43f(),proto->GetID(),TRUE);

	BOOL bForceReload=FALSE;
	if ((!_bEditMode)&&bEditMode)
	{//从运行模式切换到编辑模式的瞬间
		bForceReload=TRUE;
	}

	//载入/更新 standard
	if (TRUE)
	{
		CNodeTree *ntree=proto->GetNodeTree()->GetTree();
		if (!ntree)
			return;//not ready to change

		DWORD c;
		NodeHandle *nodes;
		nodes=ntree->Enum(NodeHandle_Root,NodeType_None,c);

		GraphNodeDescs descs;

		//对于每一个存在于proto里的protonode,看是否已经载入或者需要更新
		std::vector<CGraphStandard *>append;
		for (int i=0;i<c;i++)
		{
			const char *path=ntree->GetPath(nodes[i]);
			ProtoNodeID id=proto->FindNodeID(path);
			IProtoNode *node=proto->GetNode(id);

			if (!node)
				continue;

			GetNodeDescs(descs,proto,node,en);

			int idx=_FindStandard(id);
			if (idx==-1)
			{//如果不存在
				CGraphStandard *p=_LoadStandard(descs,node,gg);
				append.push_back(p);
			}
			else
			{
				CGraphStandard *p=_standards[idx];
				if ((p->_ver!=node->GetVer())||bForceReload)
				{//版本不一样了,重新载入并替换
					Class_Delete(p);
					_standards[idx]=_LoadStandard(descs,node,gg);
				}
			}
		}

		//对于每一个已经载入的standard,看是否存在于proto中,不存在的删掉 
		c=0;
		for (int i=0;i<_standards.size();i++)
		{
			if (proto->GetNode(_standards[i]->_id))
			{//存在，保留
				_standards[c]=_standards[i];
				c++;
			}
			else
				Class_Delete(_standards[i]);
		}
		_standards.resize(c);

		//添加入新增的
		VEC_APPEND(_standards,append);
	}

	_LoadExposes(proto,gg);

	_LoadConnects(proto);

	_focus=NULL;

	//删除用来得到desc的entity
	if (en)
	{
		en->Destroy();

		pES->GetGlobal()->ClearDynEntities();

		EntitySystemInput in;
		in.SetRPSize(i_math::size2di(0,0));
		pES->Update(in);//之所以还要update一下,是为了flush那些defered destroyed 的entity
	}


	_ver=proto->GetVer();
	_bEditMode=bEditMode;
}

//返回是否需要画一个Spawn Mark
BOOL CProtoGraph::_CalcConnSpot(_Connect *p,i_math::pos2di &pt1,i_math::pos2di &pt2)
{
	pt1=p->item[0]->GetConnectSpot(TRUE);
	pt2=p->item[1]->GetConnectSpot(FALSE);
	if ((!p->bToDyn)&&(!p->bToDefer))
		return FALSE;
	pt2.X-=4;
	return TRUE;
}

#define CONN_REMOVE_RADIUS 4

void CProtoGraph::Draw(GraphicsGraph *gg,ProtoNodeID *sels,DWORD c)
{
	for (int i=0;i<_standards.size();i++)
	{
		BOOL bHilight=FALSE;
		for (int j=0;j<c;j++)
		{
			if (sels[j]==_standards[i]->_id)
			{
				bHilight=TRUE;
				break;
			}
		}
		_standards[i]->Draw(gg,bHilight);
	}

	for (int i=0;i<_exposes.size();i++)
		_exposes[i]->Draw(gg,TRUE);

	//draw the dynamic connect
	if (!_connectDyn.IsEmpty())
	{
		i_math::pos2di pt1,pt2;
		pt2=_connectDyn.pt;
		for (int i=0;i<_connectDyn.items.size();i++)
		{
			switch(_connectDyn.type)
			{
			case ConnectDynPG::Connecting:
				pt1=_connectDyn.items[i]->GetConnectSpot(TRUE);
				gg->DrawConnectH(pt1,pt2,RGB(0,255,255));
				break;
			case ConnectDynPG::Connected:
			case ConnectDynPG::Void:
				pt1=_connectDyn.items[i]->GetConnectSpot(FALSE);
				gg->DrawConnectH(pt2,pt1,RGB(0,255,255));
				break;
			}
		}
	}

	//draw the permenant connects
	if (TRUE)
	{
		for (int i=0;i<_connects.size();i++)
		{
			_Connect *p=&_connects[i];
			if (p->item[0]&&p->item[1])
			{
				DWORD col=p->bErr?RGB(255,0,0):RGB(0,255,255);
				
				i_math::pos2di pt1,pt2;
				BOOL bSpawn=_CalcConnSpot(p,pt1,pt2);
				gg->DrawConnectH(pt1,pt2,col);
				if (bSpawn)
					gg->FillCircle(pt2,4,RGB(255,168,0),RGB(255,0,0),255);

				i_math::pos2di ptCenter=(pt1+pt2)/2;
				gg->FrameCircle(ptCenter,CONN_REMOVE_RADIUS,col,2);
			}
			else
			{
				//叉子
				if (p->item[0])
				{
					i_math::pos2di pt=p->item[0]->GetConnectSpot(TRUE);
					pt+=i_math::pos2di(-6,2);
					gg->DrawLine(0xffff0000,2,pt+i_math::pos2di(-3,-3),pt+i_math::pos2di(3,3));
					gg->DrawLine(0xffff0000,2,pt+i_math::pos2di(-3,3),pt+i_math::pos2di(3,-3));
				}
				if (p->item[1])
				{
					i_math::pos2di pt=p->item[1]->GetConnectSpot(FALSE);
					pt+=i_math::pos2di(14,2);
					gg->DrawLine(0xffff0000,2,pt+i_math::pos2di(-3,-3),pt+i_math::pos2di(3,3));
					gg->DrawLine(0xffff0000,2,pt+i_math::pos2di(-3,3),pt+i_math::pos2di(3,-3));
				}

			}
		}
	}

}

BOOL CProtoGraph::ConnectHitTest(int x,int y,PNConnect &conn)
{
	for (int i=0;i<_connects.size();i++)
	{
		_Connect *p=&_connects[i];
		if (p->item[0]&&p->item[1])
		{
			i_math::pos2di pt1,pt2;
			_CalcConnSpot(p,pt1,pt2);
			i_math::pos2di ptCenter=(pt1+pt2)/2;

			if ((x-ptCenter.x)*(x-ptCenter.x)+(y-ptCenter.y)*(y-ptCenter.y)<CONN_REMOVE_RADIUS*CONN_REMOVE_RADIUS)
			{
				conn.id[0]=p->item[0]->id;
				conn.id[1]=p->item[1]->id;
				conn.name[0]=p->item[0]->name.c_str();
				conn.name[1]=p->item[1]->name.c_str();
				return TRUE;
			}
		}
	}

	return FALSE;
}


BOOL CProtoGraph::HitTest(int x,int y,GraphHit &hit)
{
	for (int i=_exposes.size()-1;i>=0;i--)
	{
		if (_exposes[i]->HitTest(x,y,hit))
			return TRUE;
	}

	for(int i=_standards.size()-1;i>=0;i--)
	{
		if (_standards[i]->HitTest(x,y,hit))
			return TRUE;
	}
	return FALSE;
}

ProtoNodeID *CProtoGraph::RectHitTest(i_math::recti &rc,DWORD &c)
{
	_temp2.clear();
	for(int i=_standards.size()-1;i>=0;i--)
	{
		if (rc.isRectCollided(_standards[i]->_rc))
			_temp2.push_back(_standards[i]->_id);
	}

	c=_temp2.size();
	return _temp2.data();
}


BOOL CProtoGraph::SetFocusItem(GraphItem *item)
{
	if (item==_focus)
		return FALSE;
	_focus=item;
	return TRUE;
}

void CProtoGraph::ClearFocusConnect()
{
	for (int i=0;i<_connectDyn.items.size();i++)
		Class_Delete(_connectDyn.items[i]);
	_connectDyn.items.clear();
}


GraphItem **CProtoGraph::GetConnects(GraphItem *item,BOOL bConnecting,DWORD &c)
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


