#include "stdh.h"

#include "RenderSystem/ITools.h"


#include "RepairUtil.h"

#include "resource.h"

#include "GuiData.h"
#include "GuiData_OverallMap.h"

#include "GuiData_Water.h"

#include "FileSystem/IMapFile.h"

#include "WorldSystem/IEntitySystem.h"

#include "ModBlockBack.h"

#include "GuiData_forest.h"

#include "WorldSystem/IWater.h"

#include "GuiData_vegetable.h"

#include "RenderSystem/IRenderSystem.h"

IMPLEMENT_TOOL_CLASS(CRepairUtil)

CRepairUtil::CRepairUtil(void)
{
}

CRepairUtil::~CRepairUtil(void)
{
}

BOOL CRepairUtil::InitDlg(CWnd * pParent)
{
	return DefDialog(pParent,IDD_DIALOG_TOOLREPAIR);
}

void CRepairUtil::OnInitDlg(CGeActor * actor)
{
	CButton * btn = (CButton *)(m_panel.GetDlgItem(IDC_CHECK_TREPAIR_TREE));
	if(btn)
		btn->SetCheck(TRUE);
	
	btn = (CButton *)(m_panel.GetDlgItem(IDC_CHECK_TREPAIR_WATER));
	if(btn)
		btn->SetCheck(TRUE);

	btn = (CButton *)(m_panel.GetDlgItem(IDC_CHECK_TREPAIR_VEGETABLE));
	if(btn)
		btn->SetCheck(TRUE);
}

BOOL CRepairUtil::OnCommand(DWORD ctrlID ,DWORD code,LPARAM lParam,CGeActor * actor)
{
	switch(ctrlID){
		case IDC_BUTTON_TREPAIR:{
			_RepairTree(actor,FALSE);		//修复树数据
			_RepairWater(actor,FALSE);		//修复水面
			_RepairVegetable(actor,FALSE);	//修复植被
			_MoveHome(actor);				//将世界的中心移到原来的位置
			break;
		}
		case IDC_BUTTON_CLEAR:{
			if (IDYES == MessageBox(NULL, _T("Are you sure to clear the selected items?"), _T("Repair Tool"), MB_YESNO))
			{
				_RepairWater(actor,TRUE);		//清除水面
				_RepairTree(actor,TRUE);		//清除树
				_RepairVegetable(actor,TRUE);	//清除植被
				_MoveHome(actor);				//将世界的中心移到原来的位置
			}
			break;
		}
		default: break;
	}
	return CMapUtil::OnCommand(ctrlID,code,lParam,actor);
}

BOOL CRepairUtil::_MoveTo(CGeActor * actor,i_math::pos2di &ptF)
{
	_blks.clear();
	GuiData_OverallMap * dataMap = (GuiData_OverallMap *)actor->FindData("overallmap");	
	GuiData_System * dataSys = (GuiData_System *)actor->FindData("system");
	
	if(!dataSys||!dataMap)
		return FALSE;

	IMapFile * mf = dataSys->mf;
	i_math::recti rcFld = mf->GetFieldRect();
	if(0==rcFld.getArea())
		return FALSE;

	i_math::recti rcBlk = mf->GetRect();	
	int w = rcBlk.getWidth()/rcFld.getWidth();
	int h = rcBlk.getHeight()/rcFld.getHeight();

	i_math::vector3df center;
	center.x = (ptF.x+0.5f)*w*BLOCK_LENGTH;
	center.y = 10.0f;
	center.z = (ptF.y+0.5f)*h*BLOCK_LENGTH;

	if(!dataSys->pES->Locate(center))
		return FALSE;


	if(mf->FieldCheckedOut(ptF)){
		i_math::pos2di ptBlk;
		ptBlk.x = ptF.x*w;
		for(int i = 0;i<w;i++){
			ptBlk.y = ptF.y*h;
			for(int j = 0;j<h;j++){
				_blks.push_back(ptBlk);
				ptBlk.y++;
			}
			ptBlk.x++;
		}

		//世界坐标 米为单位
		_blkRC.UpperLeftCorner.x = ptF.x*w*BLOCK_LENGTH;
		_blkRC.UpperLeftCorner.y = ptF.y*h*BLOCK_LENGTH;
		_blkRC.LowerRightCorner.x = (ptF.x+1)*w*BLOCK_LENGTH;
		_blkRC.LowerRightCorner.y = (ptF.y+1)*h*BLOCK_LENGTH;
		
		return TRUE;
	}	

	return FALSE;
}

void CRepairUtil::_MoveHome(CGeActor *actor)
{
	GuiData_System * dataSys = (GuiData_System *)actor->FindData("system");
	GuiData_Camera * dataCameras = (GuiData_Camera *)actor->FindData("cameras");
	ICamera * camera = dataCameras->cams[Camera_Perspective];
	i_math::vector3df eyePos;
	camera->GetEyePos(eyePos);
	if(dataSys->pES)
		dataSys->pES->Locate(eyePos);
}

void CRepairUtil::_RepairTree(CGeActor * actor,BOOL bRemove)
{
	GuiData_OverallMap * dataMap = (GuiData_OverallMap *)actor->FindData("overallmap");	
	if(!dataMap)
		return;

	BOOL bRepair = TRUE;
	CButton * btn = (CButton *)(m_panel.GetDlgItem(IDC_CHECK_TREPAIR_TREE));
 	if(btn)
		bRepair = btn->GetCheck();
	
	IForestEditor * editor = NULL;
	GuiData_Forest * dataTree = (GuiData_Forest *)actor->FindData("forest");
	if(dataTree)
		editor = dataTree->GetEditor();
	
	if(editor&&bRepair){
		for(int i = 0;i<dataMap->fldSels.size();i++){
			i_math::pos2di &ptF = dataMap->fldSels[i];
			if(_MoveTo(actor,ptF)){
				HMapObj * hObjs = NULL;
				DWORD count = 0;
				hObjs = editor->Enum(_blkRC,count);
				for(int i = 0;i<count;i++){
					if(bRemove)
						editor->Delete(hObjs[i]);
					else
						editor->CheckVisible(hObjs[i]);
				}
			}
		}
		editor->Save();
	}
}

void CRepairUtil::_RepairVegetable(CGeActor * actor,BOOL bRemove)
{
	GuiData_OverallMap * dataMap = (GuiData_OverallMap *)actor->FindData("overallmap");	
	if(!dataMap)
		return;

	BOOL bRepair = TRUE;
	CButton * btn = (CButton *)(m_panel.GetDlgItem(IDC_CHECK_TREPAIR_VEGETABLE));
	if(btn)
		bRepair = btn->GetCheck();

	ISpgEditor * editor = NULL;
	GuiData_Vegetable * data = (GuiData_Vegetable *)actor->FindData("vegetable");
	if(data)
		editor = data->GetEditor();

	if(editor&&bRepair){
		for(int i = 0;i<dataMap->fldSels.size();i++){
			i_math::pos2di &ptF = dataMap->fldSels[i];
			if(_MoveTo(actor,ptF)){
				HMapObj * hObjs = NULL;
				DWORD count = 0;
				hObjs = editor->Enum(_blkRC,count);
				for(int i = 0;i<count;i++){
					if(bRemove)
						editor->Delete(hObjs[i]);
					else
						editor->CheckVisible(hObjs[i]);
				}
			}
			editor->Save();
		}
	}
}

void CRepairUtil::_RepairWater(CGeActor * actor,BOOL bRemove)
{
	GuiData_OverallMap * dataMap = (GuiData_OverallMap *)actor->FindData("overallmap");	
	if(!dataMap)
		return;

	BOOL bRepair = TRUE;
	CButton * btn = (CButton *)(m_panel.GetDlgItem(IDC_CHECK_TREPAIR_WATER));
	if(btn)
		bRepair = btn->GetCheck();

	IWaterEditor * editor = NULL;
	GuiData_Water * dataWater = (GuiData_Water *)actor->FindData("water");
	if(dataWater)
		editor = dataWater->GetWaterEditor();

	if(editor&&bRepair)
	for(int i = 0;i<dataMap->fldSels.size();i++){
		i_math::pos2di &ptF = dataMap->fldSels[i];
		if(_MoveTo(actor,ptF)){
			HMapObj * hObjs = NULL;
			DWORD count = 0;
			hObjs = editor->Enum(_blkRC,count);
			for(int i = 0;i<count;i++){
				if(bRemove)
					editor->Delete(hObjs[i]);
				else
					editor->CheckVisible(hObjs[i]);
			}
		}
		editor->Save();
	}
}

void CRepairUtil::RegisterMode()
{
	AddMode("Repair Tool",0);
}






