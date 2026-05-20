#include "stdh.h"
#include ".\bakeutil.h"
#include "resource.h"
#include "GuiData.h"
#include "FileSystem/IMapFile.h"
#include "WorldSystem/IEntitySystem.h"
#include "WorldSystem/ITrrn.h"
#include "WorldSystem/ITrisBaker.h"
#include "WorldSystem/IMiniMapBaker.h"
#include "WorldSystem/IOutlineMapBaker.h"
#include "WorldSystem/IGtiBaker.h"

#include "RenderSystem/IRenderSystem.h"
#include "RenderSystem/ITexture.h"


#include "ModBlockBack.h"
#include "stringparser/stringparser.h"
#include "ImageBase.h"
#include "GuiData_ETProbe.h"
#include "GuiData_OverallMap.h"
#include "GuiData_NavMesh.h"
#include "RenderSystem/IRenderPort.h"
#include "RenderSystem/ITools.h"


extern void BakeEnvTex(CGeActor * actor,const HMapObj &hObj,IRenderPort * rp);

IMPLEMENT_TOOL_CLASS(CBakeUtil)

CBakeUtil::CBakeUtil(void)
{
	_baker = NULL;
	_bMsgUpdate = TRUE;
}

CBakeUtil::~CBakeUtil(void)
{
}

BOOL CBakeUtil::InitDlg(CWnd * pParent)
{
	return DefDialog(pParent,IDD_DIALOG_TOOLBAKE);
}

void CBakeUtil::BakeMultiEnvTex()
{
	_ctrl.SetPos(0);

	CGeActor * actor = GetActor();
	IETProbeEditor * editor = NULL;
	GuiData_ETProbe * data = (GuiData_ETProbe *)actor->FindData("etprobe");	
	if(data)
		editor = data->GetEditor();

	//得到视图
	IRenderPort * rp = NULL;
	CGuiView * view = (CGuiView *)actor->FindView("perspective");
	if(!view)
		return;

	int w = 0,h = 0; //Field Length
	if(TRUE){
		GuiData_System * dataSys = (GuiData_System *)actor->FindData("system");
		i_math::recti & rcMap = dataSys->mf->GetRect();
		i_math::recti & rcFld = dataSys->mf->GetFieldRect();
		w = BLOCK_LENGTH*rcMap.getWidth()/rcFld.getWidth();
		h = BLOCK_LENGTH*rcMap.getHeight()/rcFld.getHeight();
	}
	
	CWnd * pWndInfo = m_panel.GetDlgItem(IDC_EDIT_BAKEINFO);

	if(editor){
		i_math::pos2di p0,p1;
		DWORD count = 0;
		HMapObj * hObjs = NULL;
		for(int i = 0;i<_flds.size();i++){
			i_math::pos2di pt = _flds[i];
			p0.set(pt.x*w,pt.y*h);
			p1.set((pt.x+1)*w,(pt.y+1)*h);	

			hObjs = editor->Enum(i_math::recti(p0,p1),count);
			if(!hObjs)
				continue;

			for(int k = 0;k<count;k++){
				HMapObj &hObj = hObjs[k];
				const ETProbeInfo * info = editor->GetETProbeInfo(hObj);
				if(!info) 
					continue;

				std::string msg;
				FormatString(msg,"环境贴图: %s \r\n 正在生成...",info->texPath.c_str());
				pWndInfo->SetWindowText(fromMBCS(msg.c_str()));
				pWndInfo->UpdateWindow();

				BakeEnvTex(actor,hObj,view->GetRP());
				float ratio = (float(i)+float(k)/float(count))/float(_flds.size());
				_ctrl.SetPos(int(ratio*1000));
			}
		}
		pWndInfo->SetWindowText(_T("完成"));
		_ctrl.SetPos(1000);
	}
}

void CBakeUtil::BakeStaticLight()
{
	CGeActor * actor = GetActor();
	if(!actor||!_baker)
		return;

	DWORD tick = GetTickCount();

	GuiData_System * dataSys = (GuiData_System *)actor->FindData("system");

	i_math::recti & rcMap = dataSys->mf->GetRect();
	i_math::recti & rcFld = dataSys->mf->GetFieldRect();
	int w = rcMap.getWidth()/rcFld.getWidth();
	int h = rcMap.getHeight()/rcFld.getHeight();
	
	_nfld = _flds.size();
	_curfld = 0;

	for(int i = 0;i<_flds.size();i++)
	{
		i_math::pos2di pos = _flds[i];
		
		i_math::recti rc;
		rc.UpperLeftCorner.x = pos.x*w;
		rc.UpperLeftCorner.y = pos.y*h;
		rc.LowerRightCorner.x = (pos.x+1)*w;
		rc.LowerRightCorner.y = (pos.y+1)*h;
		
		_baker->Bake(rc,_bq);
		_curfld++;
		_bMsgUpdate = TRUE;
	}
	
	std::string msg;
	FormatString(msg,"Bake 结束! \n耗时%.02f分钟,\n共Bake了%d个256米x256米的地块!",
					((float)(GetTickCount()-tick))/1000.0f/60.0f,_flds.size());
	m_panel.SetDlgItemText(IDC_EDIT_BAKEINFO, fromMBCS(msg.c_str()));
	
	GuiData_Trrn  * dataTrrn = (GuiData_Trrn *)actor->FindData("terrain");
	ITrrnMapEditor * editor = dataTrrn->GetTrrnMapEditor();	
	if(editor)
	{
		editor->BeginModify();
		ITrrnMap * map = dataTrrn->GetTrrnMap();
		map->SaveModified();
		editor->EndModify();
	}

	//added by chenxi
	if (dataSys)
		dataSys->pES->ReloadAllMap();
}

void CBakeUtil::GenNavMesh()
{
	CGeActor * actor = GetActor();
	assert(actor);

	GuiData_NavMesh * data = (GuiData_NavMesh *)actor->FindData("navmesh");
	GuiData_System * dataSys = (GuiData_System *)actor->FindData("system");
	
	ICamera * cam = NULL;
	i_math::vector3df posEye;
	CGuiView * view = (CGuiView *)actor->FindView("perspective");
	if(view){
		IRenderPort * rp = view->GetRP();
		cam = rp->GetCamera();
		cam->GetEyePos(posEye);
	}

	INavMeshEditor * editor = NULL;
	if(data)
		editor = data->GetEditor();
	
	if(editor&&!_flds.empty())
	{
		editor->SetParams(&data->buildParams);
			
		i_math::recti & rcMap = dataSys->mf->GetRect();
		i_math::recti & rcFld = dataSys->mf->GetFieldRect();
		int wb = rcMap.getWidth()/rcFld.getWidth();
		int hb = rcMap.getHeight()/rcFld.getHeight();
		int nb = int(editor->GetMapBlockLen()/BLOCK_LENGTH);
		int w = wb/nb;
		int h = hb/nb;

		_nfld = _flds.size();
		_curfld = 0;
		for(int i = 0;i<_flds.size();i++)
		{
			i_math::pos2di pf = _flds[i];
			i_math::recti rcBlock;
			rcBlock.UpperLeftCorner.x = pf.x*w;
			rcBlock.UpperLeftCorner.y = pf.y*h;
			rcBlock.LowerRightCorner.x = (pf.x+1)*w;
			rcBlock.LowerRightCorner.y = (pf.y+1)*h;

			i_math::vector3df fieldCenter; 
			fieldCenter.x = float(wb*BLOCK_LENGTH*(2*pf.x + 1))/2.0f;
			fieldCenter.y = posEye.y;
			fieldCenter.z = float(wb*BLOCK_LENGTH*(2*pf.y + 1))/2.0f;			
			data->pES->Locate(fieldCenter);

			editor->Build(data->pES,rcBlock);
			_curfld++;
			_bMsgUpdate = TRUE;

			std::string msg;
			FormatString(msg,"cur:[ %d, %d ]完成了 %.2f%%",pf.x,pf.y,100.0f*float(i+1)/float(_flds.size()));
			
			m_panel.SetDlgItemText(IDC_EDIT_BAKEINFO, fromMBCS(msg.c_str()));
			m_panel.UpdateWindow();
		}

		data->pES->Locate(posEye);
	}
}

void CBakeUtil::BakeMiniMap()
{
	CGeActor * actor = GetActor();
	if (!actor)
		return;

	GuiData_System * dataSys = (GuiData_System *)actor->FindData("system");
	GuiData_OverallMap * dataMap = (GuiData_OverallMap *)actor->FindData("overallmap");

	i_math::recti & rcMap = dataSys->mf->GetRect();
	i_math::recti & rcFld = dataSys->mf->GetFieldRect();
	int w = rcMap.getWidth()/rcFld.getWidth()*BLOCK_LENGTH;
	int h = rcMap.getHeight()/rcFld.getHeight()*BLOCK_LENGTH;

	if (TRUE)
	{
		IMiniMapBaker *baker=dataSys->pWS->CreateMiniMapBaker(dataSys->pES);
		for(int i = 0;i<_flds.size();i++)
		{
			i_math::pos2di pos = _flds[i];

			i_math::recti rc;
			rc.UpperLeftCorner.x = pos.x*w;
			rc.UpperLeftCorner.y = pos.y*h;
			rc.LowerRightCorner.x = (pos.x+1)*w;
			rc.LowerRightCorner.y = (pos.y+1)*h;

			baker->BakeRaw(rc,4.0f);

			dataMap->DiscardFieldRawMiniMapCache(pos);

// 			if (TRUE)
// 			{
// 				DWORD *data;
// 				DWORD w,h;
// 				data=baker->GetResult(w,h);
// 
// 				ITexture *tex=dataSys->pWS->GetRS()->GetWTexMgr2()->Create(w,h,D3DFMT_A8R8G8B8);
// 				DWORD pitch;
// 				void *buf=tex->Lock(pitch,TexLock_WriteOnly);
// 				memcpy(buf,data,w*h*4);
// 				tex->UnLock();
// 				std::string s;
// 				FormatString(s,"D:\\temp\\minimap_%d_%d.tga",pos.x,pos.y);
// 				tex->DumpTga(s.c_str());
// 				SAFE_RELEASE(tex);
// 			}
		}

		SAFE_RELEASE(baker);

	}

}

void CBakeUtil::BakeOutlineMap()
{
	CGeActor * actor = GetActor();
	if (!actor)
		return;

	GuiData_System * dataSys = (GuiData_System *)actor->FindData("system");
	GuiData_OverallMap * dataMap = (GuiData_OverallMap *)actor->FindData("overallmap");
	GuiData_NavMesh * dataNavMesh = (GuiData_NavMesh *)actor->FindData("navmesh");

	i_math::recti & rcMap = dataSys->mf->GetRect();
	i_math::recti & rcFld = dataSys->mf->GetFieldRect();
	int w = rcMap.getWidth()/rcFld.getWidth()*BLOCK_LENGTH;
	int h = rcMap.getHeight()/rcFld.getHeight()*BLOCK_LENGTH;

	//OutlineMap
	if (TRUE)
	{
		IOutlineMapBaker *baker=dataSys->pWS->CreateOutlineMapBaker(dataSys->pES);
		for(int i = 0;i<_flds.size();i++)
		{
			i_math::pos2di pos = _flds[i];

			i_math::recti rc;
			rc.UpperLeftCorner.x = pos.x*w;
			rc.UpperLeftCorner.y = pos.y*h;
			rc.LowerRightCorner.x = (pos.x+1)*w;
			rc.LowerRightCorner.y = (pos.y+1)*h;

			baker->Bake(rc,&dataNavMesh->buildParams);
			dataMap->DiscardFieldOutlineMap(pos);

		}

		SAFE_RELEASE(baker);

	}

	//GTI
	if (TRUE)
	{
		IGtiBaker *baker=dataSys->pWS->CreateGtiBaker(dataSys->pES);
		for(int i = 0;i<_flds.size();i++)
		{
			i_math::pos2di pos = _flds[i];

			i_math::recti rc;
			rc.UpperLeftCorner.x = pos.x*w;
			rc.UpperLeftCorner.y = pos.y*h;
			rc.LowerRightCorner.x = (pos.x+1)*w;
			rc.LowerRightCorner.y = (pos.y+1)*h;

			baker->Bake(rc);
		}

		SAFE_RELEASE(baker);

	}



}



BOOL CBakeUtil::OnSetTitle(const char * title)
{
	return TRUE;
}
BOOL CBakeUtil::OnProcess(const char * info,int cur,int full)
{
	_mainPro = float(cur)/float(full);
	_unitSub = 1.0f/float(full);
	return TRUE;
}
BOOL CBakeUtil::OnEnd_FldBake()
{
	return TRUE;
}
BOOL CBakeUtil::OnBegin_FldBake(const char *name)
{
	return TRUE;
}
BOOL CBakeUtil::OnProcess_Unit(const char * info,int cur,int full)
{
	float precent = _mainPro + (float(cur)/float(full))*_unitSub;
	
	float  precentTotal = ((float)_curfld/(float)_nfld) + (1.0f/float(_nfld))*precent;

	int pos = int(precentTotal*1000.0f);
	
	_ctrl.SetPos(pos);
	
	char msg[255];
	memset(msg,0,sizeof(msg));
	
	int s = int(precent*100.0f);
	if(s!=_oldPrec)
	{
		_bMsgUpdate = TRUE;
		_oldPrec = s;
	}

	if(_curfld<_nfld&&_bMsgUpdate)
	{
		i_math::pos2di &fldCur = _flds[_curfld]; 
		sprintf(msg,"field: %d,%d \r\ncomplete:%d%%",fldCur.x,fldCur.y,s);
		m_panel.SetDlgItemText(IDC_EDIT_BAKEINFO, fromMBCS(msg));
		m_panel.UpdateWindow();//added by chenxi
		_bMsgUpdate = FALSE;
	}

	return TRUE;
}

void CBakeUtil::OnInitDlg(CGeActor * actor)
{
	PrgHandler_SetTitle e0;
	PrgHandler_SetProgess e1;
	PrgHandler_SetBegin e2;
	PrgHandler_SetEnd e3;

	e0.bind(this,&CBakeUtil::OnSetTitle);
	e1.bind(this,&CBakeUtil::OnProcess);
	e2.bind(this,&CBakeUtil::OnBegin_FldBake);
	e3.bind(this,&CBakeUtil::OnEnd_FldBake);
	
	_progMain.SetHandler(e0);
	_progMain.SetHandler(e1);
	_progMain.SetBeginHanlder(e2);
	_progMain.SetEndHanlder(e3);

	e1.bind(this,&CBakeUtil::OnProcess_Unit);
	_progSub.SetHandler(e1);

	_ctrl.SubclassDlgItem(IDC_PROGRESS_BAKE,&m_panel);
	_ctrl.SetRange(0,1000);
	_ctrl.SetStep(1);

	CButton *btn=(CButton*)m_panel.GetDlgItem(IDC_LOW);
	btn->SetCheck(1);
	_bq=BQ_Low;
}

BOOL CBakeUtil::OnCommand(DWORD ctrlID ,DWORD code,LPARAM lParam,CGeActor * actor)
{
	if(ctrlID==IDC_BUTTON_BAKE)
	{
		GuiData_OverallMap * dataMap = (GuiData_OverallMap *)actor->FindData("overallmap");
		if(!dataMap->fldSels.empty())
		{
			_flds.assign(dataMap->fldSels.begin(),dataMap->fldSels.end());
			
			GuiData_System * dataSys = (GuiData_System *)actor->FindData("system");
			_baker = dataSys->pWS->CreateSceneBaker(dataSys->pES);
			assert(_baker);
			_baker->SetProgress(&_progMain,&_progSub);
			
			BakeStaticLight();
			SAFE_RELEASE(_baker);
		}
		_ctrl.SetPos(0);
	}
	else if(ctrlID==IDC_BUTTON_BAKEENV) //环境贴图
	{
		GuiData_OverallMap * dataMap = (GuiData_OverallMap *)actor->FindData("overallmap");
		if(dataMap)
			_flds.assign(dataMap->fldSels.begin(),dataMap->fldSels.end());
		BakeMultiEnvTex();
	}
	else if(ctrlID==IDC_BUTTON_GENNAVMESH)
	{
		GuiData_OverallMap * dataMap = (GuiData_OverallMap *)actor->FindData("overallmap");
		if(dataMap)
			_flds.assign(dataMap->fldSels.begin(),dataMap->fldSels.end());
		GenNavMesh();
	}
	else if(ctrlID==IDC_BUTTON_BAKEMINIMAP) //MiniMap
	{
		GuiData_OverallMap * dataMap = (GuiData_OverallMap *)actor->FindData("overallmap");
		if(dataMap)
			_flds=dataMap->fldSels;
		BakeMiniMap();
	}
	else if(ctrlID==IDC_BUTTON_BAKEOUTLINEMAP) //MiniMap
	{
		GuiData_OverallMap * dataMap = (GuiData_OverallMap *)actor->FindData("overallmap");
		if(dataMap)
			_flds=dataMap->fldSels;
		BakeOutlineMap();
	}

	if (ctrlID==IDC_LOW)
		_bq=BQ_Low;
	if (ctrlID==IDC_MEDIUM)
		_bq=BQ_Medium;
	if (ctrlID==IDC_HI)
		_bq=BQ_Hi;

	return CMapUtil::OnCommand(ctrlID,code,lParam,actor);
}
void CBakeUtil::RegisterMode()
{
	AddMode("bake",0);
}



