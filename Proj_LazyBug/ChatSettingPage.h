#pragma once
#include <vector>
#include <map>
#include <string>
#include <functional>
#include <wrl.h>
#include <WebView2.h>

#include "LlmLib.h"

#include "ChatTaskMgr.h"

// 回调函数类型定义
using SettingPageNavigationCompletedCallback = std::function<void(bool)>;
using SettingPageMessageReceivedCallback = std::function<void(const std::wstring&)>;
using SettingPageExitCallback = std::function<void()>;
using SettingPageEditModelsCallback = std::function<void()>;

// Tab页面结构
struct SettingTab
{
    std::wstring id;                    // Tab ID
    std::wstring title;                 // Tab标题
};

class CChatSettingPage : public CWnd
{
public:
    // 构造函数和析构函数
    CChatSettingPage();
    virtual ~CChatSettingPage();

    // 创建WebView2控件
    BOOL Create(const RECT& rect, CWnd* pParentWnd, UINT nID);
	void Update();
    
    // 导航方法
    void Navigate(const std::wstring& url);
    void NavigateToString(const std::wstring& htmlContent);
    void Reload();
    
    // 脚本执行
    void ExecuteScript(const std::wstring& script, 
                      std::function<void(const std::wstring&)> callback = nullptr);
    
    // 事件处理和回调设置
    void SetNavigationCompletedCallback(SettingPageNavigationCompletedCallback callback);
    void SetWebMessageReceivedCallback(SettingPageMessageReceivedCallback callback);
    void SetExitCallback(SettingPageExitCallback callback);
    void SetEditModelsCallback(SettingPageEditModelsCallback callback);
    
    // 获取WebView2环境和核心WebView
    ICoreWebView2* GetCoreWebView2() { return _webView; }
    ICoreWebView2Controller* GetController() { return _controller; }
    
    // 调整WebView2大小
    void ResizeWebView();

    // ===== 设置页面功能相关方法 =====
    
    // 初始化设置界面
    void InitializeSettingUI();
    
    // Tab管理
    void AddTab(const SettingTab& tab);
    void SetActiveTab(const std::wstring& tabId);
    void ClearTabs();

    // Provider数据处理方法
    void LoadProviderData();
    void SendProviderDataToWebView();
    void SendCapabilityStatusToWebView();
    void UpdateProviderKey(const std::wstring& providerTypeName, const std::wstring& key);
    
    // Provider验证方法
    void StartValidatingProvider(const LlmApiProviderTypeName& providerTypeName);
    void EndValidatingProvider(const LlmApiProviderTypeName& providerTypeName, bool available);
	bool IsValidatingProvider();

    // 检测并重新加载（如果LLM Lib配置有变化则更新显示）
    void UpdateReload();

    // 编辑扩展模型配置
    void EditModels();


protected:
    // 初始化WebView2环境
    HRESULT InitializeWebView();
    
    // 消息处理函数
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDestroy();
    
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
    SettingPageNavigationCompletedCallback _navigationCompletedCallback;
    SettingPageMessageReceivedCallback _webMessageReceivedCallback;
    SettingPageExitCallback _exitCallback;
    SettingPageEditModelsCallback _editModelsCallback;
    
    // 脚本执行回调映射
    std::map<int, std::function<void(const std::wstring&)>> _scriptCallbacks;
    int _callbackId;
    
    // 状态标志
    bool _isWebViewCreated;
    bool _isSettingInitialized;
    
    // LLM Lib版本号跟踪
    int _llmLibVersion;
    
    // 设置相关数据
    std::vector<SettingTab> _tabs;
    std::wstring _activeTabId;
    
    // 检查WebView和Setting是否已初始化
    bool _IsReady() const;
    
    // 生成唯一ID
    std::wstring _GenId();
    
    // 内部消息发送
    void _PostWebMessage(const std::wstring& action, const std::wstring& data);
    void _PostWebMessage(const std::wstring& action, const std::wstring& data, bool isFullJson);
    
    // JSON辅助方法
    std::wstring _EscapeJsonString(const std::wstring& str);
    std::wstring _BuildTabsJson();
    
    // 查找Tab
    SettingTab* _FindTab(const std::wstring& tabId);
    
    // 初始化默认Tab
    void _InitializeDefaultTabs();
    
    // 处理来自JavaScript的消息
    void _HandleWebMessage(const std::wstring& message);

	CChatTaskMgr _taskMgr;
};
