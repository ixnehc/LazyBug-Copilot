

#include "stdh.h"
#include "GuiData_Changelists.h"
#include "Log/LogDump.h"


#include "WndBase.h"




//////////////////////////////////////////////////////////////////////////
//CGuiData_Changelists

void CGuiData_Changelists::Set(CChangelists* changelists)
{
	if (_changelists == changelists)
		return;

	_changelists = changelists;
	_ver++;
}


CGuiData_Changelists::CGuiData_Changelists()
{	
	_ver = 0;
	_changelists = NULL;
	_requestOpenSelTime = 0;
}

CGuiData_Changelists::~CGuiData_Changelists()
{
}


void	CGuiData_Changelists::Clear()
{
// 	std::unordered_map<std::string,ChangelistsGroup*>::iterator it;
// 	for (it=_grps.begin();it!=_grps.end();it++)
// 	{
// 		ChangelistsGroup*grp=(*it).second;
// 		grp->ClearEntries();
// 		Safe_Class_Delete(grp);
// 	}
// 	_grps.clear();
}

