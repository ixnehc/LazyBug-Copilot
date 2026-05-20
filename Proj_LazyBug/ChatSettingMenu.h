#pragma once
#include <wrl.h>
#include <WebView2.h>
#include <functional>
#include <string>

// 回调函数类型定义
using SettingMenuNavigationCompletedCallback = std::function<void(bool)>;
using SettingMenuMessageReceivedCallback = std::function<void(const std::wstring&)>;
using SettingItemSelectedCallback = std::function<void(const std::wstring& itemName)>;

class CChatSettingMenu : public CWnd
{
public:
    CChatSettingMenu();
    virtual ~CChatSettingMenu();

    // 创建弹出式菜单窗口
    BOOL CreateSettingMenuWindow(CWnd* pParent);

    // 显示/隐藏窗口
    void ShowWindow(const RECT& btnRect);
    void HideWindow();

    // 更新（用于前台窗口检测）
    void Update();

    // 设置菜单项选中回调
    void SetItemSelectedCallback(SettingItemSelectedCallback callback) { _itemSelectedCallback = callback; }

    // 设置菜单项启用/禁用状态
    void SetItemEnabled(const std::wstring& itemName, bool enabled);

    // 导航方法
    void Navigate(const std::wstring& url);

    // 脚本执行
    void ExecuteScript(const std::wstring& script,
        std::function<void(const std::wstring&)> callback = nullptr);

    // 事件处理回调设置
    void SetNavigationCompletedCallback(SettingMenuNavigationCompletedCallback callback);
    void SetWebMessageReceivedCallback(SettingMenuMessageReceivedCallback callback);

    // 获取WebView2组件
    ICoreWebView2* GetCoreWebView2() { return _webView; }
    ICoreWebView2Controller* GetController() { return _controller; }

    // 调整WebView2大小
    void ResizeWebView();

    // 初始化界面
    void InitializeUI();

protected:
    HRESULT InitializeWebView();

    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDestroy();
    afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

    DECLARE_MESSAGE_MAP()

private:
    // WebView2相关组件
    ICoreWebView2Environment* _webViewEnvironment;
    ICoreWebView2* _webView;
    ICoreWebView2Controller* _controller;

    // 事件处理Token
    EventRegistrationToken _navigationCompletedToken = {};
    EventRegistrationToken _webMessageReceivedToken = {};

    // 回调函数
    SettingMenuNavigationCompletedCallback _navigationCompletedCallback;
    SettingMenuMessageReceivedCallback _webMessageReceivedCallback;
    SettingItemSelectedCallback _itemSelectedCallback;

    // 脚本执行回调映射
    std::map<int, std::function<void(const std::wstring&)>> _scriptCallbacks;
    int _callbackId;

    // 状态标志
    bool _isWebViewCreated;
    bool _isUIInitialized;

    // 窗口固定尺寸
    int _windowWidth;
    int _windowHeight;

    // 前台窗口监控
    DWORD _currentProcessId;

    // 计算窗口大小和位置
    CRect CalculateWindowRect(const RECT& btnRect);

    // 检查WebView和UI是否已初始化
    bool _IsReady() const;

    // 内部消息发送
    void _PostWebMessage(const std::wstring& action, const std::wstring& data);

    // 处理来自JavaScript的消息
    void _HandleWebMessage(const std::wstring& message);

    // 前台窗口监控
    void CheckForegroundWindow();
};
