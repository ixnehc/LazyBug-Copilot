
#include "stdh.h"

#include "DraftUtil.h"

#include "graphicsgraph.h"

#include "resource.h"

#include "GuiData.h"
#include "GuiData_OverallMap.h"

#include <fstream>

#include "FileSystem/IMapFile.h"

#include "FileSystem/IFileSystem.h"

#include "WorldSystem/IWorldSystem.h"

#include "maptooldefines.h"

#include "resdata/TexData.h"

IMPLEMENT_TOOL_CLASS(CDraftUtil)

CDraftUtil::CDraftUtil(void)
{
}
CDraftUtil::~CDraftUtil(void)
{
}
void CDraftUtil::RegisterAgent()
{
	CMapUtil::RegisterAgent();
}

void CDraftUtil::RegisterMode()
{
	AddMode("Draft",0);
}
BOOL CDraftUtil::OnCommand(DWORD ctrlID ,DWORD code,LPARAM lParam,CGeActor * actor)
{
	switch(ctrlID)
	{
	case IDC_BUTTON_FILEBROWSE:
		{
			CFileDialog fileBrowse(TRUE, _T("jpg"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("jpg|*.jpg|bmp|*.bmp|"));

			if(IDOK==fileBrowse.DoModal())
			{
				CString path = fileBrowse.GetPathName();
				_SetMapData(toMBCS(path.GetBuffer()));
			}
			break;
		}
	default: break;
	}

	return CMapUtil::OnCommand(ctrlID,code,lParam,actor);
}
void CDraftUtil::_SetMapData(const char * fileName)
{
	if(!m_actor)
		return;

	GuiData_System * dataSys = (GuiData_System *)m_actor->FindData("system");
	if(!dataSys||!dataSys->mf)
		return;
	
	IFileSystem *pFS = dataSys->pWS->GetFS();
	if(!pFS)
		return;
	
	IFile * pFile = pFS->OpenFile(fileName,FileAccessMode_Read);
	if(pFile)
	{
		TexData::TexDataType tp=TexData::Tex_UNKNOWN;
		if (CheckFileSuffix(fileName,"dds"))
			tp=TexData::Tex_DDS;
		if (CheckFileSuffix(fileName,"jpg"))
			tp=TexData::Tex_JPG;
		if (CheckFileSuffix(fileName,"tga"))
			tp=TexData::Tex_TGA;
		if (CheckFileSuffix(fileName,"bmp"))
			tp=TexData::Tex_BMP;

		int sz = pFile->GetSize();
		if(sz>0)
		{
			std::vector<BYTE> data(sz+sizeof(DWORD));
			BYTE *pData=data.data();
			*(DWORD*)pData=tp;
			pData+=sizeof(DWORD);
			pFile->Read(pData,sz);
			dataSys->mf->SaveUnique(TRRNDRAFT_DATA,&(data[0]),data.size());
			_RebuildImage(pData,sz);
		}
		pFile->Close();
	}
// 	std::ifstream ifs;
// 	ifs.open(fileName,std::ios_base::in|std::ios_base::binary);
// 	if(ifs.is_open()){
// 		ifs.seekg(0,std::ios_base::end);
// 		size_t sz = ifs.tellg();
// 		if(sz>0){
// 			std::vector<BYTE> data(sz);
// 			ifs.seekg(0,std::ios_base::beg);
// 			ifs.read((char *)(&(data[0])),sz);
// 		}
// 	}
}
void CDraftUtil::_RebuildImage(void * data,DWORD sz)
{
	GuiData_OverallMap * dataMap = (GuiData_OverallMap *)m_actor->FindData("overallmap");
	if(!dataMap)
		return;

	Image * pImage = (Image *)(dataMap->pImage);
	SAFE_DELETE(pImage);//销毁旧的数据

	dataMap->pImage = RebuildImage(data,sz);
}

BOOL CDraftUtil::InitDlg(CWnd * pParent)
{
	return DefDialog(pParent,IDD_DIALOG_TOOLMINMAP);
}

BOOL CDraftUtil::BeginParam(CWnd * pParent,int mode,CGeActor * actor,int level,const char * nameView)
{
	if(FALSE==CToolBase::BeginParam(pParent,mode,actor,level,nameView))
		return FALSE;

	return TRUE;
}









