#pragma once
#include "ChatTaskMgr.h"
#include "Utils_InputHint.h"

class CInputHintWindow;

class CChatTask_InputHint : public CChatTask
{
public:
    CChatTask_InputHint(const std::wstring& content, const std::string& apiName);

    const char* GetType() override { return "InputHint"; }
    void Start() override;
    void Update() override;
    void Interrupt() override;
    bool NeedLlmSession() override { return true; }

    void SetHintWindow(CInputHintWindow* pWnd, const CRect& anchorRect) { _pHintWindow = pWnd; _anchorRect = anchorRect; }

private:
    void _Fail(const std::string& reason = "");
    std::string _CollectChatContextFromOps();

    std::string              _chatContext;
    Utils::InputContent      _originalInputContent;
    Utils::InputContent      _newInputContent;
    std::string              _apiName;
    std::string              _resultText;
    bool                     _hasStartedRequest;
    bool                     _requestInterrupt;

    CInputHintWindow*        _pHintWindow;
    CRect                    _anchorRect;
};
