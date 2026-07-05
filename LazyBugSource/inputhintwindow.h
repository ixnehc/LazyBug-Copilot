#pragma once
#include <wrl.h>
#include <WebView2.h>
#include <string>
#include "Utils_InputHint.h"

// CInputHintWindow - 用于显示 Utils::DiffedInputContent 的提示窗口
// 每个字符/tag 的状态(diffStates): 0-正常显示  1-绿色背景显示  2-不显示
// 使用 WebView 显示, 窗口大小自适应内容(参考 CChatMcpTip)
class CInputHintWindow : public CWnd
{
public:
    CInputHintWindow();
    virtual ~CInputHintWindow();

    // 创建提示窗口
    BOOL CreateHintWindow(CWnd* pParent);

    // 显示提示窗口(传入已计算好状态的 DiffedInputContent)
    void ShowHint(const RECT& anchorRect, const Utils::DiffedInputContent& content);

    // 隐藏提示窗口
    void HideHint();

    // 更新(用于前台窗口检测)
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

    // 窗口尺寸范围(内容自适应时在此范围内调整)
    int _minWindowWidth;
    int _minWindowHeight;
    int _maxWindowWidth;
    int _maxWindowHeight;

    // 前台窗口监控
    DWORD _currentProcessId;

    // 当前要显示的内容
    Utils::DiffedInputContent _currentContent;
    bool _hasContent;

    // 当前锚点矩形(收到contentSize后重新计算位置时使用)
    CRect _currentAnchorRect;

    // 内容尺寸是否已就绪(用于控制显示时机)
    bool _isContentSized;

    // 计算窗口大小和位置(按指定宽高)
    CRect CalculateWindowRect(const RECT& anchorRect, int width, int height);

    // 处理来自WebView的内容尺寸消息
    void _OnContentSize(int contentWidth, int contentHeight);

    // 检查WebView是否已初始化
    bool _IsReady() const;

    // 发送消息到WebView
    void _PostWebMessage(const std::wstring& action, const std::wstring& data);

    // 将 _currentContent 转为 token JSON 并发送到 WebView
    void _SendContent();

    // 前台窗口监控
    void CheckForegroundWindow();

    // 兜底显示定时器
    UINT_PTR _showTimerId = 0;
    CRect _pendingWindowRect;

    // 兜底显示(由定时器触发)
    void _DoShowWindow();
};