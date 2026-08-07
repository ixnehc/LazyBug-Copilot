#pragma once
#include <string>
#include <functional>
#include <vector>
#include <nlohmann/json.hpp>
#include "LlmLib.h"

class CChatTaskMgr;

// Provider & API Tab 的逻辑控制器
// 通过回调与 CChatSettingPage (WebView) 通信，不继承任何窗口类
class CChatSettingProviderTab
{
public:
    using PostMsgCallback     = std::function<void(const std::wstring& action, const std::wstring& data)>;
    using PostFullJsonCallback = std::function<void(const std::wstring& fullJson)>;
    using ExecuteScriptCallback = std::function<void(const std::wstring& script)>;
    using IsReadyCallback     = std::function<bool()>;
    using FindSessionEndsCallback = std::function<std::vector<int>()>;

    CChatSettingProviderTab();
    ~CChatSettingProviderTab();

    // 初始化，传入回调和依赖
    void Init(
        PostMsgCallback postMsg,
        PostFullJsonCallback postFullJson,
        ExecuteScriptCallback executeScript,
        IsReadyCallback isReady,
        CChatTaskMgr* taskMgr,
        void* hwnd
    );

    // 设置查找会话结束位置的回调（用于压缩评估）
    void SetFindSessionEndsCallback(FindSessionEndsCallback callback);

    // 每帧更新（检查评估任务完成状态、延迟消息框等）
    void Update();

    // 刷新所有数据到 WebView（LLM Lib 版本变化时调用）
    void RefreshData();

    // 处理来自 JavaScript 的消息，返回 true 表示已处理
    bool HandleWebMessage(const std::string& action, const nlohmann::json& jsonMsg);

    // 重置评估状态（退出设置页面时调用）
    void ResetEvaluation();

    // ===== 外部调用的验证方法（通过 CChatSettingPage 转发）=====
    void StartValidatingProvider(const LlmApiProviderTypeName& providerTypeName);
    void EndValidatingProvider(const LlmApiProviderTypeName& providerTypeName, bool available, const std::string& errorMessage = "");
    bool IsValidatingProvider();

private:
    // 回调
    PostMsgCallback _postMsg;
    PostFullJsonCallback _postFullJson;
    ExecuteScriptCallback _executeScript;
    IsReadyCallback _isReady;
    CChatTaskMgr* _taskMgr;
    void* _hwnd;                // HWND
    FindSessionEndsCallback _findSessionEnds;

    // 状态
    bool _isEvaluatingSummarize;
    bool _needShowNoApiForValidation;

    // 内部方法
    void _SaveLlmJson();
    void _LoadProviderData();
    void _SendProviderDataToWebView();
    void _SendCapabilityStatusToWebView();
    void _SendCastSheetDataToWebView();
    void _UpdateCastSheetApi(const std::wstring& apiType, const std::wstring& apiName);
    void _UpdateProviderKey(const std::wstring& providerTypeName, const std::wstring& key);
    void _UpdateProviderName(const std::wstring& oldName, const std::wstring& newName);
    void _UpdateProviderEndpoint(const std::wstring& providerName, const std::wstring& endpoint);
    void _UpdateProviderFormat(const std::wstring& providerName, const std::wstring& format);
    void _UpdateApiName(const std::wstring& oldName, const std::wstring& newName);
    void _UpdateApiField(const std::wstring& apiName, const std::wstring& field, const nlohmann::json& value);
    void _AddProvider(const std::wstring& name);
    void _DeleteProvider(const std::wstring& name);
    void _AddApi(const std::wstring& providerName, const std::wstring& apiName);
    void _DeleteApi(const std::wstring& name);
    void _EvaluateCompressSummarize(const std::wstring& summarizeApiName);
};
