// ILazyBugHook.h
//
// LazyBug Hook 系统对外接口定义。
// 外部 DLL 通过实现 ILazyBugHook 并导出工厂函数 CreateLazyBugHook 来接入 LazyBug 宿主。
//
// 约定：
//   1. 该头文件可被 Hook DLL 与宿主（LazyBugPlugIn）共同包含，接口不使用 STL/ATL 类型，
//      以保证跨 DLL 的 ABI 稳定。
//   2. Hook 实例由 Hook DLL 自己分配，也必须由 Hook DLL 自己释放（Release() 内部 delete this），
//      避免跨 DLL 使用不同的 CRT 堆。
//   3. 所有字符串参数均为 UTF-8 编码。
//   4. 所有回调都发生在 VS 主 UI 线程上（与宿主的解决方案事件一致）。

#pragma once

// Hook DLL 必须导出的工厂函数名
#define LAZYBUG_HOOK_CREATE_FUNC_NAME "CreateLazyBugHook"

// 宿主在 Initialize 时传递给 Hook 的信息
struct LazyBugHookHostInfo
{
    void* hostReserved;                             // 宿主私有指针，Hook 不应解引用
};

// Hook 抽象基类：外部 DLL 实现，宿主通过工厂函数创建
class ILazyBugHook
{
public:
    virtual ~ILazyBugHook() {}

    // 加载后立即调用；返回 false 表示初始化失败，宿主将卸载该 Hook
    virtual bool Initialize(const LazyBugHookHostInfo& hostInfo) = 0;

    // 卸载前调用，用于清理资源
    virtual void Shutdown() = 0;

    // 周期性更新，由宿主（Controls）内部的定时器驱动
    virtual void Update() = 0;

    // 释放实例：必须由 Hook 实现为 delete this
    virtual void Release() = 0;
};

// Hook DLL 工厂函数原型
typedef ILazyBugHook* (*LazyBugHookCreateFunc)();

// 供 Hook DLL 作者导出工厂函数使用：
//   LAZYBUG_HOOK_EXPORT ILazyBugHook* CreateLazyBugHook() { return new MyHook(); }
#define LAZYBUG_HOOK_EXPORT extern "C" __declspec(dllexport)
