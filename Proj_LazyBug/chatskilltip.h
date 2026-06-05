#pragma once
#include <wrl.h>
#include <WebView2.h>
#include <functional>

// CChatSkillTip - 用于显示skill.md内容的tip窗口
class CChatSkillTip : public CWnd
{
public:
    CChatSkillTip();
    virtual ~CChatSkillTip();

    // 创建tip窗口
    BOOL CreateSkillTipWindow(CWnd* pParent);

    // 显示tip窗口（传入skill.md文件路径）
    void ShowTip(const RECT& anchorRect, const std::wstring& skillMdPath);

    // 隐藏tip窗口
    void HideTip();

    // 更新（用于前台窗口检测）
    void Update();

    // 导航方法
    void Navigate(const std::wstring& url);

    // 脚本执行
    void ExecuteScript(const std::wstring& script,
        std::function<void(const std::wstring&)> callback = nullptr);

    // 获取WebView2组件
    ICoreWebView2* GetCoreWebView2() { return _webView; }
    ICoreWebView2Controller* GetController() { return _controller; }

    // 调整WebView2大小
    void ResizeWebView();

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

    // 脚本执行回调映射
    std::map<int, std::function<void(const std::wstring&)>> _scriptCallbacks;
    int _callbackId;

    // 状态标志
    bool _isWebViewCreated;
    bool _isUIInitialized;

    // 窗口尺寸
    int _windowWidth;
    int _windowHeight;

    // 前台窗口监控
    DWORD _currentProcessId;

    // 当前skill路径
    std::wstring _currentSkillMdPath;

    // 计算窗口大小和位置
    CRect CalculateWindowRect(const RECT& anchorRect);

    // 检查WebView是否已初始化
    bool _IsReady() const;

    // 发送消息到WebView
    void _PostWebMessage(const std::wstring& action, const std::wstring& data);


    // 发送skill数据到WebView
    void _SendSkillContent();

    // 前台窗口监控
    void CheckForegroundWindow();

    // 显示延迟定时器
    UINT_PTR _showTimerId = 0;
    CRect _pendingWindowRect;

    // 显示窗口（由定时器触发）
    void _DoShowWindow();
};


