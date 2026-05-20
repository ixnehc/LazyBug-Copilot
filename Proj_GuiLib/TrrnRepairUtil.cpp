#include "stdh.h"
#include "TrrnRepairUtil.h"
#include "resource.h"
#include "GuiData.h"
#include "GuiData_OverallMap.h"
#include "FileSystem/IMapFile.h"
#include "WorldSystem/IEntitySystem.h"
#include "WorldSystem/IBake.h"
#include "WorldSystem/ITrrn.h"
#include "ModBlockBack.h"

#include "stringparser/stringparser.h"
#include "ImageBase.h"

IMPLEMENT_TOOL_CLASS(CTrrnRepairUtil)

CTrrnRepairUtil::CTrrnRepairUtil(void)
{
}

CTrrnRepairUtil::~CTrrnRepairUtil(void)
{
}
BOOL CTrrnRepairUtil::InitDlg(CWnd * pParent)
{
	return DefDialog(pParent,IDD_DIALOG_TOOLREPAIRTRRN);
}

void CTrrnRepairUtil::OnInitDlg(CGeActor * actor)
{
}

BOOL CTrrnRepairUtil::OnCommand(DWORD ctrlID ,DWORD code,LPARAM lParam,CGeActor * actor)
{
	if(ctrlID==IDC_BUTTON_REPAIR)
	{
		GuiData_OverallMap * dataMap = (GuiData_OverallMap *)actor->FindData("overallmap");
		GuiData_System * dataSys = (GuiData_System *)actor->FindData("system");
		GuiData_Trrn * dataTrrn = (GuiData_Trrn *)actor->FindData("terrain");
		ITrrnMapEditor * editor = dataTrrn->GetTrrnMapEditor();

		DWORD w=dataSys->mf->GetRect().getWidth()/dataSys->mf->GetFieldRect().getWidth();

		i_math::vector3df centerOld;
		dataTrrn->GetTrrnMap()->GetCenter(centerOld);

		editor->BeginUpdateDitherColor();

		for (int i=0;i<dataMap->fldSels.size();i++)
		{
			i_math::pos2di ptFld=dataMap->fldSels[i];
			i_math::recti rcBlk;
			rcBlk.set(ptFld.x,ptFld.y,ptFld.x+1,ptFld.y+1);
			rcBlk*=w;

			i_math::vector3df center;
			center.set((float)(rcBlk.getCenter().x*BLOCK_LENGTH),0,(float)(rcBlk.getCenter().y*BLOCK_LENGTH));
			dataTrrn->GetTrrnMap()->SetCenter(center);

			editor->UpdateDitherColor(rcBlk);
		}

		editor->EndUpdateDitherColor();


		dataTrrn->GetTrrnMap()->SetCenter(centerOld);

		dataTrrn->GetTrrnMap()->SaveModified();

		dataTrrn->GetTrrnMap()->ReloadAll();
	}

	return CMapUtil::OnCommand(ctrlID,code,lParam,actor);
}
void CTrrnRepairUtil::RegisterMode()
{
	AddMode("Repair Trrn ",0);
}



