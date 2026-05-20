#include "stdh.h"
#include ".\GuiLib.h"

#include <vector>
#include <string>

#include "RenderSystem/IRenderPort.h"
#include "RenderSystem/ITools.h"
#include "RenderSystem/IMesh.h"
#include "RenderSystem/IMtrl.h"


#include "GuiData.h"

#include "GuiData_Pathes.h"

#include "GuiView_main.h"
#include "RenderSystem/IRenderSystem.h"
#include "WorldSystem/ITrrn.h"
#include "WorldSystem/IAssetSystem.h"
#include "WorldSystem/IAssetRenderer.h"
#include "WorldSystem/IAssetBodyMap.h"

#include "WorldSystem/IEntitySystem.h"

#include "timer/profiler.h"

#include "resdata/AnimData.h"

#include "stringparser/stringparser.h"

#include "config/config.h"

void CalcDefaultNearFar(CConfig *cfg,float &n,float &f)
{
	f=(float)cfg->GetNumber("Engine.Common.ViewDist",500)+100.0f;//多加一点
	n=0.5f;
}

void CGuiView_Main::_OnPreDraw(IRenderPort *rp)
{
	GuiData_Camera *dataCam=(GuiData_Camera *)FindData("cameras");
	if (!dataCam)
		return;

	ICamera *cam=rp->QueryCamera();
	cam->Clone(dataCam->cams[Camera_Perspective]);
	float n,f;
	CalcDefaultNearFar(g_ssGuiLib.cfg,n,f);
	cam->SetNearFar(n,f);
}



void CGuiView_Main::_OnDraw(IRenderPort *rp)
{

	rp->ClearBuffer(ClearBuffer_DS);
//	rp->FillColor(ColorAlpha(0x7f7f7f,0xff));
	rp->FillColor(ColorAlpha(0,0xff));
//	rp->FillColor(ColorAlpha(0x0000ff,0xff));

	GuiData_System *dataSys=(GuiData_System *)FindData("system");
	if (!dataSys)
		return;

	if (TRUE)
	{
		EntitySystemInput in;
		i_math::recti rc;
		rp->GetRect(rc);
		in.SetRPSize(rc.getSize());
		in.dt=0.02f;
		dataSys->pES->Update(in);

		IAssetRenderer *adr=dataSys->pAS->GetRenderer();

  		dataSys->pRS->FlushCommand();
 		ProfilerStart(RenderMain);
		adr->Render(rp,AdrPart_NotShell);

		dataSys->pRS->FlushCommand();
		ProfilerEnd();
	}

	_DrawPathes(rp);



}

void CGuiView_Main::_OnPostDraw(IRenderPort *rp)
{
	GuiData_System *dataSys=(GuiData_System *)FindData("system");
	if (!dataSys)
		return;


	GuiData_ViewSwitch *dataViewSwitch=(GuiData_ViewSwitch*)FindData("viewswitch");
	if (dataViewSwitch)
	{
		if (dataViewSwitch->bShowProfiler)
		{
			dataViewSwitch->mgr->Dump();
			extern BOOL DrawProfile(IRenderPort *rp,ProfilerMgr *mgr,i_math::pos2di &pt);
			dataViewSwitch->mgr->Enable(FALSE);
			DrawProfile(rp,dataViewSwitch->mgr,i_math::pos2di(10,10));
			dataViewSwitch->mgr->Enable(TRUE);

			AdrStats stats;
			dataSys->pAS->GetRenderer()->DumpStats(stats);
			extern BOOL DrawAdrStats(IRenderPort *rp,AdrStats *stats,i_math::pos2di &pt);
			DrawAdrStats(rp,&stats,i_math::pos2di(380,10));
		}
	}

	if (TRUE)
	{
		ITrrnMap *trrnmap=dataSys->pES->FindTrrn();
		if (trrnmap)
		{
			ITexture *tex=trrnmap->GetTestTex();
			if (tex)
			{ 
				DrawTextureArg arg;
//				arg.bForceOpaque=TRUE;
				arg.SetDest(100,100,100+512,100+512);
				rp->DrawTexture(tex,arg);
			}
		}
	}

}



#define RADIUS_CP 0.2f  //显示控制点使用的半径

ILight *CreateCPLight(IRenderSystem *pRS)
{
	ILight *lgt=pRS->CreateLight();
	i_math::vector3df dir(1.0f,-1.0f,0.0f);
	dir.normalize();
	lgt->SetDirLight(dir,ColorAlpha(0x7f7f7f,0xff),ColorAlpha(0xafafaf,0xff),ColorAlpha(0xffffff,0xff));
	return lgt;
}

void CGuiView_Main::_DrawPathes(IRenderPort *rp)
{
	DEFINE_GUIDATA_PATHES(dataPathes);
	if (!dataPathes)
		return;
	GuiData_System *dataSys=(GuiData_System *)FindData("system");
	if (!dataSys)
		return;

	CConfig *cfg=dataSys->pAS->GetSS()->cfg;
	HelperLayor layorsHelper=cfg->GetNumber("Engine.Helper.Layors");
	if (cfg->GetNumber("Engine.Helper.NeverShow")==1)
		layorsHelper=0;
	if ((layorsHelper==0)&&(!dataPathes->bForceShow))
		return;

	_lines.clear();

	IRenderSystem *pRS=rp->GetRS();
	ILight *lgt=CreateCPLight(pRS);
	IMesh *mesh=(IMesh*)pRS->GetMeshMgr()->ObtainRes("_editor\\sphere.msh");
	IMtrl *mtrl=(IMtrl*)pRS->GetMtrlMgr()->ObtainRes("_editor\\sphere.mtl");

	//根据renderport camera更新_lgt的方向
	if (TRUE)
	{
		LightInfo *li=lgt->QueryInfo();
		rp->GetCamera()->GetEyeDir(li->dir);
	}

	std::unordered_map<std::string,ResData*>::iterator it;
	for (it=dataPathes->dataes.begin();it!=dataPathes->dataes.end();it++)
	{
		_lines.clear();
		XFormData *pth=(XFormData *)(*it).second;

		KeySet *keyset=pth->GetKeySet();

		for (int i=0;i<keyset->GetKeyCount();i++)
		{
			Key_xform *key=(Key_xform *)keyset->GetKey(i);
			if (i<=1)
				_lines.push_back(key->v.pos);
			else
			{
				_lines.push_back(_lines[_lines.size()-1]);
				_lines.push_back(key->v.pos);
			}
		}

		if (_lines.size()>1)
		{
			if (!StringEqualNoCase(dataPathes->sel.c_str(),(*it).first.c_str()))
				rp->Lines(_lines.data(),_lines.size()/2,ColorAlpha(0x00ff00,0xff));
		}

		//绘制控制点
		if (StringEqualNoCase((*it).first.c_str(),dataPathes->sel.c_str()))
		{
			i_math::matrix43f mat;
			mat.setScale(RADIUS_CP,RADIUS_CP,RADIUS_CP);
			for (int i=0;i<pth->cps.size();i++)
			{
				if (i!=dataPathes->iSelCP)
				{
					*mat.getTranslationP()=pth->cps[i].xfm.pos;
					rp->SimpleDrawMesh(mesh,mat,ColorAlpha(0x00ff00,0xff),FALSE,mtrl,lgt);
				}
			}

		}
	}

	SAFE_RELEASE(lgt);
	SAFE_RELEASE(mesh);
	SAFE_RELEASE(mtrl);
}

