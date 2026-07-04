#include "stdh.h"
#include "ChatDialogSplitter.h"

CChatDialogSplitter::CChatDialogSplitter()
	: _isDragging(false)
	, _splitterHeight(6)
	, _minY(100)
	, _maxY(1000)
	, _dragStartY(0)
{
}

CChatDialogSplitter::~CChatDialogSplitter()
{
}

BEGIN_MESSAGE_MAP(CChatDialogSplitter, CWnd)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()

BOOL CChatDialogSplitter::Create(const CRect& rect, CWnd* parent, UINT nID)
{
	// 注册窗口类
	LPCTSTR className = AfxRegisterWndClass(
		CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW,
		::LoadCursor(NULL, IDC_SIZENS), // 使用上下调整大小的光标
		(HBRUSH)(COLOR_3DFACE + 1),
		NULL
	);

	// 创建窗口
	return CWnd::Create(className, _T(""), WS_CHILD | WS_VISIBLE, rect, parent, nID);
}

void CChatDialogSplitter::SetDragCallback(SplitterDragCallback callback)
{
	_dragCallback = callback;
}

void CChatDialogSplitter::SetSplitterY(int y)
{
	if (!GetSafeHwnd())
		return;

	y = _ClampY(y);

	CRect rect;
	GetWindowRect(&rect);
	GetParent()->ScreenToClient(&rect);

	rect.top = y;
	rect.bottom = y + _splitterHeight;

	MoveWindow(&rect);
}

int CChatDialogSplitter::GetSplitterY() const
{
	if (!GetSafeHwnd())
		return 0;

	CRect rect;
	GetWindowRect(&rect);
	GetParent()->ScreenToClient(&rect);

	return rect.top;
}

void CChatDialogSplitter::SetSplitterHeight(int height)
{
	_splitterHeight = height;
	
	if (GetSafeHwnd())
	{
		CRect rect;
		GetWindowRect(&rect);
		GetParent()->ScreenToClient(&rect);
		
		rect.bottom = rect.top + _splitterHeight;
		MoveWindow(&rect);
	}
}

void CChatDialogSplitter::SetDragRange(int minY, int maxY)
{
	_minY = minY;
	_maxY = maxY;
}

int CChatDialogSplitter::_ClampY(int y) const
{
	if (y < _minY)
		return _minY;
	if (y > _maxY)
		return _maxY;
	return y;
}

void CChatDialogSplitter::OnPaint()
{
	CPaintDC dc(this);

	CRect rect;
	GetClientRect(&rect);

	// 绘制分隔条背景
	dc.FillSolidRect(&rect, RGB(200, 200, 200));

	// 绘制边框
	CPen pen(PS_SOLID, 1, RGB(160, 160, 160));
	CPen* pOldPen = dc.SelectObject(&pen);
	
	dc.MoveTo(rect.left, rect.top);
	dc.LineTo(rect.right, rect.top);
	dc.MoveTo(rect.left, rect.bottom - 1);
	dc.LineTo(rect.right, rect.bottom - 1);
	
	dc.SelectObject(pOldPen);

	// 在中间绘制两条短线作为拖动提示
	CPen handlePen(PS_SOLID, 1, RGB(120, 120, 120));
	pOldPen = dc.SelectObject(&handlePen);

	int centerY = rect.Height() / 2;
	int centerX = rect.Width() / 2;
	int handleWidth = 30;

	// 第一条短线
	dc.MoveTo(centerX - handleWidth / 2, centerY - 2);
	dc.LineTo(centerX + handleWidth / 2, centerY - 2);

	// 第二条短线
	dc.MoveTo(centerX - handleWidth / 2, centerY + 2);
	dc.LineTo(centerX + handleWidth / 2, centerY + 2);

	dc.SelectObject(pOldPen);
}

void CChatDialogSplitter::OnLButtonDown(UINT nFlags, CPoint point)
{
	_isDragging = true;
	
	// 记录拖动开始时的位置
	ClientToScreen(&point);
	_dragStartPoint = point;
	
	CRect rect;
	GetWindowRect(&rect);
	GetParent()->ScreenToClient(&rect);
	_dragStartY = rect.top;

	// 捕获鼠标
	SetCapture();

	CWnd::OnLButtonDown(nFlags, point);
}

void CChatDialogSplitter::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (_isDragging)
	{
		_isDragging = false;
		ReleaseCapture();
	}

	CWnd::OnLButtonUp(nFlags, point);
}

void CChatDialogSplitter::OnMouseMove(UINT nFlags, CPoint point)
{
	if (_isDragging)
	{
		// 将当前位置转换为屏幕坐标
		ClientToScreen(&point);

		// 计算Y方向的偏移量
		int deltaY = point.y - _dragStartPoint.y;

		// 计算新的Y坐标
		int newY = _dragStartY + deltaY;
		newY = _ClampY(newY);

		// 移动分隔条
		CRect rect;
		GetWindowRect(&rect);
		GetParent()->ScreenToClient(&rect);

		rect.top = newY;
		rect.bottom = newY + _splitterHeight;

		MoveWindow(&rect);

		// 调用回调函数通知父窗口
		if (_dragCallback)
		{
			_dragCallback(newY);
		}
	}

	CWnd::OnMouseMove(nFlags, point);
}

BOOL CChatDialogSplitter::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// 设置光标为上下调整大小的样式
	::SetCursor(::LoadCursor(NULL, IDC_SIZENS));
	return TRUE;
}


