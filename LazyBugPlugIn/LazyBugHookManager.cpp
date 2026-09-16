#include "stdh.h"

#include "LazyBugHookManager.h"

CLazyBugHookManager g_lazyBugHookManager;

CLazyBugHookManager::CLazyBugHookManager()
    : _module(NULL)
    , _hook(NULL)
{
}

CLazyBugHookManager::~CLazyBugHookManager()
{
    Unload();
}

bool CLazyBugHookManager::Load(const std::wstring& solutionDirectory)
{
    // 切换解决方案时先卸载旧实例
    Unload();

    if (solutionDirectory.empty())
        return false;

    std::wstring dllPath = solutionDirectory;
    if (dllPath.back() != L'\\' && dllPath.back() != L'/')
        dllPath += L'\\';
    dllPath += L"LazyBugHook.dll";

    HMODULE module = ::LoadLibraryW(dllPath.c_str());
    if (!module)
        return false;

    LazyBugHookCreateFunc createFunc =
        reinterpret_cast<LazyBugHookCreateFunc>(::GetProcAddress(module, LAZYBUG_HOOK_CREATE_FUNC_NAME));
    if (!createFunc)
    {
        // 未导出工厂函数，不是 Hook DLL
        ::FreeLibrary(module);
        return false;
    }

    ILazyBugHook* hook = createFunc();
    if (!hook)
    {
        ::FreeLibrary(module);
        return false;
    }

    LazyBugHookHostInfo hostInfo;
    hostInfo.hostReserved = NULL;

    if (!hook->Initialize(hostInfo))
    {
        hook->Release();
        ::FreeLibrary(module);
        return false;
    }

    _module = module;
    _hook = hook;

    return true;
}

void CLazyBugHookManager::Unload()
{
    if (_hook)
    {
        _hook->Shutdown();
        _hook->Release();
        _hook = NULL;
    }

    if (_module)
    {
        ::FreeLibrary(_module);
        _module = NULL;
    }
}
