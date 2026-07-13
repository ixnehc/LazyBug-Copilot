#include "stdh.h"
#include "Utils_InputHint.h"
#include "../Common/codediff/dmp_diff.h"
#include "LlmChat.h"
#include "LlmLib.h"
#include "utils_file.h"
#include <fstream>

extern CLlmLib g_llmLib;
extern const char* GetOpenedDBFolderPath_utf8();

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

// ─── BuildFullContent ─────────────────────────────────────────────────────────
// BuildInputContent 的逆操作：将 InputContent 重建为 CChatInput 的完整 JSON 数组字串
std::wstring BuildFullContent(const InputContent& inputContent)
{
    nlohmann::json result = nlohmann::json::array();

    const std::wstring& s = inputContent.plainContent;
    size_t pos = 0;

    for (const auto& seg : inputContent.tagSegments)
    {
        // tag 之前的普通文本
        if (pos < seg.startPos)
        {
            std::wstring textPart = s.substr(pos, seg.startPos - pos);
            if (!textPart.empty())
            {
                nlohmann::json item;
                item["type"] = "text";
                item["content"] = widechar_to_utf8(textPart.c_str());
                result.push_back(item);
            }
        }

        // tag 本身: 恢复 rawText
        try
        {
            nlohmann::json tagItem = nlohmann::json::parse(seg.rawText);
            result.push_back(tagItem);
        }
        catch (...)
        {
            // 如果 rawText 解析失败，回退为 text 类型
            nlohmann::json item;
            item["type"] = "text";
            item["content"] = widechar_to_utf8((L"[" + seg.tagText + L"]").c_str());
            result.push_back(item);
        }

        pos = seg.endPos;
    }

    // 最后一个 tag 之后的剩余文本
    if (pos < s.size())
    {
        std::wstring textPart = s.substr(pos);
        if (!textPart.empty())
        {
            nlohmann::json item;
            item["type"] = "text";
            item["content"] = widechar_to_utf8(textPart.c_str());
            result.push_back(item);
        }
    }

    // 如果没有 tag 也没有内容, 返回空数组
    std::string utf8Result = result.dump();
    return utf8_to_widechar(utf8Result);
}

// ─── CalcApplyCaretPos ─────────────────────────────────────────────────────────
int CalcApplyCaretPos(const std::wstring& oldPlain, const std::wstring& newPlain,
                      const InputContent& newInputContent, int caretPlainPos)
{
    // 无效输入检查
    if (newPlain.empty())
        return -1;

    // 对 old/new plain 做字符级 diff，构建 old->new 位置映射并记录最后一个插入位置
    MyersDiff<std::wstring> differ(oldPlain, newPlain);

    std::vector<int> oldToNew(oldPlain.size(), -1);
    size_t oldIdx = 0;
    size_t newIdx = 0;
    size_t lastInsertEnd = 0;  // 最后一个 Insert 段结束位置的下一个字符位置

    for (const auto& d : differ.diffs())
    {
        size_t len = d.text.size();
        if (d.operation == DiffOp_Equal)
        {
            for (size_t k = 0; k < len && oldIdx + k < oldToNew.size(); ++k)
                oldToNew[oldIdx + k] = (int)(newIdx + k);
            oldIdx += len;
            newIdx += len;
        }
        else if (d.operation == DiffOp_Delete)
        {
            oldIdx += len;  // 旧字符被删除，无对应
        }
        else // DiffOp_Insert
        {
            lastInsertEnd = newIdx + len;  // 记录插入段结束位置
            newIdx += len;
        }
    }

    // 计算位置 A：光标前字符的新位置
    size_t posA = 0;
    if (caretPlainPos <= 0 || oldPlain.empty())
    {
        // 光标在开头或之前，A = 0
        posA = 0;
    }
    else
    {
        int targetOldPos = caretPlainPos - 1;
        if (targetOldPos >= (int)oldToNew.size())
            targetOldPos = (int)oldToNew.size() - 1;

        // 查找该字符的映射
        int mapped = oldToNew[targetOldPos];
        if (mapped >= 0)
        {
            // 映射有效，光标落在该字符之后
            posA = (size_t)mapped + 1;
        }
        else
        {
            // 该字符被删除，向前回退到最近有效映射
            int fallback = targetOldPos - 1;
            while (fallback >= 0 && oldToNew[fallback] < 0)
                --fallback;
            if (fallback >= 0)
                posA = (size_t)oldToNew[fallback] + 1;
            else
                posA = 0;  // 没有有效映射，放到开头
        }
    }

    // 计算位置 B：最后一个 Add 字符之后
    size_t posB = lastInsertEnd;

    // 取 max(A, B) 作为最终字符位置
    size_t charPos = (posA > posB) ? posA : posB;

    // 钳到 newPlain 范围内
    if (charPos > newPlain.size())
        charPos = newPlain.size();

    // 将字符位置转换为 token 位置
    // 遍历 newInputContent：普通字符 = 1 token，tag = 1 token
    int tokenPos = 0;
    size_t charIdx = 0;
    const auto& segments = newInputContent.tagSegments;
    size_t segIdx = 0;

    while (charIdx < charPos && charIdx < newPlain.size())
    {
        // 检查当前位置是否在某个 tag 区间内
        bool inTag = false;
        while (segIdx < segments.size() && segments[segIdx].startPos <= charIdx)
        {
            if (segments[segIdx].startPos <= charIdx && charIdx < segments[segIdx].endPos)
            {
                // 在 tag 内，整个 tag 算 1 token
                tokenPos++;
                charIdx = segments[segIdx].endPos;  // 跳过整个 tag
                segIdx++;
                inTag = true;
                break;
            }
            if (segments[segIdx].endPos <= charIdx)
                segIdx++;
            else
                break;
        }

        if (!inTag)
        {
            // 普通字符，1 字符 = 1 token
            tokenPos++;
            charIdx++;
        }
    }

    return tokenPos;
}

// ─── ReplaceInputContent ──────────────────────────────────────────────────────
bool ReplaceInputContent(InputContent& inputContent, const std::wstring& oldContent, const std::wstring& newContent,
                         int caretPos)
{
    if (oldContent.empty())
        return false;

    const std::wstring oldPlain = inputContent.plainContent;

    size_t pos;
    size_t lineStart = 0;
    size_t lineEnd   = oldPlain.size();

    if (caretPos >= 0)
    {
        // ── 仅在光标所在行内查找 ──
        int cp = caretPos;
        if (cp > (int)oldPlain.size())
            cp = (int)oldPlain.size();

        // 处理光标在行末的情况:
        //   cp == oldPlain.size()   → 光标在全文末尾(最后一行末尾)
        //   oldPlain[cp] == L'\n'   → 光标在换行符处(即上一行末尾)
        if (cp == (int)oldPlain.size() || oldPlain[(size_t)cp] == L'\n')
        {
            // 光标在行末: 所在行从上一个 \n 后到 cp 位置(不含)
            lineEnd = (size_t)cp;
            lineStart = 0;
            for (int i = cp - 1; i >= 0; --i)
            {
                if (oldPlain[(size_t)i] == L'\n') { lineStart = (size_t)i + 1; break; }
            }
        }
        else
        {
            // 光标在行中: 向前后搜索 \n 确定行范围
            lineStart = 0;
            for (int i = cp; i >= 0; --i)
            {
                if (oldPlain[(size_t)i] == L'\n') { lineStart = (size_t)i + 1; break; }
            }
            lineEnd = oldPlain.size();
            for (size_t i = (size_t)cp; i < oldPlain.size(); ++i)
            {
                if (oldPlain[i] == L'\n') { lineEnd = i; break; }
            }
        }

        // 在光标行内搜索 oldContent
        pos = oldPlain.find(oldContent, lineStart);
        if (pos == std::wstring::npos || pos + oldContent.size() > lineEnd)
            return false;

        // 唯一性检查: 光标行内不得有第二个匹配
        if (oldPlain.find(oldContent, pos + oldContent.size()) < lineEnd)
            return false;
    }
    else
    {
        // ── 全局查找(兼容旧行为) ──
        pos = oldPlain.find(oldContent);
        if (pos == std::wstring::npos)
            return false;

        // 多重匹配检查
        if (oldPlain.find(oldContent, pos + oldContent.size()) != std::wstring::npos)
            return false;
    }

    // 生成替换后的新字串
    std::wstring newPlain = oldPlain;
    newPlain.replace(pos, oldContent.size(), newContent);

    // 无 tag 时直接替换
    if (inputContent.tagSegments.empty())
    {
        inputContent.plainContent = newPlain;
        return true;
    }

    // 旧字串与新字串做字符级 diff, 构建 old->new 的位置映射
    // (Equal 区间内的字符 1:1 映射, Delete 的字符无对应 => -1)
    MyersDiff<std::wstring> differ(oldPlain, newPlain);

    std::vector<int> oldToNew(oldPlain.size(), -1);
    size_t oldIdx = 0;
    size_t newIdx = 0;
    for (const auto& d : differ.diffs())
    {
        size_t len = d.text.size();
        if (d.operation == DiffOp_Equal)
        {
            for (size_t k = 0; k < len && oldIdx + k < oldToNew.size(); ++k)
                oldToNew[oldIdx + k] = (int)(newIdx + k);
            oldIdx += len;
            newIdx += len;
        }
        else if (d.operation == DiffOp_Delete)
        {
            oldIdx += len;   // 旧字符被删除, 无对应
        }
        else // DiffOp_Insert
        {
            newIdx += len;
        }
    }

    // 将每个 tag segment 的区间尝试映射到新字串; 无法完整连续映射则失败
    std::vector<InputContentTagSegment> newSegments;
    newSegments.reserve(inputContent.tagSegments.size());
    for (const auto& seg : inputContent.tagSegments)
    {
        if (seg.startPos >= seg.endPos || seg.endPos > oldPlain.size())
            return false;

        int mappedStart = oldToNew[seg.startPos];
        if (mappedStart < 0)
            return false;  // 起点被删除

        // 要求整个区间的字符都保留且映射连续(中间无删除/插入)
        for (size_t p = seg.startPos; p < seg.endPos; ++p)
        {
            int mapped = oldToNew[p];
            if (mapped != mappedStart + (int)(p - seg.startPos))
                return false;  // 某字符被删除或映射不连续 => 无法完整映射
        }

        InputContentTagSegment ns = seg;
        ns.startPos = (size_t)mappedStart;
        ns.endPos   = (size_t)mappedStart + (seg.endPos - seg.startPos);
        newSegments.push_back(ns);
    }

    inputContent.plainContent = newPlain;
    inputContent.tagSegments  = std::move(newSegments);
    return true;
}

// ─── DiffInputContent ─────────────────────────────────────────────────────────
void DiffInputContent(const InputContent& oldContent, const InputContent& newContent,
                      DiffedInputContent& oldResult, DiffedInputContent& newResult, GhostContent& ghostContent)
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

    // ── 8. 计算 GhostContent ──
    ghostContent.text.clear();
    ghostContent.tokenIndex = -1;

    // 有删除时不使用 ghost text
    bool hasDeletion = false;
    for (char s : oldResult.diffStates)
    {
        if (s == 2) { hasDeletion = true; break; }
    }
    if (hasDeletion)
        return;

    // 检查 newResult 中所有 1 是否构成一个连续区间(允许 0*1+0* 模式)
    size_t firstOne = (size_t)-1, lastOne = (size_t)-1;
    bool hasAdd = false;
    bool seenZeroAfterOne = false;
    for (size_t i = 0; i < newResult.diffStates.size(); ++i)
    {
        if (newResult.diffStates[i] == 1)
        {
            if (seenZeroAfterOne)
                return;  // 0 之后又出现 1, 不连续
            if (!hasAdd) { firstOne = i; hasAdd = true; }
            lastOne = i;
        }
        else if (hasAdd && newResult.diffStates[i] == 0)
        {
            seenZeroAfterOne = true;
        }
    }
    if (!hasAdd)
        return;  // 无新增

    // 提取 ghost text
    ghostContent.text = newResult.plainContent.substr(firstOne, lastOne - firstOne + 1);

    // 计算 token index: ghost text 在完整 newContent 中的绝对字符位置
    size_t absCharPos = newMidStart + firstOne;
    int tokenIdx = 0;
    for (const auto& t : newTokensAll)
    {
        if (t.offset >= absCharPos)
            break;
        tokenIdx++;
    }
    ghostContent.tokenIndex = tokenIdx;
}

// ─── IsValidCompletion ────────────────────────────────────────────────────────
bool IsValidCompletion(const std::wstring& oldContent, const std::wstring& newContent)
{
    // 统计行数: 补全不应引入比原输入更多的行(禁止把单行输入补成多行"方案")
    auto countLines = [](const std::wstring& s) -> int
    {
        int lines = 1;
        for (wchar_t c : s)
            if (c == L'\n')
                lines++;
        return lines;
    };

    if (countLines(newContent) > countLines(oldContent))
        return false;

    // new 相对 old 的增删字符数不应过多(续写应是"少量"增补)
    // 阈值: max(输入长度, 64) —— 既容忍短输入的合理补全, 又拦住整段回答
    int deltaChars = (int)newContent.size() - (int)oldContent.size();
    if (deltaChars < 0)
        deltaChars = -deltaChars;
    int limit = (int)oldContent.size();
    if (limit < 64)
        limit = 64;
    if (deltaChars > limit)
        return false;

    return true;
}

// ─── FixDuplicationAtJoin ─────────────────────────────────────────────────────
// 将 originalPlain 中的 oldContent 替换为 newContent 后,
// newContent 结尾处可能与原串中紧接其后的剩余后缀产生重复.
// 检测 newContent 末尾与 remainingSuffix 开头的公共子串, 若长度 >= threshold,
// 则从 newContent 末尾删除该重复部分.
// 返回 true 表示已修复(newContent 已修改).
bool FixDuplicationAtJoin(const std::wstring& originalPlain, const std::wstring& oldContent,
                          std::wstring& newContent, size_t threshold)
{
    // 找到 oldContent 在原始串中的位置, 必须唯一
    size_t pos = originalPlain.find(oldContent);
    if (pos == std::wstring::npos)
        return false;
    // 多重匹配 → 无法安全定位, 放弃修复
    if (originalPlain.find(oldContent, pos + oldContent.size()) != std::wstring::npos)
        return false;

    // 旧内容之后剩余的原串后缀
    std::wstring remainingSuffix = originalPlain.substr(pos + oldContent.size());
    if (remainingSuffix.empty())
        return false;

    // 计算 newContent 后缀与 remainingSuffix 前缀的最长公共子串长度
    size_t maxLen = newContent.size();
    if (remainingSuffix.size() < maxLen)
        maxLen = remainingSuffix.size();

    size_t overlapLen = 0;
    for (size_t k = maxLen; k >= 1; --k)
    {
        if (newContent.compare(newContent.size() - k, k,
                               remainingSuffix, 0, k) == 0)
        {
            overlapLen = k;
            break;
        }
    }

    if (overlapLen < threshold)
        return false;

    // 从 newContent 末尾删除重复部分
    newContent.erase(newContent.size() - overlapLen);
    return true;
}

// ─── RunTestCases ─────────────────────────────────────────────────────────────
void RunTestCases()
{
	const char* dbPath = GetOpenedDBFolderPath_utf8();
	std::string logDir = std::string(dbPath) + "\\_log\\InputHint";
	std::wstring testDir = utf8_to_widechar(logDir) + L"\\TestCases";
	EnsureFolder(widechar_to_utf8(testDir.c_str()).c_str());

	// 收集所有 JSON 文件
	std::vector<std::wstring> jsonFiles;
	WIN32_FIND_DATAW fd;
	HANDLE hFind = FindFirstFileW((testDir + L"\\*.json").c_str(), &fd);
	if (hFind == INVALID_HANDLE_VALUE)
		return;
	do
	{
		if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			jsonFiles.push_back(testDir + L"\\" + fd.cFileName);
	} while (FindNextFileW(hFind, &fd));
	FindClose(hFind);

	if (jsonFiles.empty())
		return;

	// 按文件名排序，保证执行顺序可预测
	std::sort(jsonFiles.begin(), jsonFiles.end());

	// 获取 auto complete API
	std::string apiName = g_llmLib.GetAutoCompleteApi();
	if (apiName.empty())
		return;

	LlmSessionSetting setting;
	if (!g_llmLib.LoadLlmSetting(setting, apiName, false, "chatrule_inputhint"))
		return;

	setting.api.tools.clear();

	for (const auto& filePath : jsonFiles)
	{
		// 读取 JSON 文件
		std::ifstream ifs(filePath);
		if (!ifs.is_open())
			continue;

		nlohmann::ordered_json testJson;
		try
		{
			ifs >> testJson;
		}
		catch (const nlohmann::json::exception&)
		{
			continue;
		}
		ifs.close();

		// 跳过已有 testResult 的测试用例
		if (testJson.contains("testResult"))
			continue;

		std::string context    = testJson.value("context", "");
		std::string origContent= testJson.value("originalContent", "");

		if (origContent.empty())
			continue;

		// 重建 user message（与 CChatTask_InputHint::Start() 格式一致，约束文字由底层 rules 自动注入）
		std::string userMsg;
		if (!context.empty())
		{
			userMsg += "Recent chat context:\n";
			userMsg += context;
			userMsg += "\n\n";
		}
		userMsg += "User's partial input:\n";
		userMsg += origContent;
		userMsg += "\n\n";
		userMsg += "Completion:";

		// 构造请求
		LlmSessionRequest request;
		request.AddUserMessage(userMsg.c_str());
		request.isStreaming = true;

		CLlmChat llmChat;
		llmChat.Init();
		if (!llmChat.Request(request, setting))
		{
			llmChat.Clear();
			continue;
		}

		// 同步等待完成（泵送窗口消息以防 UI 卡死）
		LlmSessionOutput output;
		while (true)
		{
			MSG msg;
			while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}

			if (llmChat.Process(output))
			{
				if (output.isCompleted || output.hasError)
					break;
			}
			::Sleep(50);
		}
		llmChat.Clear();

		// 保存测试结果
		if (output.isCompleted && !output.hasError)
		{
			std::string rawResult = output.fullContent;
			// 去除首尾空白
			size_t start = rawResult.find_first_not_of(" \t\r\n");
			size_t end   = rawResult.find_last_not_of(" \t\r\n");
			if (start != std::string::npos && end != std::string::npos)
				rawResult = rawResult.substr(start, end - start + 1);
			else
				rawResult.clear();

			testJson["testContent"] = origContent;
			testJson["testRaw"] = rawResult;

			// 去掉光标符号，构建 InputContent
			std::string cleanContent = origContent;
			const std::string caretUtf8 = "\xE2\x80\xB8"; // ‸ U+2038
			size_t caretPos = cleanContent.find(caretUtf8);
			if (caretPos != std::string::npos)
				cleanContent.erase(caretPos, caretUtf8.size());

			std::wstring cleanW = utf8_to_widechar(cleanContent);
			InputContent inputContent = BuildInputContent(cleanW);

			// 解析 old~~||~~new
			const std::string separator = "~~||~~";
			size_t sepPos = rawResult.find(separator);
			if (sepPos != std::string::npos)
			{
				std::string oldUtf8 = rawResult.substr(0, sepPos);
				std::string newUtf8 = rawResult.substr(sepPos + separator.size());
				std::wstring oldW = utf8_to_widechar(oldUtf8);
				std::wstring newW = utf8_to_widechar(newUtf8);

				if (!IsValidCompletion(oldW, newW))
				{
					// 不合理的补全(多行/过长的"回答式"结果), 视为无补全
					testJson["testValid"] = false;
					testJson["testResult"] = widechar_to_utf8(inputContent.plainContent.c_str());
				}
				else
				{
					testJson["testValid"] = true;
					InputContent newContent = inputContent;
					if (!ReplaceInputContent(newContent, oldW, newW))
					{
						newContent.plainContent = newW;
						newContent.tagSegments.clear();
					}
					testJson["testResult"] = widechar_to_utf8(newContent.plainContent.c_str());
				}
			}
			else
			{
				testJson["testResult"] = rawResult;
			}
		}
		else
		{
			testJson["testRaw"] = nullptr;
			testJson["testResult"] = nullptr;
			if (output.hasError && !output.errorMessage.empty())
				testJson["testError"] = output.errorMessage;
		}

		std::ofstream ofs(filePath);
		if (ofs.is_open())
		{
			ofs << testJson.dump(2);
			ofs.close();
		}
	}
}

} // namespace Utils
