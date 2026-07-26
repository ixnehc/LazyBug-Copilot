#include "stdh.h"
#include "ChatSettingDirWatchTab.h"
#include <shlobj.h>        // IFileDialog
#include <algorithm>
#include <fstream>
#include "Utils_File.h"
#include "nlohmann/json.hpp"
using json = nlohmann::ordered_json;

// 外部函数声明
extern std::string widechar_to_utf8(const wchar_t* str);
extern std::wstring utf8_to_widechar(const std::string& utf8_str);
extern const char* GetOpenedDBFolderPath_utf8();

// ========== 构造/析构 ==========
CChatSettingDirWatchTab::CChatSettingDirWatchTab()
    : _hwnd(nullptr)
{
}

CChatSettingDirWatchTab::~CChatSettingDirWatchTab()
{
    Shutdown();
}

// ========== Init ==========
void CChatSettingDirWatchTab::Init(
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

// ========== Shutdown ==========
void CChatSettingDirWatchTab::Shutdown()
{
    // 退出前立即保存
    if (_configDirty)
        _DoSaveConfig();

    for (auto& e : _entries)
    {
        if (e)
            _StopScan(*e);
    }
    _entries.clear();
}

// ========== Update ==========
void CChatSettingDirWatchTab::Update()
{
    // 检测 DB 文件夹是否变化
    const char* curDbPath = GetOpenedDBFolderPath_utf8();
    std::string curDb = (curDbPath && *curDbPath) ? curDbPath : "";
    if (curDb != _dbFolder)
    {
        _dbFolder = curDb;
        RefreshData();
        return;
    }

    // 检查是否需要写盘（防抖/tab切换/窗口隐藏）
    _UpdateSaveConfig();

    if (!_tabActive)
        return;

    bool needPush = false;
    for (auto& e : _entries)
    {
        if (!e) continue;
        std::lock_guard<std::mutex> lock(e->mtx);
        if (e->dirty)
        {
            e->dirty = false;
            needPush = true;
        }
    }

    if (needPush)
        _BuildAndPushData();
}

// ========== RefreshData ==========
void CChatSettingDirWatchTab::RefreshData()
{
    // 停止所有旧扫描
    for (auto& e : _entries)
    {
        if (e)
            _StopScan(*e);
    }
    _entries.clear();

    _LoadConfig();

    // 为每个 entry 启动扫描
    for (auto& e : _entries)
    {
        if (e)
            _LaunchScan(*e);
    }

    _BuildAndPushData();
}

// ========== HandleWebMessage ==========
bool CChatSettingDirWatchTab::HandleWebMessage(const std::string& action, const nlohmann::json& jsonMsg)
{
    if (action == "requestDirWatchData")
    {
        // 只推送当前数据，不重新加载/扫描
        _BuildAndPushData();
        return true;
    }
    else if (action == "tabChanged")
    {
        if (jsonMsg.contains("tabId") && jsonMsg["tabId"].is_string())
        {
            std::string tabId = jsonMsg["tabId"];
            bool wasActive = _tabActive;
            _tabActive = (tabId == "dirwatch");
            // 从 DirWatch 切走时立即保存
            if (wasActive && !_tabActive)
                _UpdateSaveConfig();
        }
        // 不 return true，让 CChatSettingPage 也处理
        return false;
    }
    else if (action == "pickDirWatchFolder")
    {
        std::string oldPath;
        if (jsonMsg.contains("oldPath") && jsonMsg["oldPath"].is_string())
            oldPath = jsonMsg["oldPath"];
        // 不能在 WebView 消息回调中直接弹出模态对话框（会 crash），
        // 延迟到 Windows 消息循环中执行
        _pendingPickOldPath = oldPath;
        ::PostMessage(_hwnd, WM_DIRWATCH_PICK_FOLDER, 0, 0);
        return true;
    }
    else if (action == "deleteDirWatchEntry")
    {
        if (jsonMsg.contains("path") && jsonMsg["path"].is_string())
            _Delete(jsonMsg["path"]);
        return true;
    }
    else if (action == "toggleDirWatchExtension")
    {
        if (jsonMsg.contains("path") && jsonMsg["path"].is_string() &&
            jsonMsg.contains("ext") && jsonMsg["ext"].is_string())
        {
            _ToggleExt(jsonMsg["path"], jsonMsg["ext"]);
        }
        return true;
    }
    else if (action == "toggleDirWatchEnabled")
    {
        if (jsonMsg.contains("path") && jsonMsg["path"].is_string() &&
            jsonMsg.contains("enabled") && jsonMsg["enabled"].is_boolean())
        {
            _SetEnabled(jsonMsg["path"], jsonMsg["enabled"]);
        }
        return true;
    }
    else if (action == "rescanDirWatchEntry")
    {
        if (jsonMsg.contains("path") && jsonMsg["path"].is_string())
            _Rescan(jsonMsg["path"]);
        return true;
    }

    return false;
}

// ========== _LoadConfig / _SaveConfig ==========
void CChatSettingDirWatchTab::_LoadConfig()
{
    _entries.clear();

    const char* dbPath = GetOpenedDBFolderPath_utf8();
    _dbFolder = (dbPath && *dbPath) ? dbPath : "";
    if (_dbFolder.empty())
        return;

    _configPath = _dbFolder + "\\.dirwatch";

    std::vector<DirWatchEntry> rawEntries;
    if (!LoadDirWatchConfig(_configPath.c_str(), rawEntries))
        return;

    for (auto& de : rawEntries)
    {
        de.directoryPath = Utils::GetActualFilePath(de.directoryPath.c_str());
        auto entry = std::make_unique<Entry>();
        entry->config = std::move(de);
        entry->scanStatus = Entry::Idle;
        _entries.push_back(std::move(entry));
    }
}

// ========== _SaveConfig / _DoSaveConfig / _UpdateSaveConfig ==========
void CChatSettingDirWatchTab::_SaveConfig()
{
    _configDirty = true;
    _lastModifyTick = GetTickCount();
}

void CChatSettingDirWatchTab::_DoSaveConfig()
{
    if (_dbFolder.empty())
        return;

    std::string path = _configPath;
    if (path.empty())
        path = _dbFolder + "\\.dirwatch";

    std::vector<DirWatchEntry> rawEntries;
    for (auto& e : _entries)
    {
        if (e)
            rawEntries.push_back(e->config);
    }

    SaveDirWatchConfig(path.c_str(), rawEntries);
    _configDirty = false;
}

void CChatSettingDirWatchTab::_UpdateSaveConfig()
{
    if (!_configDirty)
        return;

    bool shouldSave = false;

    // 防抖：距离上次修改超过 5 秒
    if (GetTickCount() - _lastModifyTick > 5000)
        shouldSave = true;

    // Tab 已切走
    if (!_tabActive)
        shouldSave = true;

    // 窗口不可见
    if (_hwnd && !::IsWindowVisible(_hwnd))
        shouldSave = true;

    if (shouldSave)
        _DoSaveConfig();
}

// ========== _LaunchScan / _StopScan ==========
void CChatSettingDirWatchTab::_LaunchScan(Entry& e)
{
    _StopScan(e);

    e.scanAbort = false;
    e.scanStatus = Entry::Scanning;
    {
        std::lock_guard<std::mutex> lock(e.mtx);
        e.discoveredExt.clear();
        e.dirty = true;
    }
    e.scanThread = std::thread(&CChatSettingDirWatchTab::_ScanWorker, &e);
}

void CChatSettingDirWatchTab::_StopScan(Entry& e)
{
    e.scanAbort = true;
    if (e.scanThread.joinable())
        e.scanThread.join();
}

// ========== _ScanWorker（静态，运行在独立线程中）==========
void CChatSettingDirWatchTab::_ScanWorker(Entry* entry)
{
    if (!entry) return;

    std::string rootPath = entry->config.directoryPath;
    bool recursive = entry->config.recursive;

    // 使用栈进行遍历（非递归或用栈模拟递归）
    std::vector<std::string> dirs;
    dirs.push_back(rootPath);

    while (!dirs.empty() && !entry->scanAbort)
    {
        std::string currentDir = dirs.back();
        dirs.pop_back();

        std::string searchPath = currentDir + "\\*";

        WIN32_FIND_DATAA fd;
        HANDLE hFind = FindFirstFileA(searchPath.c_str(), &fd);
        if (hFind == INVALID_HANDLE_VALUE)
            continue;

        do
        {
            if (entry->scanAbort)
                break;

            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                // 跳过 . 和 ..
                if (strcmp(fd.cFileName, ".") != 0 && strcmp(fd.cFileName, "..") != 0)
                {
                    if (recursive)
                        dirs.push_back(currentDir + "\\" + fd.cFileName);
                }
            }
            else
            {
                // 提取后缀（小写，不含点号）
                const char* dot = strrchr(fd.cFileName, '.');
                if (dot && dot[1] != '\0')
                {
                    std::string ext = dot + 1;  // 跳过点号
                    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

                    {
                        std::lock_guard<std::mutex> lock(entry->mtx);
                        entry->discoveredExt[ext]++;
                        entry->dirty = true;
                    }
                }
            }
        } while (FindNextFileA(hFind, &fd) && !entry->scanAbort);

        FindClose(hFind);
    }

    if (!entry->scanAbort)
    {
        entry->scanStatus = Entry::Done;
        {
            std::lock_guard<std::mutex> lock(entry->mtx);
            entry->dirty = true;
        }
    }
}

// ========== _BuildAndPushData ==========
void CChatSettingDirWatchTab::_BuildAndPushData()
{
    if (!_isReady || !_isReady())
        return;

    json msg;
    if (_dbFolder.empty())
    {
        // DB 未打开，推送禁用状态
        msg["action"] = "setDirWatchData";
        msg["data"]["dbReady"] = false;
        msg["data"]["entries"] = json::array();
        _postFullJson(utf8_to_widechar(msg.dump()));
        return;
    }

    json entriesJson = json::array();

    for (auto& e : _entries)
    {
        if (!e) continue;

        json entryJson;
        entryJson["path"] = e->config.directoryPath;
        entryJson["recursive"] = e->config.recursive;
        entryJson["enabled"] = e->config.enabled;
        entryJson["justChanged"] = e->justChanged;
        e->justChanged = false;  // 推送后清除

        // scanStatus
        switch (e->scanStatus.load())
        {
        case Entry::Scanning: entryJson["scanStatus"] = "scanning"; break;
        case Entry::Done:     entryJson["scanStatus"] = "done";     break;
        default:              entryJson["scanStatus"] = "idle";     break;
        }

        // 构建按钮列表：discoveredExt ∪ selectedExtensions
        // discoveredExt 数据在锁内拷贝
        std::map<std::string, int> discCopy;
        {
            std::lock_guard<std::mutex> lock(e->mtx);
            discCopy = e->discoveredExt;
        }

        // 收集所有 unique 后缀
        std::vector<std::string> allExts;
        for (auto& kv : discCopy)
            allExts.push_back(kv.first);
        for (auto& sel : e->config.extensions)
        {
            if (discCopy.find(sel) == discCopy.end())
                allExts.push_back(sel);
        }

        // 去重
        std::sort(allExts.begin(), allExts.end());
        allExts.erase(std::unique(allExts.begin(), allExts.end()), allExts.end());

        // 按 count 降序排序
        std::sort(allExts.begin(), allExts.end(), [&](const std::string& a, const std::string& b) {
            int ca = 0, cb = 0;
            auto ita = discCopy.find(a);
            auto itb = discCopy.find(b);
            if (ita != discCopy.end()) ca = ita->second;
            if (itb != discCopy.end()) cb = itb->second;
            return ca > cb;
        });

        // 构建 selected set 用于快速判断
        std::set<std::string> selectedSet(e->config.extensions.begin(), e->config.extensions.end());

        json buttonsJson = json::array();
        for (auto& ext : allExts)
        {
            json btn;
            btn["ext"] = ext;
            auto it = discCopy.find(ext);
            btn["count"] = (it != discCopy.end()) ? it->second : 0;
            btn["selected"] = (selectedSet.find(ext) != selectedSet.end());
            buttonsJson.push_back(btn);
        }

        entryJson["extButtons"] = buttonsJson;
        entriesJson.push_back(entryJson);
    }

    msg["action"] = "setDirWatchData";
    msg["data"]["dbReady"] = true;
    msg["data"]["entries"] = entriesJson;
    _postFullJson(utf8_to_widechar(msg.dump()));
}

// ========== _Find ==========
CChatSettingDirWatchTab::Entry* CChatSettingDirWatchTab::_Find(const std::string& lowerPath)
{
    for (auto& e : _entries)
    {
        if (e && e->config.directoryPath == lowerPath)
            return e.get();
    }
    return nullptr;
}

// ========== _Add ==========
void CChatSettingDirWatchTab::_Add(const std::string& path)
{
    std::string lowerPath = path;
    std::transform(lowerPath.begin(), lowerPath.end(), lowerPath.begin(), ::tolower);

    if (_Find(lowerPath))
        return;  // 已存在

    auto entry = std::make_unique<Entry>();
    entry->config.directoryPath = lowerPath;
    entry->config.recursive = true;
    entry->config.enabled = true;  // 默认启用
    entry->justChanged = true;     // 标记为新增，通知 JS 自动展开
    _entries.push_back(std::move(entry));

    _SaveConfig();
    _LaunchScan(*_entries.back());
    _BuildAndPushData();
}

// ========== _Delete ==========
void CChatSettingDirWatchTab::_Delete(const std::string& path)
{
    for (auto it = _entries.begin(); it != _entries.end(); ++it)
    {
        if (*it && (*it)->config.directoryPath == path)
        {
            _StopScan(**it);
            _entries.erase(it);
            _SaveConfig();
            _BuildAndPushData();
            return;
        }
    }
}

// ========== _UpdatePath ==========
void CChatSettingDirWatchTab::_UpdatePath(const std::string& oldPath, const std::string& newPath)
{
    std::string lowerNew = newPath;
    std::transform(lowerNew.begin(), lowerNew.end(), lowerNew.begin(), ::tolower);

    Entry* e = _Find(oldPath);
    if (!e)
        return;

    // 如果新路径与旧路径相同，不处理
    if (oldPath == lowerNew)
        return;

    // 如果新路径已存在，不处理
    if (_Find(lowerNew))
        return;

    _StopScan(*e);
    e->config.directoryPath = lowerNew;
    e->justChanged = true;  // 标记为修改路径，通知 JS 自动展开
    _SaveConfig();
    _LaunchScan(*e);
    _BuildAndPushData();
}

// ========== _ToggleExt ==========
void CChatSettingDirWatchTab::_ToggleExt(const std::string& path, const std::string& ext)
{
    Entry* e = _Find(path);
    if (!e)
        return;

    auto& exts = e->config.extensions;
    auto it = std::find(exts.begin(), exts.end(), ext);
    if (it != exts.end())
        exts.erase(it);
    else
        exts.push_back(ext);

    _SaveConfig();
    _BuildAndPushData();  // 不重新扫描，只更新选中状态
}

// ========== _SetEnabled ==========
void CChatSettingDirWatchTab::_SetEnabled(const std::string& path, bool enabled)
{
    Entry* e = _Find(path);
    if (!e || e->config.enabled == enabled)
        return;

    e->config.enabled = enabled;
    _SaveConfig();
    _BuildAndPushData();
}

// ========== _Rescan ==========
void CChatSettingDirWatchTab::_Rescan(const std::string& path)
{
    Entry* e = _Find(path);
    if (!e)
        return;

    // 如果正在扫描中，忽略
    if (e->scanStatus == Entry::Status::Scanning)
        return;

    _LaunchScan(*e);
    _BuildAndPushData();
}

// ========== _PickFolder ==========
void CChatSettingDirWatchTab::_PickFolder(const std::string& oldPath)
{
    // 使用 IFileDialog (Vista+) 现代风格的文件夹选择对话框
    IFileDialog* pfd = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
    if (FAILED(hr))
        return;

    // 设置为只选择文件夹模式
    DWORD dwOptions = 0;
    if (SUCCEEDED(pfd->GetOptions(&dwOptions)))
    {
        pfd->SetOptions(dwOptions | FOS_PICKFOLDERS);
    }
    pfd->SetTitle(L"Select Folder");

    // 如果是编辑模式，设置初始文件夹路径
    if (!oldPath.empty())
    {
        std::wstring wideOldPath = utf8_to_widechar(oldPath);
        IShellItem* psi = nullptr;
        if (SUCCEEDED(SHCreateItemFromParsingName(wideOldPath.c_str(), nullptr, IID_PPV_ARGS(&psi))))
        {
            pfd->SetFolder(psi);
            psi->Release();
        }
    }

    hr = pfd->Show(_hwnd);
    if (FAILED(hr))
    {
        pfd->Release();
        return;  // 用户取消
    }

    IShellItem* psi = nullptr;
    if (FAILED(pfd->GetResult(&psi)))
    {
        pfd->Release();
        return;
    }

    PWSTR folderPath = nullptr;
    if (FAILED(psi->GetDisplayName(SIGDN_FILESYSPATH, &folderPath)))
    {
        psi->Release();
        pfd->Release();
        return;
    }

    std::string utf8Path = widechar_to_utf8(folderPath);
    CoTaskMemFree(folderPath);
    psi->Release();
    pfd->Release();

    if (utf8Path.empty())
        return;

    // 确保以反斜杠结尾
    if (utf8Path.back() != '\\')
        utf8Path += '\\';

    std::string lowerPath = utf8Path;
    std::transform(lowerPath.begin(), lowerPath.end(), lowerPath.begin(), ::tolower);

    if (oldPath.empty())
    {
        // 新增
        _Add(lowerPath);
    }
    else
    {
        // 修改
        _UpdatePath(oldPath, lowerPath);
    }
}

// ========== OnPickFolderDeferred ==========
void CChatSettingDirWatchTab::OnPickFolderDeferred(const std::string& oldPath)
{
    // oldPath 参数来自 CChatSettingPage，为空；实际值从 _pendingPickOldPath 取
    std::string path = oldPath.empty() ? _pendingPickOldPath : oldPath;
    _pendingPickOldPath.clear();
    _PickFolder(path);
}
