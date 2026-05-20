// ResourceSelectorDlg.h : 头文件
//

#pragma once
#include "ResourceListCtrl.h"
#include "TextureViewer.h"

// CResSelectorDlg 对话框
class CResSelectorDlg : public CDialog, public CResourceSelector
{
	// 构造
public:
	CResSelectorDlg(IRenderSystem* rs, LPCTSTR pszRootDir = NULL, LPCTSTR pszFilter = NULL, CWnd* pParent = NULL);

	// 对话框数据
	enum { IDD = IDD_RESOURCESELECTOR_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

public:
	virtual void SetSelectedMode(int mode);
	virtual void SetRootDir(const char* dir);
	virtual const char* GetRootDir() const;
	virtual void SetFilter(const char* filter);
	virtual const char* GetSelResource() const;
	virtual const RECT& GetSelectedRect() const;

	void GetUnitSize(DWORD &w,DWORD &h);

public:
	void OnLbnDblClick();
	/*afx_msg*/ void OnLbnSelchangeFilelist();
	afx_msg void OnSelChange(NMHDR * pItem,LRESULT * pResult);
	// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnCopyPath();
	DECLARE_MESSAGE_MAP()

	LRESULT OnFLBrowseDirChanged(WPARAM, LPARAM);
	LRESULT OnTVUpdateInfo(WPARAM, LPARAM);

	void _GetUnitSizeRef(DWORD *&pw,DWORD *&ph);

	void _RecordUnitSizeRef();

public:
	static std::string ms_root;			// 浏览的根目录，浏览不可超过此目录到达上级目录
	static std::string ms_filter;		// 文件过滤选项
	static int ms_selectMode;		// 选择模式，部分、全选
	static std::string ms_cursel;		// 正在浏览的目录
	static std::string ms_lastsel;
	static CRect ms_lastrect;		

private:

	IRenderSystem* _pRS;
	CResourceListCtrl _lcResources;	// 资源列表
	CTextureViewer _texViewer;		// 资源显示视图
	CStatic _lbFileName;			// 正在浏览的文件名
	CComboBox _cmbFileType;			// 所有可浏览的文件类型

};