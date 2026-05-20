/********************************************************************
	created:	2007/2/7   13:49
	filename: 	e:\IxEngine\Proj_GuiLib\CTBLBaseDlg.cpp
	author:		cxi
	
	purpose:	Base class for all Trrn Brush Lib Dialog
*********************************************************************/

#include "stdh.h"
#include "TBLBaseDlg.h"

#include "TrrnBrushLibDlg.h"

#include "WorldSystem/IWorldSystemDefines.h"

#include <assert.h>




ITrrnBrushLib * CTBLBaseDlg::GetBrLib()
{		
	return _owner->GetBrLib();
}

CTBLImageLib *CTBLBaseDlg::GetImageLib()
{
	return _owner->GetImageLib();
}


void CTBLBaseDlg::RefreshAll(BOOL bReset)
{
	assert(_owner);

	_owner->Refresh(bReset);
}


void CTBLBaseDlg::InitLevelCombo()
{
	CComboBox *pCB=(CComboBox *)GetDlgItem(IDC_LEVELCOMBO);
	pCB->ResetContent();
	CString s;
	for (int i=0;i<MAX_TRRN_BRUSHLEVEL;i++)
	{
		if (BRUSHLEVEL_IsNSLevel(i))
			s.Format(_T("Brush Level %02d (法线/高光)"), i + 1);
		else
			s.Format(_T("Brush Level %02d"),i+1);
		pCB->AddString(s);
	}
	pCB->SetCurSel(0);
}

void CTBLBaseDlg::UpdateDueToPSTSChange()
{
	_owner->UpdateDueToPSTSChange();
}

void CTBLBaseDlg::SetLevelCombo(int iLevel)
{
	CComboBox *pCB=(CComboBox *)GetDlgItem(IDC_LEVELCOMBO);
	pCB->SetCurSel(iLevel);

}
