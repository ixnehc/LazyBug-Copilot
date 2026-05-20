
#pragma once

#include "GuiLib.h"

#include "GuiEditor.h"

#include "WorldSystem/IETProbe.h"

class CGuiAgent_ETProbeMove;
class CETProbeGrid;
class GuiLib_Api CGuiPanel_ETProbe :public CGuiPanel
{
public:
	CGuiPanel_ETProbe(CWnd * pParent = NULL);
	virtual ~CGuiPanel_ETProbe();
	virtual BOOL Create(CWnd *pParent);
	virtual const char *GetName(){ return "etprobe";}
	virtual const char *_GetModMgrName()	{		return "world";	}

	virtual void UpdateUI();
	virtual void OnEnterActivity();
	virtual void OnDetachView(CGeView *view,DWORD iLevel);
	void Reset(){}

protected:	
	DECLARE_MESSAGE_MAP() 
	void DoDataExchange(CDataExchange* pDX);
	void OnETProbeCreate();

private:
	BOOL  _bOnCreate; //
	CGuiAgent_ETProbeMove * _pAgentMove;
	CETProbeGrid * _pGridETProbe;
	std::vector<HMapObj> _hObjsMod;
};



