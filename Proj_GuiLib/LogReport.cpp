#include "stdh.h"
#include ".\logreport.h"
#include "resource.h"
#include "WndBase.h"

#include "log/LogFile.h"


#define ID_MENU_CATEGORY_BASE	40000

CLogReport::_LogRecord * HandleSelChanged(CLogReport * pOwner, XTP_NM_REPORTRECORDITEM*  pItemNotify, LRESULT* /*result*/)
{
	ASSERT(pItemNotify != NULL);

	if(!pItemNotify->pItem)
		return NULL;

	CXTPReportRecord * pRecord = pItemNotify->pItem->GetRecord();
	CXTPReportRecordItem * pItem = pRecord->GetItem(0);
	DWORD_PTR pData = pItem->GetItemData();

	CLogReport::_LogRecord & record = pOwner->_records[pData];
	
	LR_NMHDR &nmhdr = pOwner->_nmhdr;

	memset(&nmhdr,0,sizeof(LR_NMHDR));
	nmhdr.data = &record.data[0];
	nmhdr.size = record.data.size();
	nmhdr.pItem = pItemNotify->pItem;
	nmhdr.code = RN_REPORT_SELCHANGED;
	nmhdr.hwndFrom = pOwner->GetSafeHwnd();
	nmhdr.idFrom = pOwner->GetDlgCtrlID();

	CWnd * pWnd =  pOwner->GetOwner();
	if(!pWnd)
		pWnd = pOwner->GetParent();
	assert(pWnd);

	HWND hWnd = pWnd->GetSafeHwnd();
	assert(hWnd);

	::SendMessage(hWnd,WM_NOTIFY,pData,LPARAM(&nmhdr));

	return & record;
}

class CMessgeRecordIconItem :public CXTPReportRecordItemText
{
public:
	CMessgeRecordIconItem(CLogReport::LogType logType)
		:CXTPReportRecordItemText(_T(""))
	{
		switch(logType)
		{
		case CLogReport::Log_Error:
			{
				SetIconIndex(0);
				SetCaption(_T("error:"));
				break;
			}
		case CLogReport::Log_Waring:
			{
				SetIconIndex(1);
				SetCaption(_T("warning:"));
				break;
			}
		case CLogReport::Log_Normal:
			{
				SetIconIndex(2);
				SetCaption(_T("report:"));
				break;
			}
		}

	}
};

//////////////////////////////////////////////////////////////////////////
class CMessageRecord :public CXTPReportRecord
{
public:
	CMessageRecord(CLogReport::_LogRecord & record,int num,BOOL bMulti);
	CLogReport::GrpKey key;
};
CMessageRecord::CMessageRecord(CLogReport::_LogRecord & record,int num,BOOL bMulti)
{

	tm stm = *_localtime64(&(record.time));

	char fmttime[255];
	memset(fmttime,0,sizeof(fmttime));

	sprintf(fmttime,"%02d:%02d:%02d",stm.tm_hour,stm.tm_min,stm.tm_sec);

	int idIcon = (bMulti)?3:4;
	CXTPReportRecordItem * pItem = NULL;
	pItem = AddItem(new CXTPReportRecordItemText(_T("")));
	pItem->SetIconIndex(idIcon);

	AddItem(new CMessgeRecordIconItem(record.logType));	
	AddItem(new CXTPReportRecordItemText(fromMBCS(record.msg.c_str())));
	AddItem(new CXTPReportRecordItemText(fromMBCS(fmttime)));
	AddItem(new CXTPReportRecordItemNumber(num));
	AddItem(new CXTPReportRecordItemText(fromMBCS(record.place.c_str())));
	AddItem(new CXTPReportRecordItemText(fromMBCS(record.strCategory.c_str())));

	pItem->SetItemData(DWORD_PTR(record.idx));

	key.Set(record.place,record.strCategory,record.msg);
}
//////////////////////////////////////////////////////////////////////////
class CDetailRecord :public CXTPReportRecord
{
public:
	CDetailRecord(CLogReport::_LogRecord & record)
	{
		
		tm stm = *_localtime64(&(record.time));

		char fmttime[255];
		memset(fmttime,0,sizeof(fmttime));
		sprintf(fmttime,"%02d:%02d:%02d",stm.tm_hour,stm.tm_min,stm.tm_sec);

		CXTPReportRecordItem * pItem = NULL;
	
		pItem = AddItem(new CXTPReportRecordItemText(fromMBCS(record.msg.c_str())));
		AddItem(new CXTPReportRecordItemText(fromMBCS(fmttime)));
		
		pItem->SetItemData(DWORD_PTR(record.idx));
	}
};

BEGIN_MESSAGE_MAP(CLogReport::_CReportViewDlg,CDialog)
	ON_NOTIFY(XTP_NM_REPORT_LBUTTONDOWN,0,OnSelChanged)
	ON_WM_SIZE()
	ON_WM_SHOWWINDOW()
ON_WM_CLOSE()
END_MESSAGE_MAP()

void CLogReport::_CReportViewDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);
	_bActive = bShow;
}
void CLogReport::_CReportViewDlg::OnClose()
{
	CDialog::OnClose();
	_bActive = FALSE;
}
BOOL CLogReport::_CReportViewDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	CRect rc;
	GET_CONTROL_RECT(this,IDC_STATIC_REPORTVIEW,rc);
	if(FALSE==m_wndReport.Create(0x50010000,rc,this,0))
		return FALSE;

	m_wndReport.AddColumn(new CXTPReportColumn(0,_T("Message"),40));
	m_wndReport.AddColumn(new CXTPReportColumn(1,_T("Time"),28));
	
	m_wndReport.GetReportHeader()->AllowColumnRemove(FALSE);

	CXTPReportPaintManager * paintMgr= m_wndReport.GetPaintManager();
	paintMgr->m_clrCaptionText = RGB(0,0,255);

	return TRUE;
}
void CLogReport::_CReportViewDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	
	CRect rc,rcrp,rct;
	rc.left = 10; rc.right = cx - 10;
	rc.top = 10; rc.bottom = cy - 10;
	
	rcrp = rc;
	rcrp.bottom = rcrp.top + int((cy-20)*0.6);
	
	rct = rc;
	rct.top = rcrp.bottom;

	m_wndReport.MoveWindow(rcrp);

	CWnd * pEdit = GetDlgItem(IDC_EDIT_MESSAGE);
	pEdit->MoveWindow(rct);
}

void CLogReport::_CReportViewDlg::OnSelChanged(NMHDR*  pNotifyStruct, LRESULT* result)
{
	assert(_pOwner);
	XTP_NM_REPORTRECORDITEM* pItemNotify = (XTP_NM_REPORTRECORDITEM*) pNotifyStruct;
	ASSERT(pItemNotify != NULL);

	if(!pItemNotify->pItem)
		return;

	CLogReport::_LogRecord * record = HandleSelChanged(_pOwner,pItemNotify,result);
	SetDlgItemText(IDC_EDIT_MESSAGE, fromMBCS(record->msg.c_str()));
}

void CLogReport::_CReportViewDlg::ResetContent(CLogReport * pOwner,const GrpKey * grpKey)
{	
	m_wndReport.ResetContent();
	assert(pOwner);

	_pOwner = pOwner;

	if(grpKey){
		std::map<GrpKey,_LogGroup>::iterator itGrp;
		itGrp = pOwner->_recordGrps.find(*grpKey);

		if(itGrp==pOwner->_recordGrps.end())
			return;

		_LogGroup & grp = (*itGrp).second;
		for(int i = 0;i<grp.idxRecords.size();i++)
		{
			_LogRecord & record = pOwner->_records[grp.idxRecords[i]];
			m_wndReport.AddRecord(new CDetailRecord(record));
		}
		m_wndReport.Populate();
	}
}
//////////////////////////////////////////////////////////////////////////
bool CLogReport::_LogRecord::IsEqual(_LogRecord & log)
{
	return (place.compare(log.place)==0&&strCategory.compare(log.strCategory)==0);
}
//////////////////////////////////////////////////////////////////////////

CLogReport::CLogReport(void)
{
	_nCategorys = 0;
	_cur_Category = -1;
}

CLogReport::~CLogReport(void)
{

}

//////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CLogReport,CDialog)
	ON_NOTIFY(XTP_NM_REPORT_LBUTTONDOWN,0,OnReportSelChanged)
	ON_NOTIFY(NM_RCLICK,0,OnNMRDClick)
	ON_CBN_SELCHANGE(IDC_COMBO_SYSTYPE,OnFilterMsg)
	ON_COMMAND(IDC_BUTTON_CLEAR,OnClean)
	ON_COMMAND_RANGE(ID_MENU_CATEGORY_BASE,ID_MENU_CATEGORY_BASE+2000,OnMenuReportCategoryChange)
	ON_WM_TIMER()
	ON_WM_SIZE()
END_MESSAGE_MAP()

void CLogReport::OnMenuReportCategoryChange(UINT idCmd)
{
	_cur_Category = idCmd-ID_MENU_CATEGORY_BASE - 1;
	UpdateMsg();
	if(_dlgDetail.IsActive())
		_dlgDetail.ResetContent(this,NULL);
}

void CLogReport::OnClean()
{
	Clean();
	UpdateMsg();

	if(_dlgDetail.IsActive())
		_dlgDetail.ResetContent(this,NULL);
	
	m_comBoxList.ResetContent();
	int idx = m_comBoxList.AddString(_T("All"));
	m_comBoxList.SetItemData(idx,0xffffffff);
	m_comBoxList.SetCurSel(idx);
}

void CLogReport::OnFilterMsg()
{
	int idx = m_comBoxList.GetCurSel();
	if(idx<0)
		_cur_Category = -1;
	else
	{
		DWORD_PTR t = m_comBoxList.GetItemData(idx);
		_cur_Category = (t==0xffffffff)?-1:int(t);
	}

	UpdateMsg();
	if(_dlgDetail.IsActive())
		_dlgDetail.ResetContent(this,NULL);
}

void CLogReport::Clean()
{
	_cur_Category = -1;
	_nCategorys = 0;
	_records.clear();
	_recordGrps.clear();
	_categorys.clear();
	_idxsDirty.clear();
}

void CLogReport::RemoveRecord(const GrpKey &key)
{
	CXTPReportRecords * records = m_wndReport.GetRecords(); 
	
	int sz = records->GetCount();
	for(int i = 0;i<sz;i++)
	{
		CMessageRecord * r = static_cast<CMessageRecord *>(records->GetAt(i));
		if(r->key.IsEquals(key))
		{
			m_wndReport.RemoveRecordEx(r);
			break;
		}
	}
}

//加入一条记录
void CLogReport::AddMsg(int idx)
{
	if(idx>=_records.size()&&idx<0)
		return;

	_LogRecord & record = _records[idx];

	//当前显示种类 过滤
	if(_cur_Category!= -1&&_cur_Category != record.idx_category)
		return;

	std::map<GrpKey,_LogGroup>::iterator itGrp;
	
	GrpKey key(record.place,record.strCategory,record.msg);
	itGrp = _recordGrps.find(key);

	if(itGrp==_recordGrps.end())
	{
		_LogGroup grp;
		grp.idxRecords.push_back(record.idx);
		grp.key = key;
		_recordGrps[key] = grp;
		m_wndReport.AddRecord(new CMessageRecord(_records[idx],1,FALSE));
	}
	else
	{
		_LogGroup & grp = (*itGrp).second;

		RemoveRecord(key);
		grp.idxRecords.push_back(record.idx);
		int nRds = int(grp.idxRecords.size());
		m_wndReport.AddRecord(new CMessageRecord(_records[idx],nRds,nRds-1));
	}

	m_wndReport.Populate();

	int nItems = _recordGrps.size();
	if(nItems)
		m_wndReport.SetTopRow(nItems-1);
}

void CLogReport::OnTimer(UINT_PTR nIDEvent)
{
	//更新下列几行数据
	for(int i = 0;i<_idxsDirty.size();i++){  
		int idx = _idxsDirty[i];
		
		AddMsg(idx);
		
		assert(idx<_records.size());
		_LogRecord &record = _records[idx];
		assert(idx==record.idx);
		
		//如果当前条信息为 当前信息则更新小窗口
		GrpKey key(record.place,record.strCategory,record.msg);
		if(_dlgDetail.IsActive()&&key.IsEquals(_dlgDetail.CurPlace()))
			_dlgDetail.ResetContent(this,NULL);

		NotifyMessage(record,RN_REPORT_ADDRECORD);
	}

	_idxsDirty.clear();
}

void CLogReport::UpdateMsg()
{
	m_wndReport.ResetContent();
	
	_recordGrps.clear();
	std::map<GrpKey,_LogGroup>::iterator itGrp;
	
	// 合并 同行显示的 组
	for(int i = 0;i<_records.size();i++)
	{
		_LogRecord & record = _records[i];
		
		//过滤 非显示种类的 消息
		if(_cur_Category!= -1&&_cur_Category != record.idx_category)
			continue;
		
		//按照 位置 合并显示组
		GrpKey key(record.place,record.strCategory,record.msg);
		itGrp = _recordGrps.find(key);	
		if(itGrp==_recordGrps.end())
		{
			_LogGroup grp;
			grp.idxRecords.push_back(record.idx);
			grp.key = key;
			_recordGrps[key] = grp;
		}
		else
		{
			_LogGroup & grp = (*itGrp).second;
			grp.idxRecords.push_back(record.idx);
		}
	}
	
	int nItems = 0;

	for(itGrp = _recordGrps.begin();itGrp!=_recordGrps.end();itGrp++)
	{
		_LogGroup & grp = (*itGrp).second;
	 	int idx = grp.idxRecords.back();
		int nRds = int(grp.idxRecords.size());
		m_wndReport.AddRecord(new CMessageRecord(_records[idx],nRds,nRds>1));
		
		nItems++;
	}
	
	m_wndReport.Populate();
	
	if(nItems)
		m_wndReport.SetTopRow(nItems-1);
}

void CLogReport::OnDetail(const GrpKey *key)
{
	if(!_dlgDetail.GetSafeHwnd()&&!_dlgDetail.Create(IDD_DIALOG_REPORTVIEW,this)) 
		return;
	_dlgDetail.ResetContent(this,key);
	_dlgDetail.ShowWindow(SW_SHOW);
}

BOOL CLogReport::Create(DWORD ctrlID,CWnd * pParent,CRect rc)
{
	if(FALSE == CDialog::Create(IDD_DIALOG_REPORTRB,pParent))
		return FALSE;
	
	SetDlgCtrlID(ctrlID);

	MoveWindow(rc);

	pParent->ScreenToClient(rc);

	GetClientRect(rc);

	if(FALSE==m_wndReport.Create(WS_CHILD|WS_VISIBLE|WS_HSCROLL|WS_BORDER,rc,this,0))
		return FALSE;

	m_comBoxList.SubclassDlgItem(IDC_COMBO_SYSTYPE,this);

	m_imagelist.Create(16,16,ILC_COLOR32|ILC_MASK,0,1);
	CBitmap bitMap;
	bitMap.LoadBitmap(IDB_BMREPORT);
	m_imagelist.Add(&bitMap,RGB(255,255,255));

	m_wndReport.SetImageList(&m_imagelist);

	m_wndReport.GetReportHeader()->AllowColumnRemove(FALSE);
	m_wndReport.ModifyStyle(0, WS_CLIPCHILDREN|WS_CLIPSIBLINGS|WS_TABSTOP);

	CXTPReportColumn * pCol = 0;
	pCol = m_wndReport.AddColumn(new CXTPReportColumn(0,_T(""),20,FALSE));
	pCol->SetIconID(5);

	m_wndReport.AddColumn(new CXTPReportColumn(1,_T("Type"),8,TRUE));
	m_wndReport.AddColumn(new CXTPReportColumn(3,_T("Time"),8));
	m_wndReport.AddColumn(new CXTPReportColumn(6,_T("Category"),10));
	m_wndReport.AddColumn(new CXTPReportColumn(2,_T("Message"),40));
	pCol = m_wndReport.AddColumn(new CXTPReportColumn(5,_T("Location"),40));
	m_wndReport.AddColumn(new CXTPReportColumn(4,_T("Count"),4));
	
	CXTPReportPaintManager * paintMgr= m_wndReport.GetPaintManager();
	paintMgr->m_clrCaptionText = RGB(0,0,255);
	paintMgr->m_clrBtnFace = RGB(0,255,0);

	OnClean();
	
	SetTimer(0,1000,NULL);

	return TRUE;
}

void CLogReport::Error(const char * pErrorMsg,const char * place/*= NULL*/,const char * pCategory/* = NULL*/,void * data/* = NULL*/,size_t size/* = 0*/)
{
    Msg(pErrorMsg,place,pCategory,Log_Error,data,size);
}

void CLogReport::Warning(const char * pWarning,const char * place/*= NULL*/,const char * pCategory/* = NULL*/,void * data /*= NULL*/,size_t size/* = 0*/)
{
	Msg(pWarning,place,pCategory,Log_Waring,data,size);
}

void CLogReport::Msg(const char * pMsg,const char * place /*= NULL*/,const char * pCategory/* = NULL*/,LogType logType/* = Log_Normal*/,void * data /*= NULL*/,size_t size/* = 0*/)
{	
	if(pMsg==NULL)
		return;

	_LogRecord record;
	record.msg = pMsg;
	if(place)
		record.place = place;

	if(data&&size>0)
	{
		record.data.resize(size);
		memcpy(&record.data[0],data,size);
	}

	record.logType = logType;
	_time64(&(record.time));

	if(pCategory)
		record.strCategory = pCategory;
	
	record.idx = _records.size();	//

	std::map<std::string,int>::iterator it;
	it = _categorys.find(record.strCategory);
	
	DWORD idxCategory = 0xffffffff;
	if(it==_categorys.end())
	{
		_categorys[record.strCategory] = _nCategorys;
		idxCategory = _nCategorys;
		int idx = m_comBoxList.AddString(fromMBCS(record.strCategory.c_str()));
		m_comBoxList.SetItemData(idx,_nCategorys);
	
		_nCategorys++;
	}
	else
		idxCategory = (*it).second;


	record.idx_category = idxCategory; //

	_records.push_back(record);
	
	//加入到更新列表
	_idxsDirty.push_back(record.idx);
}

void CLogReport::NotifyMessage(_LogRecord & record,DWORD notifyCode)
{
	LR_NMHDR &nmhdr = _nmhdr;
	memset(&nmhdr,0,sizeof(LR_NMHDR));
	nmhdr.data = &record.data[0];
	nmhdr.size = record.data.size();
	nmhdr.pItem = NULL;
	nmhdr.code = notifyCode;
	nmhdr.hwndFrom = m_hWnd;
	nmhdr.idFrom = GetDlgCtrlID();

	CWnd * pWnd = GetOwner();
	if(!pWnd)
		pWnd = GetParent();
	assert(pWnd);

	HWND hWnd = pWnd->GetSafeHwnd();
	assert(hWnd);

	::SendMessage(hWnd,WM_NOTIFY,0,LPARAM(&nmhdr));
}

void CLogReport::OnReportSelChanged(NMHDR*  pNotifyStruct, LRESULT * result)
{
	XTP_NM_REPORTRECORDITEM* pItemNotify = (XTP_NM_REPORTRECORDITEM*) pNotifyStruct;
	ASSERT(pItemNotify != NULL);
	
	if(!pItemNotify->pItem)
		return;

	_LogRecord * record = HandleSelChanged(this,pItemNotify,result);
	
	if(record&&pItemNotify->pColumn->GetIndex()==0){
		GrpKey key(record->place,record->strCategory,record->msg);	
		OnDetail(&key);
	}
}

void CLogReport::OnNMRDClick(NMHDR*  pNotifyStruct, LRESULT* /*result*/)
{
	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, IDC_BUTTON_CLEAR, _T("Clear"));

	CMenu subMenu;
	subMenu.CreatePopupMenu();
	
	std::string nameActiveCategory = "All";

	DWORD checkFlag = (_cur_Category==-1)?MF_CHECKED:MF_UNCHECKED;
	subMenu.AppendMenu(MF_STRING|checkFlag,ID_MENU_CATEGORY_BASE, _T("All"));

	std::map<std::string,int>::iterator it;
	for(it=_categorys.begin();it!=_categorys.end();it++){
		std::string name = it->first;
		DWORD idCtrl = ID_MENU_CATEGORY_BASE + it->second +1;
		checkFlag = (it->second==_cur_Category)?MF_CHECKED:MF_UNCHECKED;
		subMenu.AppendMenu(MF_STRING | checkFlag, idCtrl, fromMBCS(name.c_str()));
		
		if((it->second==_cur_Category))
			nameActiveCategory = name;
	}
	
	HMENU hMenu = subMenu.GetSafeHmenu();
	std::string nameMenu = "Category[";
	nameMenu.append(nameActiveCategory);
	nameMenu.append("]");
	menu.AppendMenu(MF_POPUP,UINT_PTR(hMenu), fromMBCS(nameMenu.c_str()));
	POINT pt;
	GetCursorPos(&pt);
	menu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON,pt.x,pt.y,this);
}

void CLogReport::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	CRect rc;
	rc.left = 0;
	rc.right = cx;
	rc.top = 0;
	rc.bottom = cy;
	
	if(m_wndReport.GetSafeHwnd())
		m_wndReport.MoveWindow(rc);
}

