#pragma once

#include "GuiLib.h"


#include "GuiAgent_general.h"
#include "GuiEditor.h"

#include "WorldSystem/IWorldSystemDefines.h"
#include "WorldSystem/IAssetSystemDefines.h"
#include "ToolContainer.h"

#define INVALID_VHIT (i_math::vector3df(10000,10000,10000))

class CTBLImageLib;
class ITrrnMapEditor;

class GuiLib_Api CGuiPanel_Trrn:public CGuiPanel
{
public:
	CGuiPanel_Trrn(CWnd* pParent = NULL);
	~CGuiPanel_Trrn();

	virtual const char *GetName()	{		return "terrain";	}

	virtual void Reset(){}
	virtual void OnDetachView(CGeView *view,DWORD iLevel);

	virtual void OnEnterActivity();
	
	virtual void UpdateUI(){_tools.UpdateUI(this);}

	virtual BOOL Create(CWnd *pParent);

	ITrrnMapEditor *GetTrrnMapEditor();
	
	void UpdatePaintState();

protected:
	DECLARE_MESSAGE_MAP()

	virtual BOOL OnInitDialog();
	afx_msg void OnComBoChangeBrush();
	afx_msg void OnPaintStateChange();
	void DoDataExchange(CDataExchange* pDX);

protected:

	virtual const char *_GetModMgrName()	{		return "world";	}

	void _StartPaint();

	void _OccupyActor();

	void _AddBrushUtil();

private:

	CComboBox _comboBrushTypes;
	int m_selBrush;
	ToolContainer  _tools;
	BOOL _bPaint;
};









