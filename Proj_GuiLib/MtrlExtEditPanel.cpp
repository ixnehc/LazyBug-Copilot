/********************************************************************
	created:	2006/10/31   17:20
	filename: 	e:\IxEngine\Proj_GuiLib\MtrlEditPanel.cpp
	author:		cxi
	
	purpose:	Mtrl main edit panel
*********************************************************************/
#include "stdh.h"

#include "RenderSystem/ITools.h"
#include "RenderSystem/IRenderPort.h"
#include "RenderSystem/IMtrl.h"
#include "RenderSystem/ITexture.h"
#include "RenderSystem/ISurface.h"
#include "RenderSystem/IRenderSystem.h"
#include "RenderSystem/IFont.h"

#include "FileSystem/IFileSystem.h"

#include ".\MtrlExtEditPanel.h"

#include "resdata/ResData.h"
#include "resdata/MtrlExtData.h"
#include "shaderlib/keywords.h"
#include "shaderlib/ScriptParser.h"
#include "shaderlib/ScriptProcesser.h"
#include "shaderlib/ShaderLib.h"
#include "GuiAgent_general.h"
#include "GuiEditor_res.h"

#include "Scintilla.h"

#include "WndBase.h"
#include "FileDialogBase.h"

#include "stringparser/stringparser.h"

#include "RenderSystem/IRenderSystem.h"
#include "RenderSystem/IUtilRS.h"

#include <assert.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////////
//Reps_MtrlExt
void Reps_MtrlExt::CleanAndDelete()
{
	Zero();
	ResEditPanelState::CleanAndDelete();
}

void Reps_MtrlExt::Copy(ResEditPanelState &src0)
{
	ResEditPanelState::Copy(src0);
}



//////////////////////////////////////////////////////////////////////////
//CMtrlExtEditor
void CMtrlExtEditor::_OnModified()
{
	if (_owner)
	{
		_owner->OnSrcModified();
	}
}

//////////////////////////////////////////////////////////////////////////
//CMteGrid

BEGIN_MESSAGE_MAP(CMteGrid,CRichGrid)
END_MESSAGE_MAP()

BOOL CMteGrid::Create(const RECT& rect, CWnd* pParentWnd, UINT nID, DWORD dwListStyle)
{
	if (!CRichGrid::Create(rect,pParentWnd,nID,dwListStyle))
		return FALSE;

	SetHelpHeight(40);

	return TRUE;
}

RGState &CMteGrid::_GetRGState()
{		
	return ((Reps_MtrlExt*)_state)->stateRG;	
}


void CMteGrid::OnBeginItemChange(CXTPPropertyGridItem *item)
{
	LockPaint();
	RecordState(_GetRGState());//record the grid state before any change occurs
	//	RefreshMod();

}

void CMteGrid::OnItemChange(CXTPPropertyGridItem *item)
{
	//	RefreshMod();
}

void CMteGrid::OnEndItemChange(CXTPPropertyGridItem *item)
{
	RefreshMod(FALSE);

	UnLockPaint();
}

void CMteGrid::EnableCtrl(BOOL bActive)
{
	if(bActive)
		SetReadOnly(FALSE);
	else
		SetReadOnly(TRUE);
}


void CMteGrid::Bind(ResEditPanelState *state,BOOL bUpdateCtrl)
{
	CResEditCtrl::Bind(state,bUpdateCtrl);

	if (bUpdateCtrl)
	{
		LockPaint();
		ResetContent();

		MtrlExtData *data=_GetMtrlExtData();
		if (!data)
		{
			UnLockPaint();
			return;
		}

		BeginInsert();
		InsertCategory("材质扩展","材质扩展");

		PushInsert();


		PopInsert();

		ExpandAll();

		RestoreState(_GetRGState());
		UnLockPaint();


	}
}


//////////////////////////////////////////////////////////////////////////
//CMtrlGrid_Mte
MtrlData *CMtrlGrid_Mte::_GetMtrlData()
{		
	return &((MtrlExtData*)_GetResData())->sample;	
}



//////////////////////////////////////////////////////////////////////////
//CMtrlExtEditPanel
CMtrlExtEditPanel::CMtrlExtEditPanel():
_sampleanchor(Res_Mesh,"MtrlExtAnchor_SampleMesh",FALSE)//need not undo/redo
{
	_editor=NULL;
	_editor2=NULL;
	_anchor.SetResType(Res_MtrlExt);
	_anchor.SetLabel("MtrlExtAnchor");

	memset(_lgts,0,sizeof(_lgts));

	_curSample=0;

	_bRepaired=FALSE;
	_bError=FALSE;

	_bShowError=FALSE;
	_bShowEdit2=FALSE;
}


void CMtrlExtEditPanel::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MTRLEXTANCHOR,_anchor);
	DDX_Control(pDX, IDC_MESHANCHOR_MTRLSAMPLE, _sampleanchor);
}

BEGIN_MESSAGE_MAP(CMtrlExtEditPanel, CResEditPanel)
	ON_WM_SIZE()
	ON_CBN_SELCHANGE(IDC_VIEWOPT_MESHCOMBO,OnSampleSelChange)
	ON_BN_CLICKED(IDC_MESHANCHOR_MTRLSAMPLE,OnSampleClick)
	ON_BN_CLICKED(IDC_APPLY,OnApply)
	ON_BN_CLICKED(IDC_EDITEP,OnEditEP)
	ON_BN_CLICKED(IDC_LOCATEERR,OnLocateErr)

END_MESSAGE_MAP()

void CMtrlExtEditPanel::_RecalcLayout()
{
	i_math::recti rc,rc2,rc3;
	GetClientRect((CRect*)&rc);

	rc.inflate(0,-32,0,0);

	rc.cutout(2,rc.getWidth()/4,rc2);
	rc2.inflate(-2,0,-2,0);
	::SetWindowPos(&_gridMtrl,rc2);

//	rc.cutout(0,rc.getWidth()/4,rc2);
//	rc2.inflate(-2,0,-2,0);
//	::SetWindowPos(&_gridMte,rc2);
	if (_gridMte.GetSafeHwnd())
		_gridMte.ShowWindow(SW_HIDE);

	rc.inflate(-2,0,-2,0);

	rc.cutout(3,26,rc2);
	_rcEdit=rc;
	rc.cutout(1,20,rc3);
	LOCATE_CONTROL(this,IDC_ERRSTR,rc3);
	_rcEdit2=rc;

	if (_bShowError)
	{
		::SetWindowPos(_editor,_rcEdit2);
		::SetWindowPos(_editor2,_rcEdit2);
	}
	else
	{
		::SetWindowPos(_editor,_rcEdit);
		::SetWindowPos(_editor2,_rcEdit);
	}

	rc=rc2;
	rc.inflate(0,-2,0,-2);
	rc.cutout(2,120,rc2);
	rc2.inflate(0,0,-20,0);
	LOCATE_CONTROL(this,IDC_LOCATEERR,rc2);
	rc.cutout(2,120,rc2);
	rc2.inflate(0,0,-20,0);
	LOCATE_CONTROL(this,IDC_APPLY,rc2);

	rc.cutout(0,120,rc2);
	rc2.inflate(0,0,0,0);
	LOCATE_CONTROL(this,IDC_EDITEP,rc2);

}



void SetDefaultScintillaWndFormat(CScintillaWnd &wnd)
{
// 	wnd.SetBackground(STYLE_DEFAULT,0);
// 	wnd.SetForeground(STYLE_DEFAULT,0xffffffff);
// 	wnd.SetBackground(STYLE_LINENUMBER,0);
// 
// 	wnd.SetLexer(SCLEX_CPP);
// 
// 	wnd.SetKeywords(0,SHADER_KEYWORDS);
// 	wnd.SetKeywords(1,SHADER_KEYWORDS2);
// 
// 	int size=16;
// 	const char *face="Comic Sans MS";
// 	wnd.SetStyle(SCE_C_DEFAULT, 0xffffff,0x000000,size,face);
// 	wnd.SetStyle(SCE_C_IDENTIFIER, 0xffffff,0x000000,size,face);	
// 
// 	wnd.SetStyle(SCE_C_COMMENT, 0xafafaf,0x000000,size,face);
// 	wnd.SendMessage(SCI_STYLESETCHANGEABLE,SCE_C_COMMENT,false);
// 
// 	wnd.SetStyle(SCE_C_COMMENTLINE, 0xafafaf,0x000000,size,face);
// 	wnd.SetStyle(SCE_C_COMMENTDOC, 0xafafaf,0x000000,size,face);
// 	wnd.SetStyle(SCE_C_NUMBER, 0x00ff00,0x000000,size,face);
// 	wnd.SetStyle(SCE_C_WORD, 0x00ffff,0x000000,size,face);
// 	wnd.SetStyle(SCE_C_STRING, 0x0000ff,0x000000,size,face);
// 	wnd.SetStyle(SCE_C_CHARACTER, 0x0000ff,0x000000,size,face);
// 	wnd.SetStyle(SCE_C_UUID, 0xff00ff,0x000000,size,face);
// 	wnd.SetStyle(SCE_C_PREPROCESSOR, 0x00FFFF,0x000000,size,face);
// 	wnd.SetStyle(SCE_C_OPERATOR, 0xffff00,0x000000,size,face);
// 	wnd.SetStyle(SCE_C_IDENTIFIER, 0xffffff,0x000000,size,face);
// 	wnd.SetStyle(SCE_C_STRINGEOL, 0x0000ff,0x000000,size,face);
// 	wnd.SetStyle(SCE_C_VERBATIM, 0xff00ff,0x000000,size,face);
// 	wnd.SetStyle(SCE_C_REGEX, 0xff00ff,0x000000,size,face);
// 	wnd.SetStyle(SCE_C_COMMENTLINEDOC, 0xff00ff,0x000000,size,face);
// 	wnd.SetStyle(SCE_C_WORD2, 0xff7f7f,0,size,face);
// 	wnd.SetStyle(SCE_C_COMMENTDOCKEYWORD, 0xff7f00,0x000000,size,face);
// 	wnd.SetStyle(SCE_C_COMMENTDOCKEYWORDERROR, 0xff00ff,0x000000,size,face);
// 	wnd.SetStyle(SCE_C_GLOBALCLASS, 0xff00ff,0x000000,size,face);//使用第3号keyword
// 
// 	wnd.SetCaretFore(0xffffff);
// 
// 	wnd.SetSelColor(0x0,0xafafaf);
}



#define ID_WNDSCINTILLA 10000
#define ID_GRID 10001
#define ID_MTRLGRID 10002
#define ID_MTEGRID 10003
#define ID_WNDSCINTILLA2 10004

#define BOOKMARK_INDICATOR 1

void LoadMtrlSampleMesh(std::vector<SampleMeshInfo>&samples)
{
	samples.push_back(SampleMeshInfo("_editor\\torus.msh","torus"));
	samples.push_back(SampleMeshInfo("_editor\\quad.msh","quad"));
	samples.push_back(SampleMeshInfo("_editor\\sphere.msh","sphere"));
}

BOOL CMtrlExtEditPanel::OnInitDialog()
{
	CResEditPanel::OnInitDialog();

	LoadMtrlSampleMesh(_samples);
	_curSample=0;


	HIDE_CONTROL(this,IDC_EDITOR);

	//文本编辑框

	_editor=new CMtrlExtEditor;
	_editor->Create(NULL, NULL, WS_CHILD|WS_VISIBLE,CRect(0,0,1,1), this,ID_WNDSCINTILLA,NULL);
	_editor->_owner=this;
	SetDefaultScintillaWndFormat(*_editor->GetScintillaWnd());
	_editor->GetScintillaWnd()->DefineBookmark(BOOKMARK_INDICATOR,RGB(0,0,0),RGB(255,255,0),4);//SC_MARK_ARROW

	_editor2=new CMtrlExtEditor;
	_editor2->Create(NULL, NULL, WS_CHILD|WS_VISIBLE,CRect(0,0,1,1), this,ID_WNDSCINTILLA2,NULL);
	SetDefaultScintillaWndFormat(*_editor2->GetScintillaWnd());
	_editor2->GetScintillaWnd()->SetReadOnly(TRUE);
	_editor2->GetScintillaWnd()->DefineBookmark(BOOKMARK_INDICATOR,RGB(0,0,0),RGB(255,255,0),4);//SC_MARK_ARROW

	//material grid
	if (TRUE)
	{
		CRect rc(0,0,1,1);
		_gridMtrl.Create(rc,this,ID_MTRLGRID);
		_gridMtrl.SetWindowText(_T("MtrlGrid"));
	}
	AddCtrl(static_cast<CResEditCtrl*>(&_gridMtrl));

	//material ext grid
	if (TRUE)
	{
		CRect rc(0,0,1,1);
		_gridMte.Create(rc,this,ID_MTEGRID);
		_gridMte.SetWindowText(_T("MtrlExtGrid"));
	}
	AddCtrl(static_cast<CResEditCtrl*>(&_gridMte));


	//the view options
	if (TRUE)
	{
		CComboBox *pCB=(CComboBox *)GetDlgItem(IDC_VIEWOPT_MESHCOMBO);
		assert(pCB);

		for (int i=0;i<_samples.size();i++)
			pCB->AddString(fromMBCS(_samples[i].showname.c_str()));

		pCB->AddString(_T("[User Specified Mesh]"));

		pCB->SetCurSel(_curSample);
		if (_curSample<_samples.size())
			_sampleanchor.EnableWindow(FALSE);
	}


	_RecalcLayout();

	return TRUE;  // 除非设置了控件的焦点，否则返回 TRUE
}

void CMtrlExtEditPanel::Init3d()
{
	for (int i=0;i<_samples.size();i++)
		_samples[i].mesh=(IMesh *)g_ssGuiLib.pRS->GetMeshMgr()->ObtainRes(_samples[i].path.c_str());

	_lgts[0]=g_ssGuiLib.pRS->CreateLight();
	_lgts[1]=g_ssGuiLib.pRS->CreateLight();
	_lgts[0]->SetDirLight(i_math::vector3df(1,-1,1).normalize(),ColorAlpha(0x2f2f2f,0xff),ColorAlpha(0xdfdfdf,0xff),ColorAlpha(0xffffff,0xff));
	_lgts[1]->SetDirLight(i_math::vector3df(0,0,-1),ColorAlpha(0xffffff,0xff),ColorAlpha(0xffffff,0xff),ColorAlpha(0xffffff,0xff));

}

void CMtrlExtEditPanel::Clear3d()
{
	for (int i=0;i<_samples.size();i++)
		SAFE_RELEASE(_samples[i].mesh);

	for (int i=0;i<ARRAY_SIZE(_lgts);i++)
		SAFE_RELEASE(_lgts[i]);
}

const char *CMtrlExtEditPanel::_GetTemplate()
{
	return 
"\r\n"
"\r\n"
"EffectParam\r\n"
"{\r\n"
"\r\n"
"\r\n"
"}\r\n"
"\r\n"
"\r\n"
"global\r\n"
"$\r\n"
"\r\n"
"$;\r\n"
"\r\n"
"\r\n"
"//一些可以重载的值:\r\n"
"//assign pf_amb_light $ $;\r\n"
"//assign pf_dif_mtrl $ $;\r\n"
"//assign pf_dif_map $ $;\r\n"
"//assign pf_specflag $ $;\r\n"
"//assign pf_spec_mtrl $ $;\r\n"
"//assign pf_spec_map $ $;\r\n"
"//assign pf_illumflag $ $;\r\n"
"//assign pf_illum_mtrl $ $;\r\n"
"//assign pf_illum_map $ $;\r\n"
"//assign pf_alpha_map $ $;\r\n"
"//assign pf_alpha_mtrl $ $;\r\n"

"//assign pf_alpha $ $;\r\n"
"//assign pf_lit $ $;\r\n"
"//assign pf_fogged $ $;\r\n"
"\r\n"
"\r\n"
"\r\n"
"vs_ver = vs_2_0;\r\n"
"ps_ver = ps_2_0;\r\n"
;

}


void CMtrlExtEditPanel::OnResDataChange(ResData *data0)
{
	_stateToMod->SetData(data0);

	MtrlExtData *data=(MtrlExtData *)data0;

	if (data->src.empty())
		data->src=_GetTemplate();

	::SetWindowPos(_editor,i_math::recti(0,0,1,1));

	_editor->SetText(data->src.c_str());

	_RecalcLayout();

	_bRepaired=TRUE;

	_bError=FALSE;
	_Compile(TRUE);
}

ResEditPanelState *CMtrlExtEditPanel::_NewState()
{
	return (ResEditPanelState *)new Reps_MtrlExt;
}


//Update the controls in the panel to reflect the state
BOOL CMtrlExtEditPanel::StateToControl(ResEditPanelState *state0)
{
	if (!CResEditPanel::StateToControl(state0))
		return FALSE;

	//用资源内的内容更新cache中的内容
	if (TRUE)
	{
		MtrlExtData *data=(MtrlExtData *)_stateToMod->resdata;
		MtrlData *dataMtrl=&data->sample;
		if (dataMtrl->lods.size()>0)
		{
			MtrlData::Lod*lod=&dataMtrl->lods[0];
			for (int i=0;i<lod->fps.size();i++)
			{
				int j;
				for (j=0;j<_fpsCache.size();j++)
				{
					if (lod->fps[i].CheckSameEP(_fpsCache[j]))
					{
						_fpsCache[j].ReplaceContent(lod->fps[i]);
						break;
					}
				}
				if (j>=_fpsCache.size())
					_fpsCache.push_back(lod->fps[i]);
			}
		}
	}

	return TRUE;
}

BOOL CMtrlExtEditPanel::StateToFile(ResEditPanelState *state)
{
	if (TRUE)
	{
		std::string path=GetModuleFolderPath(NULL);
		std::string s;
		CutTailSubPath(path,s);
#ifdef _DEBUG
		CutTailSubPath(path,s);
#endif
		path=path+"\\data2\\shader.dirty";
		IFileSystem *pFS=g_ssGuiLib.pFS;
		if (pFS)
		{
			if (!pFS->ExistFileAbs(path.c_str()))
			{
				IFile *fl=pFS->OpenFileAbs(path.c_str(),FileAccessMode_Write);
				if (fl)
					fl->Close();
			}
		}
	}

	return CResEditPanel::StateToFile(state);
}


BOOL BuildWarpMaps(IRenderPort *rp,IMtrl *mtrl0,int iLod,ILight *lgt,ITexture *&warp,ITexture *&depth)
{
	if (!(mtrl0->IsWarp(iLod)||mtrl0->IsWarpML(iLod)))
		return FALSE;

	IRenderSystem *pRS=rp->GetRS();
	i_math::recti rc;
	rp->GetRect(rc);
	warp=pRS->GetRTexMgr()->Create(rc.getWidth(),rc.getHeight(),D3DFMT_A8R8G8B8,1);
	depth=pRS->GetRTexMgr()->Create(rc.getWidth(),rc.getHeight(),D3DFMT_R32F,1);

	IMesh *mesh=(IMesh *)pRS->GetMeshMgr()->ObtainRes("_std\\empty.msh");
	IMtrl *mtrl=(IMtrl *)pRS->GetMtrlMgr()->ObtainRes("_std\\empty.mtl");

	SurfHandle surfs[2];
	surfs[0].Set(warp);
	surfs[1].Set(depth);
	rp->PushState();
	rp->SetRenderTarget(surfs,1);

	//颜色贴图初始化
	rp->ClearBuffer(ClearBuffer_All,0x3f3f3f);
	extern BOOL DrawGrid(IRenderPort *rp,DWORD d,DWORD gap);
	DrawGrid(rp,10,1);

	//深度值初始化
	rp->SetRenderTarget(&surfs[1],1);
	rp->FillColor(i_math::vector4df(1000,1000,1000,1000));

	rp->SetRenderTarget(surfs,2);

	IRenderer *rdr=	rp->ObtainRenderer();

	DrawMeshArg dmg;
	rdr->BindMesh(mesh,dmg);
	rdr->BindMats(NULL,0);
	rdr->BindMtrl(mtrl,0);
	rdr->BindLight(lgt);
	rdr->AddFeature(FC_gbuf_x2);

//	rdr->Render();

	rp->PopState();

	SAFE_RELEASE(mesh);
	SAFE_RELEASE(mtrl);

	if (warp)
	{
		DrawTextureArg arg;
		arg.SetForceOpaque();
		rp->DrawTexture(warp,arg);
	}

	return TRUE;
}

ShaderCode BuildMtrlShaderCode(IMtrl *mtrl,int iLod,IMesh *mesh,ILight *lgt)
{
	BOOL bWarp=mtrl->IsWarp(iLod)||mtrl->IsWarpML(iLod);
	ShaderCode sc;
	sc=mtrl->GetShaderCode(iLod);
	if (bWarp)
		sc.Add(FC_warp);

	if (TRUE)
	{
		DWORD nFC;
		FeatureCode *fc=lgt->GetFC(nFC);
		if (fc)
			sc.Add(*fc);
	}
	sc.Add(mesh->GetFeature());

	return sc;
}

void BindMtrlEP(IShader *shader,IMtrl *mtrl,int iLod,ILight *lgt,ITexture *warp,ITexture *depth)
{
	lgt->Bind(shader,0);
	mtrl->BindEP(shader,iLod);

	BOOL bWarp=mtrl->IsWarp(iLod)||mtrl->IsWarpML(iLod);
	if (bWarp)
	{
		shader->SetEP(EP_scrnmap,warp);
		shader->SetEP(EP_scrndepth,depth);
		ShaderState state;
		mtrl->GetShaderState(iLod,state);
		int blend=state.modeBlend;
		state.modeBlend=Blend_Opaque;
		shader->SetState(state);
		shader->SetEP(EP_warpblend,blend);
	}
	else
		mtrl->BindState(shader,iLod);
}

void CMtrlExtEditPanel::Draw(IRenderPort *rp)
{
	if (!_bRepaired)
		return;

	if (_bError)
	{
		DrawFontArg arg;
		arg.SetLocation(10,10);
		rp->DrawText("{F:1}{S:16}{C:255,0,0}{O:1}{OC:255,255,0}Error!",arg);
		return;
	}

	MtrlExtData *data=(MtrlExtData *)_stateToMod->resdata;

	IMesh *mesh=NULL;
	IMtrl *mtrl=NULL;

	if (_curSample<_samples.size())
	{
		mesh=_samples[_curSample].mesh;
		mesh->AddRef();
	}
	else
	{
		if (std::string("")!=_sampleanchor.GetRelativePath())
			mesh=(IMesh *)g_ssGuiLib.pRS->GetMeshMgr()->ObtainRes(_sampleanchor.GetRelativePath());
	}
	if (!SafeForceTouch(mesh))
		SAFE_RELEASE(mesh);

	if (!mesh)
		return;

	MtrlExtData *dataFake=NULL;
	if (data)
	{
		dataFake=(MtrlExtData *)data->Clone();

//		g_ssGuiLib.pUtilRS->RepairResData(dataFake);//修补一下

		assert(dataFake->sample.lods.size()>0);
		dataFake->sample.lods[0].mte=GetAnchor()->GetRelativePath();

		mtrl=g_ssGuiLib.pRS->GetMtrlMgr()->Create(&dataFake->sample,"");
		if (!SafeForceTouch(mtrl))
			SAFE_RELEASE(mtrl);
	}

	ITexture *warp=NULL,*depth=NULL;
	BuildWarpMaps(rp,mtrl,0,_lgts[0],warp,depth);

	if (mtrl&&mesh)
	{
		IRenderer *rdr=rp->ObtainRenderer();

		if (rdr)
		{
			ShaderCode sc=BuildMtrlShaderCode(mtrl,0,mesh,_lgts[0]);

			IShader *shader=rdr->BeginRaw(sc);
			if (shader)
			{
				BindMtrlEP(shader,mtrl,0,_lgts[0],warp,depth);

				DrawMeshArg dmg;
				mesh->Draw(shader,*i_math::matrix43f::identity(),dmg);

				rdr->EndRaw(shader);
			}
			else
			{
				_bError=TRUE;
				IShader *shader=(IShader *)(rp->GetRS()->GetShaderLibMgr()->ObtainShader(sc));
				if (shader)
				{
					shader->Touch();
					_sFX=shader->GetFX();
					_err=shader->GetFXErr();
					if (!_err.empty())
					{
						std::vector<std::string>errs;
						SplitStringBy("\n",_err,&errs);
						extern BOOL ParseHLSLErrLine(const char *err0,int &nLine);
						for (int i=0;i<errs.size();i++)
						{
							if (ParseHLSLErrLine(errs[i].c_str(),_iErrLine))
							{
								_err=errs[i];
								break;
							}
						}
					}
					else
						_iErrLine=-1;
				}
			}
		}
	}

	ResData_Delete(dataFake);


	SAFE_RELEASE(mtrl);
	SAFE_RELEASE(mesh);

	SAFE_RELEASE(warp);
	SAFE_RELEASE(depth);



}

void CMtrlExtEditPanel::OnSize(UINT nType, int cx, int cy)
{
	CResEditPanel::OnSize(nType, cx, cy);

	_RecalcLayout();
}

void CMtrlExtEditPanel::_Compile(BOOL bCheckErr)
{
	MtrlExtData *data=(MtrlExtData *)_stateToMod->resdata;

	//清除已有的feature
	if (!bCheckErr)
	{
		data->bCompiled=FALSE;
		data->feature.Clean();
	}

	std::vector<std::string>lines;

	CScintillaWnd *wnd=_editor->GetScintillaWnd();

	int nLine=wnd->GetLineCount();
	lines.resize(nLine);
	for (int i=0;i<nLine;i++)
	{
		lines[i]=wnd->GetLineText(i);
		//remove the remarks
		int n=lines[i].find("//",0);
		if (n!=-1)
			lines[i]=LEFT_STRING(lines[i],n);
	}

	if (g_pScriptParser->LoadRawScript(lines))
	{
		g_pScriptParser->BeginExecute();
		if (SCRIPT_EXECUTE_TOOMANYERROR!=g_pScriptParser->Execute())
		{
			g_pScriptParser->EndExecute();

			if (!bCheckErr)
			{
				g_pScriptProcesser->FetchFeature(&data->feature);
				data->bCompiled=TRUE;
			}
		}
		else
		{
			if (bCheckErr)
			{
				_bError=TRUE;
				_sFX="";
				_err="";
				_iErrLine=-1;
				if (g_pScriptParser->GetErrorCount()>0)
				{
					CErrorInfo *info=g_pScriptParser->GetError(0);
					_iErrLine=info->m_ErrorPos.m_iLine;
					_err=g_pScriptParser->GetErrorString(info->m_et);
				}
			}
		}
		g_pScriptParser->Clear();
	}

}


void CMtrlExtEditPanel::OnSrcModified()
{
	if (!_editor)
		return;

	MtrlExtData *data=(MtrlExtData *)_stateToMod->resdata;

	data->src=_editor->GetScintillaWnd()->GetText();

	_Compile(FALSE);

	_editor->GetScintillaWnd()->DeleteAllBookmark(BOOKMARK_INDICATOR);
	_editor2->GetScintillaWnd()->DeleteAllBookmark(BOOKMARK_INDICATOR);


	_bRepaired=FALSE;

	RefreshStateMod();
}

void CMtrlExtEditPanel::OnSelect()
{
	ClearAgent();
	_AddCameraController();
}

void CMtrlExtEditPanel::OnSampleSelChange()
{
	CComboBox *pCB=(CComboBox *)GetDlgItem(IDC_VIEWOPT_MESHCOMBO);

	_curSample=pCB->GetCurSel();
	_sampleanchor.EnableWindow(_curSample>=_samples.size());
}


void CMtrlExtEditPanel::OnSampleClick()
{
	const char *path=FD_BrowseResource(Res_Mesh,_sampleanchor.GetRelativePath());
	if (path[0])
		_sampleanchor.SetRelativePath(path);
}


void CMtrlExtEditPanel::OnEditEP()
{
	extern BOOL StrLibDlg_Browse(DWORD iCategory,StringID &id,const char *grp);
	StringID id;
	StrLibDlg_Browse(STRLIB_CATEGORY_DEFAULT,id,"EffectParam定义");
	RefreshStateMod(FALSE);
}


void CMtrlExtEditPanel::OnApply()
{
	if (_bRepaired)
		return;
	MtrlExtData *data=(MtrlExtData *)_stateToMod->resdata;

	_bShowError=FALSE;
	_bShowEdit2=FALSE;
	_bError=FALSE;//清除错误

	_Compile(TRUE);

	g_ssGuiLib.pUtilRS->RepairResData(data);
	_bRepaired=TRUE;
	//修补后,用cache中的内容更新资源中的内容
	if (TRUE)
	{
		MtrlData *dataMtrl=&data->sample;
		if (dataMtrl->lods.size()>0)
		{
			MtrlData::Lod *lod=&dataMtrl->lods[0];
			for (int j=0;j<_fpsCache.size();j++)
			for (int i=0;i<lod->fps.size();i++)
			{
				if (lod->fps[i].CheckSameEP(_fpsCache[j]))
				{
					lod->fps[i].ReplaceContent(_fpsCache[j]);
					break;
				}
			}
		}
	}

	RefreshStateMod();
}

void CMtrlExtEditPanel::OnLocateErr()
{
	if (!_bRepaired)
		return;

	if (!_bError)
		return;

	_bShowError=TRUE;
	SET_CONTROL_TEXT(this, IDC_ERRSTR, fromMBCS(_err.c_str()));

	if (_sFX=="")
	{
		_bShowEdit2=FALSE;
		_editor->GetScintillaWnd()->DeleteAllBookmark(BOOKMARK_INDICATOR);
		if (_iErrLine>=0)
		{
			_editor->GetScintillaWnd()->AddBookmark(_iErrLine,BOOKMARK_INDICATOR);
			_editor->GetScintillaWnd()->GotoLine(_iErrLine);
		}
		_editor->SetFocus();
	}
	else
	{
		_bShowEdit2=TRUE;
		_editor2->ShowWindow(SW_SHOW);
		_editor2->GetScintillaWnd()->SetReadOnly(FALSE);
		_editor2->SetText(_sFX.c_str());
		_editor2->GetScintillaWnd()->SetReadOnly(TRUE);
		_editor2->GetScintillaWnd()->DeleteAllBookmark(BOOKMARK_INDICATOR);
		if (_iErrLine>=0)
		{
			_editor2->GetScintillaWnd()->AddBookmark(_iErrLine,BOOKMARK_INDICATOR);
			_editor2->GetScintillaWnd()->GotoLine(_iErrLine);
		}
	}

}


void CMtrlExtEditPanel::UpdateUI()
{
	CResEditPanel::UpdateUI();
	if (!_bRepaired)
	{
		ENABLE_CONTROL(this,IDC_APPLY);
	}
	else
	{
		DISABLE_CONTROL(this,IDC_APPLY);
	}

	if (_bError&&_bRepaired)
	{
		ENABLE_CONTROL(this,IDC_LOCATEERR);
	}
	else
	{
		DISABLE_CONTROL(this,IDC_LOCATEERR);
	}

	if (_editor&&_editor2)
	{
		if (_bShowEdit2)
		{
			_editor->ShowWindow(SW_HIDE);
			_editor2->ShowWindow(SW_SHOW);
		}
		else
		{
			_editor2->ShowWindow(SW_HIDE);
			_editor->ShowWindow(SW_SHOW);
		}

		if (_bShowError)
		{
			SHOW_CONTROL(this,IDC_ERRSTR);
			::SetWindowPos(_editor,_rcEdit2);
			::SetWindowPos(_editor2,_rcEdit2);
		}
		else
		{
			HIDE_CONTROL(this,IDC_ERRSTR);
			::SetWindowPos(_editor,_rcEdit);
			::SetWindowPos(_editor2,_rcEdit);
		}
	}

	_UpdateEffectParamFormat(*_editor->GetScintillaWnd());
}

BOOL CMtrlExtEditPanel::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam==118)
		{
			OnApply();
			return TRUE;
		}
		if (_bShowEdit2)
		{
			if (pMsg->wParam==27)
			{
				_bShowEdit2=FALSE;
//				_bShowError=FALSE;
				return TRUE;
			}
		}
	}
	if (pMsg->message == WM_KEYDOWN )
	{
		BOOL bEditFocus=FALSE;
		CWnd *pWnd=GetFocus();
		if (pWnd)
		{
			if (pWnd->GetSafeHwnd()==_editor->GetScintillaWnd()->GetSafeHwnd())
				bEditFocus=TRUE;
		}
		if (!bEditFocus)
			return FALSE;
		if ((pMsg->wParam<'A')||(pMsg->wParam>'Z'))
			return FALSE;
	}

	return CDialog::PreTranslateMessage(pMsg);

}

void CMtrlExtEditPanel::_UpdateEffectParamFormat(CScintillaWnd &wnd)
{
	std::string eps;

	StringID grp=StrLib_Get()->FindGroup("EffectParam定义");
	DWORD c;
	StringID *buf=StrLib_Get()->EnumGroupSubs(grp,c);
	MtrlExtData::EPInfo info;
	for (int i=0;i<c;i++)
	{
		ParseMteEPInfo(info,buf[i]);
		eps+=info.name;
		eps+=" ";
	}

	if (TRUE)
	{
		MtrlExtData *data=(MtrlExtData *)_stateToMod->resdata;
		MtrlData::Lod *lod;
		if (data->sample.lods.size()>0)
		{
			lod=&data->sample.lods[0];
			CShaderLib *lib=g_ssGuiLib.pRS->GetShaderLibMgr()->GetShaderLib(lod->slib.c_str());
			if (lib)
			{
				SLTemplate *tmpl=lib->GetTemplate();
				if (tmpl)
				{
					for (int i=0;i<tmpl->vars.size();i++)
					{
						if (tmpl->vars[i].category!=SVC_EffectParam)
							continue;
						eps+=tmpl->vars[i].name;
						eps+=" ";
					}
				}
			}
		}
	}

	wnd.SetKeywords(3,eps.c_str());//SCE_C_GLOBALCLASS用第3号keyword


}

