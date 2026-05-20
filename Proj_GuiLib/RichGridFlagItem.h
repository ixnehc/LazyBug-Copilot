
#pragma once

#include "RichGridButtonItem.h"

class CRichGrid_FlagSubItem:public CRichGrid_ButtonItem
{
public:
	CRichGrid_FlagSubItem(CString strCaption):CRichGrid_ButtonItem(strCaption)
	{
		m_nFlags=0;
		_maskBtn|=ID_RGIB_Remove;
	}

	virtual void OnButtonClick(DWORD idButton);

};


class CRichGrid_FlagItem:public CRichGrid_ButtonItem
{
public:
	CRichGrid_FlagItem(CString strCaption):CRichGrid_ButtonItem(strCaption)
	{
		m_nFlags=0;

		_maskBtn|=ID_RGIB_Menu;


		_sFlag=NULL;
		_flagDW=NULL;
		_flagW=NULL;
	}

	void Bind(std::string *sFlag,const char *sConstaints);
	void Bind(DWORD *flag,const char *sConstaints);
	void Bind(WORD *flag,const char *sConstaints);
	virtual void OnButtonClick(DWORD idButton);
	virtual void OnButtonMenuCmd(DWORD idCmd);

	void RemoveFlag(const char *sFlag);

protected:

	void _MakeFlagString(DWORD flag,std::string &s);
	void _ApplyCaptionHides(DWORD flag);

	void _UpdateValue();

	void _SwitchFlag(DWORD idx);

	void _ParseConstraints(const char *constraints);

	std::vector<std::string>_names;
	std::vector<DWORD>_values;
	std::vector<std::string>_hides;
	std::string *_sFlag;
	DWORD *_flagDW;
	WORD *_flagW;
	std::string _sFake;

};




