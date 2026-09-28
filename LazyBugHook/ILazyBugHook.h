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

// ---------------------------------------------------------------------------
// Tool（工具）接口：Hook 可以像 MCP server 一样暴露一组工具，
// 由宿主收集后填充给 LLM，并在 LLM 调用时回传给 Hook 执行。
// ---------------------------------------------------------------------------

// 工具定义（与 CLlmMcps::Mcp::Tool 对齐）
struct LazyBugHookToolInfo
{
    const char* name;          // 工具名，UTF-8，非空
    const char* description;   // 工具描述，UTF-8，可为空
    const char* inputSchema;   // 参数 JSON Schema 原始字符串，UTF-8，可为空
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

    // ---- 工具接口 ----

    // 返回工具数量；无工具返回 0
    virtual int GetToolCount() = 0;

    // 获取第 index 个工具定义；返回 false 表示越界
    // outTool 中的字符串指针在下次调用前有效，宿主应立即拷贝
    virtual bool GetTool(int index, LazyBugHookToolInfo& outTool) = 0;

    // 同步执行工具（宿主在后台工作线程调用，非 UI 线程）
    // toolName: 即 GetTool 返回的 name
    // argumentsJson: LLM 传入的参数 JSON 字符串，UTF-8
    // resultBuffer/resultBufferSize: 宿主提供的输出缓冲区，Hook 写入 UTF-8 结果，
    //                               结果必须以 '\0' 结尾
    // 返回 false 表示执行失败
    virtual bool CallTool(const char* toolName,
                          const char* argumentsJson,
                          char* resultBuffer,
                          int resultBufferSize) = 0;

    // 释放实例：必须由 Hook 实现为 delete this
    virtual void Release() = 0;
};

// Hook DLL 工厂函数原型
typedef ILazyBugHook* (*LazyBugHookCreateFunc)();

// 供 Hook DLL 作者导出工厂函数使用：
//   LAZYBUG_HOOK_EXPORT ILazyBugHook* CreateLazyBugHook() { return new MyHook(); }
#define LAZYBUG_HOOK_EXPORT extern "C" __declspec(dllexport)
