#pragma once

#include "GuiLib.h"

#include "WorldSystem/IWater.h"

#include "GuiEditor.h"

#include "EditListBoxEx.h"

#include "PinControls.h"

#include "BrushLibLVCtrl.h"

class IWaterEditor;

class GuiLib_Api CGuiPanel_Water:public CGuiPanel
{
public:

	CGuiPanel_Water(CWnd *pParent = NULL);
	~CGuiPanel_Water(void);

	// override actor
	virtual const char *GetName(){ return "water";}
	virtual const char *_GetModMgrName()	{		return "world";	}
	virtual void UpdateUI();
	
	virtual void OnEnterActivity();

	void OnBrushSelChange(const BRUID & uid);

	// user interface
	BOOL Create(CWnd *pParent);
	virtual BOOL OnInitDialog();
	
	void DoDataExchange(CDataExchange* pDX);
	
protected:
	IWaterEditor * _GetEditor();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaintStateChange();
	afx_msg void OnPaint();
	afx_msg void OnClear();
	afx_msg void OnRenderLvl();
	afx_msg void OnRangeChange();
	afx_msg void OnDensityChange();

	void OnBrushSzChange(NMHDR * pNotifyStruct ,LRESULT * pResult);

	CBrushLibLVCtrl _libWnd;

	CPinboardEdit    _editSize;
	CPinSpinner _spinSize;
	CPinSlider  _slideSize;
	
	DWORD _verLib;
};









