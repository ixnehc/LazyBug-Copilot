#pragma once
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include <nlohmann/json.hpp>
#include "DirWatchEntryConfig.h"

// Dir Watch Tab 的逻辑控制器
// 通过回调与 CChatSettingPage (WebView) 通信，不继承任何窗口类
class CChatSettingDirWatchTab
{
public:
    // === 回调类型（与 CChatSettingProviderTab 一致）===
    using PostMsgCallback       = std::function<void(const std::wstring& action, const std::wstring& data)>;
    using PostFullJsonCallback  = std::function<void(const std::wstring& fullJson)>;
    using ExecuteScriptCallback = std::function<void(const std::wstring& script)>;
    using IsReadyCallback       = std::function<bool()>;

    // === Entry: 每个目录条目（自包含扫描线程）===
    struct Entry
    {
        DirWatchEntry config;

        enum Status { Idle, Scanning, Done };
        std::atomic<int> scanStatus{ Idle };

        std::mutex mtx;
        std::map<std::string, int> discoveredExt;  // ext → 文件数（扫描线程实时更新）
        bool dirty = false;                         // 有新数据待推送

        std::thread scanThread;
        std::atomic<bool> scanAbort{ false };

        bool justChanged = false;  // 新增或修改路径后标记，用于通知 JS 自动展开
    };

    CChatSettingDirWatchTab();
    ~CChatSettingDirWatchTab();

    // 初始化，传入回调和依赖
    void Init(PostMsgCallback postMsg, PostFullJsonCallback postFullJson,
              ExecuteScriptCallback executeScript, IsReadyCallback isReady, HWND hwnd);

    // 每帧更新：检查各 Entry 扫描结果，有 dirty 则推送 UI
    void Update();

    // 刷新所有数据（重新加载配置 + 启动所有扫描线程）
    void RefreshData();

    // 处理来自 JavaScript 的消息，返回 true 表示已处理
    bool HandleWebMessage(const std::string& action, const nlohmann::json& jsonMsg);

    // 停止所有扫描线程
    void Shutdown();

    // 处理延迟的文件夹选择（在 Windows 消息循环中调用，避免 WebView 回调中直接弹模态对话框）
    static const UINT WM_DIRWATCH_PICK_FOLDER = WM_USER + 0x300;
    void OnPickFolderDeferred(const std::string& oldPath);

private:
    std::vector<std::unique_ptr<Entry>> _entries;
    std::string _configPath;
    std::string _dbFolder;    // 用于检测 DB 文件夹路径是否发生变化
    bool _tabActive = false;  // DirWatch tab 是否处于激活状态
    bool _configDirty = false;  // 配置已修改但尚未写入磁盘
    DWORD _lastModifyTick = 0;  // 最后一次修改配置的时间戳

    // 回调
    PostMsgCallback _postMsg;
    PostFullJsonCallback _postFullJson;
    ExecuteScriptCallback _executeScript;
    IsReadyCallback _isReady;
    HWND _hwnd;
    std::string _pendingPickOldPath;  // 延迟文件夹选择的 oldPath 暂存

    // === 内部方法 ===
    void _LoadConfig();
    void _SaveConfig();         // 标记脏位，不直接写盘
    void _DoSaveConfig();       // 实际写入 .dirwatch 文件
    void _UpdateSaveConfig();   // 在 Update 中检查是否需要写盘（防抖/tab切换/窗口隐藏）
    void _LaunchScan(Entry& e);
    void _StopScan(Entry& e);
    void _BuildAndPushData();

    Entry* _Find(const std::string& lowerPath);

    // 操作（均由 HandleWebMessage 调用）
    void _Add(const std::string& path);
    void _Delete(const std::string& path);
    void _UpdatePath(const std::string& oldPath, const std::string& newPath);
    void _ToggleExt(const std::string& path, const std::string& ext);
    void _SetEnabled(const std::string& path, bool enabled);
    void _Rescan(const std::string& path);  // 重新扫描指定目录
    void _PickFolder(const std::string& oldPath);  // oldPath 为空表示新增，非空表示修改

    // 静态线程函数
    static void _ScanWorker(Entry* entry);
};
