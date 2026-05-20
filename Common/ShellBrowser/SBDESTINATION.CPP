// SBDestination.cpp: implementation of the CSBDestination class.
//
//////////////////////////////////////////////////////////////////////
#include "Stdh.h"
#include "SBDestination.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSBDestination::CSBDestination(const HWND hParent, LPCTSTR strTitle)
	: CBrowseForFolder(hParent, NULL, strTitle)
{

}

CSBDestination::~CSBDestination()
{

}

void CSBDestination::SetInitialSelection(LPCTSTR strPath)
{
	m_strInitialSelection = strPath;
}

void CSBDestination::OnInit() const
{
	SetSelection(m_strInitialSelection.c_str());
	SetStatusText(m_strInitialSelection.c_str());
}

void CSBDestination::OnSelChanged(const LPITEMIDLIST pidl) const
{
	char buffer[MAX_PATH];
	if (!SHGetPathFromIDList(pidl, buffer))
		buffer[0]=0;
	SetStatusText(buffer);
}
