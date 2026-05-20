/********************************************************************
	created:	2007/2/15   17:35
	filename: 	e:\IxEngine\Proj_GuiLib\WEditorPanel_Trrn.cpp
	author:		cxi
	
	purpose:	Terrain EditorPanel
*********************************************************************/
#include "stdh.h"

#include <vector>
#include <string>

#include "stringparser/stringparser.h"

#include "WEditorPanel_Trrn.h"


#include "RenderSystem/IRenderSystem.h"
#include "WorldSystem/IWorldSystem.h"


#include "TBLTexSetDlg.h"
#include "TBLPttnSetDlg.h"
#include "TBLImageLib.h"
#include ".\weditorpanel_trrn.h"

#include "WndBase.h"
#include "CommonCtrlBase.h"

#include "FileDialogBase.h"

#include "ximage.h"



//////////////////////////////////////////////////////////////////////////
//CWEA_PaintTrrnBrush
void CWEA_PaintTrrnBrush::OnEnable()
{
}

void CWEA_PaintTrrnBrush::OnDisable()
{
}



BOOL CWEA_PaintTrrnBrush::OnRButtonClick(int x,int y,DWORD flag)
{
	Enable(FALSE);
	_Redraw();
	return FALSE;
}


BOOL CWEA_PaintTrrnBrush::OnMouseMove(int x,int y,DWORD flag)
{
	_PrepareSeedMap(x,y);
	if (_bPaint)
	{
		if (!_seedmap.IsEmpty())
			_DoPaint();
	}
	_Redraw();

	return TRUE;
}


BOOL CWEA_PaintTrrnBrush::OnSetCursor(int x,int y,DWORD flag)
{
	_SetCursor(IDC_POINTER_COPY);
	return FALSE;
}

BOOL CWEA_PaintTrrnBrush::_PrepareSeedMap(int x,int y)
{
	_seedmap.Clear();

	ITrrnMapEditor *editor=GetParent()->GetTrrnMapEditor(); 
	if (!editor)
		return FALSE;

	i_math::vector3df vHit;
	if (TRUE)
	{
		IRenderPort *rp=GetGuiEditor()->GetRP();
		HitProbe probe;
		if (FALSE==rp->CalcHitProbe(x,y,probe))
			return FALSE;

		if (FALSE==editor->GetHitPos(probe,vHit))
			return FALSE;
	}

	TrrnSeedMapArg arg=GetParent()->GetTrrnSeedMapArg();
	arg.vCenter=vHit;
	arg.idBr=GetParent()->GetSelBrushID();
	if (FALSE==editor->CalcSeedMap(_seedmap,arg))
		return FALSE;

	return TRUE;
}



BOOL CWEA_PaintTrrnBrush::OnDraw()
{
	if (_seedmap.IsEmpty())
		return TRUE;
	IRenderPort *rp=GetGuiEditor()->GetRP();

	rp->Lines(&_seedmap.boundary2[0],_seedmap.boundary2.size()/2,ColorAlpha(0xffff00,0xff));
	rp->Lines(&_seedmap.boundary[0],_seedmap.boundary.size()/2,ColorAlpha(0x00ff00,0xff));

	std::vector<i_math::vector3df> points;
	for (int i=0;i<_seedmap.points.size();i++)
	{
		if (_seedmap.points[i].flag!=TrrnSeedMap::SeedPointF_None)
			points.push_back(_seedmap.points[i].v);
	}

	rp->Points(&points[0],points.size(),ColorAlpha(0x00ff00,0xff));

	std::string s;
	FormatString(s,"{C0,255,0}(%f,%f,%f)",_seedmap.vCenter.x,_seedmap.vCenter.y,_seedmap.vCenter.z);

	DrawFontArg arg;
	arg.SetLocation(10,10);
	rp->DrawText(s.c_str(),arg);

	return TRUE;
}


void CWEA_PaintTrrnBrush::OnKillFocus(OpType type)
{
	if (type==OpType_Mouse)
	{
		if (_bPaint)
		{
			ITrrnMapEditor *editor=GetParent()->GetTrrnMapEditor(); 
			editor->EndModify();
			_bPaint=FALSE;
		}
	}
}


BOOL CWEA_PaintTrrnBrush::OnLButtonDown(int x,int y,DWORD flag)
{
	ITrrnMapEditor *editor=GetParent()->GetTrrnMapEditor(); 
	if (!editor)
		return FALSE;

	OccupyFocus(OpType_Mouse);

	_PrepareSeedMap(x,y);

	if (!_seedmap.IsEmpty())
	{
		editor->BeginModify();
		_DoPaint();
		_Redraw();
		_bPaint=TRUE;
	}

	return FALSE;
}

BOOL CWEA_PaintTrrnBrush::OnLButtonUp(int x,int y,DWORD flag)
{
	DiscardFocus(OpType_Mouse);


	return FALSE;
}


void CWEA_PaintTrrnBrush::_DoPaint()
{
	ITrrnMapEditor *editor=GetParent()->GetTrrnMapEditor();
	switch(GetParent()->GetTrrnSeedMapArg().purpose)
	{
		case TrrnSeedMapArg::Purpose_AddBr:
			editor->AddBrush(_seedmap,GetParent()->GetSelBrushID());
			break;
		case TrrnSeedMapArg::Purpose_SetBaseBr:
			editor->AddBaseBrush(_seedmap,GetParent()->GetSelBrushID());
			break;
		case TrrnSeedMapArg::Purpose_AddHt:
			editor->AddHeight(_seedmap,10.0f,0.0f);
			break;
		case TrrnSeedMapArg::Purpose_AddHole:
			editor->ModHole(_seedmap,FALSE);
			break;
		case TrrnSeedMapArg::Purpose_RemoveHole:
			editor->ModHole(_seedmap,TRUE);
			break;
	}

}



//////////////////////////////////////////////////////////////////////////
//CEditorPanel_Trrn

class CPttnCtrl2:public CPttnCtrl
{
protected:
	virtual CTBLImageLib *_GetImageLib()	{	return ((CEditorPanel_Trrn*)GetParent())->GetImageLib();	}
};

class CTexCtrl2:public CTexCtrl
{
protected:
	virtual CTBLImageLib *_GetImageLib()	{	return ((CEditorPanel_Trrn*)GetParent())->GetImageLib();	}
};


BEGIN_MESSAGE_MAP(CEditorPanel_Trrn, CEditorPanel)
	ON_WM_DESTROY()
	ON_CBN_SELCHANGE(IDC_BRUSHCOMBO, OnCbnSelchangeBrushcombo)
	ON_BN_CLICKED(IDC_PAINTTRRNBTN, OnBnClickedPainttrrnbtn)
	ON_BN_CLICKED(IDC_PAINTBASETRRNBTN, OnBnClickedPaintbasetrrnbtn)
	ON_BN_CLICKED(IDC_PAINTHEIGHTMAPBTN, OnBnClickedPaintheightmapbtn)
	ON_BN_CLICKED(IDC_SAVETRRNMAP, OnBnClickedSavetrrnmap)
	ON_BN_CLICKED(IDC_LOADHEIGHTMAP, OnBnClickedLoadheightmap)
	ON_BN_CLICKED(IDC_PAINTHOLEBTN, OnBnClickedPaintholebtn)
	ON_BN_CLICKED(IDC_CLEARHOLEBTN, OnBnClickedClearholebtn)
END_MESSAGE_MAP()


CEditorPanel_Trrn::CEditorPanel_Trrn(CWnd* pParent):CEditorPanel(CEditorPanel_Trrn::IDD, pParent)
{
	_pttnctrl=NULL;
	_texctrl=NULL;
	_imagelib=NULL;

	_brlib=NULL;
	_map=NULL;
	_editor=NULL;


	_idSelBr=BRUSHID_INVALID;
}



BOOL CEditorPanel_Trrn::OnInitDialog()
{
	CEditorPanel::OnInitDialog();

	if (TRUE)
	{
		_pttnctrl=new CPttnCtrl2;
		RECT rc;
		GET_CONTROL_RECT(this,IDC_PTTNSLOT,rc);
		HIDE_CONTROL(this,IDC_PTTNSLOT);
		_pttnctrl->Create(rc,IDC_PTTNSLOT,this);
		_pttnctrl->EnableEdit(FALSE);
	}

	if (TRUE)
	{
		_texctrl=new CTexCtrl2;
		RECT rc;
		GET_CONTROL_RECT(this,IDC_TEXSLOT,rc);
		HIDE_CONTROL(this,IDC_TEXSLOT);
		_texctrl->Create(rc,IDC_TEXSLOT,this);
		_texctrl->EnableEdit(FALSE);
	}

	_imagelib=new CTBLImageLib;

	_arg.purpose=TrrnSeedMapArg::Purpose_None;
	_arg.radius=1.0f;
	_arg.radius2=5.0f;

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CEditorPanel_Trrn::OnDestroy()
{
	SAFE_DELETE(_pttnctrl);
	SAFE_DELETE(_texctrl);
	SAFE_DELETE(_imagelib);

	CEditorPanel::OnDestroy();

	// TODO: Add your message handler code here
}

void CEditorPanel_Trrn::OnInitAgent()
{
	DefineEditorAgent(CWEA_PaintTrrnBrush,"PaintTrrn","",0);
}


void CEditorPanel_Trrn::_ResetContent()
{
	CComboBox *pCB=(CComboBox *)GetDlgItem(IDC_BRUSHCOMBO);
	pCB->ResetContent();

	_map=NULL;
	_editor=NULL;

	_brlib=NULL;

	_imagelib->SetBrLib(NULL);
	_imagelib->SetUtilRS(NULL);
	_imagelib->Clear();

	_idSelBr=BRUSHID_INVALID;
}

void CEditorPanel_Trrn::SetEnv(EditorEnv&env0)
{
	WEditorEnv &env=(WEditorEnv &)env0;

	_ResetContent();

	if ((!env.trrnmap)||(!env.brlib)||(!env.mapfile))
		return;

	CComboBox *pCB=(CComboBox *)GetDlgItem(IDC_BRUSHCOMBO);
	pCB->ResetContent();

	_map=env.trrnmap;
	_editor=_map->GetEditor();

	_brlib=env.brlib;
	_imagelib->SetBrLib(env.brlib);
	_imagelib->SetUtilRS(env.pWS->GetUtilRS());
	_imagelib->SyncForAll();

	for (int i=0;i<_brlib->GetBrushCount();i++)
		pCB->AddString(_brlib->GetBrushName(_brlib->GetBrushID(i)));

	pCB->SetCurSel(0);
	_idSelBr=_brlib->GetBrushID(0);
}

void CEditorPanel_Trrn::OnUpdateUI()
{
	UINT idPaintBtn[]=		
	{			
		IDC_PAINTTRRNBTN,			
		IDC_PAINTBASETRRNBTN,
		IDC_PAINTHEIGHTMAPBTN,
		IDC_PAINTHOLEBTN,
		IDC_CLEARHOLEBTN,
	};

	if (_brlib)
	{
		int iSel=ComboBox_GetListSel((CComboBox *)GetDlgItem(IDC_BRUSHCOMBO));
		BrushID idBr=_brlib->GetBrushID(iSel);

		if (_pttnctrl)
			_pttnctrl->SetPttnSet(_brlib->GetBrushPttnSet(idBr));
		if (_texctrl)
			_texctrl->SetTexSet(_brlib->GetBrushTexSet(idBr));

		for (int i=0;i<ARRAY_SIZE(idPaintBtn);i++)
		{
			ENABLE_CONTROL(this,idPaintBtn[i]);
		}

		if (TRUE)//Update the paint button state
		{
			UINT idSel=0xffffffff;
			if (IsAgentEnable("PaintTrrn"))
			{
				switch(_arg.purpose)
				{
					case TrrnSeedMapArg::Purpose_AddBr:
						idSel=IDC_PAINTTRRNBTN;
						break;
					case TrrnSeedMapArg::Purpose_SetBaseBr:
						idSel=IDC_PAINTBASETRRNBTN;
						break;
					case TrrnSeedMapArg::Purpose_AddHt:
						idSel=IDC_PAINTHEIGHTMAPBTN;
						break;
					case TrrnSeedMapArg::Purpose_AddHole:
						idSel=IDC_PAINTHOLEBTN;
						break;
					case TrrnSeedMapArg::Purpose_RemoveHole:
						idSel=IDC_CLEARHOLEBTN;
						break;
				}
			}


			for (int i=0;i<ARRAY_SIZE(idPaintBtn);i++)
			{
				if (idSel==idPaintBtn[i])
				{
					CHECK_BUTTON(this,idPaintBtn[i]);
				}
				else
				{
					UNCHECK_BUTTON(this,idPaintBtn[i]);
				}
			}
		}

	}
	else
	{
		for (int i=0;i<ARRAY_SIZE(idPaintBtn);i++)
		{
			DISABLE_CONTROL(this,idPaintBtn[i]);
		}

	}
}

void CEditorPanel_Trrn::OnCbnSelchangeBrushcombo()
{
	if (_brlib)
	{
		CComboBox *pCB=(CComboBox *)GetDlgItem(IDC_BRUSHCOMBO);
		_idSelBr=_brlib->GetBrushID(pCB->GetCurSel());
	}
}

void CEditorPanel_Trrn::OnBnClickedPainttrrnbtn()
{
	if (!IS_BUTTON_CHECK(this,IDC_PAINTTRRNBTN))
		EnableAgent("PaintTrrn",FALSE);
	else
	{
		EnableAgent("PaintTrrn",TRUE);
		_arg.purpose=TrrnSeedMapArg::Purpose_AddBr;
	}
}

void CEditorPanel_Trrn::OnBnClickedPaintbasetrrnbtn()
{
	if (!IS_BUTTON_CHECK(this,IDC_PAINTBASETRRNBTN))
		EnableAgent("PaintTrrn",FALSE);
	else
	{
		EnableAgent("PaintTrrn",TRUE);
		_arg.purpose=TrrnSeedMapArg::Purpose_SetBaseBr;
	}
}

void CEditorPanel_Trrn::OnBnClickedPaintheightmapbtn()
{
	if (!IS_BUTTON_CHECK(this,IDC_PAINTHEIGHTMAPBTN))
		EnableAgent("PaintTrrn",FALSE);
	else
	{
		EnableAgent("PaintTrrn",TRUE);
		_arg.purpose=TrrnSeedMapArg::Purpose_AddHt;
	}
}

void CEditorPanel_Trrn::OnBnClickedPaintholebtn()
{
	if (!IS_BUTTON_CHECK(this,IDC_PAINTHOLEBTN))
		EnableAgent("PaintTrrn",FALSE);
	else
	{
		EnableAgent("PaintTrrn",TRUE);
		_arg.purpose=TrrnSeedMapArg::Purpose_AddHole;
	}
}

void CEditorPanel_Trrn::OnBnClickedClearholebtn()
{
	if (!IS_BUTTON_CHECK(this,IDC_CLEARHOLEBTN))
		EnableAgent("PaintTrrn",FALSE);
	else
	{
		EnableAgent("PaintTrrn",TRUE);
		_arg.purpose=TrrnSeedMapArg::Purpose_RemoveHole;
	}
}


void CEditorPanel_Trrn::OnBnClickedSavetrrnmap()
{
	_map->SaveModified();
}

void CEditorPanel_Trrn::OnBnClickedLoadheightmap()
{
	CxImage img;
	if (FALSE==FD_BrowseImage(TRUE,&img))
		return ;

	i_math::pos2di ptOrg;
	ptOrg.set(-256,-256);

	TrrnSeedMap seedmap;
	seedmap.points.resize(img.GetWidth()*img.GetHeight());

	DWORD w,h;
	w=img.GetWidth();
	h=img.GetHeight();
	TrrnSeedMap::SeedPoint *p=&seedmap.points[0];
	for (int i=0;i<w;i++)
	for (int j=0;j<h;j++)
	{
		p->x=i+ptOrg.x;
		p->y=j+ptOrg.y;
		
		p->wt=(float)(img.GetPixelColor(i,j,FALSE).rgbGreen)/255.0f;
		p++;
	}

	seedmap.lvl=2;

	_editor->AddHeight(seedmap,40.0f,0.0f);

	_map->SaveModified();
}

