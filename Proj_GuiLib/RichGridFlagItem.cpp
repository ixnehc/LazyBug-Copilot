/********************************************************************
	created:	2006/10/31   14:54
	filename: 	e:\IxEngine\Proj_GuiLib\RichGridItems.cpp
	author:		cxi
	
	purpose:	grid items used in RichGrid--Flag Item
*********************************************************************/


#include "stdh.h"
#include "commondefines/general_stl.h"
#include "RichGrid.h"
#include "RichGridFlagItem.h"
 
#include "stringparser/stringparser.h"

#include <assert.h>

void CRichGrid_FlagSubItem::OnButtonClick(DWORD idButton)
{
	if (idButton!=ID_RGIB_Remove)
		return;

	CRichGrid_FlagItem *parent;
	parent=(CRichGrid_FlagItem *)GetParentItem();
	assert(parent);
	parent->RemoveFlag(toMBCS(GetCaption()));
}


//////////////////////////////////////////////////////////////////////////
//CRichGrid_FlagItem

void CRichGrid_FlagItem::_ParseConstraints(const char *constraints)
{
	SplitStringBy(",",std::string(constraints),&_names);
	_values.resize(_names.size());
	std::vector<std::string>temp;
	for (int i=0;i<_names.size();i++)
	{
		SplitStringBy(":",_names[i],&temp);
		if (temp.size()>1)
		{
			_names[i]=temp[0];
			_values[i]=IntFromString(temp[1].c_str());
		}
		else
			_values[i]=(1<<i);

		SplitStringBy("|",_names[i],&temp);
		if (temp.size()>1)
		{
			_names[i]=temp[0];
			_hides.push_back(temp[1]);
		}
		else
			_hides.push_back(std::string(""));

	}
}


void CRichGrid_FlagItem::Bind(std::string *sFlag,const char *sConstaints)
{
	_ParseConstraints(sConstaints);

	_sFlag=sFlag;

	_UpdateValue();
}


void CRichGrid_FlagItem::_MakeFlagString(DWORD flag,std::string &s)
{
	for (int i=0;i<_names.size();i++)
	{
		if ((flag)&_values[i])
		{
			if (s=="")
				s=_names[i];
			else
			{
				s+=",";
				s+=_names[i];
			}
		}
	}
}

void CRichGrid_FlagItem::_ApplyCaptionHides(DWORD flag)
{
	for (int i=0;i<_names.size();i++)
	{
		if (!((flag)&_values[i]))
		{
			static std::string s;
			static std::vector<std::string> temp;

			if (!_hides[i].empty())
			{
				SplitStringBy("&",_hides[i],&temp);
				for (int j=0;j<temp.size();j++)
					GetRichGrid(this)->AddCaptionHide(temp[j].c_str());
			}
		}
	}
}



void CRichGrid_FlagItem::Bind(DWORD *flag,const char *sConstaints)
{
	_ParseConstraints(sConstaints);

	_MakeFlagString(*flag,_sFake);
	_ApplyCaptionHides(*flag);

	_sFlag=&_sFake;
	_flagDW=flag;

	_UpdateValue();

}

void CRichGrid_FlagItem::Bind(WORD *flag,const char *sConstaints)
{
	_ParseConstraints(sConstaints);

	_MakeFlagString((DWORD)*flag,_sFake);
	_ApplyCaptionHides(*flag);

	_sFlag=&_sFake;
	_flagW=flag;

	_UpdateValue();

}



void CRichGrid_FlagItem::_UpdateValue()
{
	GetChilds()->Clear();

	if (!_sFlag)
	{
		SetValue(_T(""));
		return;
	}

	std::vector<std::string>vecTemp;

	SetValue(fromMBCS(_sFlag->c_str()));

	SplitStringBy(",",*_sFlag,&vecTemp);

// 	for (int i=0;i<vecTemp.size();i++)
// 		AddChildItem(new CRichGrid_FlagSubItem(vecTemp[i].c_str()));

}

void CRichGrid_FlagItem::OnButtonClick(DWORD idButton)
{
	if (idButton!=ID_RGIB_Menu)
	{
		CRichGrid_ButtonItem::OnButtonClick(idButton);
		return;
	}

	std::vector<std::string>vecTemp2;
	if (_sFlag)
		SplitStringBy(",",*_sFlag,&vecTemp2);

	CMenu menu;	
	menu.CreatePopupMenu();
	int i;
	for (i=0;i<_names.size();i++)
	{
		int idx;
		VEC_FIND(vecTemp2,_names[i],idx);
		if (idx==-1)
			menu.InsertMenu(i, MF_ENABLED | MF_STRING, i, fromMBCS(_names[i].c_str()));
		else
			menu.InsertMenu(i,MF_ENABLED|MF_STRING|MF_CHECKED,i, fromMBCS(_names[i].c_str()));
	}

	CPoint point;
	GetCursorPos(&point);

	if (i>0)
		menu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON, 
			point.x,point.y,_GetButtonFromID(idButton),NULL );
}

void CRichGrid_FlagItem::_SwitchFlag(DWORD iFlag)
{
	if (!_sFlag)
		return;

	GetRichGrid(this)->OnBeginItemChange(this);
	std::vector<std::string>vecTemp2;
	SplitStringBy(",",*_sFlag,&vecTemp2);

	std::vector<std::string>final;
	DWORD dwFinal=0;
	for (int i=0;i<_names.size();i++)
	{
		int idx;
		VEC_FIND(vecTemp2,_names[i],idx);
		if ((idx==-1)&&(iFlag!=i))
			continue;
		if ((idx!=-1)&&(iFlag==i))
			continue;
		final.push_back(_names[i]);
		dwFinal|=_values[i];
	}
	LinkStringBy(",",*_sFlag,&final);

	if (_flagDW)
		*_flagDW=dwFinal;
	if (_flagW)
		*_flagW=(WORD)dwFinal;

	_UpdateValue();

	Expand();

	GetRichGrid(this)->OnItemChange(this);
	GetRichGrid(this)->OnEndItemChange(this);
}


void CRichGrid_FlagItem::OnButtonMenuCmd(DWORD idCmd)
{
	_SwitchFlag(idCmd);
}

void CRichGrid_FlagItem::RemoveFlag(const char *sFlag)
{

	int idx;
	VEC_FIND(_names,sFlag,idx);

	_SwitchFlag(idx);
}
