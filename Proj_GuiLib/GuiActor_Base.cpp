/********************************************************************
	created:	2008/2/21   15:12
	file path:	d:\IxEngine\Proj_GuiLib
	author:		cxi
	
	purpose:	base gui actor
*********************************************************************/

#include "stdh.h"

#include "resource.h"

#include <vector>
#include <string>

#include "stringparser/stringparser.h"

#include "GuiActor_Base.h"

#include "RenderSystem/IRenderSystem.h"
#include "WorldSystem/IWorldSystem.h"






void CGuiActor_Base::DoAttach()
{
//	CGuiView *viewMain=(CGuiView *)FindView("Perspective");
//	if (viewMain)
//	{
//		viewMain->AttachActor(0,this);
//		viewMain->AddAgent(0,&_moverMain);
//		viewMain->AddAgent(0,&_rotaterMain);
//	}
//
//	CGuiView*viewAcl=(CGuiView*)FindView("AssetClassLib");
//	if (viewAcl)
//	{
//		viewAcl->AttachActor(0,this);
//		viewAcl->AddAgent(0,&_moverAcl);
//		viewAcl->AddAgent(0,&_rotaterAcl);
//	}

}

void CGuiActor_Base::UpdateUI()
{

}

void CGuiActor_Base::OnDetachView(CGeView *view,DWORD iLevel)
{

}

