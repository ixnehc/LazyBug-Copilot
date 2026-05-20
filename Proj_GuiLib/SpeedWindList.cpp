/********************************************************************
	created:	2008/07/08
	created:	8:7:2008   13:46
	filename: 	e:\IxEngine\Proj_GuiLib\SpeedWindList.cpp
	file path:	e:\IxEngine\Proj_GuiLib
	file base:	SpeedWindList
	file ext:	cpp
	author:		star
	
	purpose:	
*********************************************************************/

 
#include "stdh.h"
#include "SpeedWindList.h"

#include "CommonCtrlBase.h"

#include "Log/LogFile.h"

#include "stringparser/stringparser.h"

#include "BoneAnimPanel.h"

#include "resdata/AnimData.h"

#include "setter/setter.h"
#include "SpeedTreePanel.h"

#include <assert.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
//////////////////////////////////////////////////////////////////////////
//CSpeedWindList
void CSpeedWindList::Bind(ResEditPanelState *state0,BOOL bUpdateCtrl)
{
	CResEditCtrl::Bind(state0,bUpdateCtrl);
	if (!bUpdateCtrl)
		return;
	ResetContent();
	SptData * resData = GetResData(state0);
	if(!resData)
		return;

	for(int i = 0;i< resData->namewinds.size();i++)
		InsertString(i, fromMBCS(resData->namewinds[i].c_str()));

	if(!resData) return;
	
}
void CSpeedWindList::EnableCtrl(BOOL bActive) //
{
	if(bActive)
		EnableWindow(TRUE);
	else
		EnableWindow(FALSE);
}
void CSpeedWindList::_OnNewItem(DWORD idx,const char *name)
{
	assert(_state);
	assert(_panel);
	if (!_state->resdata)
		return;
	SptData * resData = GetResData(_state);
	
	resData->cfgwinds.push_back(SptWndCfg());
	resData->namewinds.push_back(std::string(name));

	RefreshMod();
}

void CSpeedWindList::_OnChangeItem(DWORD idx,const char *name)	//
{
	assert(_state);
	assert(_panel);
	if (!_state->resdata)
		return;
	
	SptData * resData = GetResData(_state);

	if(idx>=resData->namewinds.size())
		return;

	std::string & namewind = resData->namewinds[idx];
	namewind = name;

	RefreshMod();
}

void CSpeedWindList::_OnDeleteItem(DWORD idx)
{
	SptData * resData = GetResData(_state);
	if(!resData||idx>=resData->namewinds.size())
		return;

	resData->namewinds.erase(resData->namewinds.begin()+idx);
	resData->cfgwinds.erase(resData->cfgwinds.begin()+idx);

	RefreshMod();
}

void CSpeedWindList::_OnSwapItem(DWORD idx1,DWORD idx2)
{
	SptData *resData = GetResData(_state);

	if(!resData||idx1==idx2||idx1>=resData->namewinds.size()||idx2>=resData->namewinds.size())
		return;
	
	std::string & namewind0 = resData->namewinds[idx1];
	namewind0.swap(resData->namewinds[idx2]);
	
	SptWndCfg &cfg0 = resData->cfgwinds[idx1];
	SptWndCfg &cfg1 = resData->cfgwinds[idx2];
	SptWndCfg temp = cfg0;
	cfg0 = cfg1;
	cfg1 = temp;

	RefreshMod();
}

void CSpeedWindList::_OnSelChange(DWORD iSel)
{
	SpeedTreePanelSate * state = GetState(_state);
	if(state)
	{
		state->iSelWind = iSel;
		RefreshMod();
	}	
}







