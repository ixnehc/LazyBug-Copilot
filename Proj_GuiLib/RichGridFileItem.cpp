/********************************************************************
	created:	14:1:2009   17:48
	filename: 	d:\IxEngine\Proj_GuiLib\RichGridFileItem.cpp
	author:		chenxi
	
	purpose:	file selection item
*********************************************************************/

#include "Stdh.h"
#include "RichGrid.h"
#include "RichGridFileItem.h"

#include "stringparser/stringparser.h"

#include "Log/LogFile.h"
#include "FileDialogBase.h"


///////////////////////////////////////////////////////////////////////////////

void CRichGrid_FileItem::SetRootPath(const char *path)
{
	_rootpath=path;
}

void CRichGrid_FileItem::SetFilter(const char *suffix,const char *filter)
{
	_suffix=suffix;
	_filter=filter;
}


CRichGrid_FileItem::CRichGrid_FileItem(CString strCaption)
	: CXTPPropertyGridItem(strCaption)
{
	m_nFlags = xtpGridItemHasExpandButton|xtpGridItemHasEdit;

	_path_s=NULL;
}

void CRichGrid_FileItem::OnInplaceButtonDown(CXTPPropertyGridInplaceButton* pButton)
{
	std::string s,ss;
	s = toMBCS((LPCTSTR)GetValue());

	if (_rootpath!="")
	{
		if (s!="")
			s=_rootpath+"\\"+s;
		else
			s=_rootpath;
	}

	CString suffix = fromMBCS(_suffix.c_str());
	CString filename = fromMBCS(s.c_str());
	CString filter = fromMBCS(_filter.c_str());
	CFileDialog dlg(TRUE, suffix, filename, 0, filter, NULL);
	std::string path;
	if ( dlg.DoModal( ) == IDOK )
	{
		path = toMBCS((LPCTSTR)dlg.GetPathName());
		if ((_rootpath!="")&&(!CheckPathContaining(_rootpath.c_str(),path.c_str())))
			s="";
		else
		{
			if(_rootpath!="")
				s=CutHeadPath(path.c_str(),_rootpath.c_str());
			else
				s=path;
		}
		OnValueChanged(CString(s.c_str()));
	}
}

void CRichGrid_FileItem::OnValueChanged(CString v)
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

void CRichGrid_FileItem::Bind(std::string *s)
{
	_path_s=s;

	SetValue(CString(s->c_str()));
}
