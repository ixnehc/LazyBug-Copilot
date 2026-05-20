#pragma once

#include "GuiLib.h"

#include "GuiData.h"
#include "GuiEditor.h"

#include "GuiAgent_general.h"

#include "PinControls.h"

#include "ModBlockBack.h"

#include "WorldSystem/IEnvLight.h"


class IAssetClassLib;
class GuiLib_Api CGuiPanel_El:public CGuiPanel
{
public:
	CGuiPanel_El(CWnd* pParent = NULL);
	virtual const char *GetName()	{		return "envlight";	}
	
	BOOL Create(CWnd *pParent);
	
	virtual void OnEnterActivity();
	virtual void OnDetachView(CGeView *view,DWORD iLevel);

	void Reset();
	void UpdateSel(BOOL bForce = FALSE);

	void UpdateUI();
	void OnBackUp(CModBlockBack * mod);
	void OnRestore(CModBlockBack * mod);
protected:
	DECLARE_MESSAGE_MAP()

	void DoDataExchange(CDataExchange* pDX);
	virtual const char *_GetModMgrName()	{		return "world";	}
	void _OccupyActor();
	BOOL OnInitDialog();
	void BeginParameterChange(NMHDR * pNotifyStruct,LRESULT * pResult);
	void OnParameterChange(NMHDR * pNotifyStruct,LRESULT * pResult);
	void EndParameterChange(NMHDR * pNotifyStruct,LRESULT * pResult);
	void _OnChangeParameter();
	afx_msg void OnChangeState();
private:
	BOOL _bOnAdd,_bShowGrid,_bShowSamp;

	CPinboardEdit _editLen,_editWidth,_editHeight,_editGrid;
	CPinSpinner _spinLen,_spinWidth,_spinHeight,_spinGrid;

	CGuiAgent * _pAgentMod;
	HMapObj _hObjcurSel;
	i_math::pos2di _ptBlkEdit; //开始编辑时Obj所在的地块
	DWORD _ver;
};




