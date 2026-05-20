#include "stdh.h"
#include "BrushLibSscBtn.h"
#include ".\GuiLib.h"

#include "WorldSystem/IWorldSystem.h"
#include "RenderSystem/IRenderSystem.h"

#include "RefResDlg.h"




#pragma warning(disable:4018)

#define ID_REFRES 1000

BEGIN_MESSAGE_MAP(CBrushLibSscBtn,CSscBtn)
	ON_COMMAND(ID_REFRES,OnRefRes)
END_MESSAGE_MAP()


void CBrushLibSscBtn::_OnCustomizeMenu(CMenu *menu)
{
	int idx=menu->GetMenuItemCount();

	menu->InsertMenu(idx++, MF_ENABLED | MF_SEPARATOR, 0, _T(""));
	menu->InsertMenu(idx++,MF_ENABLED|MF_STRING,ID_REFRES, _T("引用资源..."));
}

void CBrushLibSscBtn::OnRefRes()
{
	if (!_path.empty())
	{
		std::vector<std::string>buf;
		g_ssGuiLib.pWS->GetBrushLibRes(_path.c_str(),buf);

		std::string pathResRoot=g_ssGuiLib.pRS->GetPath(Path_Res);
		pathResRoot+="\\";
		for (int i=0;i<buf.size();i++)
			buf[i]=pathResRoot+buf[i];

		CRefResDlg dlg;
		dlg.Set(buf);
		dlg.DoModal();

	}
}
