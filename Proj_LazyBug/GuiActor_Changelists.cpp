
#include "stdh.h"
#include "GuiActor_Changelists.h"
#include "GuiData_Changelists.h"
#include "GuiAgent_Changelists.h"

CGuiActor_Changelists::CGuiActor_Changelists()
{
	_agTransform.AddRef();
}
 

CWnd* CGuiActor_Changelists::GetWnd()
{
	return NULL;
}

void CGuiActor_Changelists::Reset()
{
	CGuiView *view = (CGuiView *)FindView( "Changelists" );
	CGuiData_Changelists *dataChangelists = (CGuiData_Changelists*)FindData( "Changelists" );

	if ( dataChangelists && view )
	{   
		view->AttachActor(0,this);
		view->AddAgent(0, new CGuiAgent_ChangelistsEdit(), AGENTPRIORITY_STANDARD+1);
		view->AddAgent( 0, &_agTransform);
		view->AddAgent( 0, new CGuiAgent_ChangelistsCommand() );
	}
}

void CGuiActor_Changelists::UpdateUI()
{

}

void CGuiActor_Changelists::DoCommand( DWORD idCmd )
{

}

void CGuiActor_Changelists::UpdateCommandUI( DWORD idCmd, void *param )
{

}

