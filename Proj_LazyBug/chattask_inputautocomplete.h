#pragma once
#include "ChatTaskMgr.h"
#include <string>

class CChatInputAutoCompleteWindow;

class CChatTask_InputAutoComplete : public CChatTask
{
public:
    CChatTask_InputAutoComplete(const std::string& partialInput, const std::string& apiName);

    const char* GetType() override { return "InputAutoComplete"; }
    void Start() override;
    void Update() override;
    void Interrupt() override;
    bool NeedLlmSession() override { return true; }

    void SetResultWindow(CChatInputAutoCompleteWindow* pWnd) { _pResultWindow = pWnd; }

private:
    void _Fail(const std::string& reason = "");
    std::string _CollectChatContextFromOps();

    std::string _chatContext;
    std::string _partialInput;
    std::string _apiName;
    std::string _resultText;
    bool _hasStartedRequest;
    bool _requestInterrupt;

    CChatInputAutoCompleteWindow* _pResultWindow;
};
