#pragma once
#include <string>
#include <functional>
#include <nlohmann/json.hpp>

// Database Tab 的逻辑控制器
// 通过回调与 CChatSettingPage (WebView) 通信，不继承任何窗口类
class CChatSettingDatabaseTab
{
public:
    using PostMsgCallback       = std::function<void(const std::wstring& action, const std::wstring& data)>;
    using PostFullJsonCallback  = std::function<void(const std::wstring& fullJson)>;
    using ExecuteScriptCallback = std::function<void(const std::wstring& script)>;
    using IsReadyCallback       = std::function<bool()>;
    using DeleteOldChatsCallback = std::function<void(int days, const char* checkpointsDir)>;

    CChatSettingDatabaseTab();
    ~CChatSettingDatabaseTab();

    void Init(PostMsgCallback postMsg, PostFullJsonCallback postFullJson,
              ExecuteScriptCallback executeScript, IsReadyCallback isReady, HWND hwnd);

    void SetDeleteOldChatsCallback(DeleteOldChatsCallback callback);

    void Update();

    // 刷新数据到 WebView
    void RefreshData();

    // 处理来自 JavaScript 的消息，返回 true 表示已处理
    bool HandleWebMessage(const std::string& action, const nlohmann::json& jsonMsg);

private:
    PostMsgCallback _postMsg;
    PostFullJsonCallback _postFullJson;
    ExecuteScriptCallback _executeScript;
    IsReadyCallback _isReady;
    HWND _hwnd;

    std::string _dbFolder;
    bool _isClearDBScheduled = false;
    int _cleanupChatHistoryDays = 0;
    DeleteOldChatsCallback _deleteOldChatsCallback;

    // 清理结果（延迟到 Update 末尾发送，避免在阻塞操作中嵌套 WebView 消息）
    bool _cleanupResultPending = false;
    std::string _cleanupResultType;
    bool _cleanupResultSuccess = false;
    std::string _cleanupResultMsg;

    void _SendDataToWebView();
    void _OpenDatabaseFolder();
    void _ClearDatabase();
    void _ScheduleCleanupChatHistory(int days);
    void _CleanupChatHistory(int days);
    void _PostCleanupResult(const char* type, bool success, const char* message);
};
