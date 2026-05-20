
#include "stdh.h"

#include "RenderSystem/IRenderPort.h"
#include "RenderSystem/ITools.h"
#include "RenderSystem/ISurface.h"
#include "RenderSystem/ITexture.h"



#include "GuiAgent_ETProbeBake.h"
 
#include "GuiData_ETProbe.h"

#include "AgentCmdID.h"

#include "WorldSystem/IAssetSystem.h"

#include "WorldSystem/IEntitySystem.h"

#include "RenderSystem/IUtilRS.h"

#include "WorldSystem/IAssetRenderer.h"

BOOL CGuiAgent_ETProbeBake::OnCommand(DWORD idCmd)
{
	IETProbeEditor * editor = NULL;
	GuiData_ETProbe * data = (GuiData_ETProbe *)FindData("etprobe");
	if(data)
		editor = data->GetEditor();
	
	if(!editor)
		return TRUE;

	IRenderPort * rp = GetRP();
	IRenderSystem * pRS = rp->GetRS();

	if(idCmd==ID_AGENT_ETPROBE_BAKE){
		for(int i = 0;i<data->hObjSels.size();i++){
			HMapObj &hObj = data->hObjSels[i];
			const ETProbeInfo * info = editor->GetETProbeInfo(hObj);
			BakeEnvTex(_GetActor(),data->hObjSels[i],GetRP());
		}
	}
	
	return TRUE;
}

BOOL  CGuiAgent_ETProbeBake::OnRButtonClick(int x,int y,DWORD flag)
{
	IETProbeEditor * editor = NULL;
	GuiData_ETProbe * data = (GuiData_ETProbe *)FindData("etprobe");
	if(data&&!data->hObjSels.empty())
		_AddMenu("烘焙环境贴图",ID_AGENT_ETPROBE_BAKE);

	return TRUE;
}

void BakeEnvTex(CGeActor * actor,const HMapObj &hObj,IRenderPort * rp)
{
	IETProbeEditor * editor = NULL;
	GuiData_ETProbe * data = (GuiData_ETProbe *)actor->FindData("etprobe");
	if(data)
		editor = data->GetEditor();
	
	if(!editor)
		return ;

	const ETProbeInfo * info = editor->GetETProbeInfo(hObj);
	if(!info||info->texPath.empty())
		return;
	
	ICamera * cam = rp->GetCamera();
	i_math::vector3df posEye;
	cam->GetEyePos(posEye);
	
	IRenderSystem * pRS = rp->GetRS();
	ICamera *camCone =  pRS->CreateCamera();
	camCone->Clone(cam);

	IAssetSystem * pAS = data->pES->GetAS();
	i_math::vector3df posCenter = info->pos;
	
	D3DFORMAT d3dFmtDump   = D3DFMT_DXT1;
	D3DFORMAT d3dFmtRT = D3DFMT_A8R8G8B8;
	switch(info->typeFmt){
		case 0:
			d3dFmtDump = D3DFMT_DXT1; break;
		case 1:
			d3dFmtDump = D3DFMT_DXT5; break;
		case 2:
			d3dFmtRT = d3dFmtDump = D3DFMT_R5G6B5; break;
		case 3:
			d3dFmtDump = D3DFMT_A8R8G8B8; break;
	}

	i_math::vector3df at[] = {  i_math::vector3df(1.0f,0,0),
								i_math::vector3df(-1.0f,0,0),
								i_math::vector3df(0,1.0f,0),
								i_math::vector3df(0,-1.0f,0),
								i_math::vector3df(0,0,1.0f),
								i_math::vector3df(0,0,-1.0f)
							 };
	
	i_math::vector3df up[] = {  i_math::vector3df(0,1.0f,0),
								i_math::vector3df(0,1.0f,0),
								i_math::vector3df(0,0,-1.0f),
								i_math::vector3df(0,0,1.0f),
								i_math::vector3df(0,1.0f,0),
								i_math::vector3df(0,1.0f,0)
							 };

	SurfHandle surf;
	float rad =  3.14159265358979323846f/2.0f;
	
	ITexture * texCube = pRS->GetRTexMgr()->CreateCube(info->wMap,info->wMap,d3dFmtRT,1);
	ITexture * texDump = texCube;
	if(d3dFmtRT!=d3dFmtDump)
		texDump = pRS->GetWTexMgr2()->CreateCube(info->wMap,info->wMap,d3dFmtDump,1);
	
	//使水Ragent不工作
	IAssetRenderer * astRender = data->pES->GetAS()->GetRenderer();

	if(!info->bDrawWater)
		astRender->EnableRagent(Ragent_Liquid,FALSE);

	rp->PushState();
	if(pAS->Locate(posCenter)){

		for(int i = 0;i<6;i++){	

			//设置表面
			surf.Set(texCube,i);
			rp->SetRenderTarget(&surf);

			//设置相机
			cam = rp->QueryCamera();
			cam->SetFov(rad);
			cam->SetAspectRatio(1.0f);
			cam->SetPosTarget(posCenter,posCenter+at[i],up[i]);

			//绘制一帧
			pRS->BeginFrame();
			astRender->Render(rp,AdrPart_NotShell);
			pRS->EndFrame();
		}

		//RT 不支持的贴图格式
		if(d3dFmtRT!=d3dFmtDump){
			TexStretchArg arg;
			for(int i = 0;i<6;i++){
				arg.iFaceSrc = arg.iFaceDest = i;
				texDump->Stretch(texCube,arg);
			}	
		}

		//输出
		TexData * dataRes = NULL;
		if(texDump->DumpData(dataRes)){
			std::string pathRes = pRS->GetPath(Path_Res);
			pathRes.append("\\");
			pathRes.append(info->texPath);
			data->pUtils->SaveTexData(pathRes.c_str(),dataRes);	
		}
	}
	rp->PopState();
	
	//激活Ragent
	astRender->EnableRagent(Ragent_Liquid,TRUE);

	if(texDump!=texCube)
		SAFE_RELEASE(texDump);
	SAFE_RELEASE(texCube);

	//恢复现场
	data->pES->Locate(posEye);
	cam->Clone(camCone);
	SAFE_RELEASE(camCone);
}









