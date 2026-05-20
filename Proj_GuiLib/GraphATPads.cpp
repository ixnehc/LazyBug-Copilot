/********************************************************************
	created:	2010/4/14   14:05
	file path:	d:\IxEngine\Proj_GuiLib
	author:		chenxi
	
	purpose:	所有CLinkPads 的graph的基类
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"
#include ".\GuiLib.h"

#include "GraphATPads.h"

#include "resdata/AnimTreePads.h"

#include "stringparser/stringparser.h" 

#include "graphicsgraph.h"

#include "WorldSystem/IAnimTreeCtrl.h"





#define MAX_ITEM_WIDTH 200

#define TITLE_HEIGHT 18
#define TITLE_MARGIN 24

#define ITEM_HEIGHT 20

#define STUB_ARROW_WIDTH 18

#define ITEM_SPACE 48
#define MOREBUTTON_SIZE (i_math::size2di(10,8))

#define SEP_HEIGHT 4

#define SHRINK_LENGTH 14

#define EXPOSED_HEIGHT 20
#define EXPOSED_MARGIN 4




//////////////////////////////////////////////////////////////////////////
//CGraphAtp

void CGraphAtp::Zero()
{
	_dbg=NULL;
	_dbgtype=CAnimTreePad::Dbg_None;
	_idTuner=StringID_Invalid;
	_nmTuner="";
}

void CGraphAtp::Clear()
{
	for (int i=0;i<_outs.size();i++)
		Class_Delete(_outs[i]);
	for (int i=0;i<_ins.size();i++)
		Class_Delete(_ins[i]);
	Safe_Class_Delete(_dbg);

	_outs.clear();
	_ins.clear();

	Zero();
}



BOOL CGraphAtp::_ExistInOutSeg()
{
	if ((_outs.size()<=0)&&(_ins.size()<=0))
		return FALSE;
	return TRUE;
}

BOOL CGraphAtp::_ExistCtrlSeg()
{
	return (_dbg!=NULL);
}


BOOL CGraphAtp::_NeedSep2()
{
	return (_ExistInOutSeg()&&_ExistCtrlSeg());
}


void CGraphAtp::_LocateOut(GraphPadItem *item,i_math::recti &rcItem)
{
	item->rc=rcItem;
	item->rcFocus=rcItem;
	item->rcFocus.Right()=item->rcFocus.Left()+item->sz.w+STUB_ARROW_WIDTH+4;
	item->rcFocus.inflate(-1,-1,-1,-1);
	rcItem+=i_math::pos2di(0,ITEM_HEIGHT);
}

void CGraphAtp::_LocateIn(GraphPadItem *item,i_math::recti &rcItem)
{
	item->rc=rcItem;
	item->rcFocus=rcItem;
	item->rcFocus.Left()=item->rcFocus.Right()-item->sz.w-STUB_ARROW_WIDTH-4;
	item->rcFocus.inflate(-1,-1,-1,-1);
	rcItem+=i_math::pos2di(0,ITEM_HEIGHT);
}

void CGraphAtp::_LocateDbg(GraphPadItem *item,i_math::recti &rcItem)
{
	item->rc=rcItem;
	if ((_dbgtype==CAnimTreePad::Dbg_1D)||(_dbgtype==CAnimTreePad::Dbg_Name))
	{
		item->rcFocus=rcItem;
		item->rcFocus.inflate(-1,0,-1,0);
		item->rc.Bottom()+=ITEM_HEIGHT;
		rcItem+=i_math::pos2di(0,ITEM_HEIGHT*2);
	}
	if (_dbgtype==CAnimTreePad::Dbg_2D)
	{
		item->rcFocus=rcItem;
		item->rcFocus.inflate(-1,-1,-1,-1);
		item->rcFocus.Bottom()=item->rcFocus.Top()+rcItem.getWidth();
		item->rc.Bottom()=item->rc.Top()+rcItem.getWidth()+ITEM_HEIGHT;
		rcItem+=i_math::pos2di(0,rcItem.getWidth()+ITEM_HEIGHT);
	}
}


void CGraphAtp::RecalcLayout(GraphicsGraph *gg)
{
	//先计算所有item的size
	if (TRUE)
	{
		_title.UpdateSize(gg);
		for (int i=0;i<_ins.size();i++)
			_ins[i]->UpdateSize(gg);
		for (int i=0;i<_outs.size();i++)
			_outs[i]->UpdateSize(gg);
		if (_dbg)
			_dbg->UpdateSize(gg);
	}

	DWORD w,h;
	//precalculate the w x h
	if (TRUE)
	{
		w=_title.sz.w+TITLE_MARGIN;
		DWORD ww;

		for (int i=0;i<_ins.size();i++)
		{
			if (i>=_outs.size())
				ww=_ins[i]->sz.w+STUB_ARROW_WIDTH+ITEM_SPACE;
			else
				ww=_ins[i]->sz.w+STUB_ARROW_WIDTH+
						ITEM_SPACE+
						_outs[i]->sz.w+STUB_ARROW_WIDTH;
			if (ww>w)
				w=ww;
		}

		for (int i=0;i<_outs.size();i++)
		{
			ww=_outs[i]->sz.w+STUB_ARROW_WIDTH+ITEM_SPACE;
			if (ww>w)
				w=ww;
		}

	}

	if (TRUE)
	{
		h=TITLE_HEIGHT;
		if (_outs.size()>_ins.size())
			h+=_outs.size()*ITEM_HEIGHT;
		else
			h+=_ins.size()*ITEM_HEIGHT;
		if ((_dbgtype==CAnimTreePad::Dbg_1D)||(_dbgtype==CAnimTreePad::Dbg_Name))
			h+=ITEM_HEIGHT*2;
		if (_dbgtype==CAnimTreePad::Dbg_2D)
			h+=w;
	}


	if (_NeedSep2())
		h+=SEP_HEIGHT;

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

	rcItem+=i_math::pos2di(0,TITLE_HEIGHT);

	if (TRUE)
	{
		i_math::recti rcItemOut,rcItemIn;
		rcItemOut=rcItem;
		rcItemIn=rcItem;
		for (int i=0;i<_ins.size();i++)
			_LocateIn(_ins[i],rcItemIn);


		for (int i=0;i<_outs.size();i++)
			_LocateOut(_outs[i],rcItemOut);

		if (rcItemIn.Top()>rcItemOut.Top())
			rcItem=rcItemIn;
		else
			rcItem=rcItemOut;
	}

	if (_NeedSep2())
	{
		_ySep2=rcItem.Top();
		rcItem+=i_math::pos2di(0,SEP_HEIGHT);
	}
	else
		_ySep2=-10000;

	if (_dbg)
		_LocateDbg(_dbg,rcItem);
}

void CGraphAtp::_DrawTitle(GraphPadItem*item,BOOL bHiLight,GraphicsGraph *gg)
{
	CAnimTreePads*pads=(CAnimTreePads*)item->graph->GetPads();
	CAnimTreePad*pad=(CAnimTreePad*)pads->FindPad(item->id);
	if (!_bFolder)
	{
		switch(pad->GetCategory())
		{
			case CAnimTreePad::Sequence:
				gg->GradientV(item->rc,RGB(128,0,0),RGB(128,0,0));
				break;
			case CAnimTreePad::FloatValue:
				gg->GradientV(item->rc,RGB(0,91,182),RGB(0,91,182));
				break;
			case CAnimTreePad::Path:
				gg->GradientV(item->rc,RGB(255,224,0),RGB(255,224,0));
				break;
			case CAnimTreePad::BoneCtrl:
				gg->GradientV(item->rc,RGB(182,91,0),RGB(182,91,0));
				break;
			case CAnimTreePad::IKCtrl:
				gg->GradientV(item->rc,RGB(182,91,182),RGB(182,91,182));
				break;
			default:
				gg->GradientV(item->rc,RGB(0,0,0),RGB(0,0,0));
				break;
		}
	}
	else
		gg->GradientV(item->rc,RGB(0,64,0),RGB(0,64,0));

	if (pad->GetCategory()!=CAnimTreePad::Path)
		gg->DrawText(item->show.c_str(),item->rc,DT_LEFT,FALSE,0xffffffff);//白字
	else
		gg->DrawText(item->show.c_str(),item->rc,DT_LEFT,FALSE,0xff000000);//黑字

	//画一个框表示weight

	IAnimTreeCtrl *ctrl=((CGraphATPads*)item->graph)->GetCtrl();
	if (ctrl)
	{
		float wt=ctrl->GetWeight(item->id);
		gg->DrawFrameRect(_rc,RGB(255,128,64),2,(DWORD)(wt*254.0f));
	}

	if (bHiLight)
		gg->DrawFrameRect(_rc,RGB(255,255,0),2);

	if (ctrl)
	{
		BOOL bAwake=ctrl->IsAwake(item->id);
		if (!bAwake)
			gg->FillSolidRect(_rc,RGB(128,128,128),96);
	}

	if (pads->GetDefRoot()==pad->GetID())
	{
		i_math::recti rc=item->rc;
		rc+=i_math::pos2di(0,rc.getHeight()+2);
		rc.Left()+=2;
		rc.Right()=rc.Left()+rc.getHeight();
		rc.inflate(-3,-3,-3,-3);
		gg->DrawRoundCornerRect(rc,4,RGB(0,255,0),RGB(0,128,0));
		gg->FrameRoundCornerRect(rc,4,RGB(0,0,0),1);
	}
}


void CGraphAtp::_DrawOut(GraphPadItem*item,GraphicsGraph *gg)
{
	if (item->IsFocus())
		gg->FillSolidRect(item->rcFocus,RGB(120,120,255));

	i_math::recti rc=item->rc;
	i_math::pos2di pt=rc.UpperLeftCorner;
	pt.y+=2;
	pt.x+=2;
	gg->GradientArrow(pt,RGB(139,69,0),RGB(255,126,0),TRUE);

	rc+=i_math::pos2di(STUB_ARROW_WIDTH,0);
	gg->DrawText(item->show.c_str(),rc);
}

void CGraphAtp::_DrawIn(GraphPadItem*item,GraphicsGraph *gg)
{
	if (item->IsFocus())
		gg->FillSolidRect(item->rcFocus,RGB(120,120,255));

	i_math::recti rc=item->rc;
	i_math::pos2di pt=rc.UpperLeftCorner;
	pt.y+=2;
	pt.x=rc.Right()-STUB_ARROW_WIDTH;
	gg->GradientArrow(pt,RGB(139,69,0),RGB(255,126,0),TRUE);

	rc-=i_math::pos2di(STUB_ARROW_WIDTH+2,0);
	gg->DrawText(item->show.c_str(),rc,DT_RIGHT);
}

void CGraphAtp::_DrawDbg(GraphPadItem*item,GraphicsGraph *gg)
{
	CGraphATPads *graph=(CGraphATPads *)item->graph;
	PadDyn *dyn=graph->FindPadDyn(item->id);

	if ((_dbgtype==CAnimTreePad::Dbg_1D)||(_dbgtype==CAnimTreePad::Dbg_Name))
	{
		i_math::recti rc=item->rcFocus;
		gg->FillSolidRect(rc,RGB(120,120,120));
		gg->DrawFrameRect(rc,RGB(90,90,90),1);
		if (dyn)
		{
			i_math::recti rcBox;
			rcBox.set(0,0,DVBOX_WIDTH,ITEM_HEIGHT);
			i_math::pos2di pt;
			pt.x=rc.Left()+(int)(dyn->dv.x*(float)(rc.getWidth()-DVBOX_WIDTH));
			pt.y=rc.Top();
			rcBox+=pt;
			rcBox.inflate(0,-1,0,-1);

			gg->FillSolidRect(rcBox,RGB(64,128,255));
			gg->DrawFrameRect(rcBox,RGB(40,40,40),1);
		}

		//desc
		if (TRUE)
		{
			IAnimTreeCtrl *ctrl=((CGraphATPads*)item->graph)->GetCtrl();
			std::string s=ctrl->GetDesc(item->id);
			i_math::recti rc=item->rcFocus;
			rc+=i_math::pos2di(0,ITEM_HEIGHT);
			gg->DrawText(s.c_str(),rc);
		}

	}
}

void CGraphAtp::_DrawTuner(GraphicsGraph *gg)
{
	if ((_idTuner==StringID_Invalid)&&(_nmTuner.empty()))
		return;

	i_math::pos2di pt,pt2;
	pt=_rc.UpperLeftCorner;
	pt.y+=_rc.getHeight();
	pt.x+=6;

	pt2=pt;
	pt2.y+=12;
	gg->DrawLine(0xff000000,1,pt,pt2);
	pt=pt2;
	pt2.x+=12;
	gg->DrawLine(0xff000000,1,pt,pt2);

	const char *str=StrLib_GetStr(_idTuner);
	if (!_nmTuner.empty())
		str=_nmTuner.c_str();

	pt2.y-=6;
	pt2.x+=2;
	i_math::recti rc;
	rc.set(0,0,500,100);
	rc+=pt2;
	if (TRUE)
	{
		i_math::size2di sz=gg->MessureText(str);
		i_math::recti rcBg=rc;
		rcBg.Right()=rcBg.Left()+sz.w;
		rcBg.Bottom()=rcBg.Top()+sz.h;
		rcBg.inflate(2,2,2,2);
		gg->DrawRoundCornerRect(rcBg,3,RGB(64,128,255),RGB(0,64,255));
		gg->FrameRoundCornerRect(rcBg,3,RGB(0,0,0),1);
	}
	gg->DrawText(str,rc,DT_LEFT,FALSE,RGB(0,0,0));
}


void CGraphAtp::_DrawSep(GraphicsGraph *gg,int ySep)
{
	if (ySep==-10000)
		return;
	i_math::recti rc=_rc;
	rc.Top()=ySep+1;
	rc.Bottom()=ySep+2;

	gg->FillSolidRect(rc,RGB(128,128,128));
	
}


void CGraphAtp::Draw(GraphicsGraph *gg,BOOL bHilight)
{
	if (_bCurFolder)
	{
		i_math::recti rc=_rc;
		rc.inflate(4,4,4,4);
		gg->FillSolidRect(rc,RGB(0,168,0),255);
	}

	//the bg
	if ((!_bFolder)||_bCurFolder)
	{
		gg->FillSolidRect(_rc,RGB(168,168,168),255);
		gg->DrawFrameRect(_rc,0,1);
	}
	else
	{
		gg->FillSolidRect(_rc,RGB(0,168,0),255);
		gg->DrawFrameRect(_rc,0,1);
	}



	for (int i=0;i<_ins.size();i++)
		_DrawIn(_ins[i],gg);
	for (int i=0;i<_outs.size();i++)
		_DrawOut(_outs[i],gg);
	if (_dbg)
		_DrawDbg(_dbg,gg);
	_DrawTuner(gg);

	_DrawSep(gg,_ySep2);

	_DrawTitle(&_title,bHilight,gg);


}


BOOL CGraphAtp::_ItemHitTest(int x,int y,GraphPadItem*item,int part,GraphPadHit &hit)
{
	if (item->rcFocus.isPointInside(x,y))
	{
		hit.part=(GraphPadHit::Part)part;
		hit.item=item;
		return TRUE;
	}
	return FALSE;
}
 
BOOL CGraphAtp::_ItemsHitTest(int x,int y,GraphPadItem**items,DWORD c,int part,GraphPadHit &hit)
{
	for (int i=0;i<c;i++)
	{
		if (_ItemHitTest(x,y,items[i],part,hit))
			return TRUE;
	}

	return FALSE;
}

GraphPadItem *CGraphAtp::FindItem(const char *name)
{
	int idx;
	PVEC_FIND_BY_ELEMENT(_outs,name,name,idx);
	if (idx!=-1)
		return _outs[idx];
	PVEC_FIND_BY_ELEMENT(_ins,name,name,idx);
	if (idx!=-1)
		return _ins[idx];
	return NULL;
}



BOOL CGraphAtp::HitTest(int x,int y,GraphPadHit &hit)
{
	if (!_rc.isPointInside(x,y))
		return FALSE;

	hit.id=_id;

	if (_ItemsHitTest(x,y,_outs.data(),_outs.size(),GraphPadHit::Out,hit))
		return TRUE;
	if (_ItemsHitTest(x,y,_ins.data(),_ins.size(),GraphPadHit::In,hit))
		return TRUE;
	if (_dbg)
	{
		if (_ItemsHitTest(x,y,&_dbg,1,GraphPadHit::Ctrl,hit))
			return TRUE;
	}

	hit.part=GraphPadHit::Blank;
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//CGraphATPads


CGraphPad* CGraphATPads::_LoadPad(CLinkPad *pad)
{
	CGraphAtp*p=Class_New2(CGraphAtp);

	_FillGraphPad(p,pad);

	std::string title;
	if ((!p->_bFolder)||(p->_bCurFolder))
	{
		title=pad->GetShowName();
		title=title+"("+pad->GetTypeName()+")";
	}
	else
	{
		title=pad->GetFolderName();
		if (title=="")
			title="<No Name>";
	}

	p->_title.SetShow(title.c_str());
	p->_title.graph=this;
	p->_title.id=p->_id;


	DWORD cStubs=pad->GetStubCount();
	for (int j=0;j<cStubs;j++)
	{
		PadStub stb=pad->GetStub(j);

		if (p->_bFolder&&(!p->_bCurFolder))
		{
			if (stb.type==PadStub_In)
				continue;
		}

		GraphPadItem *item=Class_New2(GraphPadItem);
		item->graph=this;
		item->id=p->_id;
		item->SetShow(stb.name);
		item->SetName(stb.name);

		item->bConnectable=TRUE;
		item->iStub=j;

		if (stb.type==PadStub_Out)
			p->_outs.push_back(item);
		if (stb.type==PadStub_In)
			p->_ins.push_back(item);
	}

	if ((!p->_bFolder)||(p->_bCurFolder))
	{
		if (((CAnimTreePad*)pad)->GetDbgType()!=CAnimTreePad::Dbg_None)
		{
			GraphPadItem *item=Class_New2(GraphPadItem);
			item->graph=this;
			item->id=p->_id;
			p->_dbg=item;
			p->_dbgtype=((CAnimTreePad*)pad)->GetDbgType();
		}

		AtpTunerInfo *tui=((CAnimTreePad*)pad)->GetTunerInfo();
		if (tui)
		{
			p->_idTuner=tui->idNm;
			p->_nmTuner=tui->nm;
		}
	}


	return p;
}

void CGraphATPads::_DrawPermConnect(GraphicsGraph *gg)
{
	for (int i=0;i<_connects.size();i++)
	{
		_Connect *p=&_connects[i];
		if (p->item[0]&&p->item[1])
		{
			DWORD col=RGB(0,0,0);
			if (_ctrl)
			{
				float wt=_ctrl->GetLinkWeight(p->item[1]->id,p->item[1]->name.c_str());
				DWORD col2=RGB(255,128,64);
				col=i_math::lerp_color(col,col2,wt);
			}
			gg->DrawConnectH(p->item[0]->GetConnectSpot(TRUE),
				p->item[1]->GetConnectSpot(FALSE),col,DRAWCONNECT_INV);
		}
	}

}


PadDyn *CGraphATPads::FindPadDyn(PadID id)
{
	std::unordered_map<PadID,PadDyn>::iterator it=_dyns.find(id);
	if (it==_dyns.end())
		return NULL;
	return &(*it).second;
}


void CGraphATPads::SyncDyn(CLinkPads *pads)
{
	if (!pads)
	{
		_dyns.clear();
		return;
	}

	//先在_dyns里添加需要添加的
	if (TRUE)
	{
		DWORD c;
		CLinkPad **p=pads->GetPads(c);
		for (int i=0;i<c;i++)
		{
			PadID id=p[i]->GetID();
			std::unordered_map<PadID,PadDyn>::iterator it=_dyns.find(id);
			if (it==_dyns.end())
			{//不存在,需要添加一个新的
				PadDyn t;
				_dyns[id]=t;
			}
		}
	}

	//再在_dyns里删除不存在的pad
	if (TRUE)
	{
		std::unordered_map<PadID,PadDyn>::iterator it=_dyns.begin();
		std::unordered_map<PadID,PadDyn>::iterator it2;
		while(it!=_dyns.end())
		{
			it2=it;
			it++;
			PadID id=(*it2).first;
			if (!pads->FindPad(id))
			{//不存在,需要删除
				_dyns.erase(it2);
			}
		}
	}

}
