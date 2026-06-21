#pragma once
#include <vector>
#include <map>
#include <string>
#include <functional>
#include <wrl.h>
#include <WebView2.h>

#include "LlmMcps.h"

// 回调函数类型定义
using McpsTreeNavigationCompletedCallback = std::function<void(bool)>;
using McpsTreeMessageReceivedCallback = std::function<void(const std::wstring&)>;
using McpJsonOpenCallback = std::function<void(const std::wstring& mcpJsonPath)>;

class CChatMcpsTree : public CWnd
{
public:
    CChatMcpsTree();
    virtual ~CChatMcpsTree();

    // 创建弹出式菜单窗口
    BOOL CreateMcpsTreeWindow(CWnd* pParent);

    // 显示/隐藏窗口
    void ShowWindow(const RECT& btnRect);
    void HideWindow();

    // 更新（用于前台窗口检测）
    void Update();

    // 设置MCP JSON打开回调（双击未enable的MCP时）
    void SetMcpJsonOpenCallback(McpJsonOpenCallback callback) { _mcpJsonOpenCallback = callback; }

    // 设置是否允许修改（enable/disable MCP/tool, new MCP等）
    // 如果为false，则只允许只读操作（如打开目录、查看错误等）
    void EnableModify(bool enable);

    // 发送修改权限状态到WebView
    void SendEnableModify();

    // 导航方法
    void Navigate(const std::wstring& url);

    // 脚本执行
    void ExecuteScript(const std::wstring& script,
        std::function<void(const std::wstring&)> callback = nullptr);

    // 事件处理回调设置
    void SetNavigationCompletedCallback(McpsTreeNavigationCompletedCallback callback);
    void SetWebMessageReceivedCallback(McpsTreeMessageReceivedCallback callback);

    // 获取WebView2组件
    ICoreWebView2* GetCoreWebView2() { return _webView; }
    ICoreWebView2Controller* GetController() { return _controller; }

    // 调整WebView2大小
    void ResizeWebView();

    // 初始化界面
    void InitializeUI();

    // 发送Mcps树数据到WebView
    void SendMcpTreeData();

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
    EventRegistrationToken _processFailedToken = {};

    // 回调函数
    McpsTreeNavigationCompletedCallback _navigationCompletedCallback;
    McpsTreeMessageReceivedCallback _webMessageReceivedCallback;
    McpJsonOpenCallback _mcpJsonOpenCallback;

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

    // 修改权限控制
    bool _enableModify = true;  // 默认允许修改

    // 计算窗口大小和位置
    CRect CalculateWindowRect(const RECT& btnRect);

    // 检查WebView和UI是否已初始化
    bool _IsReady() const;

    // 内部消息发送
    void _PostWebMessage(const std::wstring& action, const std::wstring& data);

    // JSON辅助
    std::wstring _EscapeJsonString(const std::wstring& str);

    // 构建Mcps树JSON
    std::wstring _BuildMcpTreeJson();

    // 处理来自JavaScript的消息
    void _HandleWebMessage(const std::wstring& message);

    // 新建目录
    void _CreateNewFolder(const std::wstring& parentPath, const std::wstring& folderName);

    // 新建MCP
    void _CreateNewMcp(const std::wstring& parentPath, const std::wstring& mcpName, const std::wstring& mcpType);

    // 重命名MCP
    void _RenameMcp(const std::wstring& oldName, const std::wstring& newName, const std::wstring& fullPath);

    // 前台窗口监控
    void CheckForegroundWindow();
};

