#include "stdh.h"
#include "ChatSettingProviderTab.h"
#include <algorithm>
#include <nlohmann/json.hpp>
#include "llmlibloader.h"
#include "ChatTaskMgr.h"

// 外部函数声明
extern std::string widechar_to_utf8(const wchar_t* str);
extern std::wstring utf8_to_widechar(const std::string& utf8_str);
extern CLlmLib g_llmLib;

//////////////////////////////////////////////////////////////////////////
// CChatSettingProviderTab

CChatSettingProviderTab::CChatSettingProviderTab()
    : _taskMgr(nullptr)
    , _hwnd(nullptr)
    , _isEvaluatingSummarize(false)
    , _needShowNoApiForValidation(false)
{
}

CChatSettingProviderTab::~CChatSettingProviderTab()
{
}

void CChatSettingProviderTab::Init(
    PostMsgCallback postMsg,
    PostFullJsonCallback postFullJson,
    ExecuteScriptCallback executeScript,
    IsReadyCallback isReady,
    CChatTaskMgr* taskMgr,
    void* hwnd)
{
    _postMsg = postMsg;
    _postFullJson = postFullJson;
    _executeScript = executeScript;
    _isReady = isReady;
    _taskMgr = taskMgr;
    _hwnd = hwnd;
}

void CChatSettingProviderTab::SetFindSessionEndsCallback(FindSessionEndsCallback callback)
{
    _findSessionEnds = callback;
}

void CChatSettingProviderTab::Update()
{
    // 延迟显示消息框（因为webview回调中不能直接弹出MessageBox）
    if (_needShowNoApiForValidation)
    {
        _needShowNoApiForValidation = false;
        ::MessageBox((HWND)_hwnd,
            _T("To validate API key, at least one API with a valid model name is required."),
            _T("Validation Error"),
            MB_OK | MB_ICONWARNING);
    }

    // 检测 evaluation task 完成状态
    if (_isEvaluatingSummarize && _taskMgr && !_taskMgr->IsTaskTypeRunning("CompressSummarize"))
    {
        _isEvaluatingSummarize = false;

        // 发送消息隐藏 loading 动画
        _postMsg(L"endEvaluateSummarize", L"");

        // 弹出日志文件
        extern std::string GetCompressSummarizeLogPath();
        std::string logPath = GetCompressSummarizeLogPath();
        std::wstring wLogPath = utf8_to_widechar(logPath);
        ShellExecuteW(NULL, L"open", wLogPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
    }
}

void CChatSettingProviderTab::RefreshData()
{
    _SendProviderDataToWebView();
    _SendCapabilityStatusToWebView();
    _SendCastSheetDataToWebView();
}

void CChatSettingProviderTab::ResetEvaluation()
{
    if (_isEvaluatingSummarize)
    {
        _isEvaluatingSummarize = false;
        _postMsg(L"endEvaluateSummarize", L"");
    }
}

//====================== 消息处理 ======================

bool CChatSettingProviderTab::HandleWebMessage(const std::string& action, const nlohmann::json& jsonMsg)
{
    if (action == "requestProviderData")
    {
        _LoadProviderData();
        _SendProviderDataToWebView();
    }
    else if (action == "requestCapabilityStatus")
    {
        _SendCapabilityStatusToWebView();
    }
    else if (action == "requestCastSheetData")
    {
        _SendCastSheetDataToWebView();
    }
    else if (action == "updateCastSheetApi")
    {
        if (jsonMsg.contains("apiType") && jsonMsg.contains("apiName"))
        {
            std::string apiType = jsonMsg["apiType"];
            std::string apiName = jsonMsg["apiName"];
            _UpdateCastSheetApi(utf8_to_widechar(apiType), utf8_to_widechar(apiName));
        }
    }
    else if (action == "updateProviderKey")
    {
        if (jsonMsg.contains("providerType") && jsonMsg.contains("key"))
        {
            std::string providerTypeStr = jsonMsg["providerType"];
            std::string key = jsonMsg["key"];
            _UpdateProviderKey(utf8_to_widechar(providerTypeStr), utf8_to_widechar(key));
        }
    }
    else if (action == "updateProviderName")
    {
        if (jsonMsg.contains("oldName") && jsonMsg.contains("newName"))
        {
            std::string oldName = jsonMsg["oldName"];
            std::string newName = jsonMsg["newName"];
            _UpdateProviderName(utf8_to_widechar(oldName), utf8_to_widechar(newName));
        }
    }
    else if (action == "updateProviderEndpoint")
    {
        if (jsonMsg.contains("providerName") && jsonMsg.contains("endpoint"))
        {
            std::string providerName = jsonMsg["providerName"];
            std::string endpoint = jsonMsg["endpoint"];
            _UpdateProviderEndpoint(utf8_to_widechar(providerName), utf8_to_widechar(endpoint));
        }
    }
    else if (action == "updateProviderFormat")
    {
        if (jsonMsg.contains("providerName") && jsonMsg.contains("format"))
        {
            std::string providerName = jsonMsg["providerName"];
            std::string format = jsonMsg["format"];
            _UpdateProviderFormat(utf8_to_widechar(providerName), utf8_to_widechar(format));
        }
    }
    else if (action == "updateProviderStoreResponses")
    {
        if (jsonMsg.contains("providerName") && jsonMsg.contains("storeResponses"))
        {
            std::string providerName = jsonMsg["providerName"];
            bool storeResponses = jsonMsg["storeResponses"];
            _UpdateProviderStoreResponses(utf8_to_widechar(providerName), storeResponses);
        }
    }
    else if (action == "updateApiName")
    {
        if (jsonMsg.contains("oldName") && jsonMsg.contains("newName"))
        {
            std::string oldName = jsonMsg["oldName"];
            std::string newName = jsonMsg["newName"];
            _UpdateApiName(utf8_to_widechar(oldName), utf8_to_widechar(newName));
        }
    }
    else if (action == "updateApiField")
    {
        if (jsonMsg.contains("apiName") && jsonMsg.contains("field") && jsonMsg.contains("value"))
        {
            std::string apiName = jsonMsg["apiName"];
            std::string field = jsonMsg["field"];
            _UpdateApiField(utf8_to_widechar(apiName), utf8_to_widechar(field), jsonMsg["value"]);
        }
    }
    else if (action == "addProvider")
    {
        if (jsonMsg.contains("name"))
        {
            std::string name = jsonMsg["name"];
            _AddProvider(utf8_to_widechar(name));
        }
    }
    else if (action == "addProviderFromClipboard")
    {
        if (jsonMsg.contains("name"))
        {
            std::string name = jsonMsg["name"];
            std::wstring nameW = utf8_to_widechar(name);
            if (g_llmLib.AddProvider(name))
            {
                std::string endpoint = jsonMsg.contains("endpoint") ? jsonMsg["endpoint"].get<std::string>() : "";
                std::string formatStr = jsonMsg.contains("format") ? jsonMsg["format"].get<std::string>() : "";
                if (!endpoint.empty())
                    g_llmLib.SetProviderEndpoint(name, endpoint);
                if (!formatStr.empty())
                {
                    LlmApiFormat format = LlmApiFormat::Unknown;
                    if (formatStr == "OpenAI") format = LlmApiFormat::OpenAI_;
                    else if (formatStr == "Anthropic") format = LlmApiFormat::Anthropic_;
                    else if (formatStr == "Gemini") format = LlmApiFormat::Gemini_;
                    else if (formatStr == "OpenRouter") format = LlmApiFormat::OpenRouter;
                    else if (formatStr == "Kimi") format = LlmApiFormat::Kimi;
                    else if (formatStr == "GLM") format = LlmApiFormat::GLM;
                    else if (formatStr == "Minimax") format = LlmApiFormat::Minimax;
                    else if (formatStr == "DeepSeek") format = LlmApiFormat::DeepSeek;
                    else if (formatStr == "OpenAIResponses") format = LlmApiFormat::OpenAIResponses;
                    g_llmLib.SetProviderFormat(name, format);
                }
                if (jsonMsg.contains("storeResponses"))
                {
                    bool storeResponses = jsonMsg["storeResponses"];
                    g_llmLib.SetProviderStoreResponses(name, storeResponses);
                }
                _SaveLlmJson();
                _LoadProviderData();
                _SendProviderDataToWebView();
            }
        }
    }
    else if (action == "deleteProvider")
    {
        if (jsonMsg.contains("name"))
        {
            std::string name = jsonMsg["name"];
            _DeleteProvider(utf8_to_widechar(name));
        }
    }
    else if (action == "addApi")
    {
        if (jsonMsg.contains("providerName") && jsonMsg.contains("apiName"))
        {
            std::string providerName = jsonMsg["providerName"];
            std::string apiName = jsonMsg["apiName"];
            _AddApi(utf8_to_widechar(providerName), utf8_to_widechar(apiName));
        }
    }
    else if (action == "addApiFromClipboard")
    {
        if (jsonMsg.contains("providerName") && jsonMsg.contains("apiName"))
        {
            std::string providerName = jsonMsg["providerName"];
            std::string apiName = jsonMsg["apiName"];
            if (g_llmLib.AddApi(providerName, apiName))
            {
                if (jsonMsg.contains("model")) _UpdateApiField(utf8_to_widechar(apiName), L"model", jsonMsg["model"]);
                if (jsonMsg.contains("rule")) _UpdateApiField(utf8_to_widechar(apiName), L"rule", jsonMsg["rule"]);
                if (jsonMsg.contains("maxToken")) _UpdateApiField(utf8_to_widechar(apiName), L"maxToken", jsonMsg["maxToken"]);
                if (jsonMsg.contains("contextCapacity")) _UpdateApiField(utf8_to_widechar(apiName), L"contextCapacity", jsonMsg["contextCapacity"]);
                if (jsonMsg.contains("priceInputToken")) _UpdateApiField(utf8_to_widechar(apiName), L"priceInputToken", jsonMsg["priceInputToken"]);
                if (jsonMsg.contains("priceOutputToken")) _UpdateApiField(utf8_to_widechar(apiName), L"priceOutputToken", jsonMsg["priceOutputToken"]);
                if (jsonMsg.contains("priceCacheRead")) _UpdateApiField(utf8_to_widechar(apiName), L"priceCacheRead", jsonMsg["priceCacheRead"]);
                if (jsonMsg.contains("priceCacheWrite")) _UpdateApiField(utf8_to_widechar(apiName), L"priceCacheWrite", jsonMsg["priceCacheWrite"]);
                if (jsonMsg.contains("temperature")) _UpdateApiField(utf8_to_widechar(apiName), L"temperature", jsonMsg["temperature"]);
                if (jsonMsg.contains("thinkingMode")) _UpdateApiField(utf8_to_widechar(apiName), L"thinkingMode", jsonMsg["thinkingMode"]);
                if (jsonMsg.contains("cacheControl")) _UpdateApiField(utf8_to_widechar(apiName), L"cacheControl", jsonMsg["cacheControl"]);
                if (jsonMsg.contains("role")) _UpdateApiField(utf8_to_widechar(apiName), L"role", jsonMsg["role"]);
                if (jsonMsg.contains("enable")) _UpdateApiField(utf8_to_widechar(apiName), L"enable", jsonMsg["enable"]);
                if (jsonMsg.contains("tools")) _UpdateApiField(utf8_to_widechar(apiName), L"tools", jsonMsg["tools"]);
                if (jsonMsg.contains("openRouterOptions"))
                {
                    auto& opt = jsonMsg["openRouterOptions"];
                    if (opt.contains("disableReasoning")) _UpdateApiField(utf8_to_widechar(apiName), L"disableReasoning", opt["disableReasoning"]);
                    if (opt.contains("only")) _UpdateApiField(utf8_to_widechar(apiName), L"openRouterOnly", opt["only"]);
                }
                _SaveLlmJson();
                _LoadProviderData();
                _SendProviderDataToWebView();
            }
        }
    }
    else if (action == "deleteApi")
    {
        if (jsonMsg.contains("name"))
        {
            std::string name = jsonMsg["name"];
            _DeleteApi(utf8_to_widechar(name));
        }
    }
	else if (action == "reorderProviders")
	{
		if (jsonMsg.contains("orderedNames") && jsonMsg["orderedNames"].is_array())
		{
			std::vector<std::string> orderedNames;
			for (const auto& name : jsonMsg["orderedNames"])
				if (name.is_string())
					orderedNames.push_back(name.get<std::string>());
			g_llmLib.ReorderProviders(orderedNames);
			_SaveLlmJson();
		}
	}
	else if (action == "evaluateCompressSummarize")
    {
        if (jsonMsg.contains("apiName"))
        {
            std::string apiName = jsonMsg["apiName"];
            _EvaluateCompressSummarize(utf8_to_widechar(apiName));
        }
    }
    else
    {
        return false; // 未处理
    }
    return true;
}

//====================== Provider 数据处理方法 ======================

void CChatSettingProviderTab::_LoadProviderData()
{
    // 从g_llmLib加载最新的Provider数据
    // 这里不需要特别的操作，因为我们直接从g_llmLib读取
}

void CChatSettingProviderTab::_SendCastSheetDataToWebView()
{
    if (!_isReady())
        return;

    using json = nlohmann::json;

    std::string majorChatApi = g_llmLib.GetMajorChatApi();
    std::string briefApi = g_llmLib.GetBriefApi();
    std::string summarizeApi = g_llmLib.GetSummarizeApi();
    std::string inputHintApi = g_llmLib.GetInputHintApi();
    std::string embeddingApi = g_llmLib.GetEmbeddingApi();

    std::vector<const LlmApi*> availableApis;
    const auto& allApis = g_llmLib.GetApis();
    for (const auto& api : allApis)
    {
        if (!api.enable)
            continue;
        const LlmApiProvider* provider = g_llmLib.GetProvider(api.providerTypeName);
        if (provider && provider->IsAvailable())
        {
            availableApis.push_back(&api);
        }
    }

    std::sort(availableApis.begin(), availableApis.end(),
        [](const LlmApi* a, const LlmApi* b) {
            return _stricmp(a->name.c_str(), b->name.c_str()) < 0;
        });

    json jMajorChatApis = json::array();
    json jBriefApis = json::array();
    json jSummarizeApis = json::array();
    json jInputHintApis = json::array();
    json jEmbeddingApis = json::array();

    {
        json jDisable;
        jDisable["name"] = SUMMARIZE_API_DISABLE;
        jDisable["provider"] = "";
        jDisable["model"] = "";
        jSummarizeApis.push_back(jDisable);

        json jAuto;
        jAuto["name"] = SUMMARIZE_API_AUTO;
        jAuto["provider"] = "";
        jAuto["model"] = "";
        jSummarizeApis.push_back(jAuto);
    }

    {
        json jDisable;
        jDisable["name"] = INPUTHINT_API_DISABLE;
        jDisable["provider"] = "";
        jDisable["model"] = "";
        jInputHintApis.push_back(jDisable);
    }

    {
        json jDisable;
        jDisable["name"] = EMBEDDING_API_DISABLE;
        jDisable["provider"] = "";
        jDisable["model"] = "";
        jEmbeddingApis.push_back(jDisable);
    }

    for (const auto* api : availableApis)
    {
        json jApi;
        jApi["name"] = api->name;
        jApi["provider"] = api->providerTypeName;
        jApi["model"] = api->model;

        if (api->role == LlmApiRole::Agent)
        {
            jMajorChatApis.push_back(jApi);
        }

        jBriefApis.push_back(jApi);
        jSummarizeApis.push_back(jApi);
        jInputHintApis.push_back(jApi);

        if (api->role == LlmApiRole::Embedding)
        {
            jEmbeddingApis.push_back(jApi);
        }
    }

    json jCastSheet;
    jCastSheet["majorChatApi"] = majorChatApi;
    jCastSheet["inputHintApi"] = inputHintApi;
    jCastSheet["briefApi"] = briefApi;
    jCastSheet["summarizeApi"] = summarizeApi;
    jCastSheet["embeddingApi"] = embeddingApi;
    jCastSheet["majorChatApis"] = jMajorChatApis;
    jCastSheet["briefApis"] = jBriefApis;
    jCastSheet["summarizeApis"] = jSummarizeApis;
    jCastSheet["inputHintApis"] = jInputHintApis;
    jCastSheet["embeddingApis"] = jEmbeddingApis;

    std::string utf8Json = jCastSheet.dump();
    _postMsg(L"setCastSheetData", utf8_to_widechar(utf8Json));
}

void CChatSettingProviderTab::_UpdateCastSheetApi(const std::wstring& apiTypeW, const std::wstring& apiNameW)
{
    std::string apiType = widechar_to_utf8(apiTypeW.c_str());
    std::string apiName = widechar_to_utf8(apiNameW.c_str());

    if (apiType == "majorChat")
        g_llmLib.SetMajorChatApi(apiName);
    else if (apiType == "brief")
        g_llmLib.SetBriefApi(apiName);
    else if (apiType == "summarize")
        g_llmLib.SetSummarizeApi(apiName);
    else if (apiType == "inputHint")
        g_llmLib.SetInputHintApi(apiName);
    else if (apiType == "embedding")
        g_llmLib.SetEmbeddingApi(apiName);
}

void CChatSettingProviderTab::_SendProviderDataToWebView()
{
    if (!_isReady())
        return;

    using json = nlohmann::json;

    auto roleToStr = [](LlmApiRole r) -> std::string {
        switch (r) {
        case LlmApiRole::Agent:     return "Agent";
        case LlmApiRole::Auxiliary: return "Auxiliary";
        case LlmApiRole::Embedding: return "Embedding";
        default:                    return "None";
        }
    };
    auto toolToStr = [](LlmToolType t) -> std::string {
        switch (t) {
        case LlmToolType::ReplaceInFile:    return "ReplaceInFile";
        case LlmToolType::FindSymbolDefine: return "FindSymbolDefine";
        case LlmToolType::FindInFiles:      return "FindInFiles";
        case LlmToolType::SearchFile:       return "SearchFile";
        case LlmToolType::ReadFile:         return "ReadFile";
        case LlmToolType::ReadMedia:       return "ReadMedia";
        case LlmToolType::CLI_Cmd:          return "CLI_Cmd";
        case LlmToolType::CLI_Bash:         return "CLI_Bash";
        case LlmToolType::CLI_RunScript:    return "CLI_RunScript";
        case LlmToolType::Question:         return "Question";
        case LlmToolType::QueryFinish:      return "QueryFinish";
        case LlmToolType::CreateSkill:      return "CreateSkill";
        case LlmToolType::Mcp:              return "Mcp";
        case LlmToolType::AddMcpServer:     return "AddMcpServer";
        default:                            return "None";
        }
    };
    auto thinkingToStr = [](LlmThinkingMode m) -> std::string {
        switch (m) {
        case LlmThinkingMode::Enable:  return "Enable";
        case LlmThinkingMode::Disable: return "Disable";
        default:                       return "Auto";
        }
    };
    auto cacheToStr = [](LlmApiCacheControlType c) -> std::string {
        switch (c) {
        case LlmApiCacheControlType::Anthropic_: return "Anthropic";
        case LlmApiCacheControlType::None_:      return "None";
        default:                                 return "Auto";
        }
    };
    auto formatToStr = [](LlmApiFormat f) -> std::string {
        switch (f) {
        case LlmApiFormat::OpenAI_:     return "OpenAI";
        case LlmApiFormat::Anthropic_:  return "Anthropic";
        case LlmApiFormat::Gemini_:     return "Gemini";
        case LlmApiFormat::OpenRouter:  return "OpenRouter";
        case LlmApiFormat::Kimi:        return "Kimi";
        case LlmApiFormat::GLM:         return "GLM";
        case LlmApiFormat::Minimax:     return "Minimax";
        case LlmApiFormat::DeepSeek:    return "DeepSeek";
        case LlmApiFormat::OpenAIResponses: return "OpenAIResponses";
        default:                        return "Unknown";
        }
    };

    const auto& allApis = g_llmLib.GetApis();
    int providerCount = g_llmLib.GetProviderCount();

    json jProviders = json::array();
    for (int i = 0; i < providerCount; i++)
    {
        const LlmApiProvider* p = g_llmLib.GetProvider(i);
        if (!p || p->name.empty())
            continue;

        json jProvider;
        jProvider["name"]        = p->name;
        jProvider["endpoint"]    = p->endpoint;
        jProvider["key"]         = p->key;
        jProvider["type"]        = p->name;
        jProvider["format"]      = formatToStr(p->format);
        jProvider["storeResponses"] = p->storeResponses;
        jProvider["isAvailable"] = p->IsAvailable();

        json jApis = json::array();
        for (const auto& api : allApis)
        {
            if (api.providerTypeName != p->name)
                continue;

            json jApi;
            jApi["name"]             = api.name;
            jApi["model"]            = api.model;
            jApi["rule"]             = api.rule;
            jApi["maxToken"]         = api.maxToken;
            jApi["contextCapacity"]  = api.contextCapacity;
            jApi["priceInputToken"]  = api.priceInputToken;
            jApi["priceOutputToken"] = api.priceOutputToken;
            jApi["priceCacheRead"]   = api.priceCacheRead;
            jApi["priceCacheWrite"]  = api.priceCacheWrite;
            jApi["temperature"]      = api.temperature;
            jApi["thinkingMode"]     = thinkingToStr(api.thinkingMode);
            jApi["cacheControl"]     = cacheToStr(api.cacheControlType);
            jApi["providerTypeName"] = api.providerTypeName;
            jApi["enable"]           = api.enable;
            jApi["role"]             = roleToStr(api.role);

            json jTools = json::array();
            for (auto to : api.tools)
                jTools.push_back(toolToStr(to));
            jApi["tools"] = jTools;

            jApi["openRouterOptions"]["disableReasoning"] = api.openRouterOptions.disableReasoning;
            json jOnly = json::array();
            for (const auto& s : api.openRouterOptions.only)
                jOnly.push_back(s);
            jApi["openRouterOptions"]["only"] = jOnly;

            jApis.push_back(jApi);
        }
        jProvider["apis"] = jApis;
        jProviders.push_back(jProvider);
    }

    std::string utf8Json = jProviders.dump();
    _postMsg(L"setProviderData", utf8_to_widechar(utf8Json));
}

void CChatSettingProviderTab::_UpdateProviderKey(const std::wstring& providerTypeStr, const std::wstring& key)
{
    try
    {
        std::string providerTypeName = widechar_to_utf8(providerTypeStr.c_str());
        std::string keyUtf8 = widechar_to_utf8(key.c_str());

        bool needValidate = false;
        const LlmApiProvider* provider = g_llmLib.GetProvider(providerTypeName);
        if (provider)
        {
            if (provider->status == LlmApiProvider::Status::Ok)
            {
                if (keyUtf8 != provider->key)
                    needValidate = true;
            }
            else
            {
                if (!keyUtf8.empty())
                    needValidate = true;
            }

            if (needValidate)
            {
                std::string apiName = g_llmLib.FindApiToValidateApiKey(providerTypeName);
                if (apiName.empty())
                {
                    _needShowNoApiForValidation = true;
                    needValidate = false;
                }
            }
        }

        if (g_llmLib.SetProviderKey(providerTypeName, keyUtf8))
        {
            g_llmLib.SaveSettings();
        }

        _SendCastSheetDataToWebView();
        _SendProviderDataToWebView();

        if (needValidate && _taskMgr)
            _taskMgr->AddTask_VerifyLlmApiProvider(providerTypeName);
    }
    catch (...)
    {
    }
}

//====================== 验证方法 ======================

void CChatSettingProviderTab::_SaveLlmJson()
{
    std::string dbFolder = Utils::GetDBRootFolder_utf8();
    std::string jsonPath = dbFolder + "\\llm.json";
    CLlmLibLoader::SaveJsonFile(g_llmLib, jsonPath.c_str());
}

void CChatSettingProviderTab::_UpdateProviderName(const std::wstring& oldNameW, const std::wstring& newNameW)
{
    std::string oldName = widechar_to_utf8(oldNameW.c_str());
    std::string newName = widechar_to_utf8(newNameW.c_str());
    if (g_llmLib.SetProviderName(oldName, newName))
        _SaveLlmJson();
}

void CChatSettingProviderTab::_UpdateProviderEndpoint(const std::wstring& providerNameW, const std::wstring& endpointW)
{
    std::string providerName = widechar_to_utf8(providerNameW.c_str());
    std::string endpoint     = widechar_to_utf8(endpointW.c_str());
    if (g_llmLib.SetProviderEndpoint(providerName, endpoint))
        _SaveLlmJson();
}

void CChatSettingProviderTab::_UpdateProviderFormat(const std::wstring& providerNameW, const std::wstring& formatW)
{
    std::string providerName = widechar_to_utf8(providerNameW.c_str());
    std::string formatStr    = widechar_to_utf8(formatW.c_str());

    LlmApiFormat format = LlmApiFormat::Unknown;
    if (formatStr == "OpenAI") format = LlmApiFormat::OpenAI_;
    else if (formatStr == "Anthropic") format = LlmApiFormat::Anthropic_;
    else if (formatStr == "Gemini") format = LlmApiFormat::Gemini_;
    else if (formatStr == "OpenRouter") format = LlmApiFormat::OpenRouter;
    else if (formatStr == "Kimi") format = LlmApiFormat::Kimi;
    else if (formatStr == "GLM") format = LlmApiFormat::GLM;
    else if (formatStr == "Minimax") format = LlmApiFormat::Minimax;
    else if (formatStr == "DeepSeek") format = LlmApiFormat::DeepSeek;
    else if (formatStr == "OpenAIResponses") format = LlmApiFormat::OpenAIResponses;

    if (g_llmLib.SetProviderFormat(providerName, format))
        _SaveLlmJson();
}

void CChatSettingProviderTab::_UpdateProviderStoreResponses(const std::wstring& providerNameW, bool storeResponses)
{
    std::string providerName = widechar_to_utf8(providerNameW.c_str());
    if (g_llmLib.SetProviderStoreResponses(providerName, storeResponses))
        _SaveLlmJson();
}

void CChatSettingProviderTab::_UpdateApiName(const std::wstring& oldNameW, const std::wstring& newNameW)
{
    std::string oldName = widechar_to_utf8(oldNameW.c_str());
    std::string newName = widechar_to_utf8(newNameW.c_str());
    if (g_llmLib.SetApiName(oldName, newName))
        _SaveLlmJson();
}

void CChatSettingProviderTab::_AddProvider(const std::wstring& nameW)
{
    std::string name = widechar_to_utf8(nameW.c_str());
    if (g_llmLib.AddProvider(name))
    {
        _SaveLlmJson();
        _LoadProviderData();
        _SendProviderDataToWebView();
    }
}

void CChatSettingProviderTab::_DeleteProvider(const std::wstring& nameW)
{
    std::string name = widechar_to_utf8(nameW.c_str());
    if (g_llmLib.DeleteProvider(name))
    {
        _SaveLlmJson();
        _LoadProviderData();
        _SendProviderDataToWebView();
    }
}

void CChatSettingProviderTab::_AddApi(const std::wstring& providerNameW, const std::wstring& apiNameW)
{
    std::string providerName = widechar_to_utf8(providerNameW.c_str());
    std::string apiName = widechar_to_utf8(apiNameW.c_str());
    if (g_llmLib.AddApi(providerName, apiName))
    {
        _SaveLlmJson();
        _LoadProviderData();
        _SendProviderDataToWebView();
    }
}

void CChatSettingProviderTab::_DeleteApi(const std::wstring& nameW)
{
    std::string name = widechar_to_utf8(nameW.c_str());
    if (g_llmLib.DeleteApi(name))
    {
        _SaveLlmJson();
        _LoadProviderData();
        _SendProviderDataToWebView();
    }
}

void CChatSettingProviderTab::_UpdateApiField(const std::wstring& apiNameW, const std::wstring& fieldW, const nlohmann::json& value)
{
    using json = nlohmann::json;

    std::string apiName = widechar_to_utf8(apiNameW.c_str());
    std::string field   = widechar_to_utf8(fieldW.c_str());

    auto roleFromStr = [](const std::string& s) -> LlmApiRole {
        if (s == "Agent")     return LlmApiRole::Agent;
        if (s == "Auxiliary") return LlmApiRole::Auxiliary;
        if (s == "Embedding") return LlmApiRole::Embedding;
        return LlmApiRole::None;
    };
    auto toolFromStr = [](const std::string& s) -> LlmToolType {
        if (s == "ReplaceInFile")    return LlmToolType::ReplaceInFile;
        if (s == "FindSymbolDefine") return LlmToolType::FindSymbolDefine;
        if (s == "FindInFiles")      return LlmToolType::FindInFiles;
        if (s == "SearchFile")       return LlmToolType::SearchFile;
        if (s == "ReadFile")         return LlmToolType::ReadFile;
        if (s == "ReadMedia")       return LlmToolType::ReadMedia;
        if (s == "CLI_Cmd")          return LlmToolType::CLI_Cmd;
        if (s == "CLI_Bash")         return LlmToolType::CLI_Bash;
        if (s == "CLI_RunScript")    return LlmToolType::CLI_RunScript;
        if (s == "Question")         return LlmToolType::Question;
        if (s == "QueryFinish")      return LlmToolType::QueryFinish;
        if (s == "CreateSkill")      return LlmToolType::CreateSkill;
        if (s == "AddMcpServer")     return LlmToolType::AddMcpServer;
        return LlmToolType::None;
    };

    LlmApi* api = g_llmLib.GetApiMutable(apiName);
    if (!api)
        return;

    if (field == "model" && value.is_string())
        api->model = value.get<std::string>();
    else if (field == "rule" && value.is_string())
        api->rule = value.get<std::string>();
    else if (field == "maxToken" && value.is_number())
        api->maxToken = value.get<int>();
    else if (field == "contextCapacity" && value.is_number())
        api->contextCapacity = value.get<int>();
    else if (field == "priceInputToken" && value.is_number())
        api->priceInputToken = value.get<float>();
    else if (field == "priceOutputToken" && value.is_number())
        api->priceOutputToken = value.get<float>();
    else if (field == "priceCacheRead" && value.is_number())
        api->priceCacheRead = value.get<float>();
    else if (field == "priceCacheWrite" && value.is_number())
        api->priceCacheWrite = value.get<float>();
    else if (field == "temperature" && value.is_number())
        api->temperature = value.get<float>();
    else if (field == "thinkingMode" && value.is_string())
    {
        std::string v = value.get<std::string>();
        if (v == "Enable")       api->thinkingMode = LlmThinkingMode::Enable;
        else if (v == "Disable") api->thinkingMode = LlmThinkingMode::Disable;
        else                     api->thinkingMode = LlmThinkingMode::Auto;
    }
    else if (field == "cacheControl" && value.is_string())
    {
        std::string v = value.get<std::string>();
        if (v == "Anthropic")  api->cacheControlType = LlmApiCacheControlType::Anthropic_;
        else if (v == "None")  api->cacheControlType = LlmApiCacheControlType::None_;
        else                   api->cacheControlType = LlmApiCacheControlType::Auto;
    }
    else if (field == "role" && value.is_string())
    {
        std::string v = value.get<std::string>();
        api->role = roleFromStr(v);
    }
    else if (field == "tools" && value.is_array())
    {
        api->tools.clear();
        for (const auto& elem : value)
            if (elem.is_string())
                api->tools.push_back(toolFromStr(elem.get<std::string>()));
    }
    else if (field == "disableReasoning" && value.is_boolean())
        api->openRouterOptions.disableReasoning = value.get<bool>();
    else if (field == "openRouterOnly" && value.is_array())
    {
        api->openRouterOptions.only.clear();
        for (const auto& elem : value)
            if (elem.is_string())
                api->openRouterOptions.only.push_back(elem.get<std::string>());
    }
    else if (field == "enable" && value.is_boolean())
        api->enable = value.get<bool>();
    else
        return;

    _SaveLlmJson();
}

void CChatSettingProviderTab::StartValidatingProvider(const LlmApiProviderTypeName& providerTypeName)
{
    if (!_isReady())
        return;

    std::wstring jsonMessage = L"{\"action\":\"startValidatingProvider\",\"providerType\":\"" + utf8_to_widechar(providerTypeName.c_str()) + L"\"}";
    _postFullJson(jsonMessage);
}

void CChatSettingProviderTab::EndValidatingProvider(const LlmApiProviderTypeName& providerTypeName, bool available, const std::string& errorMessage)
{
    if (!_isReady())
        return;

    std::wstring wProviderType = utf8_to_widechar(providerTypeName.c_str());

    if (errorMessage.empty())
    {
        std::wstring jsonMessage = L"{\"action\":\"endValidatingProvider\",\"providerType\":\"" + wProviderType + L"\",\"available\":" + (available ? L"true" : L"false") + L"}";
        _postFullJson(jsonMessage);
    }
    else
    {
        std::wstring wError = utf8_to_widechar(errorMessage.c_str());
        // JSON 转义
        std::wstring escapedError;
        for (wchar_t ch : wError)
        {
            switch (ch)
            {
            case L'"':  escapedError += L"\\\""; break;
            case L'\\': escapedError += L"\\\\"; break;
            case L'\n': escapedError += L"\\n";  break;
            case L'\r': escapedError += L"\\r";  break;
            case L'\t': escapedError += L"\\t";  break;
            default:    escapedError += ch;      break;
            }
        }
        std::wstring jsonMessage = L"{\"action\":\"endValidatingProvider\",\"providerType\":\"" + wProviderType + L"\",\"available\":" + (available ? L"true" : L"false") + L",\"errorMessage\":\"" + escapedError + L"\"}";
        _postFullJson(jsonMessage);
    }

    _SendCastSheetDataToWebView();
    _SendProviderDataToWebView();
}

void CChatSettingProviderTab::_SendCapabilityStatusToWebView()
{
    if (!_isReady())
        return;

    CLlmLib::WorkingCapability capability = g_llmLib.GetWorkingCapability();

    std::wstring capabilityJson = L"{";
    capabilityJson += L"\"action\":\"setCapabilityStatus\",";
    capabilityJson += L"\"capability\":" + std::to_wstring((int)capability);
    capabilityJson += L"}";

    _postFullJson(capabilityJson);
}

bool CChatSettingProviderTab::IsValidatingProvider()
{
    if (_taskMgr && _taskMgr->IsTaskTypeRunning("VerifyLlmApiProvider"))
        return true;
    return false;
}

void CChatSettingProviderTab::_EvaluateCompressSummarize(const std::wstring& summarizeApiName)
{
    if (summarizeApiName.empty())
        return;

    if (!_findSessionEnds)
        return;

    std::vector<int> sessionEnds = _findSessionEnds();
    if (sessionEnds.empty())
        return;

    _postMsg(L"startEvaluateSummarize", L"");

    extern void ClearCompressSummarize();
    ClearCompressSummarize();

    _isEvaluatingSummarize = true;

    for (int sessionEndIndex : sessionEnds)
    {
        std::string apiNameUtf8 = widechar_to_utf8(summarizeApiName.c_str());
        _taskMgr->AddTask_CompressSummarize(sessionEndIndex, apiNameUtf8, CompressSummarizeMode::Evaluation);
    }
}
