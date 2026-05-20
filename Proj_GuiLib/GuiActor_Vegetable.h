#pragma once

#include "GuiLib.h"

#include "WorldSystem/IWater.h"

#include "GuiEditor.h"

#include "PinControls.h"

#include "WorldSystem/IWorldSystem.h"

#include "BrushLibTVCtrl.h"

#include "WorldSystem/ISpg.h"

class GuiLib_Api CGuiPanel_Vegetable:public CGuiPanel
{
public:

	CGuiPanel_Vegetable(CWnd *pParent = NULL);
	~CGuiPanel_Vegetable(void);

	// override actor
	virtual const char *GetName(){ return "vegetable";}
	virtual const char *_GetModMgrName()	{		return "world";	}
	virtual void UpdateUI();
	
	virtual void OnDetachView(CGeView *view,DWORD iLevel);
	virtual void OnEnterActivity();
	virtual void Reset();

	void UpdateState();
	// user interface
	BOOL Create(CWnd *pParent);
	virtual BOOL OnInitDialog();
	
	void DoDataExchange(CDataExchange* pDX);

protected:

	ISpgEditor * GetEditor();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaintState();
	afx_msg void OnRemoveState();
	
	void OnGrpParamChange(NMHDR * pNotifyStruct,LRESULT * pResult);
	void OnWndParamChange(NMHDR * pNotifyStruct,LRESULT * pResult);
	void OnDrawParamChange(NMHDR * pNotifyStruct,LRESULT * pResult);

	BOOL _bPaint;
	BOOL _bRemove;
	
	CPinboardEdit   _editRadius;
	CPinSlider _sliderRadius;

	CPinboardEdit   _editDensity;
	CPinSlider _sliderDensity;

	CPinboardEdit    _editScaleMin;
	CPinSpinner _spinScaleMin;
	CPinboardEdit    _editScaleMax;
	CPinSpinner _spinScaleMax;

	CPinboardEdit   _editWindStrength;
	CPinSlider _sliderWindStrength;
	CPinboardEdit   _editWindSpeed;
	CPinSlider _sliderWindSpeed;
	CPinboardEdit   _editWindLen;
	CPinSlider _sliderWindLen;
		
	//
	SpgWindCfgPtr _windPtr;

	CBrushLibTVCtrl _libWnd;
};









