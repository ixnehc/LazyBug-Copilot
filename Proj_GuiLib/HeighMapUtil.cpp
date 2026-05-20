#include "stdh.h"

#include "RenderSystem/ITools.h"
#include "RenderSystem/IRenderPort.h"

#include ".\heighmaputil.h"
#include "resource.h"
#include "GuiData.h"
#include "GuiData_OverallMap.h"
#include "ModBlockBack.h"
#include "FileSystem/IMapFile.h"
#include "RenderSystem/IRenderSystem.h"
#include "WorldSystem/ITrrn.h"

IMPLEMENT_TOOL_CLASS(CHeighMapUtil)
 
CHeighMapUtil::CHeighMapUtil(void)
{
	_hasHeighMap = FALSE;
}
CHeighMapUtil::~CHeighMapUtil(void)
{
	SAFE_DELETE(_imageWnd);
}
BOOL CHeighMapUtil::OnCommand(DWORD ctrlID ,DWORD code,LPARAM lParam,CGeActor * actor)
{
	if(ctrlID == IDC_BT_ASSIGNHMAP)
	{
		if(_hasHeighMap)
			_AssignHeightMap(actor,TRUE);
	}
	else if(ctrlID == IDC_BT_HEIGHTMAP)
	{
		extern BOOL FD_BrowseImage(BOOL bOpen,CxImage *image);
		CxImage * image = _imageWnd->GetImage();
		if(FD_BrowseImage(TRUE,image))
		{
			_imageWnd->InvalidateRect(NULL);
			_hasHeighMap = TRUE;
		}
	}

	return CMapUtil::OnCommand(ctrlID,code,lParam,actor);
}
void CHeighMapUtil::_AssignHeightMap(CGeActor *actor,BOOL bOverall)
{
	CxImage * image = _imageWnd->GetImage();
	int w = image->GetWidth();
	int h = image->GetHeight();

	if (w*h<=0)
		return;

	GuiData_OverallMap * dataMap = (GuiData_OverallMap *)actor->FindData("overallmap");
	if(dataMap->fldSels.size()==0)
		return;
// 	dataMap->fldSels.resize(1);
	
	GuiData_System * dataSys = (GuiData_System *)actor->FindData("system");
	IMapFile * pMF = dataSys->mf;
	i_math::recti rcFld,rcMap;
	rcFld = pMF->GetFieldRect();
	rcMap = pMF->GetRect();
	int wFld = rcMap.getWidth()/rcFld.getWidth();
	int hFld  =rcMap.getHeight()/rcFld.getHeight();

	if (bOverall)
	{
		w/=rcFld.getWidth();
		h/=rcFld.getHeight();
	}

	for (int k=0;k<dataMap->fldSels.size();k++)
	{
		i_math::pos2di pos = dataMap->fldSels[k];
		i_math::recti rc;
		rc.UpperLeftCorner.x = pos.x*wFld;
		rc.UpperLeftCorner.y = pos.y*hFld;
		rc.LowerRightCorner.x = (pos.x+1)*wFld;
		rc.LowerRightCorner.y = (pos.y+1)*hFld;

		i_math::recti rcInner;
		rcInner.UpperLeftCorner.x = rc.UpperLeftCorner.x + 2;
		rcInner.UpperLeftCorner.y = rc.UpperLeftCorner.y + 2;
		rcInner.LowerRightCorner.x = rc.LowerRightCorner.x - 2;
		rcInner.LowerRightCorner.y = rc.LowerRightCorner.y - 2;

		std::vector<i_math::pos2di> blocks;
		for(int i = rc.UpperLeftCorner.x;i<=rc.LowerRightCorner.x;i++)
			for(int j = rc.UpperLeftCorner.Y;j<=rc.LowerRightCorner.Y;j++)
				blocks.push_back(i_math::pos2di(i,j));

		i_math::vector3df vCenter;
		vCenter.x = float( rc.getCenter().x*BLOCK_LENGTH);
		vCenter.z = float( rc.getCenter().y*BLOCK_LENGTH);
		vCenter.y= 0.0f;


		TrrnSeedMap seedMap;
		TrrnSeedMapArg seedMapArg;
		seedMapArg.purpose = TrrnSeedMapArg::Purpose_AddHt;



		const int unitGrid = 8;
		rc.UpperLeftCorner *= unitGrid;
		rc.LowerRightCorner *= unitGrid;

		i_math::pos2di posOrg = rc.getCenter();
		seedMap.lvl = 3;

		int cx = w/2;
		int cy = h/2;
		for(int i = 0;i<w;i++)
		for(int j = 0;j<h;j++)
		{
			RGBQUAD color;
			if (!bOverall)
				color= image->GetPixelColor(i,j);
			else
				color= image->GetPixelColor((pos.x-rcFld.Left())*w+i,(pos.y-rcFld.Top())*h+j);

			int x = i - cx;
			int y = j - cy;
			x += posOrg.x;
			y += posOrg.y;
			
			TrrnSeedMap::SeedPoint point;
			point.x = x;
			point.y = y;
			
			i_math::pos2di block(x/8,y/8);
			if(!rcInner.isPointInside(block)&&!_IsWriteable(block,pMF))
				continue;

			point.wt = float(color.rgbGreen)/255.0f;
			point.flag = TrrnSeedMap::SeedPointF_None;
			seedMap.points.push_back(point);
		}

		GuiData_Trrn * dataTrrn = (GuiData_Trrn *)actor->FindData("terrain");
		ITrrnMapEditor * editor = dataTrrn->GetTrrnMapEditor();
		ITrrnMap * map = dataTrrn->GetTrrnMap();

		map->SetCenter(vCenter);//把地图挪到要添加高度图的这个field的正中心

		float minH = _edit[0].GetFVal();
		float maxH = _edit[1].GetFVal();
		
		editor->BeginModify();
		editor->AddHeight(seedMap,FALSE,maxH,minH);
		editor->EndModify();

		CModManager * mgrMod = actor->GetModMgr();
		if (mgrMod)
		{
			if (FALSE)
			{
				CModBlockBack * mod = new CModBlockBack(GetView());
				
				mod->BackupBlocks(blocks.data(),blocks.size());
				map->SaveModified();
				Mod_New(mgrMod,(CModBase *&)mod);
			}
			else
			{
				map->SaveModified();
				mgrMod->Clear();
			}
		}

		//更新camera的位置,保持朝向不变,将观察目标移到这个field的中心点
		if (TRUE)
		{
			GuiData_Camera * cameras = (GuiData_Camera *)actor->FindData("cameras");
			ICamera * camera = cameras->cams[Camera_Perspective];

			i_math::vector3df posEye,posAt;
			camera->GetEyeLookAt(posAt);
			camera->GetEyePos(posEye);
			i_math::vector3df vHit;
			if (!editor->GetHitPos(vCenter.x,vCenter.z,TRUE,vHit))
			{
				vHit=vCenter;
				vHit.y=posAt.y;
			}
			i_math::vector3df vDir=posAt-posEye;
			posAt=vHit;
			posEye=vHit-vDir;
			camera->SetPosTarget(posEye,posAt,i_math::vector3df(0.0f,1.0f,0.0f));
		}
	}

}
BOOL CHeighMapUtil::_IsWriteable(i_math::pos2di & block,IMapFile * pMF)
{
	i_math::recti rc;
	rc.UpperLeftCorner.x = block.x - 2;
	rc.UpperLeftCorner.y = block.y - 2;
	rc.LowerRightCorner.x = block.x + 2;
	rc.LowerRightCorner.y = block.y + 2;

	for(int i = rc.UpperLeftCorner.x;i<=rc.LowerRightCorner.x;i++)
		for(int j = rc.UpperLeftCorner.Y;j<=rc.LowerRightCorner.Y;j++)
		{
			i_math::pos2di curBlock(i,j);
			if(!pMF->BlockCheckedOut(curBlock))
				return FALSE;
		}

	return TRUE;
}
BOOL CHeighMapUtil::InitDlg(CWnd * pParent)
{
	return DefDialog(pParent,IDD_DIALOG_HEIGHMAP);
}
void CHeighMapUtil::OnInitDlg(CGeActor * actor)
{
	_imageWnd = new CxImageWnd();
	_imageWnd->SubclassDlgItem(IDC_STATIC_HMAPVIEW,&m_panel);
	
	_edit[0].SubclassDlgItem(IDC_EDIT_MINHEIGHT,&m_panel);
	_spinner[0].SubclassDlgItem(IDC_SPIN_MINHEIGHT,&m_panel);
	_slider[0].SubclassDlgItem(IDC_SLIDER_MINHEIGHT,&m_panel);
	
	_edit[1].SubclassDlgItem(IDC_EDIT_MAXHEIGHT,&m_panel);
	_spinner[1].SubclassDlgItem(IDC_SPIN_MAXHEIGHT,&m_panel);
	_slider[1].SubclassDlgItem(IDC_SLIDER_MAXHEIGHT,&m_panel);

	_spinner[0].LinkTo(_edit);
	_slider[0].LinkTo(_edit);

	_spinner[1].LinkTo(_edit+1);
	_slider[1].LinkTo(_edit+1);

	_edit[0].SetLimits(-1000.0f,1000.0f);
	_edit[1].SetLimits(-1000.0f,1000.0f);

	_edit[0].SetLimitMax(_edit+1,true);
	_edit[1].SetLimitMin(_edit,false);
	
	_edit[1].SetValue(_value_max);
	_edit[0].SetValue(_value_min);
}

void CHeighMapUtil::RegisterMode()
{
	AddMode("Heightmap",0);
}
void CHeighMapUtil::Load(CDataPacket &dp)
{
	CMapUtil::Load(dp);
}
void CHeighMapUtil::Save(CDataPacket &dp)
{
	_value_min = _edit[0].GetFVal();
	_value_max = _edit[1].GetFVal();

	CMapUtil::Save(dp);
}








