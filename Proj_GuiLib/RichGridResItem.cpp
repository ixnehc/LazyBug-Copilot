/********************************************************************
	created:	2007/8/29   16:29
	filename: 	e:\IxEngine\Proj_GuiLib\RichGridResItem.cpp
	author:		cxi
	
	purpose:	grid items used in RichGrid--Resource Item
*********************************************************************/

#include "Stdh.h"
#include "FileSystem/IFileSystem.h"
#include "RenderSystem/IRenderSystem.h"
#include "RichGrid.h"
#include "RichGridResItem.h"

#include "FileDialogBase.h"

#include "stringparser/stringparser.h"

#include "Log/LogFile.h"



BOOL FindSameNameRes(const char *path,BOOL bFolder,void *param_)
{
	const char *name=(const char *)param_;

	if (bFolder)
		return FALSE;

	if (!CheckFileName(path,name))
		return FALSE;

	return TRUE;
}

const char *PopupResRepairMenu(const char *path,const char *pathRoot,CWnd *wnd)
{
	if (!path[0])
		return "";
	if (path[0]=='.')
		return "";
	std::string name=GetFileName(std::string(path));
	g_ssGuiLib.pFS->EnumBegin(pathRoot,TRUE,FindSameNameRes,(void *)name.c_str());
	std::vector<std::string>matches[4];
	std::vector<std::string>pieces,pieces2;
	SplitStringBy("\\",std::string(path),&pieces2);
	for (int i=0;i<g_ssGuiLib.pFS->EnumGetFileCount();i++)
	{
		std::string s=g_ssGuiLib.pFS->EnumGetFile(i);
		if (StringEqualNoCase(s.c_str(),path))
			continue;//同一个文件

		SplitStringBy("\\",s,&pieces);

		//判断路径名的末尾有几段相匹配
		int nMatch=0;
		for (int i=0;i<ARRAY_SIZE(matches);i++)
		{
			if ((pieces.size()>i)&&(pieces2.size()>i))
			{
				if (StringEqualNoCase(pieces[pieces.size()-1-i].c_str(),pieces2[pieces2.size()-1-i].c_str()))
				{
					nMatch++;
					continue;
				}
			}
			break;
		}

		if (nMatch>0)
			matches[nMatch-1].push_back(s);
	}
	g_ssGuiLib.pFS->EnumEnd();

	const int max_candidate=8;

	std::vector<std::string>candidates;
	candidates.clear();
	for (int i=ARRAY_SIZE(matches)-1;i>=0;i--)
	{
		for (int j=0;j<matches[i].size();j++)
		{
			if (candidates.size()<max_candidate)
				candidates.push_back(matches[i][j]);
		}
	}

	static std::string sel;
	if (candidates.size()>0)
	{
		CMenu menu;	
		menu.CreatePopupMenu();

		std::string s="< ";
		s+=path;
		s+=" >";
		int idx=0;
		menu.InsertMenu(idx++, MF_DISABLED | MF_STRING, 0xffffffff, fromMBCS(s.c_str()));
		menu.InsertMenu(idx++,MF_ENABLED|MF_SEPARATOR,0xffffffff,_T(""));

		for (int i=0;i<candidates.size();i++)
		{
			s=std::string("  ")+candidates[i];
			menu.InsertMenu(idx++, MF_ENABLED | MF_STRING, i + 1, fromMBCS(s.c_str()));
		}
		CPoint point;
		GetCursorPos(&point);

		int idCmd=menu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RETURNCMD|TPM_NONOTIFY,point.x,point.y,wnd,NULL );
		if (idCmd>0)
		{
			idCmd--;
			if (idCmd<candidates.size())
			{
				sel=candidates[idCmd];
				return sel.c_str();
			}
		}
	}
	return "";
}




///////////////////////////////////////////////////////////////////////////////

CRichGrid_ResItem::CRichGrid_ResItem(CString strCaption)
	: CXTPPropertyGridItem(strCaption)
{
	m_nFlags = xtpGridItemHasComboButton|xtpGridItemHasExpandButton|xtpGridItemHasEdit;

	_path_s=NULL;
}

int CRichGrid_ResItem::GetResType()
{
	return _restype;
}
CRect CRichGrid_ResItem::GetRect()
{
	CRect rc=GetValueRect();
	CRect rc2;
	m_pGrid->GetWindowRect(&rc2);
	rc+=rc2.TopLeft();
	return rc;
}

void CRichGrid_ResItem::NotifyChange(const char *path)
{
	if (!_path_s)
		return;
	if (StringEqualNoCase(_path_s->c_str(),path))
		return;
	OnValueChanged(CString(path));
}

void CRichGrid_ResItem::OnInplaceButtonDown(CXTPPropertyGridInplaceButton* pButton)
{
	if (pButton->GetID()==XTP_ID_PROPERTYGRID_EXPANDBUTTON)
	{
        _temp = "";
        if (_path_s)
        {
            if (_pathOwner != "")
            {
                extern std::string FromLocalRefPath(const char *path, const char *pathOwner);
                _temp = FromLocalRefPath(_path_s->c_str(), _pathOwner.c_str());
            }
            else
                _temp = _path_s->c_str();
        }

		const char *path=FD_BrowseResource(_restype,_temp.c_str());
		if (path[0])
		{
			if (_pathOwner!="")
			{
				extern std::string ToLocalRefPath(const char *path,const char *pathOwner);
				_temp=ToLocalRefPath(path,_pathOwner.c_str());
			}
			else
				_temp=path;
			NotifyChange(_temp.c_str());
		}
		return;
	}
	if (pButton->GetID()==XTP_ID_PROPERTYGRID_COMBOBUTTON)
	{
		if (_path_s)
		{
			extern std::string FromLocalRefPath(const char *path,const char *pathOwner);
			std::string s=FromLocalRefPath(_path_s->c_str(),_pathOwner.c_str());

			std::string sel=PopupResRepairMenu(s.c_str(),g_ssGuiLib.pRS->GetPath(Path_Res),GetGrid());
			if (sel!="")
				NotifyChange(sel.c_str());
		}
		return;
	}
	
}

void CRichGrid_ResItem::OnValueChanged(CString v)
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


void CRichGrid_ResItem::_UpdateValueValidity()
{
	CXTPPropertyGridItemMetrics*metrics=GetValueMetrics();
	metrics->m_clrFore=0;
	if (_path_s)
	{
		extern std::string FromLocalRefPath(const char *path,const char *pathOwner);
		std::string ss;
		ss=g_ssGuiLib.pRS->GetPath(Path_Res);
		ss+="\\";
		ss+=FromLocalRefPath(_path_s->c_str(),_pathOwner.c_str());
		if (!g_ssGuiLib.pFS->ExistFileAbs(ss.c_str()))
			metrics->m_clrFore=0x0000ff;
	}
}


void CRichGrid_ResItem::Bind(std::string *s,int restype)
{
	_path_s=s;
	_restype=restype;

	_UpdateValueValidity();

	SetValue(CString(s->c_str()));
}

void CRichGrid_ResItem::SetOwnerPath(const char *path)	
{		
	_pathOwner=path;	
	_UpdateValueValidity();
}
