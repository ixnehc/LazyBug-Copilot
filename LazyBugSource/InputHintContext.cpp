#include "stdh.h"
#include "InputHintContext.h"
#include "ChatOpsCtrl.h"
#include "Utils_InputHint.h"

#include <cstring>
#include <unordered_set>



void InputHintContext::UpdateFromOps(const CChatOpsCtrl& opsCtrl)
{
    const DWORD opsVersion = opsCtrl.GetVer();
    if (opsVersion == _chatOpsContentVersion)
        return;


    const std::vector<ChatOp>& ops = opsCtrl.GetOps();
    int startIndex = opsCtrl.GetDisableAfterIndex() - 1;
    if (startIndex >= static_cast<int>(ops.size()))
        startIndex = static_cast<int>(ops.size()) - 1;

    std::string result;
    const size_t kMaxLength = 8000;
    const std::string kEllipsis = "...";
    std::unordered_set<std::wstring> seenStreamingMessageIds;

    for (int i = startIndex; i >= 0; --i)
    {
        const ChatOp& op = ops[i];
        const char* prefix = nullptr;
        const std::string* content = nullptr;

        if (op.type == ChatOp::Op_AddUserMessage)
        {
            prefix = "User: ";
            content = &op.contentUtf8;
        }
        else if (op.type == ChatOp::Op_AddStreamingAIMessage)
        {
            if (op.contentUtf8.empty())
                continue;

            if (!seenStreamingMessageIds.insert(op.messageId).second)
                continue;

            prefix = "Assistant: ";
            content = &op.contentUtf8;
        }
        else
        {
            continue;
        }

        std::string entry = std::string(prefix) + *content;
        if (!result.empty())
            entry += "\n";

        const size_t remaining = kMaxLength > result.size()
            ? kMaxLength - result.size()
            : 0;

        if (entry.size() > remaining)
        {
            const size_t suffixLength = result.empty() ? 0 : 1;
            if (remaining <= kEllipsis.size() + std::strlen(prefix) + suffixLength)
                break;

            const size_t budget = remaining - kEllipsis.size() - std::strlen(prefix) - suffixLength;
            size_t start = content->size() - budget;
            // 跳过 UTF-8 续字节(10xxxxxx)，确保截断在合法字符边界
            while (start < content->size() && ((*content)[start] & 0xC0) == 0x80)
                ++start;
            entry = std::string(prefix) + kEllipsis + content->substr(start);
            if (!result.empty())
                entry += "\n";

            result = entry + result;
            break;
        }

        result = entry + result;
    }

    _chatOpsContent = result;
    _chatOpsContentVersion = opsVersion;
}


void InputHintContext::UpdateInput(const std::wstring& fullContent, int newCaretTokenPos)
{
    _caretTokenPos = newCaretTokenPos;
    _caretPlainPos = -1;
    _caretLine.clear();
    _beforeCaretLines.clear();
    _afterCaretLines.clear();

    const Utils::InputContent inputContent = Utils::BuildInputContent(fullContent);
    const std::wstring& plainContent = inputContent.plainContent;

    // 将光标的 token 位置转换为 plainContent 中的字符位置
    // token 规则: 普通字符 = 1 token, 每个 tag = 1 token(与 CChatInput 的编号一致)
    if (_caretTokenPos >= 0)
    {
        const std::vector<Utils::InputContentTagSegment>& tagSegments = inputContent.tagSegments;

        size_t plainPos = 0;
        size_t segmentIndex = 0;
        int tokenIndex = 0;

        while (plainPos <= plainContent.size())
        {
            if (tokenIndex == _caretTokenPos)
            {
                _caretPlainPos = static_cast<int>(plainPos);
                break;
            }

            if (plainPos >= plainContent.size())
                break;

            if (segmentIndex < tagSegments.size() &&
                plainPos == tagSegments[segmentIndex].startPos)
            {
                plainPos = tagSegments[segmentIndex].endPos;
                ++segmentIndex;
            }
            else
            {
                ++plainPos;
            }

            ++tokenIndex;
        }
    }

    // 在光标位置插入光标标记, 并按其所在行拆分为三部分
    std::wstring inputWithCaret = plainContent;
    if (_caretPlainPos >= 0 && _caretPlainPos <= static_cast<int>(inputWithCaret.size()))
        inputWithCaret.insert(static_cast<size_t>(_caretPlainPos), L"\x2038");

    bool foundCaret = false;
    size_t lineStart = 0;
    for (size_t i = 0; i <= inputWithCaret.size(); ++i)
    {
        if (i == inputWithCaret.size() || inputWithCaret[i] == L'\n')
        {
            std::wstring line = inputWithCaret.substr(lineStart, i - lineStart);
            bool hasCaret = (line.find(L'\x2038') != std::wstring::npos);

            if (!foundCaret && hasCaret)
            {
                _caretLine = line;
                foundCaret = true;
            }
            else if (!foundCaret)
            {
                if (!_beforeCaretLines.empty())
                    _beforeCaretLines += L'\n';
                _beforeCaretLines += line;
            }
            else
            {
                if (!_afterCaretLines.empty())
                    _afterCaretLines += L'\n';
                _afterCaretLines += line;
            }

            lineStart = i + 1;
        }
    }

    // 光标行无标记则回退到完整内容(不应发生)
    if (_caretLine.empty())
        _caretLine = inputWithCaret;
}



void InputHintContext::Clear()
{
    _chatOpsContentVersion = 0;
    _chatOpsContent.clear();
    _caretLine.clear();
    _beforeCaretLines.clear();
    _afterCaretLines.clear();
    _caretTokenPos = -1;
    _caretPlainPos = -1;
    _similarChunksText.clear();
    _historySimilarChunksText.clear();
}


const std::string& InputHintContext::GetChatOpsContent() const
{
    return _chatOpsContent;
}

uint32_t InputHintContext::GetChatOpsContentVersion() const
{
    return _chatOpsContentVersion;
}

const std::wstring& InputHintContext::GetCaretLine() const
{
    return _caretLine;
}

const std::wstring& InputHintContext::GetBeforeCaretLines() const
{
    return _beforeCaretLines;
}

const std::wstring& InputHintContext::GetAfterCaretLines() const
{
    return _afterCaretLines;
}

int InputHintContext::GetCaretTokenPos() const
{
    return _caretTokenPos;
}


int InputHintContext::GetCaretPlainPos() const
{
    return _caretPlainPos;
}

void InputHintContext::SetSimilarChunks(std::string text)
{
    _similarChunksText = std::move(text);
}

const std::string& InputHintContext::GetSimilarChunks() const
{
    return _similarChunksText;
}

void InputHintContext::SetHistorySimilarChunks(std::string text)
{
    _historySimilarChunksText = std::move(text);
}

const std::string& InputHintContext::GetHistorySimilarChunks() const
{
    return _historySimilarChunksText;
}

