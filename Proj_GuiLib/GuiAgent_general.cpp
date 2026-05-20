#include "stdh.h"

#include <vector>
#include <string>

#include "FileSystem/IMapFileDefines.h"

#include "RenderSystem/IRenderPort.h"
#include "RenderSystem/IMesh.h"
#include "RenderSystem/IMtrl.h"

#include "RenderSystem/IRecords.h"

#include "FileSystem/IMapFile.h"


#include "WorldSystem/ITrrn.h"
#include "WorldSystem/IWorldSystem.h"
#include "WorldSystem/IBake.h"
#include "WorldSystem/IEntitySystem.h"

#include "WorldSystem/IAssetSystem.h"

#include "WorldSystem/IAssetBodyMap.h"
#include "WorldSystem/IAssetRenderer.h"
#include "WorldSystem/IAnimNodes.h"

#include "AgentCmdID.h"

#include "GuiAgent_general.h"
#include "GuiAgent_MatSet.h"

#include "GuiData_entitymap.h"

#include "timer/profiler.h"
#include "stringparser/stringparser.h"
#include "config/config.h"
#include "ImageBase.h"
#include "records/recordsdefine.h"
#include "records/records.h"

#include "WMGuiLib.h"




BOOL CGuiAgent_ViewSwitcher::OnRButtonClick(int x,int y,DWORD flag)
{
	GuiData_ViewSwitch *data=(GuiData_ViewSwitch *)FindData("viewswitch");

	if (data)
	{
		_PushMenu("Show");
		if (data->bShowProfiler)
			_AddMenu("Profiler(按1切换)",ID_AGENT_ViewProfiler,MF_CHECKED);
		else
			_AddMenu("Profiler(按1切换)",ID_AGENT_ViewProfiler);


		BOOL bNeverShow=g_ssGuiLib.pES->GetSS()->cfg->GetNumber("Engine.Helper.NeverShow");
		if (bNeverShow)
			_AddMenu("Helper(按2切换)",ID_AGENT_ViewHelper);
		else
			_AddMenu("Helper(按2切换)",ID_AGENT_ViewHelper,MF_CHECKED);

		BOOL bShowSeeThru=g_ssGuiLib.pES->GetSS()->cfg->GetNumber("Engine.Renderer.ShowSeeThru");
		if (!bShowSeeThru)
			_AddMenu("视线穿透物件(按3切换)",ID_AGENT_ViewSeeThru);
		else
			_AddMenu("视线穿透物件(按3切换)",ID_AGENT_ViewSeeThru,MF_CHECKED);

		BOOL bShowStaticShdwCaster=g_ssGuiLib.pES->GetSS()->cfg->GetNumber("Engine.Renderer.ShowStaticShdwCaster");
		if (!bShowSeeThru)
			_AddMenu("静态阴影投射体(按4切换)",ID_AGENT_ViewStaticShdwCaster);
		else
			_AddMenu("静态阴影投射体(按4切换)",ID_AGENT_ViewStaticShdwCaster,MF_CHECKED);


		_PopMenu();
	}

	return TRUE;
}

BOOL CGuiAgent_ViewSwitcher::OnCommand(DWORD idCmd)
{
	GuiData_ViewSwitch *dataViewSwitch=(GuiData_ViewSwitch*)FindData("viewswitch");
	if (!dataViewSwitch)
		return TRUE;
	if (idCmd==ID_AGENT_ViewProfiler)
	{
		dataViewSwitch->bShowProfiler=!dataViewSwitch->bShowProfiler;
		_Redraw(FALSE);
	}

	if (idCmd==ID_AGENT_ViewHelper)
	{
		BOOL bNeverShow=g_ssGuiLib.pES->GetSS()->cfg->GetNumber("Engine.Helper.NeverShow");
		g_ssGuiLib.pES->GetSS()->cfg->SetNumber("Engine.Helper.NeverShow",!bNeverShow);
		_Redraw(FALSE);
	}

	if (idCmd==ID_AGENT_ViewSeeThru)
	{
		BOOL bShowSeeThru=g_ssGuiLib.pES->GetSS()->cfg->GetNumber("Engine.Renderer.ShowSeeThru");
		g_ssGuiLib.pES->GetSS()->cfg->SetNumber("Engine.Renderer.ShowSeeThru",!bShowSeeThru);
		g_ssGuiLib.pES->GetSS()->pAS->GetSS()->adr->ResetConfig();
		_Redraw(FALSE);
	}

	if (idCmd==ID_AGENT_ViewStaticShdwCaster)
	{
		BOOL bShowStaticShdwCaster=g_ssGuiLib.pES->GetSS()->cfg->GetNumber("Engine.Renderer.ShowStaticShdwCaster");
		g_ssGuiLib.pES->GetSS()->cfg->SetNumber("Engine.Renderer.ShowStaticShdwCaster",!bShowStaticShdwCaster);
		g_ssGuiLib.pES->GetSS()->pAS->GetSS()->adr->ResetConfig();
		_Redraw(FALSE);
	}


	return TRUE;

}

BOOL CGuiAgent_ViewSwitcher::OnKeyDown(char c,DWORD flag)
{
	if (c=='1')
	{
		OnCommand(ID_AGENT_ViewProfiler);
		return FALSE;
	}
	if (c=='2')
	{
		OnCommand(ID_AGENT_ViewHelper);
		return FALSE;
	}
	if (c=='3')
	{
		OnCommand(ID_AGENT_ViewSeeThru);
		return FALSE;
	}
	if (c=='4')
	{
		OnCommand(ID_AGENT_ViewStaticShdwCaster);
		return FALSE;
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////
//CGuiAgent_BakeLocal

BOOL CGuiAgent_BakeLocal::OnRButtonClick(int x,int y,DWORD flag)
{
	HitProbe probe;
	IRenderPort *rp=GetRP();
	if (rp)
	{
		if (!rp->CalcHitProbe(x,y,probe))
			return TRUE;
	}

	BOOL bHit=FALSE;
	if (TRUE)
	{
		GuiData_EntityMap*data=(GuiData_EntityMap*)FindData("entitymap");
		if ((data)&&(data->mp))
		{
			EntityAddress addr=data->pES->HitTestOnMap(probe);
			if (data->mp->ResolveBlockPos(addr,_ptBlk))
				bHit=TRUE;
		}
	}

	if (!bHit)
	{//try terrain
		GuiData_Trrn * data =  (GuiData_Trrn *)FindData("terrain");
		if(data)
		{
			ITrrnMapEditor * editor = data->GetTrrnMapEditor();
			if(editor)	
			{
				i_math::vector3df vHit;
				if (editor->GetHitPos(probe,TRUE,vHit))
				{
					vHit.floor(BLOCK_LENGTH);
					_ptBlk.x=(int)(vHit.x/BLOCK_LENGTH);
					_ptBlk.y=(int)(vHit.z/BLOCK_LENGTH);

					bHit=TRUE;
				}
			}
		}
	}

	if (bHit)
	{
		_AddMenuSep();
		_AddMenu("Bake here(低品质)",ID_AGENT_BakeLocalLQ);
// 		_AddMenu("Bake here(缺省品质)",ID_AGENT_BakeLocalMQ);
		_AddMenu("Bake here(高品质)",ID_AGENT_BakeLocalHQ);
		_AddMenuSep();
		_AddMenu("Bake All(低品质)",ID_AGENT_BakeGlobalLQ);
		_AddMenu("Bake All(高品质)",ID_AGENT_BakeGlobalHQ);
		_AddMenuSep();
	}

	return TRUE;

}

//Copied from trrndefines.h
#define BLOCK_PER_LB_SR 8 //SR代表Square Root

BOOL CGuiAgent_BakeLocal::OnCommand(DWORD idCmd)
{
	if ((idCmd==ID_AGENT_BakeGlobalLQ)||(idCmd==ID_AGENT_BakeGlobalHQ))
	{
		if (AfxMessageBox(_T("确认?"), MB_OKCANCEL) != IDOK)
			return TRUE;
		GuiData_System * dataSys = (GuiData_System *)FindData("system");

		i_math::recti rcMap=dataSys->mf->GetRect();

		CString titleOrg;
		AfxGetMainWnd()->GetWindowText(titleOrg);

		CString title;

		int idx=0;
		int total=0;
		for (int i=rcMap.Left();i<rcMap.Right();i+=BLOCK_PER_LB_SR)
		for (int j=rcMap.Top();j<rcMap.Bottom();j+=BLOCK_PER_LB_SR)
			total++;

		for (int i=rcMap.Left();i<rcMap.Right();i+=BLOCK_PER_LB_SR)
		for (int j=rcMap.Top();j<rcMap.Bottom();j+=BLOCK_PER_LB_SR)
		{
			title.Format(_T("%s -- %sBaking(%.1f%%)"), (LPCTSTR)titleOrg,
				idCmd == ID_AGENT_BakeGlobalLQ ? _T("低品质") : _T("高品质"),
				((float)idx)/((float)total)*100.0f);
			AfxGetMainWnd()->SetWindowText((LPCTSTR)title);
			AfxPumpMessage();

			idx++;

			ISceneBaker *baker= dataSys->pWS->CreateSceneBaker(dataSys->pES);

			i_math::pos2di pt=i_math::pos2di(i,j);
			pt.scale_signed(BLOCK_PER_LB_SR);
			pt*=BLOCK_PER_LB_SR;
			i_math::recti rcBlk(pt.x,pt.y,pt.x+BLOCK_PER_LB_SR,pt.y+BLOCK_PER_LB_SR);

			BakeQuality quality;
			switch(idCmd)
			{
				case ID_AGENT_BakeGlobalHQ:
					quality=BQ_Hi;
					break;
				case ID_AGENT_BakeGlobalLQ:
					quality=BQ_Low;
					break;
			}
			baker->Bake(rcBlk,quality);

			SAFE_RELEASE(baker);

		}


		GuiData_Trrn  * dataTrrn = (GuiData_Trrn *)FindData("terrain");
		if (dataTrrn)
		{
			if (dataTrrn->GetTrrnMap())
				dataTrrn->GetTrrnMap()->SaveModified();
		}

		dataSys->pES->ReloadAllMap();


		_Redraw(FALSE);

		AfxGetMainWnd()->SetWindowText((LPCTSTR)titleOrg);

		return FALSE;
	}

	if ((idCmd==ID_AGENT_BakeLocalLQ)||(idCmd==ID_AGENT_BakeLocalMQ)||(idCmd==ID_AGENT_BakeLocalHQ))
	{
		GuiData_System * dataSys = (GuiData_System *)FindData("system");
		ISceneBaker *baker= dataSys->pWS->CreateSceneBaker(dataSys->pES);

		i_math::pos2di pt=_ptBlk;
		pt.scale_signed(BLOCK_PER_LB_SR);
		pt*=BLOCK_PER_LB_SR;
		i_math::recti rcBlk(pt.x,pt.y,pt.x+BLOCK_PER_LB_SR,pt.y+BLOCK_PER_LB_SR);
		std::vector<i_math::pos2di> blks;
		for (int i=rcBlk.Left();i<rcBlk.Right();i++)
		for (int j=rcBlk.Top();j<rcBlk.Bottom();j++)
			blks.push_back(i_math::pos2di(i,j));

		BakeQuality quality;
		switch(idCmd)
		{
			case ID_AGENT_BakeLocalHQ:
				quality=BQ_Hi;
				break;
			case ID_AGENT_BakeLocalMQ:
				quality=BQ_Medium;
				break;
			case ID_AGENT_BakeLocalLQ:
				quality=BQ_Low;
				break;
		}
		baker->Bake(rcBlk,quality);

		//dump the maps
		if (FALSE)
		{
			DWORD c=	baker->GetDumpMapCount();
			for (int i=0;i<c;i++)
			{
				DWORD w,h;
				DWORD *data;
				data=baker->GetDumpMap(i,w,h);

				std::string path;
				FormatString(path,"D:\\temp\\%04d.bmp",i);
				SaveImage(data,w,h,path.c_str());
			}
		}



		SAFE_RELEASE(baker);

		GuiData_Trrn  * dataTrrn = (GuiData_Trrn *)FindData("terrain");
		if (dataTrrn)
		{
			if (dataTrrn->GetTrrnMap())
				dataTrrn->GetTrrnMap()->SaveModified();
		}

		dataSys->pES->ReloadMap(blks.data(),blks.size());



		_Redraw(FALSE);

		return FALSE;
	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//CGuiAgent_PlayHere

BOOL CGuiAgent_PlayHere::OnRButtonClick(int x,int y,DWORD flag)
{
	HitProbe probe;
	IRenderPort *rp=GetRP();
	if (rp)
	{
		if (!rp->CalcHitProbe(x,y,probe))
			return TRUE;
	}

	if (TRUE)
	{
		GuiData_System*data=(GuiData_System*)FindData("system");
		if ((data)&&(data->pAS))
		{
			if (FALSE==data->pAS->GetBodyMap()->StaticHitTest(probe,_pos))
				return TRUE;
		}
	}

	_AddMenuSep();
	_AddMenu("在此处进入地图浏览模式",ID_AGENT_PLAYHERE);
	_AddMenuSep();

	return TRUE;

}

BOOL CGuiAgent_PlayHere::OnCommand(DWORD idCmd)
{
	if (idCmd==ID_AGENT_PLAYHERE)
	{
		AfxGetMainWnd()->SendMessage(GLM_Game_PlayHere,(WPARAM)&_pos,0);

		return FALSE;
	}
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//CGuiAgent_CameraFov
BOOL CGuiAgent_CameraFov::OnMouseWheel(int delta,DWORD flag)
{

	return TRUE;
}


void AddGeneralAgents(CGuiView *view)
{
	view->AddAgent(0,new CGuiAgent_ViewSwitcher);
	view->AddAgent(0,new CGuiAgent_BakeLocal);
	view->AddAgent(0,new CGuiAgent_PlayHere,AGENTPRIORITY_STANDARD-10);
	view->AddAgent(0,new CGuiAgent_CameraLock,AGENTPRIORITY_STANDARD+50);
}


//////////////////////////////////////////////////////////////////////////
//CGuiAgent_CameraLock
BOOL CGuiAgent_CameraLock::_UpdateCamAngle()
{
	if (_angelCam!=-10000.0f)
		return TRUE;
	GuiData_EntityMap *data=(GuiData_EntityMap *)_view->FindData("entitymap");
	if (!data)
		return FALSE;

	std::string nmMap;
	extern RecordID SeekMapRecordID(IWorldSystem *pWS,IMapFile *mf,std::string &nmMap);
	RecordID idRec=SeekMapRecordID(data->pAS->GetWS(),data->mf,nmMap);
	if (idRec==RecordID_Invalid)
		return FALSE;

	IRenderSystem *pRS=data->pES->GetWS()->GetRS();

	IRecords *records=(IRecords *)pRS->GetRecordsMgr()->ObtainRes("maps.rcs");
	if(records)
	{
		CRecords *recs=records->GetRecords();
		if (recs)
		{
			CRecord *rec=recs->GetRecord(idRec);

			GElemBase *elem=recs->FindElem("angleCam");
			if (elem)
			{
				void *var;
				if (elem->GetVar(rec->GetGObj()->GetOwner(),&var))
				{
					float *p=(float*)var;
					if (p)
					{
						_angelCam=*p;
						_angelCam*=(float)i_math::GRAD_PI2;
					}
				}
			}
		}
	}
	SAFE_RELEASE(records);

	if (_angelCam!=-10000.0f)
		return TRUE;

	return FALSE;
}

void CGuiAgent_CameraLock::_UpdateCam(int x,int y)
{
	GuiData_EntityMap *data=(GuiData_EntityMap *)_view->FindData("entitymap");
	if (!data)
		return;

	IRenderPort *rp=GetRP();
	if (!rp)
		return;

	HitProbe probe;
	if (rp->CalcHitProbe(x,y,probe,2000.0f))
	{
		i_math::vector3df pos;
		if (data->pAS->GetBodyMap()->WalkableHitTest(probe,pos))
		{
			const float pitch=45.0f*(float)i_math::GRAD_PI2;
			const float fov=0.4f;
			i_math::vector3df dirCam;
			if (TRUE)//算法与AstAvatarCameraFixed::_GetEyeDir()中一致
			{
				i_math::vector3df elur(_angelCam,pitch,0);
				i_math::quatf qRot;
				qRot.fromEuler(elur);
				i_math::vector3df dir(0,0,-1);
				dirCam=qRot*dir;
			}

			if (_dist<0.0f)
			{
				i_math::vector3df posOrg;
				if (rp->GetCamera()->GetEyePos(posOrg))
					_dist=posOrg.getDistanceFrom(pos);
				else
					_dist=30.0f;
			}

			i_math::vector3df posCam;
			if (TRUE)
			{
				i_math::vector3df dir;
				dir=dirCam;
				dir.setLength(_dist);
				posCam=pos-dir;
			}


			GuiData_Camera *dataCam=(GuiData_Camera *)_view->FindData("cameras");
			if (dataCam)
			{
				dataCam->cams[Camera_Perspective]->SetPosTarget(posCam,pos);
				dataCam->cams[Camera_Perspective]->SetFov(0.4f);
				dataCam->cams[Camera_Perspective]->SetNearFar(40.0f,100.0f);

				i_math::recti rc;
				_GetClientRect(rc);
				_SetCursorPos(rc.getCenter()+i_math::pos2di(0,1));
				_Redraw(FALSE);
			}

			_pos=pos;
		}
	}

	if (_an)
	{
		i_math::matrix43f mat;
		mat.setTranslation(_pos);
		_an->Set(mat);
	}
}


BOOL CGuiAgent_CameraLock::OnBeginDrag(int x,int y,DWORD flag)
{
	GuiData_EntityMap *data=(GuiData_EntityMap *)_view->FindData("entitymap");
	if (!data)
		return FALSE;

	if (!_UpdateCamAngle())
		return FALSE;

	GuiData_Camera *dataCam=(GuiData_Camera *)_view->FindData("cameras");
	if (dataCam)
	{
		dataCam->cams[Camera_Perspective]->GetFov(_fovOrg);
	}

	_dist=35.0f;
	if (flag&CtrlOpFlag_ShiftDown)
		_dist=-1.0f;

	if (data->pAS)
	{
		AssetSystemState *ss=data->pAS->GetSS();
		if (ss)
		{
			_an=ss->ans->CreateMatFixed();
			_hCT=ss->ratomsDefault->RegisterSeeThruTarget(_an,0.3f);
		}
	}

	_UpdateCam(x,y);
	_xLast=x;
	_yLast=y;

	return TRUE;
}

void CGuiAgent_CameraLock::OnEndDrag(int x,int y,DWORD flag)
{
	GuiData_EntityMap *data=(GuiData_EntityMap *)_view->FindData("entitymap");
	if (!data)
		return;

	_UpdateCam(x,y);


	if (data->pAS)
	{
		AssetSystemState *ss=data->pAS->GetSS();
		if (ss)
		{
			ss->ratomsDefault->UnRegisterSeeThruTarget(_hCT);
			SAFE_RELEASE(_an);
			_hCT=SeeThruTargetHandle_Null;
		}
	}


// 	GuiData_Camera *dataCam=(GuiData_Camera *)_view->FindData("cameras");
// 	if (dataCam)
// 	{
// 		dataCam->cams[Camera_Perspective]->SetFov(_fovOrg);
// 	}

}

void CGuiAgent_CameraLock::OnDrag(int x,int y,DWORD flag)
{
	if ((_xLast!=x)||(_yLast!=y))
	{
		_UpdateCam(x,y);
		_xLast=x;
		_yLast=y;
	}

}


BOOL CGuiAgent_CameraLock::OnDraw()
{
	GuiData_EntityMap *data=(GuiData_EntityMap *)_view->FindData("entitymap");
	if (!data)
		return TRUE;

	if (!_bInDrag)
		return TRUE;

	IRenderPort *rp=GetRP();
	if (!rp)
		return TRUE;

	if (TRUE)
	{
		IMesh *msh=(IMesh *)data->pAS->GetWS()->GetRS()->GetMeshMgr()->ObtainRes("角色\\兰提亚\\1.msh");
		IMtrl*mtl=(IMtrl*)data->pAS->GetWS()->GetRS()->GetMtrlMgr()->ObtainRes("角色\\兰提亚\\1.mtl");
		i_math::matrix43f mat;
		mat.setTranslation(_pos);
		rp->SimpleDrawMesh(msh,mat,0xffffffff,FALSE,mtl);
		SAFE_RELEASE(msh);
		SAFE_RELEASE(mtl);
	}

	return TRUE;
}
