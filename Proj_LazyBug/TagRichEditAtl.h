#pragma once

#include <atlbase.h> // 基本 ATL 类
#include <atlwin.h>  // ATL 窗口类
#include <richedit.h> // 对于 CHARFORMAT2, CHARRANGE 等 RichEdit 特定结构
#pragma comment(lib, "Msftedit.lib") // Rich Edit 4.1

#include <vector>
#include <map>
// #include <string> // ATL::CString 足够

// 标签信息结构
struct TagInfo
{
	CHARRANGE GetRangeWithMarkers() const
	{
		CHARRANGE ret = position;
		if (ret.cpMin >= (LONG)startMarker.GetLength()) // Ensure no underflow
		{
			ret.cpMin -= startMarker.GetLength();
		}
		else
		{
			ret.cpMin = 0; // Or handle error appropriately
		}
		ret.cpMax += endMarker.GetLength();
		return ret;
	}
	int id;                 // 标签唯一ID
	ATL::CString text;           // 标签显示文本 (以前是 CStringW)
	ATL::CString description;    // 标签描述信息 (以前是 CStringW)
	CHARRANGE position;     // 标签在文档中的位置
	COLORREF bgColor;       // 标签背景色
	COLORREF textColor;     // 标签文本色
	ATL::CString startMarker;    // 标签开始标记 (以前是 CStringW)
	ATL::CString endMarker;      // 标签结束标记 (以前是 CStringW)
};

struct TagRichEditContent
{
	ATL::CString m_text; // 以前是 CString
	std::map<int, TagInfo> m_mapTags;       // 标签集合
	int m_nNextTagID;                        // 下一个标签ID
};

// 标签富文本编辑控件类
class CTagRichEditAtl : public CWindowImpl<CTagRichEditAtl, CWindow>
{
public:
	DECLARE_WND_SUPERCLASS(NULL, MSFTEDIT_CLASS)

	CTagRichEditAtl();
	virtual ~CTagRichEditAtl();
	
	// ATL uses Create method from CWindowImpl, or you can override it.
	// The pParentWnd is HWND in ATL.
	HWND Create(HWND hWndParent, RECT& rcPos, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			UINT_PTR nID = 0, LPVOID lpCreateParam = NULL);

	// virtual BOOL PreCreateWindow(CREATESTRUCT& cs); // MFC specific, handle in Create or Window Traits

	void GetContent(TagRichEditContent& content);
	void SetContent(TagRichEditContent& content);

	// 添加标签
	int AddTag(LPCTSTR lpszTagText, LPCTSTR lpszDescription = NULL,
		COLORREF bgColor = RGB(80, 80, 120), COLORREF textColor = RGB(255, 255, 255));

	int ReplaceSelWithTag(LPCTSTR lpszTagText, LPCTSTR lpszDescription = NULL,
		COLORREF bgColor = RGB(80, 80, 120), COLORREF textColor = RGB(255, 255, 255));


	// 根据ID删除标签
	BOOL RemoveTag(int nTagID);

	// 根据位置删除标签
	BOOL RemoveTagAtPosition(long nPosition);

	// 获取标签信息
	BOOL GetTagInfo(int nTagID, TagInfo& tagInfo) const;

	// 设置标签描述
	BOOL SetTagDescription(int nTagID, LPCTSTR lpszDescription);

	// 获取标签数量
	int GetTagCount() const;

	// 获取所有标签ID
	void GetAllTagIDs(std::vector<int>& tagIDs) const;

	// 清除所有标签
	void ClearAllTags();

	// 根据位置获取标签ID
	int GetTagIDAtPosition(long nPosition) const;

	// 获取不含标记的内容
	ATL::CString GetContentString() const; // 以前是 CString

	// 重置所有非标签部分的格式为默认格式
	void ResetNonTagFormat();

	// 检查光标位置是否在标签内(光标位置在tag的前沿和后沿都不算在标签内
	BOOL IsCursorInTag(long nPosition, int& nTagID) const;

	//某个字符是否属于标签
	BOOL IsCharInTag(long nPosition, int& nTagID) const;

// MFC Message Map
// protected:
//	DECLARE_MESSAGE_MAP()

public: // ATL Message Map
	BEGIN_MSG_MAP(CTagRichEditAtl)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDblClk)
		MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
		MESSAGE_HANDLER(WM_CHAR, OnChar)
		MESSAGE_HANDLER(WM_PASTE, OnPaste)
		NOTIFY_CODE_HANDLER(EN_CHANGE, OnEnChange) // RichEdit notifications are through WM_NOTIFY
		NOTIFY_CODE_HANDLER(EN_PROTECTED, OnProtected)
		// ON_MESSAGE(WM_NOTIFY + 1, OnSelectionChanged) // 自定义消息，用于捕获选择变化 - How was this sent?
                                                     // If it's EM_SELECTIONCHANGE, it's EN_SELCHANGE via WM_NOTIFY
        NOTIFY_CODE_HANDLER(EN_SELCHANGE, OnSelectionChange) // Assuming this was for EN_SELCHANGE
		REFLECT_NOTIFICATIONS() // Important for control notifications like EN_CHANGE
	END_MSG_MAP()


	// 消息处理 (Signatures need to change for ATL)
	LRESULT OnEnChange(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& bHandled);
	LRESULT OnProtected(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLButtonDblClk(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSelectionChange(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& bHandled);
	LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPaste(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);


private:
	std::map<int, TagInfo> m_mapTags;       // 标签集合
	int m_nNextTagID;                        // 下一个标签ID
	bool m_bUpdating;                        // 控件更新标志
	ATL::CString m_strTagFormat;                  // 标签格式模板 (以前是 CStringW)

	CHARFORMAT2 m_defCF;

	// 生成唯一标记
	void GenerateTagMarkers(TagInfo& tagInfo);

	// 绘制标签
	void DrawTag(const TagInfo& tagInfo);

	// 更新标签位置
	void UpdateTagPositions();

	// 通过标记找到标签位置
	BOOL FindTagPositionByMarkers(const ATL::CString& startMarker, const ATL::CString& endMarker, int nTagLength,CHARRANGE& position);

	// 保护标签不被编辑分割
	void ProtectTags();

	// 获取当前光标位置
	long GetCurrentPosition() const;
};
