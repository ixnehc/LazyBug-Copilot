/********************************************************************
	created:	2010/4/14   14:05
	file path:	d:\IxEngine\Proj_GuiLib
	author:		chenxi
	
	purpose:	所有CLinkPads 的graph的基类
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"
#include ".\GuiLib.h"

#include "GraphBgPads.h"

#include "behaviorgraph/BehaviorGraphPads.h"
#include "behaviorgraph/Behavior.h"

#include "behaviorgraph/BgnState.h"

#include "behaviorgraph/BehaviorDebug_RegistryBase.h"

#include "stringparser/stringparser.h" 


#include "graphicsgraph.h"

#include "RenderSystem/IRenderSystem.h"
#include "RenderSystem/IRecords.h"

#include "records/records.h"





#define MAX_ITEM_WIDTH 250

#define TITLE_HEIGHT 18
#define TITLE_MARGIN 24

#define ITEM_HEIGHT 20
#define MIN_WIDTH 140

#define STUB_ARROW_WIDTH 18

#define ITEM_SPACE 48
#define MOREBUTTON_SIZE (i_math::size2di(10,8))

#define SEP_HEIGHT 4

#define SHRINK_LENGTH 14

#define EXPOSED_HEIGHT 20
#define EXPOSED_MARGIN 4

#define DESC_MARGIN 4

#define CONTROLMARK_WIDTH 12
#define CONTROLMARK_HEIGHT 7
#define CONTROL_HEIGHT 8

#define WIDTH_CINMARK  60

//////////////////////////////////////////////////////////////////////////
//FillDescAssist_GuiLib
const char *FillDescAssist_GuiLib::_GetRecName(RecordID idRec,const char *path)
{
	const char *nm="";
	IRecords *records=(IRecords*)g_ssGuiLib.pRS->GetRecordsMgr()->ObtainRes(path);
	if (records)
	{
		CRecords *r=records->GetRecords();
		if (r)
			nm=r->GetName(idRec);
	}
	SAFE_RELEASE(records);

	return nm;
}

CRecord *FillDescAssist_GuiLib::_GetClonedRec(RecordID idRec,const char *path)
{
	CRecord *recRet=NULL;
	const char *nm="";
	IRecords *records=(IRecords*)g_ssGuiLib.pRS->GetRecordsMgr()->ObtainRes(path);
	if (records)
	{
		CRecords *r=records->GetRecords();
		if (r)
		{
			CRecord *rec=r->GetSafeRecord(idRec);
			if (rec)
				recRet=rec->Clone();
		}
	}
	SAFE_RELEASE(records);

	return recRet;
}


float FillDescAssist_GuiLib::CalcHeight(const char *str)
{
	if (!_gg)
		return 0.0f;
	i_math::size2di sz=_gg->MessureText(str,MAX_ITEM_WIDTH);
	return (float)sz.h;
}

BehaviorMemType FillDescAssist_GuiLib::GetMemType(StringID nm)
{
	if (!_pads)
		return BehaviorMemType_None;

	extern BehaviorMemType ResolveSimpleVarType(StringID nmVar);
	BehaviorMemType tp=ResolveSimpleVarType(nm);
	if (tp!=BehaviorMemType_None)
		return tp;
	return _pads->GetVarMemType(nm);
}




////////////////////////////////////////////////////////////////////////
//GraphBgPadItem

i_math::pos2di GraphBgPadItem::GetConnectSpot(BOOL bConnecting)
{
	PadStubType tp=GetStubType();
	i_math::pos2di pt;
	switch(tp)
	{
		case PadStub_In:
			pt.set(rcFocus.Left()-2,rcFocus.getCenter().y);
			break;
		case PadStub_Out:
			pt.set(rcFocus.Right()+2,rcFocus.getCenter().y);
			break;
		case PadStub_COut:
			pt.set(rcFocus.getCenter().x,rcFocus.Bottom());
			break;
		case PadStub_CIn:
			pt.set(rcFocus.getCenter().x,rcFocus.Top());
			break;
	}

	return pt;
}



//////////////////////////////////////////////////////////////////////////
//CGraphBgPad

void CGraphBgPad::Zero()
{
	_bBase=FALSE;
	_bOverriden=FALSE;
}

void CGraphBgPad::Clear()
{
	for (int i=0;i<_outs.size();i++)
		Class_Delete(_outs[i]);
	for (int i=0;i<_ins.size();i++)
		Class_Delete(_ins[i]);
	for (int i=0;i<_cins.size();i++)
		Class_Delete(_cins[i]);
	for (int i=0;i<_couts.size();i++)
		Class_Delete(_couts[i]);

	_outs.clear();
	_ins.clear();
	_couts.clear();
	_cins.clear();

	Zero();
}



BOOL CGraphBgPad::_ExistInOutSeg()
{
	if ((_outs.size()<=0)&&(_ins.size()<=0))
		return FALSE;
	return TRUE;
}

BOOL CGraphBgPad::_ExistCtrlSeg()
{
	if (_couts.size()<=0)
		return FALSE;
	return TRUE;
}


BOOL CGraphBgPad::_NeedSep2()
{
	return _ExistCtrlSeg();
}


void CGraphBgPad::_LocateOut(GraphPadItem *item,i_math::recti &rcItem)
{
	item->rc=rcItem;
	item->rcFocus=rcItem;
	item->rcFocus.Left()=item->rcFocus.Right()-item->sz.w-STUB_ARROW_WIDTH-4;
	item->rcFocus.inflate(-1,-1,-1,-1);
	rcItem+=i_math::pos2di(0,ITEM_HEIGHT);
}

void CGraphBgPad::_LocateIn(GraphPadItem *item,i_math::recti &rcItem)
{
	item->rc=rcItem;
	item->rcFocus=rcItem;
	item->rcFocus.Right()=item->rcFocus.Left()+item->sz.w+STUB_ARROW_WIDTH+4;
	item->rcFocus.inflate(-1,-1,-1,-1);
	rcItem+=i_math::pos2di(0,ITEM_HEIGHT);
}


void CGraphBgPad::RecalcLayout(GraphicsGraph *gg)
{
	//先计算所有item的size
	if (TRUE)
	{
		_title.UpdateSize(gg);
		for (int i=0;i<_ins.size();i++)
			_ins[i]->UpdateSize(gg);
		for (int i=0;i<_outs.size();i++)
			_outs[i]->UpdateSize(gg);
		for (int i=0;i<_cins.size();i++)
			_cins[i]->UpdateSize(gg);
		for (int i=0;i<_couts.size();i++)
			_couts[i]->UpdateSize(gg);
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

	if (!_desc.empty())
	{
		i_math::size2di sz=gg->MessureText(_desc.c_str(),1000);
		sz.w+=16;
		if (sz.w>MAX_ITEM_WIDTH)
			sz.w=MAX_ITEM_WIDTH;

		if (sz.w>w)
			w=sz.w;
	}

	if (TRUE)
	{
		DWORD wCOuts=_couts.size()*WIDTH_CINMARK;
		if (wCOuts>w)
			w=wCOuts;
	}

	if (w<MIN_WIDTH)
		w=MIN_WIDTH;

	if (TRUE)
	{
		h=TITLE_HEIGHT;
		if (_outs.size()>_ins.size())
			h+=_outs.size()*ITEM_HEIGHT;
		else
			h+=_ins.size()*ITEM_HEIGHT;
	}

	DWORD hDesc=0;
	if (TRUE)
	{
		i_math::size2di sz=gg->MessureText(_desc.c_str(),w-DESC_MARGIN*2);
		hDesc=sz.h;
		h+=hDesc+2*DESC_MARGIN;
	}

	if (_NeedSep2())
		h+=CONTROL_HEIGHT;

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

	_rcDesc=rcItem;
	_rcDesc.Bottom()=_rcDesc.Top()+hDesc+2*DESC_MARGIN;
	_rcDesc.inflate(-DESC_MARGIN,-DESC_MARGIN,-DESC_MARGIN,-DESC_MARGIN);

	rcItem+=i_math::pos2di(0,hDesc+2*DESC_MARGIN);


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


	if (_cins.size()>0)
	{
		int step=w/(_cins.size()+1);
		int x=_pt.x+step;
		for (int i=0;i<_cins.size();i++)
		{
			i_math::recti rc;
			rc.set(x,_pt.y,x,_pt.y);
			rc.inflate(CONTROLMARK_WIDTH/2,CONTROLMARK_HEIGHT,CONTROLMARK_WIDTH/2,0);
			_cins[i]->rc=rc;
			_cins[i]->rcFocus=rc;
			_cins[i]->rcFocus.inflate(1,1,1,1);
			x+=step;
		}
	}

	if (_couts.size()>0)
	{
		int step=w/(_couts.size()+1);
		int x=_pt.x+step;
		for (int i=0;i<_couts.size();i++)
		{
			i_math::recti rc;
			rc.set(x,rcItem.Top()+2,x,rcItem.Top()+2);
			rc.inflate(CONTROLMARK_WIDTH/2,0,CONTROLMARK_WIDTH/2,CONTROLMARK_HEIGHT);
			_couts[i]->rc=rc;
			_couts[i]->rcFocus=rc;
			_couts[i]->rcFocus.inflate(1,1,1,1);
			x+=step;
		}
	}


	if (_NeedSep2())
	{
		_ySep2=rcItem.Top();
		rcItem+=i_math::pos2di(0,CONTROL_HEIGHT);
	}
	else
		_ySep2=-10000;

}

void DrawPersistFlag(GraphicsGraph *gg,i_math::recti &rc0)
{
	i_math::recti rc=rc0;
	rc.Right()=rc.Left()+rc.getHeight();

	rc.inflate(-3,-3,-3,-3);

	gg->GradientPie(rc,RGB(255,255,128),RGB(255,128,128));
	gg->FrameCircle(rc.getCenter(),rc.getWidth()/2,RGB(0,0,0),0);

	rc.inflate(-4,-4,-4,-4);
	gg->GradientPie(rc,RGB(0,0,0),RGB(0,0,0));
}

void DrawSyncFlag(GraphicsGraph *gg,i_math::recti &rc0)
{
	i_math::recti rc=rc0;
	rc.Right()=rc.Left()+rc.getHeight();
	rc.inflate(-3,-3,-3,-3);

	gg->GradientPyrimid(rc,RGB(255,255,128),RGB(255,128,128));

}

void CGraphBgPad::_DrawBP(GraphPadItem*item,GraphicsGraph *gg)
{
	CBehaviorGraphPads*pads=(CBehaviorGraphPads*)item->graph->GetPads();
	CBehaviorGraphPad*pad=(CBehaviorGraphPad*)pads->FindPad(item->id);
	if (!pad)
		return;

	//检查
	extern CBehaviorDebugClient_RegistryBase *GetBehaviorDebugClient();
	CBehaviorDebugClient_RegistryBase *dbgclient=GetBehaviorDebugClient();

	BehaviorDebugState *state=dbgclient->GetState();

	//early check
	if (TRUE)
	{
		BOOL bAnyPotential=FALSE;
		for (int i=0;i<state->bps.size();i++)
		{
			if (state->bps[i].idPad==pad->GetID())
			{
				bAnyPotential=TRUE;
				break;
			}
		}

		if (!bAnyPotential)
		{
			if(state->bpCur.idPad==pad->GetID())
				bAnyPotential=TRUE;
		}

		if (!bAnyPotential)
			return;
	}

	extern StringID GetGraphName(CLinkPads *pads);
	StringID nmBg=GetGraphName(pads);
	if (nmBg==StringID_Invalid)
		return;

	BehaviorDebugStep step;
	step.nmBG=nmBg;
	step.idPad=pad->GetID();

	if (state->FindBP(step)>=0)
	{//有断点
		i_math::recti rc=item->rc;
		rc.Right()=rc.Left()-4;
		rc.Left()=rc.Left()-rc.getHeight()-4;
		gg->DrawRoundCornerRect(rc,8,RGB(255,0,0),RGB(128,0,0));
		gg->FrameRoundCornerRect(rc,6,RGB(0,0,0),RGB(0,0,0));
//		gg->GradientCross(rc.UpperLeftCorner,RGB(255,0,0),RGB(255,0,0));
	}

	if (step.Equal(state->bpCur))
	{
		i_math::recti rc=item->rc;
		i_math::pos2di pt=rc.UpperLeftCorner;
		pt.x-=rc.getHeight()-4;
		if (state->bpCur.result==A_Ok)
			gg->GradientCheck(pt,RGB(0,128,0),RGB(0,255,0));
		else
		{
			if (state->bpCur.result==A_Fail)
				gg->GradientCross(pt,RGB(0,128,0),RGB(0,255,0));
			else
			{
				if (!state->bpCur.bBreaking)
					gg->GradientArrow(pt,RGB(0,128,0),RGB(0,255,0),FALSE);
				else
					gg->GradientArrow(pt,RGB(128,0,0),RGB(255,0,0),FALSE);
			}
		}
	}

}

BOOL CGraphBgPad::_IsPending(GraphPadItem*item, GraphicsGraph *gg)
{
	CBehaviorGraphPads*pads=(CBehaviorGraphPads*)item->graph->GetPads();
    extern CBehaviorDebugClient_RegistryBase *GetBehaviorDebugClient();
    CBehaviorDebugClient_RegistryBase *dbgclient = GetBehaviorDebugClient();

    extern StringID GetGraphName(CLinkPads *pads);
    StringID nmBg = GetGraphName(pads);
    if (nmBg != StringID_Invalid)
    {
		BehaviorDebugFrameData *framedata = dbgclient->FindSelFrameData(nmBg);
		if (framedata)
		{
            int idx;
            VEC_FIND(framedata->pendings, item->id, idx);
            if (idx != -1)
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}


void CGraphBgPad::_DrawTitle(GraphPadItem*item,BOOL bHiLight,GraphicsGraph *gg)
{
	CBehaviorGraphPads*pads=(CBehaviorGraphPads*)item->graph->GetPads();
	CBehaviorGraphPad*pad=(CBehaviorGraphPad*)pads->FindPad(item->id);
	if (!pad)
		return;
	switch(pad->GetCategory())
	{
		case BgpCtgr_State:
//				gg->GradientV(item->rc,RGB(255,0,0),RGB(128,0,0));
			gg->DrawRoundCornerRect(item->rc,6,RGB(255,0,0),RGB(128,0,0));
			gg->FrameRoundCornerRect(item->rc,6,RGB(0,0,0),RGB(0,0,0));
			break;
		case BgpCtgr_Func:
			gg->DrawRoundCornerRect(item->rc,6,RGB(255,128,255),RGB(128,64,128));
			gg->FrameRoundCornerRect(item->rc,6,RGB(0,0,0),RGB(0,0,0));
			break;
		case BgpCtgr_Talk:
			gg->DrawRoundCornerRect(item->rc,6,RGB(128,128,255),RGB(64,64,128));
			gg->FrameRoundCornerRect(item->rc,6,RGB(0,0,0),RGB(0,0,0));
			break;
		case BgpCtgr_Action:
			gg->DrawRoundCornerRect(item->rc,6,RGB(255,128,64),RGB(128,64,32));
			gg->FrameRoundCornerRect(item->rc,6,RGB(0,0,0),RGB(0,0,0));
			break;
		case BgpCtgr_Controller:
		{
			extern BOOL IsRelayPad(CBehaviorGraphPad*pad);
			if (!IsRelayPad(pad))
			{
				gg->DrawRoundCornerRect(item->rc,6,RGB(64,164,164),RGB(32,64,64));
				gg->FrameRoundCornerRect(item->rc,6,RGB(0,0,0),RGB(0,0,0));
			}
			else
			{
				gg->DrawRoundCornerRect(item->rc,6,RGB(64,128,64),RGB(32,64,32));
				gg->FrameRoundCornerRect(item->rc,6,RGB(0,0,0),RGB(0,0,0));
			}
			break;
		}
		case BgpCtgr_Condition:
			gg->DrawRoundCornerRect(item->rc,6,RGB(64,128,255),RGB(32,64,128));
			gg->FrameRoundCornerRect(item->rc,6,RGB(0,0,0),RGB(0,0,0));
			break;
		case BgpCtgr_Helper:
			gg->DrawRoundCornerRect(item->rc,6,RGB(190,190,90),RGB(128,64,32));
			gg->FrameRoundCornerRect(item->rc,6,RGB(0,0,0),RGB(0,0,0));
			break;
	}

	extern BOOL IsPersistPad(CBehaviorGraphPad*pad);
	extern BOOL IsSyncPad(CBehaviorGraphPad*pad);
	if (!IsPersistPad(pad)&&(!IsSyncPad(pad)))
		gg->DrawText(item->show.c_str(),item->rc,DT_LEFT,FALSE,0xffffffff);//白字
	else
	{
		i_math::recti rc=item->rc;
		if (IsPersistPad(pad))
		{
			DrawPersistFlag(gg,rc);
			rc.Left()+=rc.getHeight();
			rc.Right()+=rc.getHeight();
		}
		if (IsSyncPad(pad))
		{
			DrawSyncFlag(gg,rc);
			rc.Left()+=rc.getHeight();
			rc.Right()+=rc.getHeight();
		}
		gg->DrawText(item->show.c_str(),rc,DT_LEFT,FALSE,0xffffffff);//白字
	}

	if (bHiLight)
//		gg->FrameRoundCornerRect(_rc,6,RGB(255,255,0),RGB(255,255,0));
		gg->DrawFrameRect(_rc,RGB(255,255,0),2);


}


void CGraphBgPad::_DrawOut(GraphPadItem*item,GraphicsGraph *gg)
{
	if (item->IsFocus())
		gg->FillSolidRect(item->rcFocus,RGB(120,120,255));

	i_math::recti rc=item->rc;
	i_math::pos2di pt=rc.UpperLeftCorner;
	pt.y+=2;
	pt.x=rc.Right()-STUB_ARROW_WIDTH;
	gg->GradientArrow(pt,RGB(139,69,0),RGB(255,126,0),FALSE);

	rc-=i_math::pos2di(STUB_ARROW_WIDTH+2,0);
	gg->DrawText(item->show.c_str(),rc,DT_RIGHT);

}

void CGraphBgPad::_DrawIn(GraphPadItem*item,GraphicsGraph *gg)
{
	if (item->IsFocus())
		gg->FillSolidRect(item->rcFocus,RGB(120,120,255));

	i_math::recti rc=item->rc;
	i_math::pos2di pt=rc.UpperLeftCorner;
	pt.y+=2;
	pt.x+=2;
	gg->GradientArrow(pt,RGB(139,69,0),RGB(255,126,0),FALSE);

	rc+=i_math::pos2di(STUB_ARROW_WIDTH,0);
	if (item->show=="开始")
		gg->DrawText(item->show.c_str(),rc);
	else
		gg->DrawText(item->show.c_str(),rc,DT_LEFT,FALSE,RGB(64,64,64));
}


void CGraphBgPad::_DrawCIn(GraphPadItem*item,GraphicsGraph *gg)
{
	if (item->IsFocus())
		gg->FillSolidRect(item->rcFocus,RGB(120,120,255));

	i_math::recti rc=item->rc;
	rc.inflate(-1,-1,-1,-1);
	gg->GradientPyrimid(rc,RGB(64,128,255),RGB(32,64,128));
}

void CGraphBgPad::_DrawCOut(GraphPadItem*item,GraphicsGraph *gg)
{
	if (item->IsFocus())
		gg->FillSolidRect(item->rcFocus,RGB(120,120,255));

	i_math::recti rc=item->rc;
	rc.inflate(-1,-1,-1,-1);
	gg->GradientPyrimid(rc,RGB(64,128,255),RGB(32,64,128));

	extern BOOL IsStringBeginWith(const char *s,const char *head);

	if (!IsStringBeginWith(item->show.c_str(),"COut"))
	{
		i_math::recti rcText=item->rc;
		rcText+=i_math::pos2di(rcText.getWidth(),rcText.getHeight());

		i_math::size2di sz=gg->MessureText(item->show.c_str(),10000);
		rcText.Right()=rcText.Left()+sz.w+8;
		rcText.Bottom()=rcText.Top()+sz.h;

		gg->DrawRoundCornerRect(rcText,3,RGB(64,128,255),RGB(32,64,128));
		gg->DrawText(item->show.c_str(),rcText,DT_LEFT,FALSE,RGB(0,0,0));
	}

}


void CGraphBgPad::_DrawSep(GraphicsGraph *gg,int ySep)
{
	if (ySep==-10000)
		return;
	i_math::recti rc=_rc;
	rc.Top()=ySep+1;
	rc.Bottom()=ySep+2;

	gg->FillSolidRect(rc,RGB(128,128,128));
	
}

BOOL IsStatePad(CBehaviorGraphPad*pad)
{
	if (pad->GetClass()->CheckName("CBgp_State"))
		return TRUE;
	if (pad->GetClass()->CheckName("CBgp_TerminateState"))
		return TRUE;
	return FALSE;
}

BOOL IsPersistPad(CBehaviorGraphPad*pad)
{
	if (pad->GetClass()->CheckName("CBgp_State"))
		return (((CBgp_State *)pad)->_flag&BehaviorMemFlag_Persist);

	return FALSE;

}

BOOL IsSyncPad(CBehaviorGraphPad*pad)
{
	if (pad->GetClass()->CheckName("CBgp_State"))
		return (((CBgp_State *)pad)->_flag&BehaviorMemFlag_Sync);

	return FALSE;
}


BOOL IsRelayPad(CBehaviorGraphPad*pad)
{
	if (pad->GetClass()->CheckName("CBgp_Relay"))
		return TRUE;
	return FALSE;
}


BOOL CGraphBgPad::_IsState()
{
	CBehaviorGraphPads*pads=(CBehaviorGraphPads*)_title.graph->GetPads();
	CBehaviorGraphPad*pad=(CBehaviorGraphPad*)pads->FindPad(_title.id);
	return IsStatePad(pad);
}

#define TRY_DRAWDESC(clss)																							\
	extern BOOL DrawDesc_##clss(GraphicsGraph *gg0,CBehaviorGraphPad*pad0,std::string &s,i_math::recti &rcDesc);					\
	if (DrawDesc_##clss(gg,(CBehaviorGraphPad*)pad,_desc,_rcDesc))											\
	break;


void CGraphBgPad::Draw(GraphicsGraph *gg,BOOL bHilight)
{
	if (_bBase)
	{
		i_math::recti rc=_rc;
		rc.inflate(7,7,7,7);
		if (_bCurFolder||_bFolder)
			rc.inflate(2,2,2,2);
		if (!_bOverriden)
			gg->DrawRoundCornerRect(rc,8,RGB(90,46,18),RGB(45,23,10));
		else
			gg->DrawRoundCornerRect(rc,8,RGB(64,128,255),RGB(50,100,200));
	}
	if (_bCurFolder||_bFolder)
	{
		i_math::recti rc=_rc;
		rc.inflate(2,2,2,2);
		gg->FillSolidRect(rc,RGB(0,0,0),255);
	}

    //Pending mark
    if (_IsPending(&_title, gg))
    {
        i_math::recti rc = _rc;
        rc.inflate(10, 10, 10, 10);
        gg->DrawRoundCornerRect(rc, 8, RGB(255, 0, 255), RGB(128, 0, 0));
    }

	//the bg
	gg->DrawRoundCornerRect(_rc,8,RGB(168,168,168),RGB(168,168,168));
	gg->FrameRoundCornerRect(_rc,8,RGB(0,0,0),RGB(0,0,0));

	BOOL bEnabled=FALSE;
	CBehaviorGraphPad::ReturnStyle styleRet=CBehaviorGraphPad::Default;
	const char *comment="";
	while(TRUE)
	{
		i_math::recti rc=_rcDesc;
		rc.inflate(DESC_MARGIN/2,DESC_MARGIN/2,DESC_MARGIN/2,DESC_MARGIN/2);
		gg->DrawRoundCornerRect(rc,3,RGB(220,190,190),RGB(142,128,128));

		CLinkPad *pad=_title.graph->GetPads()->FindPad(_id);
		if (!pad)
			break;
		bEnabled=((CBehaviorGraphPad*)pad)->IsEnabled();
		styleRet=((CBehaviorGraphPad*)pad)->GetReturnStyle();
		comment=((CBehaviorGraphPad*)pad)->GetComment();

		TRY_DRAWDESC(CBgp_SwitchState);
		TRY_DRAWDESC(CBgp_ActivateStates);
		TRY_DRAWDESC(CBgp_StartRelay);
		//XXXXX:more BehaviorGraphPad

		if (!_desc.empty())
			gg->DrawText(_desc.c_str(),_rcDesc);

		break;
	}

	for (int i=0;i<_ins.size();i++)
		_DrawIn(_ins[i],gg);
	for (int i=0;i<_outs.size();i++)
		_DrawOut(_outs[i],gg);

	for (int i=0;i<_cins.size();i++)
		_DrawCIn(_cins[i],gg);
	for (int i=0;i<_couts.size();i++)
		_DrawCOut(_couts[i],gg);

	_DrawSep(gg,_ySep2);

	_DrawTitle(&_title,bHilight,gg);

	if (!bEnabled)
		gg->DrawRoundCornerRect(_rc,8,RGB(128,128,128),RGB(128,128,128),128);

	_DrawBP(&_title,gg);

	if (TRUE)
	{
		i_math::recti rcRet=_rc;
		rcRet+=i_math::pos2di(0,-20);
		rcRet.Bottom()=rcRet.Top()+18;
		switch(styleRet)
		{
			case CBehaviorGraphPad::Not:
			{
				rcRet.Right()=rcRet.Left()+40;
				gg->DrawRoundCornerRect(rcRet,2,RGB(255,255,255),RGB(220,220,220));
				gg->FrameRoundCornerRect(rcRet,2,RGB(0,0,0),RGB(0,0,0));

				gg->DrawText("Not",rcRet,DT_CENTER);
				break;
			}
			case CBehaviorGraphPad::AlwaysTrue:
			case CBehaviorGraphPad::AlwaysFalse:
			{
				rcRet.Right()=rcRet.Left()+60;
				gg->DrawRoundCornerRect(rcRet,2,RGB(255,255,255),RGB(220,220,220));
				gg->FrameRoundCornerRect(rcRet,2,RGB(0,0,0),RGB(0,0,0));

				if (styleRet==CBehaviorGraphPad::AlwaysTrue)
					gg->DrawText("True",rcRet,DT_CENTER);
				else
					gg->DrawText("False",rcRet,DT_CENTER);
				break;
			}
		}
	}

	//Comment
	if (comment[0])
	{
		i_math::recti rcRet=_rc;
		rcRet.Top()=_rc.Bottom()+2;

		i_math::size2di sz=gg->MessureText(comment,rcRet.getWidth());

		rcRet.Bottom()=rcRet.Top()+sz.h;
		rcRet.Right()=rcRet.Left()+sz.w+2;

		gg->FillSolidRect(rcRet,RGB(0,0,0),128);
		gg->DrawText(comment,rcRet,DT_LEFT,FALSE,RGB(255,255,255));

	}

}


BOOL CGraphBgPad::_ItemHitTest(int x,int y,GraphPadItem*item,int part,GraphPadHit &hit)
{
	if (item->rcFocus.isPointInside(x,y))
	{
		hit.part=(GraphPadHit::Part)part;
		hit.item=item;
		return TRUE;
	}
	return FALSE;
}
 
BOOL CGraphBgPad::_ItemsHitTest(int x,int y,GraphPadItem**items,DWORD c,int part,GraphPadHit &hit)
{
	for (int i=0;i<c;i++)
	{
		if (_ItemHitTest(x,y,items[i],part,hit))
			return TRUE;
	}

	return FALSE;
}

GraphPadItem *CGraphBgPad::FindItem(const char *name)
{
	int idx;
	PVEC_FIND_BY_ELEMENT(_outs,name,name,idx);
	if (idx!=-1)
		return _outs[idx];
	PVEC_FIND_BY_ELEMENT(_ins,name,name,idx);
	if (idx!=-1)
		return _ins[idx];

	PVEC_FIND_BY_ELEMENT(_couts,name,name,idx);
	if (idx!=-1)
		return _couts[idx];
	PVEC_FIND_BY_ELEMENT(_cins,name,name,idx);
	if (idx!=-1)
		return _cins[idx];

	return NULL;
}



BOOL CGraphBgPad::HitTest(int x,int y,GraphPadHit &hit)
{

	if (_ItemsHitTest(x,y,_cins.data(),_cins.size(),GraphPadHit::CIn,hit))
	{
		hit.id=_id;
		return TRUE;
	}

	if (!_rc.isPointInside(x,y))
		return FALSE;

	hit.id=_id;

	if (_ItemsHitTest(x,y,_outs.data(),_outs.size(),GraphPadHit::Out,hit))
		return TRUE;
	if (_ItemsHitTest(x,y,_ins.data(),_ins.size(),GraphPadHit::In,hit))
		return TRUE;

	if (_ItemsHitTest(x,y,_couts.data(),_couts.size(),GraphPadHit::COut,hit))
		return TRUE;

	hit.part=GraphPadHit::Blank;
	return TRUE;
}




CGraphPad* CGraphBgPads::_LoadPad(CLinkPad *pad)
{
	CGraphBgPad*p=Class_New2(CGraphBgPad);

	FillDescAssist_GuiLib assist;
	assist.SetBehaviorGraphPads((CBehaviorGraphPads*)GetPads());

	_FillGraphPad(p,pad);

	if (((CBehaviorGraphPad *)pad)->_nmBase!=StringID_Invalid)
		p->_bBase=TRUE;

	if (((CBehaviorGraphPad *)pad)->_bOverriden!=0)
		p->_bOverriden=TRUE;

	BOOL bState=IsStatePad((CBehaviorGraphPad *)pad);
	BOOL bRelay=IsRelayPad((CBehaviorGraphPad *)pad);

	std::string title;
	if ((!p->_bFolder)||(p->_bCurFolder))
	{
		if (bState||bRelay)
			((CBehaviorGraphPad *)pad)->FillDesc(title,&assist);
		else
			title=title+"["+pad->GetShowName()+"]";
	}
	else
	{
		title=pad->GetFolderName();
		if (title=="")
		{
			if (bState||bRelay)
				((CBehaviorGraphPad *)pad)->FillDesc(title,&assist);
			else
				title=title+"["+pad->GetShowName()+"]";
		}
	}

	p->_title.SetShow(title.c_str());
	p->_title.graph=this;
	p->_title.id=p->_id;


	DWORD cStubs=pad->GetStubCount();
	std::string nm;
	for (int j=0;j<cStubs;j++)
	{
		PadStub stb=pad->GetStub(j);

		if (p->_bFolder&&(!p->_bCurFolder))
		{
			if ((stb.type==PadStub_Out)||(stb.type==PadStub_COut))
				continue;
		}

		GraphPadItem *item=Class_New2(GraphBgPadItem);
		item->graph=this;
		item->id=p->_id;
		nm=stb.name;
		if (nm.c_str()[0]=='!')
		{
			if (nm.c_str()[1]=='!')
			{
				nm=&nm.c_str()[2];
				nm=StrLib_GetStr((StringID)IntFromString(nm.c_str()));
			}
			if ((nm.c_str()[1]=='s')&&(nm.c_str()[2]=='!'))
			{
				nm=&nm.c_str()[3];
				extern const char *GetRecordName(const char *nameRecords,RecordID id);
				nm=GetRecordName("skills",(RecordID)IntFromString(nm.c_str()));
			}
		}
		item->SetShow(nm.c_str());
		item->SetName(stb.name);

		item->bConnectable=TRUE;
		item->iStub=j;

		if (stb.type==PadStub_Out)
			p->_outs.push_back(item);
		if (stb.type==PadStub_In)
			p->_ins.push_back(item);
		if (stb.type==PadStub_CIn)
			p->_cins.push_back(item);
		if (stb.type==PadStub_COut)
			p->_couts.push_back(item);
// 		if ((stb.type==PadStub_CIn)||(stb.type==PadStub_COut))
// 			item->SetShow("");
	}

	if(!(bState||bRelay))
	{
		((CBehaviorGraphPad *)pad)->FillDesc(p->_desc,&assist);
	}

	return p;
}


void CGraphBgPads::_DrawDynConnect(GraphicsGraph *gg,ConnectDyn &conn)
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
				pt1=conn.items[i]->GetConnectSpot(FALSE);
				gg->DrawConnectH(pt1,pt2,RGB(255,128,64),0);
				break;
			case ConnectDyn::ConnectingC:
				pt1=conn.items[i]->GetConnectSpot(FALSE);
				gg->DrawConnectV(pt2,pt1,RGB(64,128,255),0);
				break;

			case ConnectDyn::Connected:
			case ConnectDyn::Void:
				pt1=conn.items[i]->GetConnectSpot(TRUE);
				gg->DrawConnectH(pt2,pt1,RGB(255,128,64),0);
				break;

			case ConnectDyn::ConnectedC:
				pt1=conn.items[i]->GetConnectSpot(TRUE);
				gg->DrawConnectV(pt1,pt2,RGB(64,128,255),0);
				break;

			}
		}
	}
}


void CGraphBgPads::_DrawPermConnect(GraphicsGraph *gg)
{
	for (int i=0;i<_connects.size();i++)
	{
		_Connect *p=&_connects[i];
		if (p->item[0]&&p->item[1])
		{
			PadStubType tp=p->item[0]->GetStubType();
			DWORD col=RGB(0,0,0);
			if ((tp==PadStub_CIn)||(tp==PadStub_COut))
				gg->DrawConnectV(p->item[1]->GetConnectSpot(TRUE),
					p->item[0]->GetConnectSpot(FALSE),RGB(64,128,255),0);
			else
				gg->DrawConnectH(p->item[0]->GetConnectSpot(FALSE),
					p->item[1]->GetConnectSpot(TRUE),RGB(255,128,64),0);
		}
	}

}


