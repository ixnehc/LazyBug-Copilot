#pragma once

//#include "resource.h"

// 分隔条拖动回调函数类型
// 参数：分隔条的新Y坐标（相对于父窗口客户区）
typedef std::function<void(int newSplitterY)> SplitterDragCallback;

class CChatDialogSplitter : public CWnd
{
public:
	CChatDialogSplitter();
	virtual ~CChatDialogSplitter();

	// 创建分隔条窗口
	// rect: 分隔条的初始位置和大小
	// parent: 父窗口
	// nID: 控件ID
	BOOL Create(const CRect& rect, CWnd* parent, UINT nID);

	// 设置拖动回调函数
	void SetDragCallback(SplitterDragCallback callback);

	// 设置分隔条的Y坐标（相对于父窗口）
	void SetSplitterY(int y);

	// 获取分隔条的Y坐标
	int GetSplitterY() const;

	// 设置分隔条的高度
	void SetSplitterHeight(int height);

	// 获取分隔条的高度
	int GetSplitterHeight() const { return _splitterHeight; }

	// 设置可拖动的范围（相对于父窗口客户区）
	void SetDragRange(int minY, int maxY);

protected:
	DECLARE_MESSAGE_MAP()

	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);

private:
	bool _isDragging;           // 是否正在拖动
	CPoint _dragStartPoint;     // 拖动开始时的鼠标位置（屏幕坐标）
	int _dragStartY;            // 拖动开始时分隔条的Y坐标
	int _splitterHeight;        // 分隔条的高度
	int _minY;                  // 最小Y坐标
	int _maxY;                  // 最大Y坐标
	
	SplitterDragCallback _dragCallback; // 拖动回调函数

	// 限制Y坐标在有效范围内
	int _ClampY(int y) const;
};


