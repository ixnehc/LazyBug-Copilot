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
    int GetLlmSessionCount() override { return 1; }//设为1表示,屏蔽check complete,设为2打开

private:
    void _Fail(const std::string& reason = "");
    std::string _CollectChatContextFromOps();

    // 处理 inputhint(补全) 会话的输出, 完成后填充 _pendingNewDiff/_pendingOldDiff 等
    void _ProcessInputHintSession();
    // 处理 checkcomplete(完整性判断) 会话的输出
    void _ProcessCheckCompleteSession();
    // 两个请求都完成后, 决定是显示还是隐藏补全提示
    void _TryFinalize();

    std::string              _chatContext;
    Utils::InputContent      _originalInputContent;
    Utils::InputContent      _newInputContent;
    int                      _caretPlainPos;   // 光标在 _originalInputContent.plainContent 中的字符位置(-1 表示无效)
    std::string              _apiName;
    std::string              _resultText;
    std::wstring             _inputWithCaret;  // 发送给 LLM 的带光标标记的内容
    bool                     _hasStartedRequest;
    bool                     _requestInterrupt;

    // 并行的 checkcomplete 请求(使用 _llmChats[1], 与 inputhint 同时发送, 无先后)
    bool                     _checkCompleteStarted;

    // 两个请求各自的完成状态与结果(用于两者都完成后统一决定显示/隐藏)
    bool                     _inputHintFinished;      // inputhint 请求是否已处理完毕
    bool                     _checkCompleteFinished;  // checkcomplete 请求是否已处理完毕
    bool                     _isInputComplete;        // checkcomplete 结果: true 表示 [complete]
    bool                     _hintValid;              // inputhint 是否产生了有效补全
    Utils::DiffedInputContent _pendingNewDiff;
    Utils::DiffedInputContent _pendingOldDiff;

    CRect                    _anchorRect;
};

