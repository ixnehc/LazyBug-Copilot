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

    CChatSettingDatabaseTab();
    ~CChatSettingDatabaseTab();

    void Init(PostMsgCallback postMsg, PostFullJsonCallback postFullJson,
              ExecuteScriptCallback executeScript, IsReadyCallback isReady, HWND hwnd);

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

    void _SendDataToWebView();
    void _OpenDatabaseFolder();
    void _ClearDatabase();
};
