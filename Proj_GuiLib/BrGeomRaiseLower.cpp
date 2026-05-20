#include "stdh.h"

#include ".\brgeomraiselower.h"

#include "resource.h"

#include "GuiData.h"


IMPLEMENT_TOOL_CLASS(CBrGeomRaiseLower)

CBrGeomRaiseLower::CBrGeomRaiseLower(void)
{
}

CBrGeomRaiseLower::~CBrGeomRaiseLower(void)
{
}
BOOL CBrGeomRaiseLower::DlgProc(UINT message,WPARAM wParam,LPARAM lParam,CGeActor * actor,int mode)
{
	if(message!=WM_NOTIFY)
		return FALSE;
	
	NMHDR * pNInfo = (NMHDR *)(lParam);
	if(pNInfo->code!=PBN_ONCHANGE)
		return FALSE;
	
	//只处理 code为 PBN_ONCHANGE 的WM_NOTIFY 消息 
	NMHDR_PB * pInfo = (NMHDR_PB *)(pNInfo);
	CPinboard * pEdit = pInfo->pinboard;
	DWORD ctrID = (DWORD)pInfo->idFrom;
	switch(ctrID){
		case IDC_EDIT_INNERRADIUS:
			{
				_arg.radius = pEdit->GetFVal();
				break;
			}
		case IDC_EDIT_OUTTERRADIUS:
			{
				_arg.radius2 = pEdit->GetFVal();
				break;
			}
		case IDC_EDIT_HEIGHTTRRN:
			{
				_height = pEdit->GetFVal();
				break;
			}
		case IDC_EDIT_HEIGHTTRRN2:
			{
				_height2 = pEdit->GetFVal();
				break;
			}
		case IDC_EDIT_PAINTSPEED:
			{
				_speed = pEdit->GetFVal();
				break;
			}
		case IDC_EDIT_HARDNESS:
			{
				_hardness = pEdit->GetFVal();
				break;
			}
		default:
			break;
	}

	return FALSE;
}
BOOL CBrGeomRaiseLower::OnCommand(DWORD ctrlID ,DWORD code,LPARAM lParam,CGeActor * actor)
{
	if(ctrlID==IDC_CHECK_PRECISION)
	{
		GuiData_Trrn * data = (GuiData_Trrn *)actor->FindData("terrain");
		if(data)
		{
			HWND hWnd = GetDlgItem(m_hwnd,IDC_CHECK_PRECISION);
			_bAccurate = (BST_CHECKED == ::SendMessage(hWnd,BM_GETCHECK,0,NULL));
		}
	}
	else if(ctrlID==IDC_CHECK_HEIGHTSET)
	{
		BOOL bChecked = _check_height.GetCheck();
		_edit_height.Enable(bChecked);
	}
	else if(ctrlID==IDC_CHECK_HEIGHTSET2)
	{
		BOOL bChecked = _check_height2.GetCheck();
		_edit_height2.Enable(bChecked);
	}

	return TRUE;	
}

BOOL CBrGeomRaiseLower::BeginParam(CWnd * pParent,int mode,CGeActor * actor,int level,const char * nameView,int priority/* = AGENTPRIORITY_STANDARD*/)
{
	if(FALSE == CBrushUtil::BeginParam(pParent,mode,actor,level,nameView))
		return FALSE;
	
	m_mode = mode;
	
	if(mode==2||mode==3||mode==4||mode==5)
	{
		_edit_hardness.Enable(FALSE);
	}
	if(mode==3)
	{
		_edit_ir.Enable(FALSE);
	}
	if(mode==4||mode==5)
	{
		_edit_speed.Enable(FALSE);
		_edit_or.Enable(FALSE);
	}

	if(mode==2)
	{
		_check_height.EnableWindow(TRUE);
		if(_check_height.GetCheck())
			_edit_height.Enable(TRUE);
		_check_height2.EnableWindow(TRUE);
		if(_check_height2.GetCheck())
			_edit_height2.Enable(TRUE);
	}
	else
	{
		_edit_height.Enable(FALSE);
		_check_height.EnableWindow(FALSE);
		_edit_height2.Enable(FALSE);
		_check_height2.EnableWindow(FALSE);
	}
	
	if(mode<4)
		_purpose = TrrnSeedMapArg::Purpose_AddHt;
	else if(mode == 4)
		_purpose = TrrnSeedMapArg::Purpose_AddHole;
	else if(mode == 5)
		_purpose = TrrnSeedMapArg::Purpose_RemoveHole;

	if(mode>=0){
	  LoadParam(mode);
		
	  switch(mode){
		  case 3: //smooth
			  _edit_ir.SetValue(0);
			  break;
		  case 4:	//hole
		  case 5:
			  _edit_or.SetValue(20.0f);
			  break;
		  default :break;
	  }
	}
	return TRUE;
}
void CBrGeomRaiseLower::EndParam(int mode)
{
	if(m_mode==2||m_mode==3||m_mode==4||m_mode==5)
	{
		_edit_hardness.Enable(TRUE);
	}
	if(m_mode==3)
	{
		_edit_ir.Enable(TRUE);
	}
	if(m_mode==4||m_mode==5)
	{
		_edit_speed.Enable(TRUE);
		_edit_or.Enable(TRUE);
	}
	
	if(m_mode>=0&&m_hwnd)
		SaveParam(m_mode);

	CBrushUtil::EndParam(mode);
}
void CBrGeomRaiseLower::SaveParam(int i)
{
	_params[i].hardness = _edit_hardness.GetFVal();
	_params[i].speed = _edit_speed.GetFVal();
	_params[i].radius = _edit_ir.GetFVal();
	_params[i].radius2 = _edit_or.GetFVal();
	_params[i].height = _edit_height.GetFVal();
	_params[i].height2 = _edit_height2.GetFVal();
}
void CBrGeomRaiseLower::LoadParam(int i)
{
	_edit_hardness.SetValue(_params[i].hardness,TRUE);
	_edit_speed.SetValue(_params[i].speed,TRUE);

	_edit_ir.SetValue(_params[i].radius,TRUE);
	_edit_or.SetValue(_params[i].radius2,TRUE);

	_edit_height.SetValue(_params[i].height,TRUE);
	_edit_height2.SetValue(_params[i].height2,TRUE);
}
BOOL CBrGeomRaiseLower::InitDlg(CWnd * pParent)
{
	if(FALSE == DefDialog(pParent,IDD_BRPANEL_RAISELOWER))
		return FALSE;

#define CBrGeomRaiseLower_TouchInit(editor,spinner,slider,ID0,ID1,ID2)\
	{																	\
		editor.SubclassDlgItem(ID0,&m_panel);							\
		spinner.SubclassDlgItem(ID1,&m_panel);							\
		slider.SubclassDlgItem(ID2,&m_panel);							\
		CPinboard * linkIn = (CPinboard *)(&editor);			\
		slider.LinkTo(linkIn);											\
		spinner.LinkTo(linkIn);											\
	}

		CBrGeomRaiseLower_TouchInit(_edit_ir,_spinner_ir,_slider_ir,
							IDC_EDIT_INNERRADIUS,IDC_SPIN_INNERRADIUS,IDC_SLIDER_INNERRADIUS);

		CBrGeomRaiseLower_TouchInit(_edit_or,_spinner_or,_slider_or,
							IDC_EDIT_OUTTERRADIUS,IDC_SPIN_OUTTERRADIUS,IDC_SLIDER_OUTTERRADIUS);

		CBrGeomRaiseLower_TouchInit(_edit_speed,_spinner_speed,_slider_speed,
							IDC_EDIT_PAINTSPEED,IDC_SPIN_PAINTSPEED,IDC_SLIDER_PAINTSPEED);
		
		CBrGeomRaiseLower_TouchInit(_edit_hardness,_spinner_hardness,_slider_hardness,
			IDC_EDIT_HARDNESS,IDC_SPIN_HARDNESS,IDC_SLIDER_HARDNESS);
#undef CBrGeomRaiseLower_TouchInit

		_edit_height.SubclassDlgItem(IDC_EDIT_HEIGHTTRRN,&m_panel);
		_check_height.SubclassDlgItem(IDC_CHECK_HEIGHTSET,&m_panel);
		_check_height.SetCheck(FALSE);
		_edit_height.Enable(FALSE);

		_edit_height2.SubclassDlgItem(IDC_EDIT_HEIGHTTRRN2,&m_panel);
		_check_height2.SubclassDlgItem(IDC_CHECK_HEIGHTSET2,&m_panel);
		_check_height2.SetCheck(FALSE);
		_edit_height2.Enable(FALSE);

	return TRUE;
}
void CBrGeomRaiseLower::OnInitDlg(CGeActor * actor)
{
	_edit_ir.SetLimits(0.0f,20.0f);
	_edit_or.SetLimits(0.0f,20.0f);
	_edit_speed.SetLimits(1.0f,100.0f);
	_edit_hardness.SetLimits(1.0f,100.0f);
	_edit_height.SetLimits(-20000.0f,20000.0f);
	_edit_height2.SetLimits(-20000.0f,20000.0f);

	_edit_ir.SetLimitMax(&_edit_or,false);
	_edit_or.SetLimitMin(&_edit_ir,true);

	GuiData_Trrn * data = (GuiData_Trrn *)actor->FindData("terrain");
	if(data)
	{
		_speed = _edit_speed.GetFVal();
		_hardness = _edit_hardness.GetFVal();
		_arg.radius = _edit_ir.GetFVal();
		_arg.radius2 = _edit_or.GetFVal();
		
		_lnk_radius0 = &_edit_ir;
		_lnk_radius1 = &_edit_or;
	}
}
void CBrGeomRaiseLower::RegisterAgent()
{
	AddAgent(_agentTerrainRLOp);
	_agentTerrainRLOp.SetTool(this);
}
void CBrGeomRaiseLower::RegisterMode()
{
	AddMode("升高",0);
	AddMode("降低",1);
	AddMode("平整",2);
	AddMode("光滑",3);
	AddMode("挖洞",4);	
	AddMode("补洞",5); 
}







