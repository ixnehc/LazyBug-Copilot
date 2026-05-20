#include "stdh.h"
#include ".\GuiLib.h"

#include <vector>
#include <string>

#include "GuiData.h"

#include "GuiView_proto.h"

#include "RenderPortBase.h"

#include "gds/GProp.h"




void CGuiView_Proto::_OnDraw(IRenderPort *rp)
{
	GuiData_Proto *dataProto=(GuiData_Proto*)FindData("proto");
	if (!dataProto)
		return;

	rp->ClearBuffer(ClearBuffer_All,ColorAlpha(0x3f3f3f,0xff));

	rp->SetCamera(dataProto->cam);

	DrawGrid(rp,10,1);

	dataProto->UpdateEntity();

	IAssetRenderer *rdr=dataProto->pES->GetAS()->GetRenderer();

	rdr->SetRP(rp);
	rdr->Render();

}


BOOL CGuiView_Proto::Respond(CtrlOp &co)
{
	CGeView::Respond(co);
	return TRUE;
}


