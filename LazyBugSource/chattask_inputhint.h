#pragma once
#include "ChatTaskMgr.h"
#include "Utils_InputHint.h"

class CChatTask_InputHint : public CChatTask
{
public:
    CChatTask_InputHint(const std::wstring& content, const std::string& apiName, int caretTokenPos, const CRect& anchorRect, int contentVersion);

    // 补全结果解析方式: 分隔符字串 或 JSON
    enum class InputHintFormat
    {
        Separator,  // 现有: old~~||~~new
        Json        // 新增: {"old":"...","new":"..."}
    };
    // 切换新旧解析方式: 修改此常量即可(默认保持旧行为)
    static const InputHintFormat kInputHintFormat;

    const char* GetType() override { return "InputHint"; }
    void Start() override;
    void Update() override;
    void Interrupt() override;
    int GetLlmSessionCount() override { return 1; }//设为1表示,屏蔽check complete,设为2打开

private:
    void _Fail(const std::string& reason = "");

    // 启动 inputhint(补全)会话, 返回 true 表示成功发送请求
    bool _StartInputHintSession();
    // 启动 checkcomplete(完整性判断)会话, 返回 true 表示成功发送请求
    bool _StartCheckCompleteSession();
    // 处理 inputhint(补全) 会话的输出, 完成后填充 _pendingNewDiff/_pendingOldDiff 等
    void _ProcessInputHintSession();
    // 从 LLM 原始返回文本中提取 (old, new), 根据 kInputHintFormat 选择解析方式
    bool _ExtractOldNew(const std::string& result, std::wstring& oldW, std::wstring& newW);
    // 处理 checkcomplete(完整性判断) 会话的输出
    void _ProcessCheckCompleteSession();
    // 两个请求都完成后, 决定是显示还是隐藏补全提示
    void _TryFinalize();

    Utils::InputContent      _originalInputContent;
    Utils::InputContent      _newInputContent;
    int                      _caretPlainPos;   // 光标在 _originalInputContent.plainContent 中的字符位置(-1 表示无效)
    std::string              _apiName;
    std::string              _resultText;
    std::wstring             _inputWithCaret;  // 发送给 LLM 的带光标标记的内容
    std::wstring             _caretLine;       // 光标所在行(含光标标记, 来自 InputHintContext)
    std::wstring             _beforeCaretLines;// 光标行之前的行(来自 InputHintContext)
    std::wstring             _afterCaretLines; // 光标行之后的行(来自 InputHintContext)
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
    Utils::GhostContent       _pendingGhost;
    int                       _contentVersion;

    CRect                    _anchorRect;
};

