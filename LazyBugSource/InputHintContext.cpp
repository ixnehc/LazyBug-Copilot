#include "stdh.h"
#include "InputHintContext.h"
#include "ChatOpsCtrl.h"
#include "Utils_InputHint.h"
#include "Utils_File.h"
#include "SolutionDBAPI.h"

#include <cstring>
#include <unordered_set>
#include <algorithm>
#include <cstdio>



InputHintContext::~InputHintContext()
{
    _StopQueryThread();
}


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
    _StopQueryThread();

    _solutionDBFolder.clear();
    _embeddingDBVersion.store(0, std::memory_order_relaxed);
    _lastEmbeddingRequestSuccess.store(true, std::memory_order_relaxed);

    _chatOpsContentVersion = 0;
    _chatOpsContent.clear();
    _caretLine.clear();
    _beforeCaretLines.clear();
    _afterCaretLines.clear();
    _caretTokenPos = -1;
    _caretPlainPos = -1;
    _inputChunks.clear();
    _inputChunksVersion = 0;
    _historyChunks.clear();
    _historyChunksVersion = 0;
}

void InputHintContext::Init(const std::string& solutionDBFolder)
{
    _solutionDBFolder = solutionDBFolder;
    _StartQueryThread();
}


void InputHintContext::_StartQueryThread()
{
    if (_queryThreadRunning.load())
        return;

    _queryThreadRunning.store(true);
    _queryThread = std::thread(&InputHintContext::_QueryThreadProc, this);
}

void InputHintContext::_StopQueryThread()
{
    if (!_queryThreadRunning.load())
        return;

    _queryThreadRunning.store(false);
    _queryCv.notify_all();

    if (_queryThread.joinable())
        _queryThread.join();
}

void InputHintContext::_QueryThreadProc()
{
    while (_queryThreadRunning.load())
    {
        if (!_solutionDBFolder.empty())
        {
            SolutionDBMsg_EmbeddingDBVersion result = SolutionDB_GetEmbeddingDBVersion(_solutionDBFolder.c_str());
            if (result.success)
                _embeddingDBVersion.store(result.version, std::memory_order_relaxed);

            SolutionDBMsg_LastEmbeddingRequestSuccess successResult = SolutionDB_GetLastEmbeddingRequestSuccess(_solutionDBFolder.c_str());
            if (successResult.success)
                _lastEmbeddingRequestSuccess.store(successResult.lastRequestSuccess, std::memory_order_relaxed);
        }

        std::unique_lock<std::mutex> lock(_queryCvMutex);
        _queryCv.wait_for(lock, std::chrono::milliseconds(500), [this]() {
            return !_queryThreadRunning.load();
        });
    }
}

const std::string& InputHintContext::GetChatOpsContent() const
{
    return _chatOpsContent;
}

uint32_t InputHintContext::GetChatOpsContentVersion() const
{
    return _chatOpsContentVersion;
}

const std::string& InputHintContext::GetSolutionDBFolder() const
{
    return _solutionDBFolder;
}

uint64_t InputHintContext::GetEmbeddingDBVersion() const
{
    return _embeddingDBVersion.load(std::memory_order_relaxed);
}

bool InputHintContext::GetLastEmbeddingRequestSuccess() const
{
    return _lastEmbeddingRequestSuccess.load(std::memory_order_relaxed);
}

uint64_t InputHintContext::GetInputChunksVersion() const
{
    return _inputChunksVersion;
}

uint64_t InputHintContext::GetHistoryChunksVersion() const
{
    return _historyChunksVersion;
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

void InputHintContext::SetInputSimilarChunks(std::vector<EmbeddingSimilarChunk> chunks, uint64_t embeddingDBVersion)
{
    _inputChunks = std::move(chunks);
    _inputChunksVersion = embeddingDBVersion;
}

void InputHintContext::SetHistorySimilarChunks(std::vector<EmbeddingSimilarChunk> chunks, uint64_t embeddingDBVersion)
{
    _historyChunks = std::move(chunks);
    _historyChunksVersion = embeddingDBVersion;
}

std::string InputHintContext::GetMergedSimilarChunksText() const
{
    // 合并两个来源的 chunks
    std::vector<EmbeddingSimilarChunk> all;
    all.reserve(_inputChunks.size() + _historyChunks.size());
    for (const auto& c : _inputChunks)
        all.push_back(c);
    for (const auto& c : _historyChunks)
        all.push_back(c);

    if (all.empty())
        return std::string();

    // 按 (filePath, range.first, range.second) 排序后去重，保留最高 similarity
    std::sort(all.begin(), all.end(), [](const auto& a, const auto& b) {
        if (a.filePath != b.filePath) return a.filePath < b.filePath;
        if (a.range.first != b.range.first) return a.range.first < b.range.first;
        if (a.range.second != b.range.second) return a.range.second < b.range.second;
        return a.similarity > b.similarity;
    });

    std::vector<EmbeddingSimilarChunk> deduped;
    for (const auto& c : all)
    {
        if (!deduped.empty())
        {
            const auto& last = deduped.back();
            if (last.filePath == c.filePath && last.range == c.range)
                continue;
        }
        deduped.push_back(c);
    }

    // 按 similarity 降序排序
    std::sort(deduped.begin(), deduped.end(), [](const auto& a, const auto& b) {
        return a.similarity > b.similarity;
    });

    // 逐 chunk 校验文件时间并拼接，按行数限制总量
    const int kMaxTotalLines = 200;
    int remainingLines = kMaxTotalLines;
    std::string similarText;

    for (const auto& c : deduped)
    {
        if (remainingLines <= 0)
            break;

        // 检查文件是否在 embedding 生成后被修改过
        time_t curFileTime = Utils::GetFileTimeT(c.filePath.c_str());
        if (curFileTime != c.genTime)
            continue;

        // range 是 [startLine, endLine)，GetFilePartIntoUTF8 接受闭区间 [start, end]
        int startLine = c.range.first;
        int endLine = c.range.second - 1;
        if (endLine < startLine)
            endLine = startLine;

        int chunkLines = endLine - startLine + 1;
        if (chunkLines > remainingLines)
        {
            endLine = startLine + remainingLines - 1;
            chunkLines = remainingLines;
        }

        Utils::FileContentCodingFormat codingFmt;
        int totalLineCount = 0;
        std::string chunkContent;
        if (!Utils::GetFilePartIntoUTF8(c.filePath.c_str(), startLine, endLine, chunkContent, codingFmt, totalLineCount))
            continue;

        remainingLines -= chunkLines;

        char header[512];
        std::snprintf(header, sizeof(header),
            "[File: %s L%d-L%d similarity: %.2f]\n",
            c.filePath.c_str(), startLine, endLine + 1, c.similarity);

        similarText += header;
        similarText += chunkContent;
        similarText += "\n\n";
    }

    return similarText;
}

