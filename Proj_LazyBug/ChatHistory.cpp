#include "stdh.h"
#include "ChatHistory.h"

#include "stringparser/stringparser.h"
#include "chatopsctrl.h"
#include "Utils.h"
#include <algorithm>
#include <iostream>
#include <direct.h>
#include <io.h>

CChatHistory::CChatHistory()
{
}

void CChatHistory::Init(const char* folder)
{
    Clear();
    _folderPath = folder;
    
    // 枚举文件夹下所有.chat文件
    std::vector<std::string> fileList;
    _EnumerateFiles(folder, fileList);
    
    // 获取.chat文件信息
    for (const auto& fileName : fileList)
    {
        std::string fullPath = std::string(folder) + "\\" + fileName;
        
        // 获取文件修改时间
        if (Utils::IsFileExist(fullPath.c_str()))
        {
            Entry entry;
            entry.fileName = utf8_to_widechar(fileName.c_str());
			entry.modifiedTime = Utils::GetFileTime(fullPath.c_str());
            
            _entries.push_back(entry);
        }
    }
    
    // 按修改时间排序，最新的在前面
    std::sort(_entries.begin(), _entries.end(), 
        [](const Entry& a, const Entry& b) {
            return CompareFileTime(&a.modifiedTime, &b.modifiedTime) > 0;
        });
}

void CChatHistory::Clear()
{
    _entries.clear();
    _folderPath.clear();
}

void CChatHistory::_EnumerateFiles(const char* folder, std::vector<std::string>& fileList)
{
    fileList.clear();
    
    // 将 UTF-8 路径转换为宽字符
    std::wstring wFolder = utf8_to_widechar(folder);
    std::wstring searchPattern = wFolder + L"\\*.chat";
    WIN32_FIND_DATAW findFileData;
    HANDLE hFind = FindFirstFileW(searchPattern.c_str(), &findFileData);
    
    if (hFind == INVALID_HANDLE_VALUE)
        return;
    
    do
    {
        // 跳过目录，只添加.chat文件
        if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            // 将宽字符文件名转换为 UTF-8
            fileList.push_back(widechar_to_utf8(findFileData.cFileName));
        }
    } while (FindNextFileW(hFind, &findFileData));
    
    FindClose(hFind);
}

void CChatHistory::Add(const char* fileName)
{
    std::wstring wFileName = utf8_to_widechar(fileName);
    
    // 查找是否已存在
    auto it = std::find_if(_entries.begin(), _entries.end(),
        [&wFileName](const Entry& entry) {
            return entry.fileName == wFileName;
        });
    
    if (it != _entries.end())
    {
        // 已存在，移动到最前面
        Entry entry = *it;
        _entries.erase(it);

		std::string fullPath = _folderPath + "\\" + fileName;
        entry.modifiedTime=Utils::GetFileTime(fullPath.c_str());
        
        _entries.insert(_entries.begin(), entry);
    }
    else
    {
        // 不存在，添加新项
        Entry entry;
        entry.fileName = wFileName;
        
        std::string fullPath = _folderPath + "\\" + fileName;
        entry.modifiedTime = Utils::GetFileTime(fullPath.c_str());
        if (Utils::EqualFileTime(entry.modifiedTime,Utils::GetZeroFileTime()))
			GetSystemTimeAsFileTime(&entry.modifiedTime);
        
        _entries.insert(_entries.begin(), entry);
    }
}

void CChatHistory::GetRecentMenuItems(DWORD count, std::vector<MenuItemInfo>& items)
{
    items.clear();
    
    DWORD actualCount = min(count, (DWORD)_entries.size());
    items.reserve(actualCount);
    
    for (DWORD i = 0; i < actualCount; ++i)
    {
        MenuItemInfo info;
        _Entry2MenuItemInfo(_entries[i], info);
        items.push_back(info);
    }
}

void CChatHistory::_Entry2MenuItemInfo(const Entry& entry, MenuItemInfo& info)
{
    // uid就是文件名
    info.uid = entry.fileName;
    
    // content: 读取.chat文件中第一个Op_SetTitle的内容
    info.content = L"[ Untitled Chat ]"; // 默认标题
    std::wstring fullPath = utf8_to_widechar(_folderPath) + L"\\" + entry.fileName;
    
    try
    {
        std::ifstream file(fullPath, std::ios::binary);
        if (file.is_open())
        {
            // 读取文件头标识
            DWORD magic;
            file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
            if (magic == 0x4F504348) // "HCPO"
            {
                // 读取版本号
                DWORD version;
                file.read(reinterpret_cast<char*>(&version), sizeof(version));

                FilesCheckpointUID checkpointId;
                file.read(reinterpret_cast<char*>(&checkpointId), sizeof(checkpointId));
				if(version>= CHATOPSCTRL_VERSION_1_1)
					file.read(reinterpret_cast<char*>(&checkpointId), sizeof(checkpointId));
				int recentPromptToken;
				if (version >= CHATOPSCTRL_VERSION_1_2)
					file.read(reinterpret_cast<char*>(&recentPromptToken), sizeof(recentPromptToken));

                // 读取操作记录数量
                DWORD opCount;
                file.read(reinterpret_cast<char*>(&opCount), sizeof(opCount));
                
                // 流式读取操作记录，查找第一个Op_SetTitle操作
                for (DWORD i = 0; i < opCount; ++i)
                {
                    ChatOp op;
                    op.Load(file,version);
                    
                    if (op.type == ChatOp::Op_SetTitle)
                    {
                        info.content = op.title;
                        break;
                    }
                }
            }
            file.close();
        }
    }
    catch (...)
    {
        // 文件读取出错，使用默认标题
    }
    
    // stamp: 将文件修改时间转换为可读字符串
    SYSTEMTIME sysTime;
    FileTimeToSystemTime(&entry.modifiedTime, &sysTime);
    
    SYSTEMTIME currentSysTime;
    GetSystemTime(&currentSysTime);
    
    // 计算时间差
    FILETIME currentFileTime;
    SystemTimeToFileTime(&currentSysTime, &currentFileTime);
    
    ULARGE_INTEGER currentTime, fileTime;
    currentTime.LowPart = currentFileTime.dwLowDateTime;
    currentTime.HighPart = currentFileTime.dwHighDateTime;
    fileTime.LowPart = entry.modifiedTime.dwLowDateTime;
    fileTime.HighPart = entry.modifiedTime.dwHighDateTime;
    
    // 时间差（以100纳秒为单位）
    ULONGLONG timeDiff = currentTime.QuadPart - fileTime.QuadPart;
    
    // 转换为秒
    ULONGLONG secondsDiff = timeDiff / 10000000ULL;
    
    wchar_t timeStr[64];
    
    if (secondsDiff < 3600) // 1小时内
    {
        ULONGLONG minutes = secondsDiff / 60;
        if (minutes == 0)
            swprintf_s(timeStr, 64, L"刚刚");
        else
            swprintf_s(timeStr, 64, L"%llum ago", minutes);
    }
    else if (secondsDiff < 86400) // 24小时内
    {
        ULONGLONG hours = secondsDiff / 3600;
        ULONGLONG minutes = (secondsDiff % 3600) / 60;
        if (minutes == 0)
            swprintf_s(timeStr, 64, L"%lluh ago", hours);
        else
            swprintf_s(timeStr, 64, L"%lluh%llum ago", hours, minutes);
    }
    else if (secondsDiff < 2592000) // 30天内
    {
        ULONGLONG days = secondsDiff / 86400;
        swprintf_s(timeStr, 64, L"%llud ago", days);
    }
    else // 超过30天，使用具体日期
    {
        swprintf_s(timeStr, 64, L"%02d/%02d", sysTime.wMonth, sysTime.wDay);
    }
    
    info.stamp = timeStr;
}

std::string CChatHistory::GetRecentFileName()
{
    if (_entries.empty())
        return "";

    return widechar_to_utf8(_entries[0].fileName.c_str());
}

