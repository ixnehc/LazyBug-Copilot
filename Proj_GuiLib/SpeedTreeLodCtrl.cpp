

#include "stdh.h"
#include "resdata/AnimData.h"
#include "RichGridIntItem.h"

#include "SpeedTreeLodCtrl.h"

#include "Log/LogFile.h"
#include "CommonCtrlBase.h"
#include "stringparser/stringparser.h"
#include "SpeedTreePanel.h"
#include "resdata/SptData.h"
#include "stringparser/stringparser.h"
#include <assert.h>



#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////////
//CSpeedTreeLodCtrl

BEGIN_MESSAGE_MAP(CSpeedTreeLodCtrl, CXTPPropertyGrid)
	ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)

END_MESSAGE_MAP()


BOOL CSpeedTreeLodCtrl::Create(const RECT& rc, CWnd* pParentWnd, UINT nID, DWORD dwListStyle)
{
	if (!CXTPPropertyGrid::Create(rc,pParentWnd,nID,dwListStyle))
		return FALSE;

	SetPropertySort(xtpGridSortNoSort);
	ShowToolBar(FALSE);
	ShowHelp(FALSE);
	SetTheme(xtpGridThemeOffice2003);

	return TRUE;

}

void CSpeedTreeLodCtrl::EnableCtrl(BOOL bActive)
{
	if(bActive)
		EnableWindow(TRUE);
	else
		EnableWindow(FALSE);
}

void CSpeedTreeLodCtrl::Bind(ResEditPanelState*state,BOOL bUpdateCtrl)
{
	CResEditCtrl::Bind(state,bUpdateCtrl);
	if (!bUpdateCtrl)
		return;
	
	ResetContent();
	SptData * resData = GetResData();
	if(!resData)
		return;

	int idx = GetState()->iSelWind;
	
	SptWndCfg &cfg = resData->cfgwinds[idx];
	
	LockPaint();
	BeginInsert();

	InsertCategory("LOD","the wind lod.");
	PushInsert();
		for(int i = 0;i<resData->numLods;i++)
		{
			std::string s;
			FormatString(s,"lod[%d]",i);
			InsertCategory(s.c_str(),"lod setting");
			PushInsert();
				InsertFloatItem("distance","lod distances",resData->transitionDists+i,1.0f,1000.0f);
				InsertFloatItem("precent","lod precents",resData->transitionPrecent+i,0.0f,1.0f);
			PopInsert();
		}
	PopInsert();
	
	EndInsert();
	UnLockPaint();
	ExpandAll();
}
void CSpeedTreeLodCtrl::OnBeginItemChange(CXTPPropertyGridItem *item)
{
	RecordState(state);
	LockPaint();
}
void CSpeedTreeLodCtrl::OnItemChange(CXTPPropertyGridItem *item)
{

}
void CSpeedTreeLodCtrl::OnEndItemChange(CXTPPropertyGridItem *item)
{
	RefreshMod();
	UnLockPaint();
	RestoreState(state);
}
LRESULT CSpeedTreeLodCtrl::OnGridNotify(WPARAM wParam, LPARAM lParam)
{
	return 0;
}




