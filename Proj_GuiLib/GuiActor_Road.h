
#pragma once

#include "GuiLib.h"

#include "GuiEditor.h"

#include "LyObjGrid.h"

#include "WorldSystem/IRoad.h"

#include "GuiAgent_PaintRoadOpacity.h"

class CGuiAgent_RoadPoints;
class GuiLib_Api CGuiPanel_Road :public CGuiPanel
{
public:
	CGuiPanel_Road(CWnd * pParent = NULL);
	virtual ~CGuiPanel_Road();
	virtual BOOL Create(CWnd *pParent);
	virtual const char *GetName(){ return "road";}
	virtual const char *_GetModMgrName()	{		return "world";	}

	virtual void UpdateUI();
	virtual void OnEnterActivity();
	virtual void OnDetachView(CGeView *view,DWORD iLevel);
	void Reset(){}

	class CRoadPropGrid :public CLyObjGrid<RoadProp>
	{
	public:
		void OnEndItemChange(CXTPPropertyGridItem *item);
		void SetOwner(CGuiPanel_Road * owner){_owner = owner;}
	private:	
		CGuiPanel_Road * _owner;
	};

protected:
	DECLARE_MESSAGE_MAP() 

	void DoDataExchange(CDataExchange* pDX);
	void OnRoadCreate();

private:
	CGuiAgent_RoadPoints * _agentPoints;
    CGuiAgent_PaintRoadOpacity * _agentPaintOpacity;
    CRoadPropGrid _roadPropGrid;
	CButton _btnStateCreate;
};



