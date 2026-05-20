#include "stdh.h"
#include "resource.h"
#include "WndBase.h"

#include "DebugPanel_Watch.h"

#include "GuiData.h"
#include "GuiData_FrameProxy.h"
#include "GuiData_Debugger.h"

//////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CDbgPanel_Watch,CGuiPanel)
	ON_WM_SIZE()
END_MESSAGE_MAP()



CDbgPanel_Watch::CDbgPanel_Watch(CWnd* pParent):CGuiPanel(IDD_DEBUG_LOCALS, pParent)
{
	_breakid=0;
}

BOOL CDbgPanel_Watch::Create(CWnd *pParent)	
{		
	return CDialog::Create(IDD_DEBUG_LOCALS,pParent);	
}


BOOL CDbgPanel_Watch::OnInitDialog()
{
	CGuiPanel::OnInitDialog();

	CRect rc;
	GET_CONTROL_RECT(this,IDC_LIST,rc);
	HIDE_CONTROL(this,IDC_LIST);
	if(FALSE==_grid.Create(rc,this,0))
		return FALSE;

	_RecalcLayout();

	_breakid=0;

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDbgPanel_Watch::_RecalcLayout()
{
	extern void SetWindowPos(CWnd *pWnd,i_math::recti &rc);

	i_math::recti rc;

	GetClientRect((LPRECT)&rc);

	SetWindowPos(&_grid,rc);

}



void CDbgPanel_Watch::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	_RecalcLayout();
}

void CDbgPanel_Watch::Reset()
{

}

void CDbgPanel_Watch::UpdateUI()
{
	return;
// 	GuiData_Debugger*dataDebugger=(GuiData_Debugger*)FindData("debugger");
// 	assert(dataDebugger);
// 
// 	CPrlFrameProxy *proxy=((GuiData_PrlFrameProxy*)FindData("prlframeproxy"))->proxy;
// 
// 	IDebugger *dbgr=dataDebugger->context->dbgr;
// 	BreakID bid=dbgr->GetBreakID();
// 	if (bid!=_breakid)
// 	{
// 		_grid.ResetContent();
// 		if (bid)
// 		{
// 			DWORD c=dbgr->GetLocalCount();
// 
// 			for (int i=0;i<c;i++)
// 			{
// 				DebugVarDesc*var=dbgr->GetLocal(i);
// 
// 				_grid.Add(*var);
// 			}
// 
// 		}
// 		_breakid=bid;
// 	}

}
