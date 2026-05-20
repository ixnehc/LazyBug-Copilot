
#pragma once

class ITrrnBrushLib;
class CTrrnBrushLibDlg;

class CTBLImageLib;
// CTBLBrushDlg 对话框
class CTBLBaseDlg : public CDialog
{
// 构造
public:
	CTBLBaseDlg(UINT idDlg,CWnd *pParent):CDialog(idDlg,pParent)
	{
		_owner=NULL;
	}

	void SetOwner(CTrrnBrushLibDlg *owner)	{		_owner=owner;	}
	ITrrnBrushLib * GetBrLib();
	CTBLImageLib *GetImageLib();

	virtual void Refresh(BOOL bReset)	{	}//due to the brush lib modifying

	void RefreshAll(BOOL bReset);
	void InitLevelCombo();

	void UpdateDueToPSTSChange();

	virtual void OnOK()	{	}
	virtual void OnCancel(){	OnOK();}

	virtual void SetLevelCombo(int iLevel);


// 实现
protected:
	CTrrnBrushLibDlg *_owner;
};
