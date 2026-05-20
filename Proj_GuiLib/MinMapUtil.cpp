
#include "stdh.h"

#include "MinMapUtil.h"

#include "graphicsgraph.h"

#include "resource.h"

#include "GuiData.h"

#include <fstream>

#include "FileSystem/IMapFile.h"

#include "maptooldefines.h"

IMPLEMENT_TOOL_CLASS(CMinMapUtil)

CMinMapUtil::CMinMapUtil(void)
{
}
CMinMapUtil::~CMinMapUtil(void)
{
}
void CMinMapUtil::RegisterAgent()
{
	CMapUtil::RegisterAgent();
}

void CMinMapUtil::RegisterMode()
{
	AddMode("Draft",0);
}
BOOL CMinMapUtil::OnCommand(DWORD ctrlID ,DWORD code,LPARAM lParam,CGeActor * actor)
{
	switch(ctrlID)
	{
	case IDC_BUTTON_FILEBROWSE:
		{
			CFileDialog fileBrowse(TRUE,"jpg",NULL,
									OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
									"jpg|*.jpg|bmp|*.bmp|");
			
			if(IDOK==fileBrowse.DoModal()){
				CString path = fileBrowse.GetPathName();
				_SetMapData(path.GetBuffer());
			}
			break;
		}
	default: break;
	}

	return TRUE;
}
void CMinMapUtil::_SetMapData(const char * fileName)
{
	if(!m_actor)
		return;

	GuiData_System * dataSys = (GuiData_System *)m_actor->FindData("system");
	if(!dataSys||!dataSys->mf)
		return;
	
	std::ifstream ifs;
	ifs.open(fileName,std::ios_base::in|std::ios_base::binary);

	if(ifs.is_open()){
		ifs.seekg(0,std::ios_base::end);
		size_t sz = ifs.tellg();
		if(sz>0){
			std::vector<BYTE> data(sz);
			ifs.seekg(0,std::ios_base::beg);
			ifs.read((char *)(&(data[0])),sz);
			dataSys->mf->SaveUnique(MINMAP_DATA,&(data[0]),sz);
			_RebuildImage(&(data[0]),sz);
		}
	}
}
void CMinMapUtil::_RebuildImage(void * data,DWORD sz)
{
	GuiData_Map * dataMap = (GuiData_Map *)m_actor->FindData("map");
	if(!dataMap)
		return;

	Image * pImage = (Image *)(dataMap->pImage);
	SAFE_DELETE(pImage);//销毁旧的数据

	dataMap->pImage = RebuildImage(data,sz);
}

BOOL CMinMapUtil::InitDlg(CWnd * pParent)
{
	return DefDialog(pParent,IDD_DIALOG_TOOLMINMAP);
}

BOOL CMinMapUtil::BeginParam(CWnd * pParent,int mode,CGeActor * actor,int level,const char * nameView)
{
	if(FALSE==CToolBase::BeginParam(pParent,mode,actor,level,nameView))
		return FALSE;

	return TRUE;
}









