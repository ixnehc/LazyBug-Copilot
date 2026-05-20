
#pragma once
#include "GuiLib.h"

#include "resource.h"
#include <vector>
#include "ResEditCtrl.h"
#include "RichGrid.h"

class CResEditPanel;
struct SptData;
struct ResEditPanelState;
struct SpeedTreePanelSate;

class GuiLib_Api CSpeedTreeLodCtrl: public CRichGrid ,public CResEditCtrl
{
public:
	CSpeedTreeLodCtrl()
	{
	}

public:
	virtual BOOL Create(const RECT& rect, CWnd* pParentWnd, UINT nID, DWORD dwListStyle = LBS_OWNERDRAWFIXED| LBS_NOINTEGRALHEIGHT);
	virtual void Bind(ResEditPanelState *state,BOOL bUpdateCtrl);
	virtual void EnableCtrl(BOOL bActive=TRUE);

	virtual void OnBeginItemChange(CXTPPropertyGridItem *item);
	virtual void OnItemChange(CXTPPropertyGridItem *item);
	virtual void OnEndItemChange(CXTPPropertyGridItem *item);

	DECLARE_MESSAGE_MAP()
protected:
	
	afx_msg LRESULT OnGridNotify(WPARAM wParam, LPARAM lParam);

	SptData * GetResData(){return (SptData *)_GetResData();}
	SpeedTreePanelSate * GetState(){return (SpeedTreePanelSate *)(_state);}

	friend class CEventItemSpin;	
	
	RGState state;

};

