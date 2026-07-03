#include "stdh.h"
#include "Utils_InputComplete.h"
#include "../Common/codediff/dmp_diff.h"

// 外部函数声明
extern std::string widechar_to_utf8(const wchar_t* str);
extern std::wstring utf8_to_widechar(const char* utf8_str);
extern std::wstring utf8_to_widechar(const std::string& utf8_str);

namespace Utils
{

// ─── DiffInputContent 内部辅助 ────────────────────────────────────────────────
namespace
{
    // 一个 Token: 普通字符 或 一个完整 tag(整体当作一个字符参与比较)
    struct InputToken
    {
        bool         isTag  = false;
        wchar_t      ch     = 0;   // 普通字符时有效
        std::wstring tagText;      // tag 时有效(用于比较)
        size_t       offset = 0;   // 在 plainContent 中的起始字符索引
        size_t       length = 0;   // 在 plainContent 中占据的字符数(普通字符为 1)

        bool operator==(const InputToken& o) const
        {
            if (isTag != o.isTag) return false;
            if (isTag) return tagText == o.tagText;
            return ch == o.ch;
        }
        bool operator!=(const InputToken& o) const { return !(*this == o); }
    };

    // 将 InputContent 的 plainContent 拆解为 Token 序列(tag 作为单个 Token)
    void BuildInputTokens(const InputContent& c, std::vector<InputToken>& tokens)
    {
        tokens.clear();
        const std::wstring& s = c.plainContent;
        size_t i = 0;
        while (i < s.size())
        {
            // 查找是否有 tag 恰好从 i 处开始
            const InputContentTagSegment* seg = nullptr;
            for (const auto& tag : c.tagSegments)
            {
                if (tag.startPos == i) { seg = &tag; break; }
            }

            InputToken t;
            if (seg)
            {
                t.isTag   = true;
                t.tagText = seg->tagText;
                t.offset  = seg->startPos;
                t.length  = seg->endPos - seg->startPos;
                i = seg->endPos;
            }
            else
            {
                t.isTag  = false;
                t.ch     = s[i];
                t.offset = i;
                t.length = 1;
                ++i;
            }
            tokens.push_back(t);
        }
    }

    // plainContent 中一行的字符区间 [start, end)(end 包含行末换行符)
    struct LineRange { size_t start; size_t end; };

    // 按行拆分(每行含行末换行符)
    void SplitPlainLines(const std::wstring& s, std::vector<LineRange>& lines)
    {
        lines.clear();
        size_t start = 0;
        for (size_t i = 0; i < s.size(); ++i)
        {
            wchar_t c = s[i];
            if (c == L'\r')
            {
                if (i + 1 < s.size() && s[i + 1] == L'\n') ++i;
                lines.push_back({ start, i + 1 });
                start = i + 1;
            }
            else if (c == L'\n')
            {
                lines.push_back({ start, i + 1 });
                start = i + 1;
            }
        }
        if (start < s.size())
            lines.push_back({ start, s.size() });
    }

    // 精确比较两行(Level 3: 完全相等)
    bool LinesExactEqual(const std::wstring& s1, const LineRange& r1,
                         const std::wstring& s2, const LineRange& r2)
    {
        size_t len1 = r1.end - r1.start;
        size_t len2 = r2.end - r2.start;
        if (len1 != len2) return false;
        return s1.compare(r1.start, len1, s2, r2.start, len2) == 0;
    }
}

// ─── BuildInputContent ────────────────────────────────────────────────────────
// 从 CChatInput 的完整 JSON 内容中解析出 InputContent，同时计算每个 tag 在
// plainContent 中的字符区间。
// NOTE: 此函数与 ExtractPlainTextUtf8()（见 chatinputtag.cpp）的解析逻辑必须保持一致——
// 尤其是 text/tag 的 plainContent 拼接方式（tag 文本选取 data vs text、包裹 [ ] 等）。
// 修改此处时请同步检查另一端。
InputContent BuildInputContent(const std::wstring& fullContent)
{
    InputContent result;

    if (fullContent.empty())
        return result;

    try
    {
        std::string utf8Content = widechar_to_utf8(fullContent.c_str());
        nlohmann::json contentArray = nlohmann::json::parse(utf8Content);

        if (!contentArray.is_array())
        {
            result.plainContent = fullContent;
            return result;
        }

        for (const auto& item : contentArray)
        {
            if (!item.is_object() || !item.contains("type"))
                continue;

            std::string type = item["type"];
            if (type == "text" && item.contains("content"))
            {
                result.plainContent += utf8_to_widechar(item["content"].get<std::string>());
            }
            else if (type == "tag")
            {
                // 确定 tag 展示文本: file 类优先用 data，否则用 text
                std::string tagTextUtf8;
                if (item.contains("tagType") && item["tagType"] == "file" && item.contains("data"))
                    tagTextUtf8 = item["data"].get<std::string>();
                else if (item.contains("text"))
                    tagTextUtf8 = item["text"].get<std::string>();

                if (tagTextUtf8.empty())
                    continue;

                std::wstring wTagText = utf8_to_widechar(tagTextUtf8);
                std::wstring tagDisplay = L"[" + wTagText + L"]";

                InputContentTagSegment seg;
                seg.startPos = result.plainContent.size();
                result.plainContent += tagDisplay;
                seg.endPos = result.plainContent.size();
                seg.rawText = item.dump();
                seg.tagText = wTagText;

                result.tagSegments.push_back(seg);
            }
        }
    }
    catch (const nlohmann::json::exception&)
    {
        result.plainContent = fullContent;
    }

    return result;
}

// ─── ReplaceInputContent ──────────────────────────────────────────────────────
bool ReplaceInputContent(InputContent& inputContent, const std::wstring& oldContent, const std::wstring& newContent)
{
    if (oldContent.empty())
        return false;

    size_t pos = inputContent.plainContent.find(oldContent);
    if (pos == std::wstring::npos)
        return false;

    size_t posEnd = pos + oldContent.size();

    // 检查替换区间是否与任何 tag segment 重叠
    for (const auto& seg : inputContent.tagSegments)
    {
        if (pos < seg.endPos && posEnd > seg.startPos)
            return false;  // 替换侵入 tag 区域
    }

    // 执行替换
    inputContent.plainContent.replace(pos, oldContent.size(), newContent);

    // 调整 tagSegments 偏移
    int delta = (int)newContent.size() - (int)oldContent.size();
    if (delta != 0)
    {
        for (auto& seg : inputContent.tagSegments)
        {
            if (seg.startPos >= posEnd)
            {
                seg.startPos += delta;
                seg.endPos   += delta;
            }
        }
    }

    return true;
}

// ─── DiffInputContent ─────────────────────────────────────────────────────────
void DiffInputContent(const InputContent& oldContent, const InputContent& newContent,
                      DiffedInputContent& oldResult, DiffedInputContent& newResult)
{
    const std::wstring& oldPlain = oldContent.plainContent;
    const std::wstring& newPlain = newContent.plainContent;

    // ── 1. 行级拆分 ──
    std::vector<LineRange> oldLines, newLines;
    SplitPlainLines(oldPlain, oldLines);
    SplitPlainLines(newPlain, newLines);

    // ── 2. 头尾去除完全相等(Level 3)的行 ──
    size_t headCount = 0;
    while (headCount < oldLines.size() && headCount < newLines.size() &&
           LinesExactEqual(oldPlain, oldLines[headCount], newPlain, newLines[headCount]))
    {
        ++headCount;
    }

    size_t tailCount = 0;
    while (tailCount < oldLines.size() - headCount &&
           tailCount < newLines.size() - headCount &&
           LinesExactEqual(oldPlain, oldLines[oldLines.size() - 1 - tailCount],
                           newPlain, newLines[newLines.size() - 1 - tailCount]))
    {
        ++tailCount;
    }

    // ── 3. 计算中间变化区块的字符区间 ──
    // old 中间区块 [oldMidStart, oldMidEnd)
    size_t oldMidStart = (headCount < oldLines.size()) ? oldLines[headCount].start : oldPlain.size();
    size_t oldMidEnd   = (tailCount > 0) ? oldLines[oldLines.size() - tailCount].start : oldPlain.size();
    if (oldMidEnd < oldMidStart) oldMidEnd = oldMidStart;

    // new 中间区块 [newMidStart, newMidEnd)
    size_t newMidStart = (headCount < newLines.size()) ? newLines[headCount].start : newPlain.size();
    size_t newMidEnd   = (tailCount > 0) ? newLines[newLines.size() - tailCount].start : newPlain.size();
    if (newMidEnd < newMidStart) newMidEnd = newMidStart;

    // ── 4. 构建 Token 序列, 并截取落在中间区块内的 Token ──
    std::vector<InputToken> oldTokensAll, newTokensAll;
    BuildInputTokens(oldContent, oldTokensAll);
    BuildInputTokens(newContent, newTokensAll);

    std::vector<InputToken> oldMid, newMid;
    for (const auto& t : oldTokensAll)
        if (t.offset >= oldMidStart && t.offset < oldMidEnd) oldMid.push_back(t);
    for (const auto& t : newTokensAll)
        if (t.offset >= newMidStart && t.offset < newMidEnd) newMid.push_back(t);

    // ── 5. 准备 oldResult ──
    oldResult.plainContent = oldContent.plainContent;
    oldResult.tagSegments  = oldContent.tagSegments;
    oldResult.diffStates.assign(oldPlain.size(), 0);

    // ── 6. 准备 newResult(仅包含中间变化区块) ──
    newResult.plainContent = newPlain.substr(newMidStart, newMidEnd - newMidStart);
    newResult.diffStates.assign(newResult.plainContent.size(), 0);
    newResult.tagSegments.clear();
    for (const auto& tag : newContent.tagSegments)
    {
        if (tag.startPos >= newMidStart && tag.endPos <= newMidEnd)
        {
            InputContentTagSegment seg = tag;
            seg.startPos -= newMidStart;
            seg.endPos   -= newMidStart;
            newResult.tagSegments.push_back(seg);
        }
    }

    // ── 7. 对中间区块做 Token 级 Myers 比较, 标记 Add / Delete ──
    typedef std::vector<InputToken> TokenSeq;
    MyersDiff<TokenSeq> differ(oldMid, newMid);

    for (const auto& d : differ.diffs())
    {
        for (auto it = d.text.from; it != d.text.till; ++it)
        {
            const InputToken& tok = *it;
            if (d.operation == DiffOp_Delete)
            {
                // 映射回 oldResult(位置为 old 原始 plainContent 索引)
                for (size_t k = 0; k < tok.length; ++k)
                    if (tok.offset + k < oldResult.diffStates.size())
                        oldResult.diffStates[tok.offset + k] = 2;
            }
            else if (d.operation == DiffOp_Insert)
            {
                // 映射回 newResult(位置需减去 newMidStart 偏移)
                size_t base = tok.offset - newMidStart;
                for (size_t k = 0; k < tok.length; ++k)
                    if (base + k < newResult.diffStates.size())
                        newResult.diffStates[base + k] = 1;
            }
        }
    }
}

} // namespace Utils
