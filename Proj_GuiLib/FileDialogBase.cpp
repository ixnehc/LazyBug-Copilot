/********************************************************************
	created:	2007/2/9   11:29
	filename: 	e:\IxEngine\Proj_GuiLib\FileDialogBase.cpp
	author:		cxi
	
	purpose:	useful function of FileDialog
*********************************************************************/
#include "stdh.h"

#include "WorldSystem/IAssetShell.h"

#include "FileDialogBase.h"

#include <vector>
#include <string>

#include "stringparser/stringparser.h"
#include "ximage.h"
#include "../Common/Log/LogFile.h"

#include "ResSelectorDlg.h"
#include "ResSelectDialog.h"
#include "resdata/ResDataDefines.h"

const char * FD_BrowseTex(BOOL bOpen,const char * pathRoot)
{
	static CFileDialog dlg( TRUE, _T("tex"), _T(""),
		0,_T("Texture file(*dds)|*.dds|Bitmap file(*bmp)|*bmp|JPEG file(*.jpg)|*.jpg|TGA file(*.tga)|*.tga|All Files (*.*)|*.*||"),NULL);
	static std::string path;
	if ( dlg.DoModal( ) == IDOK )
	{
		path = toMBCS(dlg.GetPathName());
		if (pathRoot&&(!CheckPathContaining(pathRoot,path.c_str())))
		{
			LogFile::Prompt("\"%s\" is not under \"%s\"!",path.c_str(),pathRoot);
			return "";
		}
		return path.c_str();
	}
	return "";
}
/*
const char *FD_BrowseTex(BOOL bOpen)
{
	static CFileDialog dlg( bOpen, _T("tex"), _T(""),
		0,_T("IxEngine texture file(*.tex)|*.tex|Texture file(*dds)|*.dds|Bitmap file(*bmp)|*bmp|JPEG file(*.jpg)|*.jpg|TGA file(*.tga)|*.tga|All Files (*.*)|*.*||"),NULL);


	static std::string path;
	if (IDOK==dlg.DoModal())
	{
		path=(LPCTSTR)dlg.GetPathName();
		return path.c_str();
	}
	return "";
}
*/

BOOL FD_BrowseImage(BOOL bOpen,CxImage *image)
{
	static CFileDialog dlg( bOpen, _T("image"), _T(""),
		0,_T("image file(*.bmp,*.jpg,*.tga)|*.bmp;*.jpg;*.tga|All Files (*.*)|*.*||"),NULL);


	static std::string path;
	if (IDOK==dlg.DoModal())
	{
		path = toMBCS(dlg.GetPathName());
		CxImage temp;
		temp.Load(fromMBCS(path.c_str()));
		image->Create(temp.GetWidth(),temp.GetHeight(),24);
		return image->Transfer(temp);
	}

	return FALSE;

}

GuiLib_Api const char* FD_BrowseTex2(IRenderSystem* pRS, const char * pathRoot, BOOL bAllowPartSelect,const char *lastsel)
{
	static std::string sResName;
	static IRenderSystem* spRS = pRS;
	if (!pRS)
		pRS = spRS;

	static CResSelectorDlg dlg(pRS, fromMBCS(pathRoot), _T(".bmp|.jpg|.tga|.dds"));

	// At first, assign the last result
	sResName=lastsel;

	if (TRUE)
	{
		if (bAllowPartSelect)
		{
			std::string path;
			i_math::rect_sh rc;
			ParseShellImageStr(lastsel,path,rc);
			dlg.ms_cursel=path;
			rectiToRECT(rc,dlg.ms_lastrect);
		}
		else
		{
			dlg.ms_cursel=lastsel;
			dlg.ms_lastrect.SetRect(0,0,0,0);
		}
	}

	// Set the selected mode
	dlg.SetSelectedMode(bAllowPartSelect ? RSM_PARTIAL : RSM_ENTIRE);
	if (dlg.DoModal() == IDOK)
	{
		std::string relPath;
		std::string res = dlg.GetSelResource();
		if (res.find(dlg.GetRootDir()) != std::string::npos)
		{
			size_t len = strlen(dlg.GetRootDir());
			relPath = res.substr(len + 1);
		}

		const RECT& rcSelection = dlg.GetSelectedRect();
		i_math::recti rc(rcSelection.left, rcSelection.top, rcSelection.right, rcSelection.bottom);

		if (bAllowPartSelect)
		{
			ComposeShellImageStr(sResName, relPath, rc);
		}
		else
			sResName=relPath;
	}
	else
		return "";
	return sResName.c_str();
}

GuiLib_Api const char * FD_BrowseResource(DWORD resfilter,const char *pathDef)
{
	static CResSelectDialog dlg;
	static BOOL bInit=FALSE;
	if (!bInit)
	{
		dlg.SetRootPath(g_ssGuiLib.pRS->GetPath(Path_Res));
		bInit=TRUE;
	}
	dlg.SetFilter((ResType)resfilter);
    if (pathDef[0])
        dlg.SetDefaultResPath(pathDef);
	if(IDOK==dlg.DoModal())
		return dlg.GetResPath();
	else
		return "";
}


const char * FD_BrowseProto(BOOL bOpen,const char * pathRoot)
{
	static CFileDialog dlg(bOpen, _T("prt"), _T(""),
		0,_T("Proto file(*.prt)|*.prt|All Files (*.*)|*.*||"),NULL);
	static std::string path;
	if ( dlg.DoModal( ) == IDOK )
	{
		path = toMBCS(dlg.GetPathName());
		if (pathRoot&&(!CheckPathContaining(pathRoot,path.c_str())))
		{
			LogFile::Prompt("\"%s\" is not under \"%s\"!",path.c_str(),pathRoot);
			return "";
		}
		path=CutHeadPath(path.c_str(),pathRoot);
		return path.c_str();
	}
	return "";
}
