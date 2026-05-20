
#include "stdh.h"
#include "GuiActor_ValueSet.h"
#include "GuiData_ValueSet.h"
#include "GuiAgent_ValueSet.h"

CGuiActor_ValueSet::CGuiActor_ValueSet()
{
	_agTransform.AddRef();
}
 

CWnd* CGuiActor_ValueSet::GetWnd()
{
	return NULL;
}

void CGuiActor_ValueSet::Reset()
{
	CGuiView *view = (CGuiView *)FindView( "ValueSet" );
	CGuiData_ValueSet *dataValueSet = (CGuiData_ValueSet*)FindData( "ValueSet" );

	if ( dataValueSet && view )
	{   
		view->AttachActor(0,this);
		view->AddAgent( 0, &_agTransform);
		view->AddAgent( 0, new CGuiAgent_ValueSetCommand() );
		view->AddAgent( 0, new CGuiAgent_ValueSetEdit() );
	}
}

void CGuiActor_ValueSet::UpdateUI()
{

}

void CGuiActor_ValueSet::DoCommand( DWORD idCmd )
{

}

void CGuiActor_ValueSet::UpdateCommandUI( DWORD idCmd, void *param )
{

}


BOOL CGuiActor_ValueSet::Fit(const char *path)
{
	return _agTransform.Fit(path);
}
