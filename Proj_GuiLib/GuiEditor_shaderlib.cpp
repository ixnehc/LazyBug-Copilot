
#include "stdh.h"

#include "GuiEditor_shaderlib.h"

#include "ShaderLibGlobal.h"

#include "graphicsgraph.h"

//////////////////////////////////////////////////////////////////////////
//CGuiView_ShaderLib
void CGuiView_ShaderLib::_OnDraw(GraphicsGraph *gg)
{
	gg->ClearBg(RGB(128,128,128));

	GuiData_ShaderLib *data=(GuiData_ShaderLib*)FindData("shaderlib");

	if (data)
	{
		if (data->global)
		{
			data->graph.Load(data->global,data->nmLib.c_str(),gg);
			data->graph.Draw(gg);
		}

	}

}

static CShaderLibGlobal *GetGlobal(CGuiAgent_ShaderCommand *owner)
{
	GuiData_ShaderLib*data=(GuiData_ShaderLib*)owner->FindData("shaderlib");
	if (!data)
		return NULL;
	return data->GetGlobal();

}

static CShaderLib2 *GetLib(CGuiAgent_ShaderCommand *owner)
{
	GuiData_ShaderLib*data=(GuiData_ShaderLib*)owner->FindData("shaderlib");
	if (!data)
		return NULL;
	CShaderLibGlobal *global=data->GetGlobal();
	return global->FindLib(data->nmLib.c_str());
}


//////////////////////////////////////////////////////////////////////////
//CGuiAgent_ShaderVarCommand
#define ID_ShaderCommand_NewVar 1
BOOL CGuiAgent_ShaderCommand::OnRButtonClick(int x,int y,DWORD flag)
{
	GuiData_ShaderLib*data=(GuiData_ShaderLib*)FindData("shaderlib");
	if (!data)
		return TRUE;
	_ScreenToGG(x,y);

	_ptStart.set(x,y);


	_AddMenu("新增变量",ID_ShaderCommand_NewVar);

	return FALSE;

}

BOOL CGuiAgent_ShaderCommand::OnCommand(DWORD idCmd)
{
	CShaderLib2 *lib=GetLib(this);
	if (!lib)
		return TRUE;
	if (idCmd==ID_ShaderCommand_NewVar)
	{
		return FALSE;
	}
	return TRUE;
}




//////////////////////////////////////////////////////////////////////////
//CGuiActor_ShaderLib
void CGuiActor_ShaderLib::Reset()
{
	CGuiView *view=(CGuiView *)FindView("shaderlib");
	GuiData_ShaderLib*data=(GuiData_ShaderLib*)FindData("shaderlib");
	if (data&&view)
	{   
		view->AttachActor(0,this);

		view->AddAgent(0,new CGuiAgent_ShaderLibGraphScroll,AGENTPRIORITY_STANDARD);
		view->AddAgent(0,new CGuiAgent_ShaderCommand,AGENTPRIORITY_STANDARD);
	}

}
