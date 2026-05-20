/********************************************************************
	created:	2007/11/2   9:54
	filename: 	e:\IxEngine\Proj_GuiLib\RichGridFolderItem.cpp
	author:		cxi
	
	purpose:	grid items used in RichGrid--Folder Item
*********************************************************************/

#include "Stdh.h"
#include "RichGrid.h"
#include "RichGridFolderItem.h"

#include "stringparser/stringparser.h"

#include "Log/LogFile.h"



///////////////////////////////////////////////////////////////////////////////

CRichGrid_FolderItem::CRichGrid_FolderItem(CString strCaption)
	: CXTPPropertyGridItem(strCaption)
{
	m_nFlags = xtpGridItemHasExpandButton|xtpGridItemHasEdit;

	_path_s=NULL;
}

void CRichGrid_FolderItem::OnInplaceButtonDown(CXTPPropertyGridInplaceButton* pButton)
{
	std::string s,ss;
	s = toMBCS((LPCTSTR)GetValue());

	if (_pathRoot!="")
	{
		if (s=="")
			s=_pathRoot;
		else
			s=_pathRoot+"\\"+s;
	}


	CXTBrowseDialog dlg;
	dlg.SetOptions(BIF_DONTGOBELOWDOMAIN);
	dlg.SetSelPath(fromMBCS(s.c_str()));

	if (IDOK==dlg.DoModal())
	{
		s = toMBCS(dlg.GetSelPath());
		if (_pathRoot!="")
		{
			if (!CheckPathContaining(_pathRoot.c_str(),s.c_str()))
				s="";
			else
				s=CutHeadPath(s.c_str(),_pathRoot.c_str());
		}
		OnValueChanged(CString(s.c_str()));
	}
}

void CRichGrid_FolderItem::OnValueChanged(CString v)
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

void CRichGrid_FolderItem::Bind(std::string *s)
{
	_path_s=s;

	SetValue(CString(s->c_str()));
}
