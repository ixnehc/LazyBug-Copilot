/********************************************************************
	created:	2006/8/27   17:51
	filename: 	d:\IxEngine\Proj_GuiLib\AnchorPopup.cpp
	author:		ixnehc

	purpose:	popup for an edit control
	*********************************************************************/


#include "stdh.h"
#include "resource.h"
#include "RichGridComboItem.h"
#include "StubPopup.h"

#include "WMGuiLib.h"

#include "WorldSystem/IEntitySystemDefines.h"

#include "WndBase.h"

#include "gds/GDefines.h"


CStubPopup::CStubPopup(CWnd* pParent /*=NULL*/)
	: CXTPDialog(IDD_STUBPOPUP, pParent)
{
	_arg=NULL;
}

void CStubPopup::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CStubPopup, CXTPDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



// CStubPopup 消息处理程序

BOOL CStubPopup::Popup(StubArg *arg)
{
	_arg=arg;
	if (IDCANCEL==DoModal())
		return FALSE;

	_arg->sem=_code;

	return TRUE;

}

BOOL CStubPopup::OnInitDialog()
{
	CDialog::OnInitDialog();

	CRect rc;

	GET_CONTROL_RECT(this,IDC_EDIT,rc);
	HIDE_CONTROL(this,IDC_EDIT);
	_grid.Create(rc,this,IDC_EDIT);

	_code=(GSemCode)_arg->sem;

	_grid.BeginInsert();
	_grid.InsertCategory("接口信息","接口信息");


	_grid.PushInsert();

	_grid.InsertNameItem("名称","接口的名称",&_arg->name);

	if (TRUE)
	{
		switch(_arg->type)
		{
			case GStub_Property:
				_type="Property";
				break;
			case GStub_Signal:
				_type="Signal";
				break;
			case GStub_Slot:
				_type="Slot";
				break;
			case GStub_Call:
				_type="Function";
		}
		CXTPPropertyGridItem *item=_grid.InsertNameItem("接口类型","接口的类型",&_type);
		item->SetReadOnly();
	}

	if (TRUE)
	{
		_nameGVT=_arg->nameGVT;
		if (_nameGVT=="")
			_nameGVT="百搭";
		CXTPPropertyGridItem *item=_grid.InsertNameItem("参数类型","参数的类型",&_nameGVT);
		item->SetReadOnly();
	}

	if (TRUE)
	{
		std::string s=GetSemList();
		std::vector<std::string>buf;
		SplitStringBy(",",s,&buf);
		_grid.InsertComboItem<int>("语意","这个stub的语意(主要影响编辑方式)",(int*)&_code,buf);
	}

	_grid.InsertNameItem("语意Constraint","描述额外的编辑信息的字符串",&_arg->constaint);

	_grid.InsertNameItem("文字描述","接口的详细解释",&_arg->desc);

	_grid.InsertBoolItem("可否连接","接口可否连接",&_arg->bConnectable);

	_grid.PopInsert();

	_grid.PopInsert();

	_grid.ExpandAll();

	return TRUE;  // 除非设置了控件的焦点，否则返回 TRUE
}

 

