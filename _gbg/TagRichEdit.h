#pragma once
#include <vector>
#include <map>
#include <string>

// 标签信息结构
struct TagInfo
{
	CHARRANGE GetRangeWithMarkers() const
	{
		CHARRANGE ret=position;
		ret.cpMin -= startMarker.GetLength();
		ret.cpMax += endMarker.GetLength();
		return ret;
	}
	int id;                 // 标签唯一ID
	CStringW text;           // 标签显示文本
	CStringW description;    // 标签描述信息
	CHARRANGE position;     // 标签在文档中的位置
	COLORREF bgColor;       // 标签背景色
	COLORREF textColor;     // 标签文本色
	CStringW startMarker;    // 标签开始标记
	CStringW endMarker;      // 标签结束标记
};

struct TagRichEditContent
{
	CString m_text;//utf8格式
	std::map<int, TagInfo> m_mapTags;       // 标签集合
	int m_nNextTagID;                        // 下一个标签ID
};

// 标签富文本编辑控件类
class CTagRichEdit : public CRichEditCtrl
{
	DECLARE_DYNAMIC(CTagRichEdit)

public:
	CTagRichEdit();
	virtual ~CTagRichEdit();
	
	// 重写创建相关方法，确保能接收通知消息
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

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
	CStringW GetContentString() const;

	// 重置所有非标签部分的格式为默认格式
	void ResetNonTagFormat();

	// 检查光标位置是否在标签内(光标位置在tag的前沿和后沿都不算在标签内
	BOOL IsCursorInTag(long nPosition, int& nTagID) const;

	//某个字符是否属于标签
	BOOL IsCharInTag(long nPosition, int& nTagID) const;

protected:
	DECLARE_MESSAGE_MAP()

	// 消息处理
	afx_msg void OnEnChange();
	afx_msg void OnProtected(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg LRESULT OnSelectionChanged(WPARAM wParam, LPARAM lParam);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnPaste();

private:
	std::map<int, TagInfo> m_mapTags;       // 标签集合
	int m_nNextTagID;                        // 下一个标签ID
	bool m_bUpdating;                        // 控件更新标志
	CStringW m_strTagFormat;                  // 标签格式模板

	CHARFORMAT2 m_defCF;

	// 生成唯一标记
	void GenerateTagMarkers(TagInfo& tagInfo);

	// 绘制标签
	void DrawTag(const TagInfo& tagInfo);

	// 更新标签位置
	void UpdateTagPositions();

	// 通过标记找到标签位置
	BOOL FindTagPositionByMarkers(const CStringW& startMarker, const CStringW& endMarker, int nTagLength,CHARRANGE& position);

	// 保护标签不被编辑分割
	void ProtectTags();

	// 获取当前光标位置
	long GetCurrentPosition() const;
};
