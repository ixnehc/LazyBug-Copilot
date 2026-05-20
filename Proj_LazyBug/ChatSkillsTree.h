#pragma once
#include <vector>
#include <map>
#include <string>
#include <functional>
#include <wrl.h>
#include <WebView2.h>

#include "LlmSkills.h"

// 回调函数类型定义
using SkillsTreeNavigationCompletedCallback = std::function<void(bool)>;
using SkillsTreeMessageReceivedCallback = std::function<void(const std::wstring&)>;
using SkillSelectedCallback = std::function<void(const std::wstring& skillPath)>;
using SkillEnableChangedCallback = std::function<void(const std::wstring& skillPath, bool enable)>;

class CChatSkillsTree : public CWnd
{
public:
    CChatSkillsTree();
    virtual ~CChatSkillsTree();

    // 创建弹出式菜单窗口
    BOOL CreateSkillsTreeWindow(CWnd* pParent);

    // 显示/隐藏窗口
    void ShowWindow(const RECT& btnRect);
    void HideWindow();

    // 更新（用于前台窗口检测）
    void Update();

    // 设置Skill选中回调
    void SetSkillSelectedCallback(SkillSelectedCallback callback) { _skillSelectedCallback = callback; }

    // 设置Skill启用状态变化回调
    void SetSkillEnableChangedCallback(SkillEnableChangedCallback callback) { _skillEnableChangedCallback = callback; }

    // 导航方法
    void Navigate(const std::wstring& url);

    // 脚本执行
    void ExecuteScript(const std::wstring& script,
        std::function<void(const std::wstring&)> callback = nullptr);

    // 事件处理回调设置
    void SetNavigationCompletedCallback(SkillsTreeNavigationCompletedCallback callback);
    void SetWebMessageReceivedCallback(SkillsTreeMessageReceivedCallback callback);

    // 获取WebView2组件
    ICoreWebView2* GetCoreWebView2() { return _webView; }
    ICoreWebView2Controller* GetController() { return _controller; }

    // 调整WebView2大小
    void ResizeWebView();

    // 初始化界面
    void InitializeUI();

    // 发送Skills树数据到WebView
    void SendSkillTreeData();

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
    SkillsTreeNavigationCompletedCallback _navigationCompletedCallback;
    SkillsTreeMessageReceivedCallback _webMessageReceivedCallback;
    SkillSelectedCallback _skillSelectedCallback;
    SkillEnableChangedCallback _skillEnableChangedCallback;

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

    // JSON辅助
    std::wstring _EscapeJsonString(const std::wstring& str);

    // 构建Skills树JSON
    std::wstring _BuildSkillTreeJson();

    // 处理来自JavaScript的消息
    void _HandleWebMessage(const std::wstring& message);

	// 重命名Skill或目录（修改目录名，如果是skill目录则同时修改SKILL.md中的name字段）
	void _RenameSkill(const std::wstring& oldName, const std::wstring& newName, const std::wstring& fullPath, bool isLeaf);

	// 新建目录
	void _CreateNewFolder(const std::wstring& parentPath, const std::wstring& folderName);

	// 新建Skill
	void _CreateNewSkill(const std::wstring& parentPath, const std::wstring& skillName);

	// 前台窗口监控
	void CheckForegroundWindow();
};

