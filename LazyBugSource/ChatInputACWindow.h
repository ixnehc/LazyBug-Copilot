#pragma once
#include <vector>
#include <string>
#include <functional>

#include <unordered_map>

// 前向声明
namespace Gdiplus {
    class Graphics;
}
using namespace Gdiplus;

#include "ChatInputACListBuilder.h"
#include "ChatInputACItemTip.h"


// 回调函数类型定义
using ACItemSelectedCallback = std::function<void(const ChatInputACItem&)>; // 项目被选中
using ACListCancelledCallback = std::function<void()>; // 列表被取消
using ACRequestItemsCallback = std::function<void(const std::string&)>; // 请求候选项

class CChatInput;
class CChatInputACList;
// 自动补全窗口类
class CChatInputACWindow : public CWnd
{
public:
    CChatInputACWindow();
    virtual ~CChatInputACWindow();

	void Update();

	// 绑定到编辑控件
	BOOL BindToEdit(HWND hWnd);
	void UnbindFromEdit();

	CChatInputACList* GetOwner()	{		return _owner;	}

    // 创建窗口
    BOOL CreateACWindow(CWnd* pEditCtrl, CChatInputACList *owner);

    // 显示/隐藏窗口
    void ShowWindow(const std::vector<ChatInputACItem>& items, int x, int y, int selectedIndex = 0);
    void HideWindow();

    // 更新内容
    void SetSelectedIndex(int index);

    // 设置回调
    void SetItemSelectedCallback(ACItemSelectedCallback callback) { _itemSelectedCallback = callback; }
    
    // 获取项目高度
    int GetItemHeight() const { return _itemHeight; }

	// 重置滚动位置
	void ResetScroll() { _scrollOffset = 0; }

protected:
    afx_msg void OnPaint();
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnCaptureChanged(CWnd* pWnd);
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
    DECLARE_MESSAGE_MAP()

private:

	// 编辑控件窗口过程的子类化
	static LRESULT CALLBACK EditWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    std::vector<ChatInputACItem> _items;
    int _selectedIndex;
    int _hoverIndex;
    ACItemSelectedCallback _itemSelectedCallback;
    
    // UI参数
    int _itemHeight;
    int _maxVisibleItems;
    CFont _font;
    CFont _descFont;
    
    // GDI+缓冲位图相关
    Gdiplus::Bitmap* _bufferBitmap;
    int _bufferWidth;
    int _bufferHeight;
    int _scrollOffset;  // 当前滚动偏移量
    
    // 计算窗口大小
    CSize CalculateWindowSize();
    
    // 使用GDI+绘制项目
    void DrawItemGDIPlus(Graphics* graphics, const ChatInputACItem& item, const CRect& rect, bool selected, bool hovered);
    
    // 确保缓冲位图有效
    void EnsureBufferBitmap(int width, int height);
    
    // 获取项目矩形
    CRect GetItemRect(int index);
    
    // 根据点击位置获取项目索引
    int GetItemFromPoint(CPoint point);
    
    // 确保选中项可见
    void EnsureSelectedVisible();
    
    
    // 前台窗口监控相关
    void CheckForegroundWindow();

	CChatInputACList* _owner;

	// 前台窗口监控
	DWORD _currentProcessId;            // 当前进程ID

    // Tip相关
    CChatInputACItemTip _itemTip;
    UINT_PTR _tipTimerId;
    static const UINT_PTR TIP_TIMER_ID = 1001;
    static const UINT TIP_DELAY_MS = 800; // 800ms后显示tip
    
    void StartTipTimer();
    void StopTipTimer();
    void ShowItemTip();

};

