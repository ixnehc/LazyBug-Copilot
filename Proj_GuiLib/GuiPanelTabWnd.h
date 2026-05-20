#pragma once

#include "resource.h"

#include <unordered_map>


// CGuiPanelCombo dialog

class CGuiActor;
class CGuiPanel;
class CSlideContainer;
class GuiLib_Api CGuiPanelCombo : public CXTPDialog
{
	DECLARE_DYNAMIC(CGuiPanelCombo)

public:
	CGuiPanelCombo(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGuiPanelCombo();

	enum { IDD = IDD_GUIPANELCOMBO };

	BOOL Create(CWnd *pParent);

	void AddGuiPanel(const char *name,CGuiPanel *panel);

	CGuiActor *GetActiveActor();

	virtual BOOL PreTranslateMessage(MSG* pMsg);



protected:

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);

protected:
	void _RecalcLayout();
	void _UpdateSel();


	std::unordered_map<std::string,CSlideContainer *>_panels;

	CSlideContainer *_sel;
public:
	afx_msg void OnCbnSelchangeCombo();
};


class GuiLib_Api CGuiPanelTabWnd:public CXTPTabControl
{
public:
	BOOL Create(CWnd *pParent);
	void AddTab(const char *nameTab,int idIcon);
	void AddGuiPanel(const char *nameTab,const char *nameCombo,CGuiPanel *panel);

	CGuiActor *GetActiveActor();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();

	std::vector<CGuiPanelCombo *>_combos;
	
};