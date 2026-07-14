#pragma once
#include "ChatTaskMgr.h"
#include "Utils_InputHint.h"

class CChatTask_InputHint2 : public CChatTask
{
public:
    CChatTask_InputHint2(const std::wstring& content, const std::string& apiName, int caretTokenPos, const CRect& anchorRect, int contentVersion);

    const char* GetType() override { return "InputHint2"; }
    void Start() override;
    void Update() override;
    void Interrupt() override;
    int GetLlmSessionCount() override { return 1; }

private:
    void _Fail(const std::string& reason = "");
    std::string _CollectChatContextFromOps();

    std::string              _chatContext;
    Utils::InputContent      _originalInputContent;
    Utils::InputContent      _newInputContent;
    int                      _caretPlainPos;
    std::string              _apiName;
    std::string              _resultText;
    bool                     _hasStartedRequest;
    bool                     _requestInterrupt;
    int                      _contentVersion;

    CRect                    _anchorRect;
};
