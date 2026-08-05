#include "stdh.h"
#include "ChatSettingDatabaseTab.h"
#include <shellapi.h>
#include "solutiondbapi.h"
#include "nlohmann/json.hpp"
using json = nlohmann::ordered_json;

extern std::wstring utf8_to_widechar(const std::string& utf8_str);
extern const char* GetOpenedDBFolderPath_utf8();

// ========== 构造/析构 ==========
CChatSettingDatabaseTab::CChatSettingDatabaseTab()
    : _hwnd(nullptr)
    , _isClearDBScheduled(false)
{
}

CChatSettingDatabaseTab::~CChatSettingDatabaseTab()
{
}

// ========== Init ==========
void CChatSettingDatabaseTab::Init(
    PostMsgCallback postMsg,
    PostFullJsonCallback postFullJson,
    ExecuteScriptCallback executeScript,
    IsReadyCallback isReady,
    HWND hwnd)
{
    _postMsg = postMsg;
    _postFullJson = postFullJson;
    _executeScript = executeScript;
    _isReady = isReady;
    _hwnd = hwnd;

    RefreshData();
}

// ========== SetDeleteOldChatsCallback ==========
void CChatSettingDatabaseTab::SetDeleteOldChatsCallback(DeleteOldChatsCallback callback)
{
    _deleteOldChatsCallback = callback;
}

// ========== Update ==========
void CChatSettingDatabaseTab::Update()
{
    // 检测 DB 文件夹是否变化（打开/切换 solution 时会变）
    const char* curDbPath = GetOpenedDBFolderPath_utf8();
    std::string curDb = (curDbPath && *curDbPath) ? curDbPath : "";
    if (curDb != _dbFolder)
    {
        _dbFolder = curDb;
        _SendDataToWebView();
    }

    if (_cleanupChatHistoryDays > 0)
    {
        const int days = _cleanupChatHistoryDays;
        _cleanupChatHistoryDays = 0;

        // 通知前端开始清理（由于清理阻塞 UI 线程，此消息实际会在清理完成后才被处理；
        // 保留此消息供后续改为后台线程时使用）
        _postMsg(L"cleanupStarted", L"{}");

        _CleanupChatHistory(days);

        _PostCleanupResult("cleanupChatHistory", true, "Chat history cleanup completed.");
    }

    if (_isClearDBScheduled)
    {
        _isClearDBScheduled = false;

        _postMsg(L"cleanupStarted", L"{}");

//         // TODO: 测试用，验证 UI 线程阻塞效果，正式上线前移除
//         Sleep(60000);

        bool success = true;
        std::string errMsg;

        const char* dbPath = GetOpenedDBFolderPath_utf8();
        if (dbPath && *dbPath)
        {
            // SolutionDB_ClearDB 内部通过 ConvertFullPathToName 提取最后的目录名作为
            // solution name，因此传入 dbFolderPath 和传入 slnPath 效果相同
            SolutionDB_ClearDB(dbPath);
        }
        else
        {
            success = false;
            errMsg = "Database path is not available.";
        }

        // 刷新显示的路径信息
        RefreshData();

        _PostCleanupResult("clearDatabase", success,
            success ? "Symbol &amp; index cleaned successfully." : errMsg.c_str());
    }

    // 发送清理结果到前端
    if (_cleanupResultPending)
    {
        _cleanupResultPending = false;

        json result;
        result["type"] = _cleanupResultType;
        result["success"] = _cleanupResultSuccess;
        result["message"] = _cleanupResultMsg;

        _postMsg(L"cleanupFinished",
            utf8_to_widechar(result.dump()));
    }
}

// ========== RefreshData ==========
void CChatSettingDatabaseTab::RefreshData()
{
    const char* dbPath = GetOpenedDBFolderPath_utf8();
    _dbFolder = (dbPath && *dbPath) ? dbPath : "";

    _SendDataToWebView();
}

// ========== HandleWebMessage ==========
bool CChatSettingDatabaseTab::HandleWebMessage(const std::string& action, const nlohmann::json& jsonMsg)
{
    if (action == "requestDatabaseData")
    {
        _SendDataToWebView();
        return true;
    }
    else if (action == "openDatabaseFolder")
    {
        _OpenDatabaseFolder();
        return true;
    }
    else if (action == "clearDatabase")
    {
        _ClearDatabase();
        return true;
    }
    else if (action == "cleanupChatHistory")
    {
        const int days = jsonMsg.value("days", 0);
        if (days > 0)
            _ScheduleCleanupChatHistory(days);
        return true;
    }

    return false;
}

// ========== _SendDataToWebView ==========
void CChatSettingDatabaseTab::_SendDataToWebView()
{
    if (!_isReady || !_isReady())
        return;

    json data;
    data["dbReady"] = !_dbFolder.empty();
    data["dbFolderPath"] = _dbFolder;

    json msg;
    msg["action"] = "setDatabaseData";
    msg["data"] = data;

    _postFullJson(utf8_to_widechar(msg.dump()));
}

// ========== _OpenDatabaseFolder ==========
void CChatSettingDatabaseTab::_OpenDatabaseFolder()
{
    const char* dbFolderPath = GetOpenedDBFolderPath_utf8();
    if (dbFolderPath && dbFolderPath[0] != '\0')
    {
        ShellExecuteA(NULL, "open", "explorer.exe", dbFolderPath, NULL, SW_SHOWNORMAL);
    }
}

// ========== _ClearDatabase ==========
void CChatSettingDatabaseTab::_ClearDatabase()
{
    // 延迟到 Update 中执行，因为在 WebView 消息回调中直接调用
    // SolutionDB_ClearDB（同步 pipe 消息）可能会阻塞消息循环
    _isClearDBScheduled = true;
}

// ========== _ScheduleCleanupChatHistory ==========
void CChatSettingDatabaseTab::_ScheduleCleanupChatHistory(int days)
{
    // 避免在 WebView 回调中执行可能耗时的文件清理。
    _cleanupChatHistoryDays = days;
}

// ========== _CleanupChatHistory ==========
void CChatSettingDatabaseTab::_CleanupChatHistory(int days)
{
    if (!_deleteOldChatsCallback || _dbFolder.empty())
        return;

    std::string checkpointsDir = _dbFolder + "\\_checkpoints";
    _deleteOldChatsCallback(days, checkpointsDir.c_str());
}

// ========== _PostCleanupResult ==========
void CChatSettingDatabaseTab::_PostCleanupResult(const char* type, bool success, const char* message)
{
    _cleanupResultType = type;
    _cleanupResultSuccess = success;
    _cleanupResultMsg = message;
    _cleanupResultPending = true;
}
