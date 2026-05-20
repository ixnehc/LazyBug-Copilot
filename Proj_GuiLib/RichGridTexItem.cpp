/********************************************************************
	created:	2006/10/31   16:27
	filename: 	e:\IxEngine\Proj_GuiLib\RichGridTexItem.cpp
	author:		cxi
	
	purpose:	grid items used in RichGrid--Texture Item
*********************************************************************/

#include "Stdh.h"
#include "RichGrid.h"
#include "RichGridTexItem.h"

#include "stringparser/stringparser.h"

#include "FileSystem/IFileSystem.h"
#include "RenderSystem/IRenderSystem.h"
#include "WorldSystem/IAssetShell.h"

#include "Log/LogFile.h"
#include "FileDialogBase.h"


///////////////////////////////////////////////////////////////////////////////


CRichGrid_TexItem::CRichGrid_TexItem(CString strCaption)
	: CXTPPropertyGridItem(strCaption)
{
	m_nFlags = xtpGridItemHasComboButton|xtpGridItemHasExpandButton|xtpGridItemHasEdit;

	_path_s=NULL;

	_bSelPart=FALSE;

}

std::string ToLocalRefPath(const char *path,const char *pathOwner)
{
	std::string s=path;
	std::string s1=GetFileFolderPath(std::string(pathOwner));

	if (pathOwner[0])
	{
		if (CheckPathContaining(s1.c_str(),path))
		{
			s=CutHeadPath(path,s1.c_str());
			s=std::string(".\\")+s;
		}
	}
	return s;

}

std::string FromLocalRefPath(const char *path,const char *pathOwner)
{
	std::string s=path;
	if (pathOwner[0])
	if (path[0]=='.')
	{
		if (path[1]=='\\')
		{
			s=GetFileFolderPath(std::string(pathOwner));
			s+="\\";
			s+=path+2;
		}
	}
	return s;
}

void CRichGrid_TexItem::OnInplaceButtonDown(CXTPPropertyGridInplaceButton* pButton)
{
	if (pButton->GetID()==XTP_ID_PROPERTYGRID_EXPANDBUTTON)
	{
		std::string rootpath=g_ssGuiLib.pRS->GetPath(Path_Res);
		std::string lastsel;
		if (_path_s)
		{
			lastsel=*_path_s;
			if ((_pathOwner!="")&&(!_bSelPart))
			{
				std::string t=lastsel;
				const char *p=t.c_str();
				if (p[0]=='.')
				{
					if (p[1]=='\\')
						lastsel=GetFileFolderPath(_pathOwner)+(p+1);
				}
			}
		}

		std::string s=FD_BrowseTex2(g_ssGuiLib.pRS,rootpath.c_str(),_bSelPart,lastsel.c_str());
		if(s!="")
		{
			//检查这个路径和owner path是否在同一个目录,如果是的话,转换为相对路径
			s=ToLocalRefPath(s.c_str(),_pathOwner.c_str());
			OnValueChanged(CString(s.c_str()));
		}
		return;
	}
	if (pButton->GetID()==XTP_ID_PROPERTYGRID_COMBOBUTTON)
	{
		if (_path_s)
		{
			std::string s=*_path_s;
			i_math::rect_sh rc;
			if (_bSelPart)
				ParseShellImageStr(*_path_s,s,rc);

			s=FromLocalRefPath(s.c_str(),_pathOwner.c_str());
			extern const char *PopupResRepairMenu(const char *path,const char *pathRoot,CWnd *wnd);
			std::string sel=PopupResRepairMenu(s.c_str(),g_ssGuiLib.pRS->GetPath(Path_Res),GetGrid());
			if (sel!="")
			{
				if (_bSelPart)
				{
					s=sel;
					ComposeShellImageStr(sel,s,rc);
				}
				sel=ToLocalRefPath(sel.c_str(),_pathOwner.c_str());
				OnValueChanged(fromMBCS(sel.c_str()));
			}
		}
		return;
	}
}

void CRichGrid_TexItem::OnValueChanged(CString v)
{
	CXTPPropertyGridItem::OnValueChanged(v);
	m_pGrid->SetFocus();

	if (!_path_s)
		return;

	GetRichGrid(this)->OnBeginItemChange(this);

	*_path_s = toMBCS((LPCTSTR)v);

	GetRichGrid(this)->OnItemChange(this);
	GetRichGrid(this)->OnEndItemChange(this);
}

void CRichGrid_TexItem::SetOwnerPath(const char *path)
{		
	_pathOwner=path;	
	_UpdateValueValidity();
}


void CRichGrid_TexItem::_UpdateValueValidity()
{
	CXTPPropertyGridItemMetrics*metrics=GetValueMetrics();
	metrics->m_clrFore=0;
	if (_path_s)
	{
		std::string path=*_path_s;
		if (_bSelPart)
		{
			i_math::rect_sh rc;
			ParseShellImageStr(*_path_s,path,rc);
		}
		extern std::string FromLocalRefPath(const char *path,const char *pathOwner);
		std::string ss;
		ss=g_ssGuiLib.pRS->GetPath(Path_Res);
		ss+="\\";
		ss+=FromLocalRefPath(path.c_str(),_pathOwner.c_str());
		if (!g_ssGuiLib.pFS->ExistFileAbs(ss.c_str()))
			metrics->m_clrFore=0x0000ff;
	}
}


void CRichGrid_TexItem::Bind(std::string *s,BOOL bSelPart)
{
	_path_s=s;
	_bSelPart=bSelPart;

	_UpdateValueValidity();

	SetValue(CString(s->c_str()));
}
