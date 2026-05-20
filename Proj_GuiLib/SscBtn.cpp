#include "stdh.h"
#include ".\GuiLib.h"

#include <vector>
#include <string>


#include "interface/interface.h"
#include "resource.h"

#include "assert.h"

#include "stringparser/stringparser.h"

#include "SscBase.h"
#include ".\sscbtn.h"


#pragma warning(disable:4018)

BEGIN_MESSAGE_MAP(CSscBtn, CXTButton)
	ON_CONTROL_REFLECT(BN_CLICKED, OnBnClicked)
	ON_COMMAND(ID_SSC_REFRESH,OnRefresh)
	ON_COMMAND(ID_SSC_ADDTOVSS,OnAdd)
	ON_COMMAND(ID_SSC_RMFROMVSS,OnRemove)
	ON_COMMAND(ID_SSC_CHECKIN,OnCheckIn)
	ON_COMMAND(ID_SSC_CHECKIN_KEEPOUT,OnCheckIn_KeepOut)
	ON_COMMAND(ID_SSC_CHECKOUT,OnCheckOut)
	ON_COMMAND(ID_SSC_GET,OnGet)
END_MESSAGE_MAP()


CSscBtn::CSscBtn()
{
	_ssc=NULL;
	_state=SSC_NOTCONTROLLED;

	_dlgtNotifySave=NULL;
	_dlgtNotifyLoad=NULL;

}

void CSscBtn::Bind(const char *path,CSscSystemWrapper *ssc)
{
	BOOL bModify=FALSE;
	if (!StringEqualNoCase(_path.c_str(),path))
		bModify=TRUE;
	if (_ssc!=ssc)
		bModify=TRUE;
	if (bModify)
	{
		_path=path;
		_ssc=ssc;
		_Refresh();

		std::string nm=GetFileName(_path);
		SetWindowText(fromMBCS(nm.c_str()));
	}
}



void CSscBtn::_Refresh()
{
	if (_ssc)
	{
		if (FALSE==_ssc->GetState(_path.c_str(),_state))
			_state=SSC_NOTCONTROLLED;
	}

	switch(_state)
	{
		case SSC_NOTCONTROLLED:
			SetBitmap(CSize(26,16),IDB_SSCEMPTY); 
			break;
		case SSC_CHECKEDOUT:
			SetBitmap(CSize(26,16),IDB_SSCFORBID); 
			break;
		case SSC_NOTCHECKEDOUT:
			SetBitmap(CSize(26,16),IDB_SSCCHECKIN); 
			break;
		case SSC_CHECKEDOUT_ME:
			SetBitmap(CSize(26,16),IDB_SSCCHECKOUT); 
			break;
	}
}

void CSscBtn::OnBnClicked()
{
	if (!_ssc)
		return;

	CMenu menu;	
	menu.CreatePopupMenu();

	DWORD idx=0;
	menu.InsertMenu(idx++, MF_ENABLED | MF_STRING, ID_SSC_REFRESH, _T("Refresh State"));

	if (_state==SSC_NOTCONTROLLED)
		menu.InsertMenu(idx++,MF_ENABLED|MF_STRING,ID_SSC_ADDTOVSS, _T("Add to SourceSafe Control"));

	if (_state==SSC_CHECKEDOUT_ME)
	{
		menu.InsertMenu(idx++,MF_ENABLED|MF_STRING,ID_SSC_CHECKIN, _T("Check In"));
		menu.InsertMenu(idx++,MF_ENABLED|MF_STRING,ID_SSC_CHECKIN_KEEPOUT, _T("Check In(Keep CheckOut)"));
	}

	if (_state==SSC_NOTCHECKEDOUT)
		menu.InsertMenu(idx++,MF_ENABLED|MF_STRING,ID_SSC_CHECKOUT, _T("Check Out"));

	if ((_state==SSC_NOTCHECKEDOUT)||(_state==SSC_CHECKEDOUT))
		menu.InsertMenu(idx++, MF_ENABLED | MF_STRING, ID_SSC_GET, _T("Get Latest Version"));

	if (_state!=SSC_NOTCONTROLLED)
		menu.InsertMenu(idx++, MF_ENABLED | MF_STRING, ID_SSC_RMFROMVSS, _T("Remove from SourceSafe Control"));

	_OnCustomizeMenu(&menu);

	CRect rc;
	GetWindowRect(&rc);

	if (idx>0)
		XTFuncContextMenu(&menu,TPM_LEFTALIGN|TPM_LEFTBUTTON,rc.left,rc.bottom,this,IDR_TOOLBAREXT);
}


void CSscBtn::OnRefresh()
{
	_Refresh();
}

void CSscBtn::OnCheckIn()
{
	if (_dlgtNotifySave)
	{
		if(!_dlgtNotifySave())
			return;
	}
	_ssc->CheckIn(_path.c_str());
	_Refresh();
}

void CSscBtn::OnCheckIn_KeepOut()
{
	if (_dlgtNotifySave)
	{
		if(!_dlgtNotifySave())
			return;
	}
	_ssc->CheckIn(_path.c_str(),131072);//VSSFLAG_KEEPYES
	_Refresh();
}


void CSscBtn::OnCheckOut()
{
	_ssc->CheckOut(_path.c_str());
	_Refresh();

	if (_dlgtNotifyLoad)
		_dlgtNotifyLoad();
}
void CSscBtn::OnGet()
{
	_ssc->GetLatestVersion(_path.c_str());
	_Refresh();
	if (_dlgtNotifyLoad)
		_dlgtNotifyLoad();
}
void CSscBtn::OnRemove()
{
	_ssc->Delete(_path.c_str());
	_Refresh();
}
void CSscBtn::OnAdd()
{
	if (_dlgtNotifySave)
	{
		if(!_dlgtNotifySave())
			return;
	}

	_ssc->CheckIn(_path.c_str());
	_Refresh();
}
