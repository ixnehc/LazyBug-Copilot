#include "stdh.h"
#include ".\GuiLib.h"

#include "RenderSystem/IRenderSystem.h"

#include <vector>
#include <string>
#include ".\resanchor.h"

#include "resdata/ResData.h"

#include "FileDialogBase.h"

#include "ResEditPanel.h"

#include "stringparser/stringparser.h"

#include "Log/LogFile.h"

#include "WMGuiLib.h"


#pragma warning(disable:4018)


//////////////////////////////////////////////////////////////////////////
//CResAnchorBase



//////////////////////////////////////////////////////////////////////////
//CResAnchor

CImageList CResAnchor::_imagelist;

CImageList *CResAnchor::GetImageList()
{
	if (_imagelist.GetSafeHandle())
		return &_imagelist;

	extern BOOL CreateImageList(CImageList& il, UINT nID,int w,int h);
	CreateImageList(_imagelist,IDB_RESTREEICON,16,16);

	return &_imagelist;
}




CResAnchor::CResAnchor(int type,const char *label,BOOL bUndo)
{
	_type=type;
	_label=label;
	_bUndo=bUndo;
}

BEGIN_MESSAGE_MAP(CResAnchor, CXTButton)
// 	ON_CONTROL_REFLECT(BN_CLICKED, OnBnClicked)
	ON_WM_CREATE()
END_MESSAGE_MAP()

int CResAnchor::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (-1==CXTButton::OnCreate(lpCreateStruct))
		return -1;
	return 0;
}

CRect CResAnchor::GetRect()
{
	CRect rc;
	GetWindowRect(&rc);
	return rc;
}


const char *CResAnchor::GetPath()
{
	return _path.c_str();
}

const char *CResAnchor::GetRelativePath()
{
	return _pathRelative.c_str();
}


void CResAnchor::SetPath(const char *path)
{
	_path=path;
	_pathRelative="";

	std::string pathRoot=g_ssGuiLib.pRS->GetPath(Path_Res);
	if (_type==Res_BehaviorGraph)
		pathRoot=g_ssGuiLib.pRS->GetPath(Path_BehaviorGraph);
	if (_path!="")
	{
		if (!CheckPathContaining(pathRoot.c_str(),_path.c_str()))
		{
			LogFile::Prompt("Warning:Res file \"%s\" is not under the res root path \"%s\"!",
													_path.c_str(),pathRoot.c_str());
		}
		else
			_pathRelative=CutHeadPath(_path.c_str(),pathRoot.c_str());
	}


	std::string s;
	if (_pathRelative=="")
		s="<Empty>";
	else
		s=_pathRelative;

	CString sOld;
	GetWindowText(sOld);
	if (sOld!=s.c_str())
		SetWindowText(fromMBCS(s.c_str()));
}

void CResAnchor::SetRelativePath(const char *path)
{
	std::string pathRoot=g_ssGuiLib.pRS->GetPath(Path_Res);
	std::string s=pathRoot+"\\"+path;
	SetPath(s.c_str());
}



void CResAnchor::OnBnClicked()
{
	// TODO: Add your control notification handler code here
	const char *path=FD_BrowseResource(_type,GetRelativePath());
	if (path[0])
		NotifyChange(path);
}


void CResAnchor::SetDefaultStyle()
{
	int idxIcon;
	switch(_type)
	{
		case Res_Mesh:
			idxIcon=3;			break;
		case ResA_Bones2:
			idxIcon=5;			break;
		case ResA_XForm:
			idxIcon=4;			break;
		case Res_Mtrl:
			idxIcon=6;			break;
		case ResA_MtrlColor:
			idxIcon=7;			break;
		case ResA_MapCoord:
			idxIcon=8;			break;
		case Res_Dummies:
			idxIcon=9;		    break;
		case Res_Spt:
		case Res_Mopp:
		case Res_Spg:
		case Res_AnimTree:
		case Res_MtrlExt:
		case Res_Sound:
		case Res_Records:
		case Res_Ragdoll:
		case Res_Dtr:
		case Res_BehaviorGraph:
			idxIcon=13;		    break;//need a new icon

		//XXXXX:more res type
		default:
			idxIcon=6;
	}

	SetIcon(CSize(16,16), GetImageList()->ExtractIcon(idxIcon));
	SetTheme(new CXTButtonThemeOffice2003(TRUE));
}

void CResAnchor::NotifyChange(const char *path)
{
	SetRelativePath(path);
//	((CResEditPanel*)GetParent())->NotifyAnchorChange(this,path);
}
