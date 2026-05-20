#pragma once
#include "GuiLib.h"
#include <map>
#include <set>

#define RN_REPORT_SELCHANGED  0x1
#define RN_REPORT_ADDRECORD   0x2

struct LR_NMHDR :public NMHDR
{
	CXTPReportRecordItem * pItem;
	int size;
	void * data;
};
class CLogReport;
class GuiLib_Api CLogReport :public CXTPDialog
{

public:
	~CLogReport(void);
	CLogReport(void);

	enum LogType
	{
		Log_Error,
		Log_Waring,
		Log_Normal,
	};
public:
	//
	BOOL Create(DWORD ctrlID,CWnd * pParent,CRect rc);
	
	void Error(const char * pErrorMsg,const char * place = NULL,const char * pCategory = NULL,void * data = NULL,size_t size = 0);
	void Warning(const char * pWarning,const char * place = NULL,const char * pCategory = NULL,void * data = NULL,size_t size = 0);
	void Msg(const char * pMsg,const char * place = NULL,const char * pCategory = NULL,LogType logType = Log_Normal,void * data = NULL,size_t size = 0);
	void Clean();
	
//	CLogReport & operator << (const char * pMsg);
	//<< category 
	//	kind of msg --num -- msg -- palce -- time
	
	struct _LogRecord
	{
		_LogRecord(){ idx_category = -1;idxGrp = -1;}
		int idx_category;
		std::string strCategory;
		std::string msg;
		std::string place;
		__time64_t time;
		LogType logType;

		DWORD idx;
		std::vector<BYTE> data;
		
		int idxGrp;

		bool IsEqual(_LogRecord & log);
	};

	struct GrpKey
	{
		std::string place;
		std::string category;
		std::string msg;
		GrpKey(void){place="";category="";}
		
		BOOL IsEquals(const GrpKey &oth)
		{
			return (place.compare(oth.place)==0)&&(category.compare(oth.category)==0)&&(msg.compare(oth.msg)==0);
		}

		GrpKey(const std::string& p,const std::string &cat,const std::string &message)
		{
			Set(p,cat,message);
		}		
		
		void Set(const std::string& p,const std::string& cat,const std::string& message)
		{
			place = p;
			category = cat;
			msg = message;
		}
		
		bool operator <(const GrpKey &oth) const
		{
			return (place.compare(oth.place)<0||
				(place.compare(oth.place)==0&&category.compare(oth.category)<0)||
				(place.compare(oth.place)==0&&category.compare(oth.category)==0&&msg.compare(msg)));
		}
	};

	struct _LogGroup
	{
		GrpKey key;
		std::vector<int> idxRecords;
	};

	class _CReportViewDlg :public CDialog
	{
	public:
		_CReportViewDlg(){_pOwner = NULL; _bActive = FALSE;}
		BOOL OnInitDialog();
		void ResetContent(CLogReport * pOwner,const GrpKey * grpKey);
		BOOL IsActive(){return _bActive;}
		const GrpKey & CurPlace(){return _key;}
	protected:
		DECLARE_MESSAGE_MAP()
		afx_msg void OnSelChanged(NMHDR*  pNotifyStruct, LRESULT* /*result*/);

		CXTPReportControl m_wndReport;
		CLogReport * _pOwner;
		GrpKey _key;
		BOOL _bActive;
	public:
		afx_msg void OnSize(UINT nType, int cx, int cy);
		afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
		afx_msg void OnClose();
	};

protected:

	DECLARE_MESSAGE_MAP()
	afx_msg void OnReportSelChanged(NMHDR*  pNotifyStruct, LRESULT* /*result*/);
	afx_msg void OnFilterMsg();
	afx_msg void OnClean();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnNMRDClick(NMHDR*  pNotifyStruct, LRESULT* /*result*/);
	afx_msg void OnMenuReportCategoryChange(UINT idCmd);

	void OnOK(){}
	void OnCancel(){}

	void OnDetail(const GrpKey * key);
	
	void UpdateMsg();

	// Add a new message  idx:record index
	void AddMsg(int idx);
	
	void NotifyMessage(_LogRecord & record,DWORD notifyCode);

	void RemoveRecord(const GrpKey &key);

	friend class CReportViewDlg;
	friend CLogReport::_LogRecord * HandleSelChanged(CLogReport * pOwner, XTP_NM_REPORTRECORDITEM*  pItemNotify, LRESULT* /*result*/);


protected:

	CXTPOfficeBorder<CXTPReportControl,false> m_wndReport;
	
	//消息种类 到 种类索引的 映射
	std::map<std::string,int>  _categorys;

	//消息种类的集合 （消息种类的名称 到 该种类所有消息索引集合 的映射）
	std::map<GrpKey,_LogGroup> _recordGrps;

	// 消息的集合
	std::vector<_LogRecord>	_records;

	int _nCategorys;
		
	CComboBox m_comBoxList;

	LR_NMHDR _nmhdr;
	CImageList m_imagelist;
	
	_CReportViewDlg _dlgDetail;
	
	//防止递归的调用 ，更新界面引起的一系列操作（如：WM_PAINT）如果用户在WM_PAINT中发Log信息则会发生递归调用
	//模式： 用户Log_Dump->更新操作->用户Log_Dump
	std::vector<int> _idxsDirty;
	int _cur_Category;

public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
};
