#pragma once
#include <string>
#include <vector>

namespace Utils
{

// ─── InputContent: 输入内容的结构化表示 ───
// 将一个 CChatInput 的完整内容(JSON 数组格式, 含 tag)拆解为:
//   - plainContent : 提取后的纯文本(tag 以 [tagText] 形式嵌入)
//   - tagSegments  : 标记 plainContent 中哪些字符区间属于 tag 文本

/// 记录 plainContent 中一个 tag 所占的字符区间及其在原始内容中的字串
struct InputContentTagSegment
{
    size_t       startPos = 0;  // plainContent 中的起始索引(指向 '[')
    size_t       endPos   = 0;  // plainContent 中的结束索引(指向 ']' 之后, 即 exclusive)
    std::string  rawText;       // 该 tag 在 fullContent 中的原始 JSON 字串(utf8)
    std::wstring tagText;       // tag 的显示文本(不含 [ ])
};

/// 输入内容的结构化视图
struct InputContent
{
    std::wstring                        plainContent;  // 提取后的纯文本
    std::vector<InputContentTagSegment> tagSegments;   // plainContent 中 tag 占据的区间列表
};

struct DiffedInputContent:public InputContent
{
	std::vector<char> diffStates;//记录plainContent里每个字符的状态. 0:No change, 1: Add, 2: Delete
};

// 从 CChatInput 的原始内容(fullContent)构建 InputContent(含 tag 区间信息)
InputContent BuildInputContent(const std::wstring& fullContent);

// 将 inputContent.plainContent 中的 oldContent 替换为 newContent。
// 替换后与原字串做 diff, 尝试将原有 tagSegment 的区间映射到新字串:
//   - 若某个 tag 区间无法在新字串中完整、连续地映射(被删除或被打断), 返回 false 且不修改。
//   - 否则更新 plainContent 及各 tagSegment 的新位置, 返回 true。
bool ReplaceInputContent(InputContent& inputContent, const std::wstring& oldContent, const std::wstring& newContent);

void DiffInputContent(const InputContent& oldContent, const InputContent& newContent, DiffedInputContent& oldResult, DiffedInputContent& newResult);

} // namespace Utils
