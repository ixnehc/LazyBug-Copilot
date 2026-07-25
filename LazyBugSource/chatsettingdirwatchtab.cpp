#include "stdh.h"
#include "ChatSettingDirWatchTab.h"
#include <shlobj.h>        // SHBrowseForFolderW
#include <algorithm>
#include <fstream>
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
        RefreshData();
        return true;
    }
    else if (action == "tabChanged")
    {
        if (jsonMsg.contains("tabId") && jsonMsg["tabId"].is_string())
        {
            std::string tabId = jsonMsg["tabId"];
            _tabActive = (tabId == "dirwatch");
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
    else if (action == "updateDirWatchRecursive")
    {
        if (jsonMsg.contains("path") && jsonMsg["path"].is_string())
        {
            bool recursive = jsonMsg.value("recursive", false);
            _SetRecursive(jsonMsg["path"], recursive);
        }
        return true;
    }

    return false;
}

// ========== _LoadConfig / _SaveConfig ==========
void CChatSettingDirWatchTab::_LoadConfig()
{
    _entries.clear();

    const char* dbPath = GetOpenedDBFolderPath_utf8();
    if (!dbPath || !*dbPath)
        return;

    _configPath = std::string(dbPath) + "\\.dirwatch";

    std::vector<DirWatchEntry> rawEntries;
    if (!LoadDirWatchConfig(_configPath.c_str(), rawEntries))
        return;

    for (auto& de : rawEntries)
    {
        auto entry = std::make_unique<Entry>();
        entry->config = std::move(de);
        entry->scanStatus = Entry::Idle;
        _entries.push_back(std::move(entry));
    }
}

void CChatSettingDirWatchTab::_SaveConfig()
{
    if (_configPath.empty())
        return;

    std::vector<DirWatchEntry> rawEntries;
    for (auto& e : _entries)
    {
        if (e)
            rawEntries.push_back(e->config);
    }

    SaveDirWatchConfig(_configPath.c_str(), rawEntries);
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

    json entriesJson = json::array();

    for (auto& e : _entries)
    {
        if (!e) continue;

        json entryJson;
        entryJson["path"] = e->config.directoryPath;
        entryJson["recursive"] = e->config.recursive;

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

    std::string utf8Json = entriesJson.dump();
    _postMsg(L"setDirWatchData", utf8_to_widechar(utf8Json));
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

// ========== _SetRecursive ==========
void CChatSettingDirWatchTab::_SetRecursive(const std::string& path, bool recursive)
{
    Entry* e = _Find(path);
    if (!e)
        return;

    if (e->config.recursive == recursive)
        return;

    e->config.recursive = recursive;
    _SaveConfig();
    // 递归设置变化需要重新扫描
    _LaunchScan(*e);
    _BuildAndPushData();
}

// ========== _PickFolder ==========
void CChatSettingDirWatchTab::_PickFolder(const std::string& oldPath)
{
    BROWSEINFOW bi = {};
    bi.hwndOwner = _hwnd;
    bi.lpszTitle = L"Select a directory to watch";
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

    LPITEMIDLIST pidl = SHBrowseForFolderW(&bi);
    if (!pidl)
        return;  // 用户取消

    wchar_t folderPath[MAX_PATH] = {};
    if (!SHGetPathFromIDListW(pidl, folderPath))
    {
        CoTaskMemFree(pidl);
        return;
    }
    CoTaskMemFree(pidl);

    std::string utf8Path = widechar_to_utf8(folderPath);
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
