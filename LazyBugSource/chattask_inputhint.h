#pragma once
#include "ChatTaskMgr.h"
#include "Utils_InputHint.h"

class CChatTask_InputHint : public CChatTask
{
public:
    CChatTask_InputHint(const std::wstring& content, const std::string& apiName, int caretTokenPos, const CRect& anchorRect);

    const char* GetType() override { return "InputHint"; }
    void Start() override;
    void Update() override;
    void Interrupt() override;
    bool NeedLlmSession() override { return true; }

private:
    void _Fail(const std::string& reason = "");
    std::string _CollectChatContextFromOps();

    std::string              _chatContext;
    Utils::InputContent      _originalInputContent;
    Utils::InputContent      _newInputContent;
    int                      _caretPlainPos;   // 光标在 _originalInputContent.plainContent 中的字符位置(-1 表示无效)
    std::string              _apiName;
    std::string              _resultText;
    bool                     _hasStartedRequest;
    bool                     _requestInterrupt;

    CRect                    _anchorRect;
};
