#include "stdh.h"
#include "GuiView_ValueSet.h"
#include "GuiData_ValueSet.h"
#include "graphicsgraph.h"
#include "Log/LogDump.h"
#include "stringparser/stringparser.h"

#include "valueset/valueset.h"

#include "RichGrid.h"
#include "RichGridValueSetItem.h"



CGuiView_ValueSet::CGuiView_ValueSet()
{
	_bYInverse=TRUE;
	_bReadOnly=FALSE;
}

CGuiView_ValueSet::~CGuiView_ValueSet()
{

}

void CGuiView_ValueSet::_DrawGrid(GraphicsGraph *gg)
{
	i_math::recti rc0;
	_wnd->GetClientRect((CRect*)&rc0);

	float marksX[100],marksY[100];

	float steps[]={0.001f,0.002f,0.005f,0.01f,0.02f,0.05f,0.1f,0.2f,0.5f,1.0f,2.0f,5.0f,10.0f,20.0f,50.0f,100.0f,200.0f,500.0f,1000.0f,2000.0f,5000.0f,10000.0f,0.0f};

	int nMarksX=_rulerX.BuildMarks(marksX,steps,60);
	int nMarksY=_rulerY.BuildMarks(marksY,steps,30);

	//格线
	for (int i=0;i<nMarksX;i++)
	{
		int x=_rulerX.ToRS(marksX[i]);
		_gg->DrawLine(ColorAlpha(0x5f5f5f,0xff),1.0f,i_math::pos2di(x,rc0.Top()),i_math::pos2di(x,rc0.Bottom()));
		int y=_rulerY.ToRS(marksY[i]);
		_gg->DrawLine(ColorAlpha(0x5f5f5f,0xff),1.0f,i_math::pos2di(rc0.Left(),y),i_math::pos2di(rc0.Right(),y));
	}

	if (TRUE)
	{
		int x=_rulerX.ToRS(0.0f);
		_gg->DrawLine(ColorAlpha(0x0,0xff),2.0f,i_math::pos2di(x,rc0.Top()),i_math::pos2di(x,rc0.Bottom()));
		int y=_rulerY.ToRS(0);
		_gg->DrawLine(ColorAlpha(0x0,0xff),2.0f,i_math::pos2di(rc0.Left(),y),i_math::pos2di(rc0.Right(),y));
	}
}



void CGuiView_ValueSet::_DrawRuler(GraphicsGraph *gg)
{
	i_math::recti rc0;
	_wnd->GetClientRect((CRect*)&rc0);

	float marksX[100],marksY[100];

	float steps[]={0.001f,0.002f,0.005f,0.01f,0.02f,0.05f,0.1f,0.2f,0.5f,1.0f,2.0f,5.0f,10.0f,20.0f,50.0f,100.0f,200.0f,500.0f,1000.0f,2000.0f,5000.0f,10000.0f,0.0f};

	int nMarksX=_rulerX.BuildMarks(marksX,steps,60);
	int nMarksY=_rulerY.BuildMarks(marksY,steps,30);

	//标尺
	if (TRUE)
	{
		i_math::recti rc,rcRuler;
		rc=rc0;
		rc.cutout(3,RULERX_THICK,rcRuler);

		_gg->GradientV(rcRuler,0xdfdfdf,0x9f9f9f);

		std::string s;
		for (int i=0;i<nMarksX;i++)
		{
			int x=_rulerX.ToRS(marksX[i]);
			int y=rcRuler.Bottom()-15;

			_gg->DrawLine(0xff000000,1.0f,i_math::pos2di(x,rcRuler.Top()),i_math::pos2di(x,rcRuler.Top()+8));

			x++;
			i_math::recti rcTxt;
			rcTxt.set(x,y,x+100,y+100);
			FormatString(s,"%g",marksX[i]);
			_gg->DrawText(s.c_str(),rcTxt);
		}
		_gg->DrawLine(0xff000000,1.0f,i_math::pos2di(rcRuler.Left(),rcRuler.Top()),i_math::pos2di(rcRuler.Right(),rcRuler.Top()));
	}

	if (TRUE)
	{

		i_math::recti rcRuler,rc;
		rc=rc0;
		rc.cutout(0,RULERY_THICK,rcRuler);

		_gg->GradientH(rcRuler,0xdfdfdf,0x9f9f9f,0xff,0xff);

		std::string s;
		for (int i=0;i<nMarksY;i++)
		{
			int y=_rulerY.ToRS(marksY[i]);
			int x=rcRuler.Left();

			_gg->DrawLine(0xff000000,1.0f,i_math::pos2di(rcRuler.Right()-24,y),i_math::pos2di(rcRuler.Right(),y));

			y++;
			i_math::recti rcTxt;
			rcTxt.set(x,y,x+rcRuler.getWidth()+200,y+200);
			rcTxt.Left()+=3;
			rcTxt.Top()-=18;
			FormatString(s,"%g",marksY[i]);
			_gg->DrawText(s.c_str(),rcTxt,DT_LEFT);
		}
		_gg->DrawLine(0xff000000,1.0f,i_math::pos2di(rcRuler.Right(),rcRuler.Top()),i_math::pos2di(rcRuler.Right(),rcRuler.Bottom()));

	}
}

void CGuiView_ValueSet::_DrawKey(i_math::pos2di &pt,GraphicsGraph *gg,BOOL bHilight,Key_f *k)
{
	i_math::recti rc(pt.x,pt.y,pt.x,pt.y);
	rc.inflate(KEY_RADIUS,KEY_RADIUS,KEY_RADIUS,KEY_RADIUS);
	if (bHilight)
		gg->GradientV(rc,ColorAlpha(0x00ff00,0xff),ColorAlpha(0x00df00,0xff));
	else
		gg->GradientV(rc,ColorAlpha(0x3f7fff,0xff),ColorAlpha(0x2f6fef,0xff));
	gg->DrawFrameRect(rc,0xffffffff,1);

	if (bHilight)
	{
		std::string s;
		FormatString(s,"%.3f,%.3f",ANIMTICK_TO_SECOND(k->t),k->v);
		i_math::size2di sz=_gg->MessureText(s.c_str());

		i_math::recti rcTxt;
		rcTxt.set(0,0,sz.w,sz.h);
		rcTxt+=i_math::pos2di(rc.Right()+2,rc.Top());

		gg->FillSolidRect(rcTxt,0,100);
		gg->DrawText(s.c_str(),rcTxt,DT_LEFT,FALSE,0x00ff00);
	}

}

void CGuiView_ValueSet::_DrawLimitRect(i_math::rectf &rcLimit,GraphicsGraph *gg)
{
	i_math::recti rc;
	rc.Left()=_rulerX.ToRS(rcLimit.Left());
	rc.Right()=_rulerX.ToRS(rcLimit.Right());
	rc.Top()=_rulerY.ToRS(rcLimit.Top());
	rc.Bottom()=_rulerY.ToRS(rcLimit.Bottom());
	rc.repair();

	gg->DrawFrameRect(rc,ColorAlpha(0x0000ff,0xff),1,255,TRUE);
}


void CGuiView_ValueSet::_DrawFloat(ValueSet *vs,GraphicsGraph *gg,DWORD col,BOOL bHilight,int iSel)
{
	if (!vs)
		return;
	if (vs->GetKeyType()!=KT_Float)
		return;
	if ((!vs->_bVisible)&&(!bHilight))
		return;

	DWORD colLine;
	if (bHilight)
		colLine=col;
	else
		colLine=col;
	float widthLine=2.0f;
	if (bHilight)
		widthLine=2.0f;

	std::vector<i_math::pos2di> keys;
	keys.resize(vs->GetKeyCount());
	for (int i=0;i<vs->GetKeyCount();i++)
	{
		Key_f *k=(Key_f *)vs->GetKey(i);

		keys[i].x=_rulerX.ToRS(ANIMTICK_TO_SECOND(k->t));
		keys[i].y=_rulerY.ToRS(k->v);
	}

	for (int i=0;i<keys.size()-1;i++)
		gg->DrawLine(colLine,widthLine,keys[i],keys[i+1]);

	if (keys.size()>0)
	{
		i_math::pos2di ptEnd=keys.back();
		ptEnd.x=1000000;
		gg->DrawLine(colLine,widthLine,keys.back(),ptEnd);
	}

	if (bHilight)
	{
		for (int i=0;i<keys.size();i++)
		{
			if (i!=iSel)
				_DrawKey(keys[i],gg,FALSE,(Key_f*)vs->GetKey(i));
		}
		if (iSel!=-1)
			_DrawKey(keys[iSel],gg,TRUE,(Key_f*)vs->GetKey(iSel));

	}

}

void BuildColorKeysRect(CRuler &ruler,ValueSet *vs,i_math::recti &rcKeys,std::vector<i_math::recti>&rcs)
{
	rcs.clear();
	const int KeyWidth=12;
	if (!vs->IsEmpty())
	{
		Key_col *k;
		int xStart=ruler.ToRS(0.0f);
		int x;
		for (int i=0;i<vs->GetKeyCount();i++)
		{
			k=(Key_col *)vs->GetKey(i);

			x=ruler.ToRS(ANIMTICK_TO_SECOND(k->t));
			i_math::recti rcKey=rcKeys;

			rcKey.Left()=x-KeyWidth/2;
			rcKey.Right()=rcKey.Left()+KeyWidth;

			if (rcKey.Left()<xStart)
			{
				rcKey.Left()=xStart;
				rcKey.Right()=xStart+KeyWidth;
			}

			rcs.push_back(rcKey);
		}

	}
}

void CalcColorBandVer(int idx,int &top,int &bottom,BOOL bHilight)
{
	top=COLOR_BAND_GAP+idx*(COLOR_BAND_THICK+COLOR_BAND_GAP*2);
	bottom=top+COLOR_BAND_THICK;

	if (bHilight)
	{
		top-=COLOR_BAND_EDGE;
		bottom+=COLOR_BAND_EDGE;
	}
}

void DrawColorBand(std::vector<int>&keys,std::vector<DWORD>cols,i_math::recti &rc0,GraphicsGraph *gg)
{
	i_math::recti rc=rc0;
	gg->FillHatchRect(rc,0xffffff,0x0,22);//22,39

	if (keys.size()>0)
	{
		int i;
		for (i=0;i<keys.size()-1;i++)
		{
			rc.Left()=keys[i];
			rc.Right()=keys[i+1];

			gg->GradientH(rc,COLOR_SWAP_RB(cols[i]),COLOR_SWAP_RB(cols[i+1]),ColorAlpha_Alpha(cols[i]),ColorAlpha_Alpha(cols[i+1]));
		}

		rc.Left()=keys[i];
		rc.Right()=rc0.Right();
		if (rc.Right()<rc.Left())
			rc.Right()=rc.Left();
		gg->FillSolidRect(rc,COLOR_SWAP_RB(cols[i]),ColorAlpha_Alpha(cols[i]));
	}


}

void CGuiView_ValueSet::_DrawColor(ValueSet *vs,DWORD idx,GraphicsGraph *gg,BOOL bHilight,int iSel)
{
	if (!vs)
		return;
	if (vs->GetKeyType()!=KT_Color)
		return;

	int top,bottom;
	CalcColorBandVer(idx,top,bottom,bHilight);

	i_math::recti rc;
	rc.Top()=top;
	rc.Bottom()=bottom;

	std::vector<int> keys;
	std::vector<DWORD> cols;
	keys.resize(vs->GetKeyCount());
	cols.resize(vs->GetKeyCount());
	for (int i=0;i<vs->GetKeyCount();i++)
	{
		Key_col *k=(Key_col *)vs->GetKey(i);

		keys[i]=_rulerX.ToRS(ANIMTICK_TO_SECOND(k->t));
		cols[i]=k->color;
	}

	if (TRUE)
	{
		rc.Left()=_rulerX.ToRS(0);
		rc.Right()=gg->GetWidth();
		if (rc.Right()<rc.Left())
			rc.Right()=rc.Left();
		DrawColorBand(keys,cols,rc,gg);
	}

	if (keys.size()>0)
	{
		if (bHilight)
		{
			std::vector<i_math::recti> rcs;
			BuildColorKeysRect(_rulerX,vs,rc,rcs);

			for (int i=0;i<rcs.size();i++)
			{
				gg->DrawFrameRect(rcs[i],0xffffff,1,255,FALSE);
				i_math::recti rcT;
				rcT=rcs[i];
				rcT.inflate(-1,-1,-1,-1);
				gg->DrawFrameRect(rcT,0,1,255,FALSE);
			}

			if (iSel!=-1)
			{
				gg->DrawFrameRect(rcs[iSel],0x00ff00,1,255,FALSE);

				if (TRUE)
				{
					std::string s;
					FormatString(s,"%.4f",ANIMTICK_TO_SECOND(vs->GetKey(iSel)->t));
					i_math::size2di sz=_gg->MessureText(s.c_str());

					i_math::recti rcTxt;
					rcTxt.set(0,0,sz.w,sz.h);
					rcTxt+=i_math::pos2di(rcs[iSel].Right()+2,rcs[iSel].Top()+2);

					gg->FillSolidRect(rcTxt,0,100);
					gg->DrawText(s.c_str(),rcTxt,DT_LEFT,FALSE,0x00ff00);
				}
			}

		}
	}

}





void CGuiView_ValueSet::_DrawValueSets(GraphicsGraph *gg)
{
	CGuiData_ValueSet *data=(CGuiData_ValueSet*)FindData("ValueSet");

	CRichGrid *grid=data->GetGrid();
	if (!grid)
		return;

	ValueSetGroup * grp=data->GetSelGroup();
	if (!grp)
		return;

	//Loop Range
	if (TRUE)
	{
		ValueSetEntry *entry=data->GetSelEntry();
		if (entry)
		{
			ValueSet *vs=entry->GetValueSet(NULL);
			int idxLoop=vs->GetLoopIndex();
			if ((idxLoop>=0)&&(idxLoop<vs->GetKeyCount()-1))
			{
				Key *k=vs->GetKey(idxLoop);
				int from=_rulerX.ToRS(ANIMTICK_TO_SECOND(k->t));
				k=vs->GetKey(vs->GetKeyCount()-1);
				int to=_rulerX.ToRS(ANIMTICK_TO_SECOND(k->t));

				i_math::recti rc;
				rc.set(from,0,to,gg->GetHeight());
				gg->FillSolidRect(rc,RGB(64,128,255),96);
			}
		}
	}

	std::vector<ValueSet *>colorbands;
	ValueSet *hilight=NULL;
	i_math::rectf rcLimit;
	DWORD colHilight;
	for (int i=0;i<grp->entries.size();i++)
	{
		ValueSetEntry *entry=&grp->entries[i];

		if (!entry->ref)
			continue;
		CRichGrid_ValueSetItem*item=(CRichGrid_ValueSetItem*)entry->ref->GetStuff();
		if (!item)
			continue;

		ValueSet *vs=item->GetBind();
		std::string path=grid->PathFromItem(item);

		if (vs->GetKeyType()==KT_Color)
		{
			if (vs->_bVisible||(path==data->_selentry))
				colorbands.push_back(vs);
		}

		if(path==data->_selentry)
		{
			hilight=vs;
			colHilight=item->GetLineCol();
			rcLimit=item->GetLimitRect();
		}
		else
			_DrawFloat(vs,gg,item->GetLineCol(),FALSE,-1);
	}

	if (hilight)
	{
		_DrawFloat(hilight,gg,colHilight,TRUE,data->GetSelKey());
//		_DrawLimitRect(rcLimit,gg);
	}

	//画color
	if (colorbands.size()>0)
	{
		i_math::recti rc;
		rc.Left()=4;
		rc.Right()=gg->GetWidth();
		rc.Bottom()=gg->GetHeight();
		rc.Top()=0;
		rc.Bottom()=rc.Top()+(COLOR_BAND_GAP*2+COLOR_BAND_THICK)*colorbands.size();
		rc.Top()+=2;

		gg->FillSolidRect(rc,0,64);
		gg->DrawFrameRect(rc,0xffffff,1);

		for (int i=0;i<colorbands.size();i++)
			_DrawColor(colorbands[i],i,gg,colorbands[i]==hilight,data->GetSelKey());

	}


}


void CGuiView_ValueSet::_OnDraw( GraphicsGraph *gg )
{	
	gg->ClearBg( ColorAlpha(0x6f6f6f,0xff));
	gg->SetSmoothingMode( SmoothingModeHighQuality );

	i_math::recti rc0;
	_wnd->GetClientRect((CRect*)&rc0);

	i_math::pos2df off;
	i_math::pos2df scale;
	gg->GetTranform(off,scale);
	gg->ResetTransform();

	_rulerX.SetLength(rc0.getWidth());
	_rulerX.SetOff(off.x);
	_rulerX.SetScale(scale.x);

	_rulerY.SetLength(rc0.getHeight());
	_rulerY.SetOff(off.y);
	_rulerY.SetScale(scale.y);


	_DrawGrid(gg);

	_DrawValueSets(gg);

	_DrawRuler(gg);


	gg->Transform(off,scale);

}


BOOL CGuiView_ValueSet::Respond(CtrlOp &co)
{
	if (_bReadOnly)
	{
		if ((co.op==CtrlOp::Op_Down)||(co.op==CtrlOp::Op_Up)||(co.op==CtrlOp::Op_DblClick)||(co.op==CtrlOp::Op_Click))
		{
			if ((co.vk==VK_LBUTTON)||(co.vk==VK_RBUTTON))
				return TRUE;
		}
		if (co.GetType()==OpType_Keyboard)
			return TRUE;
	}
	return CGuiView::Respond(co);
}
