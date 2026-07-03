#pragma once
#include <wrl.h>
#include <WebView2.h>
#include <functional>
#include <string>

// CChatMcpTip - 用于显示MCP Tool描述的tip窗口
class CChatMcpTip : public CWnd
{
public:
    CChatMcpTip();
    virtual ~CChatMcpTip();

    // 创建tip窗口
    BOOL CreateMcpTipWindow(CWnd* pParent);

    // 显示tip窗口（传入tool的md描述文本）
    void ShowTip(const RECT& anchorRect, const std::string& toolMd);

    // 隐藏tip窗口
    void HideTip();

    // 更新（用于前台窗口检测）
    void Update();

protected:
    HRESULT InitializeWebView();

    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDestroy();
    afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnTimer(UINT_PTR nIDEvent);

    DECLARE_MESSAGE_MAP()

private:
    // WebView2相关组件
    ICoreWebView2Environment* _webViewEnvironment;
    ICoreWebView2* _webView;
    ICoreWebView2Controller* _controller;

    // 事件处理Token
    EventRegistrationToken _navigationCompletedToken = {};
    EventRegistrationToken _webMessageReceivedToken = {};

    // 状态标志
    bool _isWebViewCreated;
    bool _isUIInitialized;

    // 窗口尺寸
    int _windowWidth;
    int _windowHeight;

    // 窗口尺寸范围（内容自适应时在此范围内调整）
    int _minWindowWidth;
    int _minWindowHeight;
    int _maxWindowWidth;
    int _maxWindowHeight;

    // 前台窗口监控
    DWORD _currentProcessId;

    // 当前tool的md描述
    std::string _currentToolMd;

    // 当前锚点矩形（收到contentSize后重新计算位置时使用）
    CRect _currentAnchorRect;

    // 内容尺寸是否已就绪（用于控制显示时机）
    bool _isContentSized;

    // 计算窗口大小和位置（按指定宽高）
    CRect CalculateWindowRect(const RECT& anchorRect, int width, int height);

    // 处理来自WebView的内容尺寸消息
    void _OnContentSize(int contentWidth, int contentHeight);


    // 检查WebView是否已初始化
    bool _IsReady() const;

    // 发送消息到WebView
    void _PostWebMessage(const std::wstring& action, const std::wstring& data);

    // 发送tool内容到WebView
    void _SendToolContent();

    // 前台窗口监控
    void CheckForegroundWindow();

    // 显示延迟定时器
    UINT_PTR _showTimerId = 0;
    // 尺寸稳定定时器（最后一次收到contentSize后300ms无新尺寸才显示）
    UINT_PTR _sizeStableTimerId = 0;
    CRect _pendingWindowRect;

    // 显示窗口（由定时器触发）
    void _DoShowWindow();
};


