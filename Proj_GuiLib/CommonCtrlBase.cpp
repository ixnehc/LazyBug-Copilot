/********************************************************************
	created:	2006/9/6   11:29
	filename: 	e:\IxEngine\Proj_GuiLib\CommonCtrlBase.cpp
	author:		cxi
	
	purpose:	useful functions for general common controls
*********************************************************************/
#include "stdh.h"

#include "CommonCtrlBase.h"

#include <vector>
#include <string>

#include "stringparser/stringparser.h"

#include "InputDlg.h"

#pragma warning(disable:4018)


//find the item whose text is name
//return -1 if not found
int ListBox_Find(CListBox *lb,const char *name,BOOL bCaseSensitive)
{
	for(int i=0;i<lb->GetCount();i++)
	{
		CString s;
		lb->GetText(i,s);
		if (bCaseSensitive)
		{
			if (s==name)
				return i;
		}
		else
		{
			if (StringEqualNoCase(toMBCS((LPCTSTR)s), name))
				return i;
		}
	}
	return -1;
}

//check whether the iItem has duplication name with other item in the list
//return TRUE if dupe found
//return FALSE ,if iItem is not a valid item index
BOOL ListBox_CheckDupe(CListBox *lb,DWORD iItem,BOOL bCaseSensitive)
{
	if (iItem>=lb->GetCount())
		return FALSE;
	CString sItem;
	lb->GetText(iItem,sItem);
	std::string sItemM = toMBCS((LPCTSTR)sItem);
	for(int i=0;i<lb->GetCount();i++)
	{
		if (i==iItem)
			continue;
		CString s;
		lb->GetText(i,s);
		if (bCaseSensitive)
		{
			if (s==sItem)
				return TRUE;
		}
		else
		{
			if (StringEqualNoCase(toMBCS((LPCTSTR)s),sItemM.c_str()))
				return TRUE;
		}
	}
	return FALSE;
}


void ListBox_UpdateItems(CListBox *lb,std::vector<std::string>&items,BOOL bCaseSensitive)
{
	//Firstly,try to match them,if all matched(or could be appended at tail),we need not 
	//ResetContent()
	int i;
	for (i=0;i<lb->GetCount();i++)
	{
		if (i>=items.size())
			break;
		CString s;
		lb->GetText(i,s);
		if (bCaseSensitive)
		{
			if (!(s==items[i].c_str()))
				break;
		}
		else
		{
			if (!StringEqualNoCase(toMBCS((LPCTSTR)s), items[i].c_str()))
				break;
		}
	}
	if (i<lb->GetCount())//could match,need ResetContent()
	{
		lb->ResetContent();
		i=0;
	}

	for (;i<items.size();i++)
		lb->AddString(fromMBCS(items[i].c_str()));
}

BOOL ListBox_IsSelected(CListBox *lb,DWORD iItem)
{
	return lb->GetSel(iItem)>0;
}

const char *ListBox_GetSelString(CListBox *lb)
{
	static std::string ret;
	int iSel=lb->GetCurSel();
	if (iSel==-1)
		ret="";
	else
	{
		CString s;
		lb->GetText(iSel,s);
		ret = toMBCS((LPCTSTR)s);
	}
	return ret.c_str();
}

const char *ListBox_GetString(CListBox *lb,DWORD iItem)
{
	static std::string ret;
	CString s;
	lb->GetText(iItem,s);
	ret=toMBCS((LPCTSTR)s);
	return ret.c_str();
}

BOOL ListBox_SetItemText(CListBox *lb,DWORD iItem,const char *str)
{
	DWORD olddata=(DWORD)lb->GetItemData(iItem);
	if (LB_ERR==lb->DeleteString(iItem))
		return FALSE;
	lb->InsertString(iItem, fromMBCS(str));
	lb->SetItemData(iItem,olddata);
	return TRUE;

}




//find the item whose text is name
//return -1 if not found
int ComboBox_Find(CComboBox *cb,const char *name,BOOL bCaseSensitive)
{
	for(int i=0;i<cb->GetCount();i++)
	{
		CString s;
		cb->GetLBText(i,s);
		if (bCaseSensitive)
		{
			if (s==name)
				return i;
		}
		else
		{
			if (StringEqualNoCase(toMBCS((LPCTSTR)s), name))
				return i;
		}
	}
	return -1;
}

const char *ComboBox_GetSelString(CComboBox *cb)
{
	static std::string ret;
	int iSel=cb->GetCurSel();
	if (iSel==-1)
		ret="";
	else
	{
		CString s;
		cb->GetLBText(iSel,s);
		ret = toMBCS((LPCTSTR)s);
	}
	return ret.c_str();
}

//Get the select of the list box
int ComboBox_GetListSel(CComboBox *cb)
{
	COMBOBOXINFO cbi;
	cbi.cbSize=sizeof(COMBOBOXINFO);
	cb->GetComboBoxInfo(&cbi);
	
	int iSel=-1;
	if (cbi.hwndList)
	{
		CListBox lb;
		lb.Attach(cbi.hwndList);
		iSel=lb.GetCurSel();
		lb.Detach();
	}

	return iSel;
}




//Prompt a dialog to input a string
BOOL InputString(const char *prompt,std::string &s)
{
	CInputDlg dlg;
	dlg.m_s=s.c_str();
	dlg.m_prompt=prompt;
	if (IDOK!=dlg.DoModal())
		return FALSE;
	s = toMBCS(dlg.m_s);
	return TRUE;
}

