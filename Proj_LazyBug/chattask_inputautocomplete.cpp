#include "stdh.h"
#include "ChatTask_InputAutoComplete.h"
#include "LlmChat.h"
#include "LlmLib.h"
#include "ChatOpsCtrl.h"
#include "chatinputautocompletewindow.h"
#include <unordered_set>

CChatTask_InputAutoComplete::CChatTask_InputAutoComplete(const std::string& partialInput, const std::string& apiName)
{
    _partialInput = partialInput;
    _apiName = apiName;
    _hasStartedRequest = false;
    _requestInterrupt = false;
    _pResultWindow = nullptr;
}

void CChatTask_InputAutoComplete::_Fail(const std::string& reason)
{
    if (_pResultWindow)
    {
        std::string errMsg = "[AutoComplete Error]";
        if (!reason.empty())
            errMsg += " " + reason;
        _pResultWindow->ShowCompletion(errMsg);
    }
    _status = TaskStatus::Failure;
}

std::string CChatTask_InputAutoComplete::_CollectChatContextFromOps()
{
    if (!_context || !_context->chatOpsCtrl)
        return "";

    const std::vector<ChatOp>& ops = _context->chatOpsCtrl->GetOps();

    // 从 disable 边界开始往前找（跳过被 disabled 的消息）
    int disableAfterIndex = _context->chatOpsCtrl->GetDisableAfterIndex();
    int startIdx = disableAfterIndex - 1;
    if (startIdx >= (int)ops.size())
        startIdx = (int)ops.size() - 1;

    std::string result;
    const int MAX_LEN = 8000;
    std::unordered_set<std::wstring> seenStreamingIds;

    for (int i = startIdx; i >= 0; i--)
    {
        const ChatOp& op = ops[i];
        std::string prefix;
        const std::string* pContent = nullptr;

        if (op.type == ChatOp::Op_AddUserMessage)
        {
            prefix = "User: ";
            pContent = &op.contentUtf8;
        }
        else if (op.type == ChatOp::Op_AddStreamingAIMessage)
        {
            if (op.contentUtf8.empty())
                continue;
            if (!seenStreamingIds.insert(op.messageId).second)
                continue;
            prefix = "Assistant: ";
            pContent = &op.contentUtf8;
        }
        else
        {
            continue;
        }

        std::string entry = prefix + *pContent;
        if (!result.empty())
            entry += "\n";

        // 超出总量限制则停止收集，但不会截断单条消息
        if ((int)(entry.size() + result.size()) > MAX_LEN)
            break;

        result = entry + result;
    }

    return result;
}

void CChatTask_InputAutoComplete::Start()
{
    if (_apiName.empty())
    {
        _Fail("No API name");
        return;
    }

    if (_partialInput.empty())
    {
        _Fail("Empty input");
        return;
    }

    // 在 task 内收集上下文
    _chatContext = _CollectChatContextFromOps();

    LlmSessionSetting setting;
    if (!g_llmLib.LoadLlmSetting(setting, _apiName, false, ""))
    {
        _Fail("Failed to load LLM setting");
        return;
    }

    setting.api.tools.clear();
    setting.rulesFiles.clear();

    LlmSessionRequest request;

    std::string prompt;
    prompt = "You are an input auto-completion assistant. Based on the recent chat context and the user's partial input,\n"
             "suggest a plausible completion of what the user might be typing next.\n"
             "You may also correct typos or improve the existing partial input when appropriate.\n"
             "The completion should be concise and in the same language as the partial input.\n"
             "You MUST strictly follow this output format and output NOTHING else:\n"
             "<<Old Content>>~~||~~<<New Content>>\n"
             "Where <Old Content> is the user's partial input, and <New Content> is the corrected and/or completed text.\n"
             "Old and new content are separated by ~~||~~.\n\n"
             "Examples:\n"
             "Partial input \"Hello\" -> output: Hello~~||~~Hello world\n"
             "Partial input \"How are\" -> output: How are~~||~~How are you\n"
             "Partial input \"请帮我\" -> output: 请帮我~~||~~请帮我写一个函数\n"
             "Partial input \"hwo to\" (typo) -> output: hwo to~~||~~how to write\n"
             "Partial input \"请帮我谢一个\" (typo) -> output: 请帮我谢一个~~||~~请帮我写一个函数\n\n";

    if (!_chatContext.empty())
    {
        prompt += "Recent chat context:\n";
        prompt += _chatContext;
        prompt += "\n\n";
    }

    prompt += "User's partial input:\n";
    prompt += _partialInput;
    prompt += "\n\n";
    prompt += "Completion:";

    request.AddUserMessage(prompt.c_str());
    request.isStreaming = true;

    if (!_llmChat->Request(request, setting))
    {
        _Fail("Failed to send LLM request");
        return;
    }

    _hasStartedRequest = true;
    _status = TaskStatus::Running;
}

void CChatTask_InputAutoComplete::Update()
{
    if (_status != TaskStatus::Running)
        return;

    if (!_llmChat)
        return;

    if (_llmChat->HasActiveSession())
    {
        LlmSessionOutput output;
        if (_llmChat->Process(output, _requestInterrupt))
        {
            if (output.isCompleted)
            {
                if (_requestInterrupt)
                {
                    _Fail("Interrupted");
                }
                else if (output.hasError)
                {
                    _Fail(output.errorMessage);
                }
                else
                {
                    _resultText = output.fullContent;

                    size_t start = _resultText.find_first_not_of(" \t\r\n");
                    size_t end = _resultText.find_last_not_of(" \t\r\n");
                    if (start != std::string::npos && end != std::string::npos)
                        _resultText = _resultText.substr(start, end - start + 1);
                    else
                        _resultText.clear();

                    if (_pResultWindow && !_resultText.empty())
                        _pResultWindow->ShowCompletion(_resultText);

                    _status = TaskStatus::Success;
                }
            }
        }
    }
    else if (_hasStartedRequest)
    {
        _Fail("LLM session ended unexpectedly");
    }
}

void CChatTask_InputAutoComplete::Interrupt()
{
    _requestInterrupt = true;
    Update();
}
