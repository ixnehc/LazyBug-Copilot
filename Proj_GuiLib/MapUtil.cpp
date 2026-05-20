#include "stdh.h"

#include ".\maputil.h"

#include "GuiData.h"
#include "GuiData_OverallMap.h"

#include "FileSystem/IMapFile.h"

#include "WorldSystem/IEntitySystem.h"

#include "SscBase.h"

#include "resource.h"

#include "GdiplusHeaders.h"

#include "graphicsgraph.h"

CMapUtil::CMapUtil(void)
{
}

CMapUtil::~CMapUtil(void)
{
}

const char * CMapUtil::GetTypeName()
{
	return "MapUtil";
}
DWORD CMapUtil::GetType()
{
	return TOOL_MAPCONTROL;
}
void CMapUtil::RegisterAgent()
{
	_agentMapUitl.SetTool(this);
	_agentDraw.SetSelectMode(SelectMode());
	AddAgent(_agentMapUitl);
	AddAgent(_agentMinMap);
	AddAgent(_agentDraw);
}
BOOL CMapUtil::InitDlg(CWnd * pParent)
{
	return DefDialog(pParent,IDD_TOOL_SSCMAP);
}
BOOL CMapUtil::BeginParam(CWnd * pParent,int mode,CGeActor * actor,int level,const char * nameView)
{
	if(FALSE == CToolBase::BeginParam(pParent,mode,actor,level,nameView))
		return FALSE;
	
	GuiData_OverallMap * dataMap = (GuiData_OverallMap *)actor->FindData("overallmap");
	if(dataMap&&SelectMode()==CGuiAgent_Draw2D::Select_Sing&&dataMap->fldSels.size()>1)
	{
		dataMap->fldSels.resize(1);
		CGeView * view = GetView();
		if(view)
			view->Draw();
	}

	return TRUE;
}
BOOL CMapUtil::OnCommand(DWORD ctrlID ,DWORD code,LPARAM lParam,CGeActor * actor)
{
	GuiData_System  * dataSys = (GuiData_System *)actor->FindData("system");
	GuiData_OverallMap * dataMap = (GuiData_OverallMap *)actor->FindData("overallmap");

	if(!dataSys||!dataMap)
		return TRUE;

	IMapFile * pMF = dataSys->mf;

	i_math::recti rcFld;
	for(int i = 0;i<dataMap->fldSels.size();i++)
		rcFld.merge(dataMap->fldSels[i]);

	switch(ctrlID)
	{
		case IDC_BT_CHECKIN:
		case IDC_BT_CHECKIN_KEEPOUT:
		case IDC_BT_CHECKOUT:
		case IDC_BT_GETLATESTVERSION:
		{
			if (g_ssGuiLib.ssc)
			{
				std::string fileMapInfo = pMF->GetInfoSscPath();

				DWORD nPathes;
				const char **pathes=pMF->PreSscOp(rcFld,nPathes);
				switch(ctrlID)
				{
					case IDC_BT_CHECKIN_KEEPOUT:
					case IDC_BT_CHECKIN:
					{
						long flag=(ctrlID==IDC_BT_CHECKIN)?0:131072;
						if(!fileMapInfo.empty())
							g_ssGuiLib.ssc->CheckIn(fileMapInfo.c_str(),flag);

						for (int i=0;i<nPathes;i++)
							g_ssGuiLib.ssc->CheckIn(pathes[i],flag);
						break;
					}
					case IDC_BT_CHECKOUT:
					{
						g_ssGuiLib.ssc->CheckOut(pathes,nPathes);
						break;
					}
					case IDC_BT_GETLATESTVERSION:
					{
						g_ssGuiLib.ssc->GetLatestVersion(pathes,nPathes);
						break;
					}
				}

				pMF->PostSscOp(rcFld);

				switch(ctrlID)
				{
					case IDC_BT_CHECKOUT:
					case IDC_BT_GETLATESTVERSION:
					{
						dataSys->pES->ReloadAllMap();
						break;
					}
				}
			}
			break;
		}
	}

	return TRUE ;
}
//////////////////////////////////////////////////////////////////////////
BOOL CMapUtil::_CGuiAgent_MapUtil::OnCommand(DWORD idCmd)
{
	CGeActor * actor = _owner->GetActor();
	if(actor)
		_owner->OnCommand(idCmd,0,NULL,actor);
	return TRUE;
}
BOOL CMapUtil::_CGuiAgent_MapUtil::OnRButtonClick(int x,int y,DWORD flag)
{
	_AddMenu("Check In",IDC_BT_CHECKIN);
	_AddMenu("Check In(Keep CheckOut)",IDC_BT_CHECKIN_KEEPOUT);
	_AddMenu("Check Out",IDC_BT_CHECKOUT);
	_AddMenu("Get Latest Version",IDC_BT_GETLATESTVERSION);
	_AddMenuSep();
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
BOOL CMapUtil::_CGuiAgentMinMap::OnDraw()
{
	GuiData_OverallMap * dataMap = (GuiData_OverallMap *)FindData("overallmap");
	if(!dataMap)
		return TRUE;

	dataMap->bDrawImage = FALSE; //初始化状态

	GraphicsGraph * gg = GetGG();
//	gg->ClearBg(0x888888);

	dataMap->bDrawImage = _Draw();

	return TRUE;
}
BOOL CMapUtil::_CGuiAgentMinMap::_Draw()
{
	i_math::recti rcImage;
	GraphicsGraph * gg = GetGG();
	gg->ClearBg(0x888888);


	Graphics * grp = gg->GetGraphics();
	GuiData_OverallMap * dataMap = (GuiData_OverallMap *)FindData("overallmap");
	GuiData_System * dataSys = (GuiData_System *)FindData("system");

	if((!dataSys)||(!dataSys->mf))
		return FALSE;


	if (TRUE)//Draw the minimap&outline map
	{
		i_math::pos2df off,scaleOld;
		gg->GetTranform(off,scaleOld);
		i_math::pos2df scale=scaleOld;
		scale.y=fabsf(scale.y);
		gg->Transform(off,scale);

		IMapFile *mf=dataSys->mf;
		if (FALSE)
		if (mf)
		{
			i_math::recti rcFld=dataSys->mf->GetFieldRect();
			DWORD wFld=dataSys->mf->GetFieldWidth()*BLOCK_LENGTH;
			for (int i=rcFld.Left();i<rcFld.Right();i++)
			for (int j=rcFld.Top();j<rcFld.Bottom();j++)
			{
				DWORD w,h;
				DWORD *data=dataMap->GetFieldRawMiniMap(i_math::pos2di(i,j),w,h);
				if (data)
				{
					int y=-j-1;
					i_math::recti rcDest;
					rcDest.set(i*wFld,y*wFld,(i+1)*wFld,(y+1)*wFld);

					gg->DrawImageData(rcDest.convert<float>(),data,w,h);
				}

				data=dataMap->GetFieldOutlineMap(i_math::pos2di(i,j),w,h);
				if (data)
				{
					int y=-j-1;
					i_math::recti rcDest;
					rcDest.set(i*wFld,y*wFld,(i+1)*wFld,(y+1)*wFld);

					gg->DrawImageData(rcDest.convert<float>(),data,w,h);
				}
			}
		}
		gg->Transform(off,scaleOld);

	}
	
	if(!dataMap->pImage)
		return FALSE;

	i_math::recti rcMap;
	if(TRUE){
		rcMap = dataSys->mf->GetRect();
		rcMap *= BLOCK_LENGTH;
	}
	
	Image * pImage = (Image *)(dataMap->pImage);

	int w = pImage->GetWidth();
	int h = pImage->GetHeight();

	RectF rcSrc(0,0,float(w),float(h));
	RectF rcDst;
	rcDst.X = float(rcMap.UpperLeftCorner.x);
	rcDst.Y = float(rcMap.UpperLeftCorner.y);
	rcDst.Width  = float(rcMap.getWidth());
	rcDst.Height = float(rcMap.getHeight());

	if(Ok != grp->DrawImage(pImage,rcDst,0,0,float(w),float(h),UnitPixel))
		return FALSE;
	
	return TRUE;
}


