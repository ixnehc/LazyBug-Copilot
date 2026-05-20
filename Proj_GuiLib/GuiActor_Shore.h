
#pragma once

#include "GuiLib.h"

#include "GuiEditor.h"

#include "WorldSystem/IShore.h"

#include "PinControls.h"

#include "BrushLibLVCtrl.h"

class CGuiAgent_ShorePoints;
class GuiLib_Api CGuiPanel_Shore :public CGuiPanel
{
public:
	CGuiPanel_Shore(CWnd * pParent = NULL);
	virtual ~CGuiPanel_Shore();
	virtual BOOL Create(CWnd *pParent);
	virtual const char *GetName(){ return "shore";}
	virtual const char *_GetModMgrName()	{		return "world";	}

	virtual void UpdateUI();
	virtual void OnEnterActivity();
	virtual void OnDetachView(CGeView *view,DWORD iLevel);
	void Reset(){}
	IShoreEditor * GetEditor();
	void OnBrChange();

	class CShoreCPGrid:public CLyObjGrid<CtrlPt_Shore>
	{
		virtual void OnItemChange(CXTPPropertyGridItem *item);
		virtual void OnEndItemChange(CXTPPropertyGridItem *item);
		void _CommitChange();			
	};
	class CShoreInfoGrid :public CLyObjGrid<ShoreInfo>
	{
		virtual void OnEndItemChange(CXTPPropertyGridItem *item);
	};
protected:	

	DECLARE_MESSAGE_MAP() 
	void DoDataExchange(CDataExchange* pDX);
	afx_msg void OnApplyShape();
	afx_msg void OnApplyBrush();
	afx_msg void OnShowWireframe();

private:
	CShoreCPGrid			_shoreCPGrid;
	CShoreInfoGrid			_shoreInfoGrid;
	CBrushLibLVCtrl			_shoreWaveLibWnd;	
	CGuiAgent_ShorePoints * _agentMove;
	BOOL _bWireframe;
};




