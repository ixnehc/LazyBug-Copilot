#pragma once

#include "GuiLib.h"

#include "FileSystem/ISscSystemDefines.h"

#include "fastdelegate/FastDelegate.h"

#include <map>
#include <string>



class CSscSystemWrapper;

typedef fastdelegate::FastDelegate0<BOOL>  SscBtnCallback;

class GuiLib_Api CSscBtn:public CXTButton
{
public:
	CSscBtn();
	void Bind(const char *path,CSscSystemWrapper *ssc);
	void BindNotifySave(SscBtnCallback dlgt)	{		_dlgtNotifySave=dlgt;	}
	void BindNotifyLoad(SscBtnCallback dlgt)	{		_dlgtNotifyLoad=dlgt;	}
protected:

	virtual void _OnCustomizeMenu(CMenu *menu)	{	}

	void _Refresh();

	std::string _path;
	CSscSystemWrapper *_ssc;
	SscState _state;

	SscBtnCallback _dlgtNotifySave;
	SscBtnCallback _dlgtNotifyLoad;



public:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClicked();
	afx_msg void OnRefresh();
	afx_msg void OnCheckIn();
	afx_msg void OnCheckIn_KeepOut();
	afx_msg void OnCheckOut();
	afx_msg void OnGet();
	afx_msg void OnRemove();
	afx_msg void OnAdd();

};


