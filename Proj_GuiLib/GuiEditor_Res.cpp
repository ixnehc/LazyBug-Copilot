#include "stdh.h"
#include ".\GuiLib.h"

#include <vector>
#include <string>

#include "RenderSystem/IRenderPort.h"


#include "GuiData.h"

#include "GuiEditor_res.h"

#include "ResEditPanel.h"

#include "graphicsgraph.h"

#include "Registry/Registry.h"



void CGuiView_Res::_OnPreDraw(IRenderPort *rp)
{
	CGuiData_Res *data=(CGuiData_Res *)FindData("resource");
	if (!data)
		return;

	if (TRUE)
	{
		int fov=75;
		if (g_ssGuiLib.reg)
			fov=g_ssGuiLib.reg->ReadInt("General","EditorCamFov",75);
		data->cam->SetFov(((float)fov)*(float)i_math::GRAD_PI2);
	}

	rp->SetCamera(data->cam);



	rp->ClearBuffer(ClearBuffer_All,0x3f3f3f);
	extern BOOL DrawGrid(IRenderPort *rp,DWORD d,DWORD gap);
	DrawGrid(rp,10,1);
}

void CGuiView_Res::_OnDraw(IRenderPort *rp)
{
	CGuiData_Res *data=(CGuiData_Res *)FindData("resource");
	if (data)
		if (data->panel)
			data->panel->Draw(rp);
}

void CGuiView_Res::_OnPostDraw(IRenderPort *rp)
{

}



void CGuiView_Res2::_OnDraw(GraphicsGraph *gg)
{
	i_math::recti rc(-20000,-20000,20000,20000);
	gg->FillHatchRect(rc,0x7f7f7f,0x6f6f6f,4);

	CGuiData_Res *data=(CGuiData_Res *)FindData("resource");
	if (data)
	if (data->panel)
		data->panel->Draw(gg);
}

