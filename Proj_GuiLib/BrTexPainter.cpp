
#include "stdh.h"
#include "BrTexPainter.h"
#include "resource.h"
#include "GuiData.h"
#include "WorldSystem/IWorldSystem.h"
#include "WndBase.h"
#include "TrrnBrushLibDlg.h"
#include "WorldSystem/IEntitySystem.h"
#include "FileSystem/IFileSystem.h"
#include "WorldSystem/IAssetSystem.h"
#include "WorldSystem/IAssetEventer.h"

#include "log/logfile.h"

IMPLEMENT_TOOL_CLASS(CBrTexPainter)

CBrTexPainter::CBrTexPainter(void)
	:_texctrl(&_imagelib)
{
	for(int i = 0;i<4;i++)
		_bChecked[i] = TRUE;
	_strLib =  "";
}

BOOL CBrTexPainter::DlgProc(UINT message,WPARAM wParam,LPARAM lParam,CGeActor * actor,int mode)
{	
	if(message==WM_NOTIFY){
		NMHDR * pNInfo = (NMHDR *)(lParam);
		if(pNInfo->code==PBN_ONCHANGE)
			_UpdateGuiData(actor);
	}
	return FALSE;
}

void CBrTexPainter::OnUpdateUI(CGeActor * actor)
{
	if (m_mode==0)
	{
		_RefreshSscState(actor);
		_CheckLibPathChange();
	}
}

void CBrTexPainter::_CheckLibPathChange()
{
	ITrrnBrushLib * pTrrnBrLib = NULL;
	GuiData_Trrn * data = (GuiData_Trrn *)GetActor()->FindData("terrain");
	if(data)
		pTrrnBrLib = data->GetBrushLib();

	std::string strLib;
	if(pTrrnBrLib)
		strLib = pTrrnBrLib->GetSscPath();

	if(strLib.compare(_strLib)!=0){
		_strLib = strLib;
		_RefreshTrrnLib(pTrrnBrLib);
	}
}

void CBrTexPainter::_UpdateGuiData(CGeActor * actor)
{
	float radius = _edit[0].GetFVal();
	float weight = _edit[1].GetFVal();
	float hardness = _edit[2].GetFVal();
	
	_arg.radius2 = radius;
	_arg.radius = (hardness/100.0f)*radius;
	_speed = weight;
}

void CBrTexPainter::_RefreshSscState(CGeActor * actor)
{
	ITrrnBrushLib * pBrLib = NULL;
	GuiData_Trrn * data =(GuiData_Trrn *)actor->FindData("terrain");
	GuiData_System * dataSys = (GuiData_System *)actor->FindData("system");
	if(data&&dataSys)
		pBrLib = data->GetBrushLib();

	if(pBrLib){
		_btnSsc.Bind(pBrLib->GetSscPath(),dataSys->ssc);
		const char * path = pBrLib->GetSscPath();
		IFileSystem *pFS = dataSys->pWS->GetFS();
		DWORD atr= pFS->GetFileAttr(path);

		HWND hwnd = ::GetDlgItem(m_hwnd,IDC_EDITLIB);
		if(atr==File_ReadOnly||
			atr==File_Miss)
			::EnableWindow(hwnd,FALSE);
		else
			::EnableWindow(hwnd,TRUE);
	}
	else{
		_btnSsc.Bind("",NULL);
	}
}

BOOL CBrTexPainter::OnCommand(DWORD ctrlID ,DWORD code,LPARAM lParam,CGeActor * actor)
{
	switch(ctrlID)
	{
	case IDC_COMBO_BRUSHLIST:
		{
			int idx = _brushList.GetCurSel();
			ITrrnBrushLib * pBrLib = NULL;
			GuiData_Trrn * data =(GuiData_Trrn *)actor->FindData("terrain");
			if(data)
				pBrLib = data->GetBrushLib();
			if(pBrLib&&idx>=0)
			{
				assert(idx<pBrLib->GetBrushCount());
				_idBr = pBrLib->GetBrushID(idx);
				_texctrl.SetTexSet(pBrLib->GetBrushTexSet(_idBr));
			}
			break;
		}
	case IDC_EDITLIB:
		{
			ITrrnBrushLib * pBrLib = NULL;

			GuiData_Trrn * data =(GuiData_Trrn *)actor->FindData("terrain");
			if(data)
				pBrLib = data->GetBrushLib();

			if(pBrLib){
				CTrrnBrushLibDlg dlg;
				dlg.SetPath(pBrLib->GetSscPath());
				dlg.SetWS(data->pES->GetWS());
				//set param
				if(IDOK==dlg.DoModal()){
					if(dlg.IsModified())
					{
						_OnReLoadBrushLib();
					}
				}
			}
			break;
		}
	case IDC_CHECK_FULLWEIGHT:
		{
			HWND hWnd = GetDlgItem(m_hwnd,IDC_CHECK_FULLWEIGHT);
			BOOL bFullWeight = (BST_CHECKED == ::SendMessage(hWnd,BM_GETCHECK,0,NULL));
			
			if(bFullWeight){
				_edit[1].SetValue(100.0f);
			}
			_SetStatus(bFullWeight);
			break;
		}
	}

	return FALSE;
}

void CBrTexPainter::_SetStatus(BOOL bChecked)
{
	if(bChecked)
	{
		_edit[1].SetValue(100.0f);
		_edit[1].Enable(FALSE);
	}
	else
		_edit[1].Enable(TRUE);
}

BOOL CBrTexPainter::BeginParam(CWnd * pParent,int mode,CGeActor * actor,int level,const char * nameView,int priority/* = AGENTPRIORITY_STANDARD*/)
{
	if(FALSE == CBrushUtil::BeginParam(pParent,mode,actor,level,nameView))
		return FALSE;

	GuiData_Trrn * data = (GuiData_Trrn *)actor->FindData("terrain");
	m_mode = mode;

	_purpose = TrrnSeedMapArg::Purpose_AddBr;

	if(mode==0)
	{
		_texctrl.ShowWindow(SW_SHOW);
		_btnSsc.ShowWindow(SW_SHOW);
		_brushList.ShowWindow(SW_SHOW);

		ITrrnBrushLib * pTrrnBrLib = NULL;
		GuiData_Trrn * data = (GuiData_Trrn *)actor->FindData("terrain");
		if(data)
			pTrrnBrLib = data->GetBrushLib();

		if(TRUE)
		{
			_imagelib.SetBrLib(pTrrnBrLib);

			GuiData_System *dataSys=(GuiData_System *)actor->FindData("system");
			if(dataSys)
				_imagelib.SetRS(dataSys->pWS->GetRS(),dataSys->pWS->GetUtilRS());
			_imagelib.SyncForAll();
		}

		_RefreshTrrnLib(pTrrnBrLib);

	}
	else
	{
		_brushList.ResetContent();
		_brushList.ShowWindow(SW_HIDE);
		_texctrl.ShowWindow(SW_HIDE);
		_btnSsc.ShowWindow(SW_HIDE);
	}

	
	_LoadParam(mode);

	return TRUE;
}

void CBrTexPainter::EndParam(int mode)
{
	if(m_mode==mode&&m_hwnd)
		_SaveParam(m_mode);
	CBrushUtil::EndParam(mode);
}

void CBrTexPainter::OnInitDlg(CGeActor * actor)
{
		
	_lnk_radius0 = _edit;

	_edit[0].SetLimits(0.0f,20.0f);
	_edit[1].SetLimits(0.0f,100.0f);
	_edit[2].SetLimits(1.0f,100.0f);
}

void CBrTexPainter::_RefreshTrrnLib(ITrrnBrushLib *pTrrnBrLib)
{	
	_brushList.ResetContent();

	if(pTrrnBrLib)
	{
		int nBrushs = pTrrnBrLib->GetBrushCount();
		for(int i = 0;i<nBrushs;i++)
		{
			BrushID idBr = pTrrnBrLib->GetBrushID(i);
			const char * name = pTrrnBrLib->GetBrushName(idBr);
			_brushList.AddString(fromMBCS(name));
		}

		_texctrl._lib->SetBrLib(pTrrnBrLib);
		_texctrl._lib->SyncForAll();

		if(nBrushs>0)
		{
			_brushList.SetCurSel(0);
			BrushID idBr = pTrrnBrLib->GetBrushID(0);
			_texctrl.SetTexSet(pTrrnBrLib->GetBrushTexSet(idBr));
			_idBr = idBr;
		}
	}
}

BOOL CBrTexPainter::_OnReLoadBrushLib(void)
{
	ITrrnBrushLib * pTrrnBrLib = NULL;
	GuiData_Trrn * data = (GuiData_Trrn *)GetActor()->FindData("terrain");
	GuiData_System * dataSystem = (GuiData_System *)GetActor()->FindData("system");
	
	_strLib = "";
	if(data){
		pTrrnBrLib = data->GetBrushLib();
		if(pTrrnBrLib){
			const char *pathCache = pTrrnBrLib->GetCachePath();
			IFileSystem * pFS = dataSystem->pWS->GetFS();
			if(pFS)
				pFS->RemoveFileAbs(pathCache);
			_strLib = pTrrnBrLib->GetSscPath();
		}
	}

	if(dataSystem){
		HkReloadTrrnBrushLib e;
		dataSystem->sevent->SendHook(e);
	}

	if(data)
		pTrrnBrLib = data->GetBrushLib();
	
	_RefreshTrrnLib(pTrrnBrLib);

	return TRUE;
}


BOOL CBrTexPainter::InitDlg(CWnd * pParent)
{ 
	if(FALSE == DefDialog(pParent,IDD_BRPANEL_TEXPAINTER))
		return FALSE;
    
	_brushList.SubclassDlgItem(IDC_COMBO_BRUSHLIST,&m_panel);
	if (TRUE)
	{
		CWnd * pThis = (&m_panel);
		RECT rc;
		GET_CONTROL_RECT(pThis,IDC_PAINT_TEXCTRL,rc);
		HIDE_CONTROL(pThis,IDC_PAINT_TEXCTRL);
		_texctrl.Create(rc,IDC_PAINT_TEXCTRL,pThis);
		_texctrl.ShowWindow(SW_HIDE);
	}

	if (TRUE)
	{
		RECT rc;
		GET_CONTROL_RECT(&m_panel,IDC_SSCBTN,rc);
		HIDE_CONTROL(&m_panel,IDC_SSCBTN);
		_btnSsc.Create(_T(""), WS_CHILD | WS_VISIBLE, rc, &m_panel, 5343);
		_btnSsc.SetFont(m_panel.GetFont(),TRUE);
		
		SscBtnCallback dlgbt;
		dlgbt.bind(this,&CBrTexPainter::_OnReLoadBrushLib);
		_btnSsc.BindNotifyLoad(dlgbt);

		_btnSsc.Bind("",NULL);
	}

	
#define CBrTexPainter_TouchInit(idx,ID0,ID1,ID2)\
	{																		\
	_edit[idx].SubclassDlgItem(ID0,&m_panel);								\
	_spinner[idx].SubclassDlgItem(ID1,&m_panel);							\
	_slider[idx].SubclassDlgItem(ID2,&m_panel);							\
	CPinboard * linkIn = (CPinboard *)(_edit + idx);				\
	_slider[idx].LinkTo(linkIn);											\
	_spinner[idx].LinkTo(linkIn);											\
	}

	CBrTexPainter_TouchInit(0,
		IDC_EDIT_RADIUS,IDC_SPIN_RADIUS,IDC_SLIDER_RADIUS);

	CBrTexPainter_TouchInit(1,
		IDC_EDIT_SPEED,IDC_SPIN_SPEED,IDC_SLIDER_SPEED);

	CBrTexPainter_TouchInit(2,
		IDC_EDIT_HARDNESS,IDC_SPIN_HARDNESS,IDC_SLIDER_HARDNESS);

#undef CBrTexPainter_TouchInit

	_texctrl.EnableEdit(FALSE);

	return TRUE;
}
void CBrTexPainter::RegisterAgent()
{
	AddAgent(_painterAgent);
	_painterAgent.SetTool(this);
}

void CBrTexPainter::_LoadParam(int mode)
{
	if(m_hwnd&&mode>=0&&mode<4)
	{
		_edit[0].SetValue(_params[mode].radius,TRUE);
		_edit[1].SetValue(_params[mode].speed,TRUE);
		_edit[2].SetValue(_params[mode].hardness,TRUE);

		HWND hWnd = GetDlgItem(m_hwnd,IDC_CHECK_FULLWEIGHT);
		DWORD stat = (_bChecked[mode])?BST_CHECKED:BST_UNCHECKED;
		SendMessage(hWnd,BM_SETCHECK,stat,NULL);
		
		_SetStatus(_bChecked[mode]);
	}

}
void CBrTexPainter::_SaveParam(int mode)
{
	if(mode>=0&&mode<4)
	{
		_params[mode].radius = _edit[0].GetFVal();
		_params[mode].speed = _edit[1].GetFVal();
		_params[mode].hardness = _edit[2].GetFVal();
		
		HWND hWnd = GetDlgItem(m_hwnd,IDC_CHECK_FULLWEIGHT);
		_bChecked[mode] = (BST_CHECKED == ::SendMessage(hWnd,BM_GETCHECK,0,NULL));
	}
}
void CBrTexPainter::RegisterMode()
{
	AddMode("绘制纹理",0);
	AddMode("绘制透明度",1);
	AddMode("清除透明度",2);
}




