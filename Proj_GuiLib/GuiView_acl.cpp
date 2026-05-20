#include "stdh.h"
#include ".\GuiLib.h"

#include <vector>
#include <string>

#include "GuiData.h"

#include "GuiView_acl.h"

#include "RenderPortBase.h"




void CGuiView_Acl::_OnDraw(IRenderPort *rp)
{
	rp->ClearBuffer(ClearBuffer_All);
	rp->FillColor(ColorAlpha(0x3f3f3f,0xff));

	DrawGrid(rp,10,1);

	GuiData_Acl *dataAcl=(GuiData_Acl *)_mgr->FindData("assetclasslib");
	GuiData_Camera *dataCam=(GuiData_Camera *)_mgr->FindData("cameras");

	if ((!dataAcl)||(!dataCam))
		return;

	rp->SetCamera(dataCam->cams[Camera_AssetClassLib]);

	IAssetRenderer *adr=dataAcl->pAS->GetRenderer();
	adr->SetRP(rp);

	adr->EnableAllRagents(TRUE);
	adr->EnableRagent(Ragent_Shell,FALSE);
	adr->EnableRagent(Ragent_Trrn,FALSE);
	adr->Render();
}




