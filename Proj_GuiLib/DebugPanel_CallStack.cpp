#include "stdh.h"
#include "resource.h"
#include "WndBase.h"

#include "DebugPanel_CallStack.h"

#include "GuiData.h"
#include "GuiData_FrameProxy.h"
#include "GuiData_Debugger.h"

//////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CDbgPanel_CallStack,CGuiPanel)
	ON_WM_SIZE()
	ON_NOTIFY(NM_DBLCLK, 0, OnReportItemDblClick)
END_MESSAGE_MAP()



CDbgPanel_CallStack::CDbgPanel_CallStack(CWnd* pParent):CGuiPanel(IDD_DEBUG_CALLSTACK, pParent)
{
	_breakid=0;
	_bRunning=FALSE;
}

BOOL CDbgPanel_CallStack::Create(CWnd *pParent)	
{		
	return CDialog::Create(IDD_DEBUG_CALLSTACK,pParent);	
}


BOOL CDbgPanel_CallStack::OnInitDialog()
{
	CGuiPanel::OnInitDialog();

	CRect rc;
	GET_CONTROL_RECT(this,IDC_LIST,rc);
	HIDE_CONTROL(this,IDC_LIST);
	if(FALSE==_report.Create(0x50010000,rc,this,0))
		return FALSE;

	_imagelist.Create(16,16,ILC_COLOR32|ILC_MASK,0,1);
	CBitmap bmp;
	bmp.LoadBitmap(IDB_CALLSTACK);
	_imagelist.Add(&bmp,RGB(255,255,255));

	_report.SetImageList(&_imagelist);

	CXTPReportColumn *column=new CXTPReportColumn(0,_T(""),4);
	column->SetMinWidth(32);
	_report.AddColumn(column);

	_report.AddColumn(new CXTPReportColumn(1,_T("Location"),64));
	_report.AddColumn(new CXTPReportColumn(2,_T("Funciton"),256));
	_report.AddColumn(new CXTPReportColumn(3,_T("Language"),32));

	_report.GetReportHeader()->AllowColumnRemove(FALSE);

	_RecalcLayout();

	_breakid=0;

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDbgPanel_CallStack::_RecalcLayout()
{
	extern void SetWindowPos(CWnd *pWnd,i_math::recti &rc);

	i_math::recti rc;

	GetClientRect((LPRECT)&rc);

	SetWindowPos(&_report,rc);

}



void CDbgPanel_CallStack::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	_RecalcLayout();
}

void CDbgPanel_CallStack::Reset()
{

}

void CDbgPanel_CallStack::_AddStackInfo(DebugStackInfo *info,int iStack)
{
	CXTPReportRecord *record=new CXTPReportRecord;

	CXTPReportRecordItem * pItem = NULL;
	pItem = record->AddItem(new CXTPReportRecordItemText(_T("")));
	pItem->SetItemData(FORCE_TYPE(DWORD_PTR,iStack));

	if (!info->bCPP)
	{
		std::string s;
		s=s+" ["+info->nameProto+"."+info->nameProtoNode+"]";
		record->AddItem(new CXTPReportRecordItemText(fromMBCS(s.c_str())));
		record->AddItem(new CXTPReportRecordItemText(fromMBCS(info->nameFunc.c_str())));
		record->AddItem(new CXTPReportRecordItemText(_T("lua")));
	}
	else
	{
		std::string s;
		if (info->nameProto!="")
			s=s+" ["+info->nameProto+"."+info->nameProtoNode+"]";
		else
            s=s+" ["+info->location+"(Outside)]     "+info->nameFunc;
		pItem = new CXTPReportRecordItemText(fromMBCS(s.c_str()));
		pItem->SetTextColor(RGB(0x5f,0x5f,0x5f));
		record->AddItem(pItem);
		pItem=new CXTPReportRecordItemText(fromMBCS(info->nameFunc.c_str()));
		pItem->SetTextColor(RGB(0x5f,0x5f,0x5f));
		record->AddItem(pItem);
		pItem = new CXTPReportRecordItemText(_T("c++"));
		pItem->SetTextColor(RGB(0x5f,0x5f,0x5f));
		record->AddItem(pItem);
	}

	_report.AddRecord(record);
}

void CDbgPanel_CallStack::_UpdateIndicator()
{
	GuiData_Debugger*dataDebugger=(GuiData_Debugger*)FindData("debugger");
	if (!dataDebugger)
		return;

	int iCurStack=dataDebugger->context->dbgr->GetCurStack();

	CXTPReportRecords*records=_report.GetRecords();

	BOOL bModified=FALSE;
	for (int i=0;i<records->GetCount();i++)
	{
		CXTPReportRecord *record=records->GetAt(i);
		CXTPReportRecordItem * item= record->GetItem(0);

		int idxOld=item->GetIconIndex();
		if (i==iCurStack)
		{
			if (i==0)
				item->SetIconIndex(0);
			else
				item->SetIconIndex(1);
		}
		else
		{
			if (i==0)
				item->SetIconIndex(0);
			else
				item->SetIconIndex(-1);
		}
		if (idxOld!=item->GetIconIndex())
			bModified=TRUE;
	}

	if (bModified)
		_report.RedrawControl();

}



void CDbgPanel_CallStack::UpdateUI()
{

	GuiData_Debugger*dataDebugger=(GuiData_Debugger*)FindData("debugger");
	assert(dataDebugger);

	CPrlFrameProxy *proxy=((GuiData_PrlFrameProxy*)FindData("prlframeproxy"))->proxy;

	IDebugger *dbgr=dataDebugger->context->dbgr;
	BreakID bid=dbgr->GetBreakID();
	BOOL bRunning=dataDebugger->context->IsRunning();
	if (bid!=_breakid)
	{
		_report.ResetContent(TRUE);
		if (bid)
		{

			DWORD c=dbgr->GetStackCount();

			for (int i=0;i<c;i++)
			{
				DebugStackInfo *info=dbgr->GetStackInfo(i);

				if (i==0)
					proxy->GotoLuaSrc(info->protoid,info->nodeid,info->iLine);

				_AddStackInfo(info,i);
			}

			_report.Populate();
		}
		else
		{
			//切换回当前正在测试的proto的appearance画面
			if ((dataDebugger->context->protoid!=ProtoID_Null)&&(!dataDebugger->context->bNeedStop))
			{
				if (!proxy->GotoAppearance(ProtoID_Null))
					proxy->GotoAppearance(dataDebugger->context->protoid);
			}
		}
		_breakid=bid;
	}

	if (_bRunning!=bRunning)
	{
		if ((bRunning)&&(!dataDebugger->context->bNeedStop))
		{
			if (!proxy->GotoAppearance(ProtoID_Null))
				proxy->GotoAppearance(dataDebugger->context->protoid);			//切换回当前正在测试的proto的appearance画面
		}

		_bRunning=bRunning;
	}

	_UpdateIndicator();
}

void CDbgPanel_CallStack::OnReportItemDblClick(NMHDR * pNotifyStruct, LRESULT * result)
{
	XTP_NM_REPORTRECORDITEM* pItemNotify = (XTP_NM_REPORTRECORDITEM*) pNotifyStruct;

	if (pItemNotify->pRow)
	{
		CXTPReportRecord *record=pItemNotify->pRow->GetRecord();
		CXTPReportRecordItem * pItem =record->GetItem(0);

		int iStack=(int)pItem->GetItemData();

		GuiData_Debugger*dataDebugger=(GuiData_Debugger*)FindData("debugger");
		assert(dataDebugger);
		CPrlFrameProxy *proxy=((GuiData_PrlFrameProxy*)FindData("prlframeproxy"))->proxy;

		if (dataDebugger->context->dbgr->IsBreak())
		{
			DebugStackInfo *info=dataDebugger->context->dbgr->GetStackInfo(iStack);
			if (info)
			{
				dataDebugger->context->dbgr->SetCurStack(iStack);
				proxy->GotoLuaSrc(info->protoid,info->nodeid,info->iLine);
			}
		}
	}

}
