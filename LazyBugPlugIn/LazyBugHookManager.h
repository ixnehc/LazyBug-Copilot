// LazyBugHookManager.h
//
// 单实例 Hook 管理器：从解决方案目录加载 LazyBugHook.dll，
// 校验工厂函数后持有唯一的 ILazyBugHook 实例。
//
// 加载流程：<slnDir>\LazyBugHook.dll -> LoadLibrary
//           -> GetProcAddress("CreateLazyBugHook") -> 创建实例
//           -> Initialize。
// 卸载流程：Shutdown -> Release -> FreeLibrary。

#pragma once

#include <windows.h>
#include <string>

#include "../LazyBugHook/ILazyBugHook.h"

class CLazyBugHookManager
{
public:
    CLazyBugHookManager();
    ~CLazyBugHookManager();

    // 从解决方案目录加载 <slnDir>\LazyBugHook.dll。
    // 若已加载则先卸载旧实例；目录为空或加载失败返回 false。
    bool Load(const std::wstring& solutionDirectory);

    // 卸载当前 Hook（Shutdown -> Release -> FreeLibrary）
    void Unload();

    // 当前 Hook 指针（未加载时为 nullptr）
    ILazyBugHook* GetHook() const { return _hook; }

    bool IsLoaded() const { return _hook != nullptr; }

private:
    HMODULE       _module;
    ILazyBugHook* _hook;
};

// 全局 Hook 管理器实例
extern CLazyBugHookManager g_lazyBugHookManager;
