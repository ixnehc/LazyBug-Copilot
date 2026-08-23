#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "CoreDefines.h"

class CChatOpsCtrl;


// InputHint 使用的实时上下文。
//
// 该结构只保存当前状态，不保存单条 ChatOp，也不保存 symbol、关键字等派生信息。
// 聊天上下文在 CChatOpsCtrl 版本变化时由 UpdateFromOps 全量重建；
// 输入内容在输入框变化时由 UpdateInput 更新。
struct InputHintContext
{
public:
    // 根据当前 ops 全量更新聊天上下文。
    // 如果 ops 版本未变化，实现时可直接跳过重建。
    void UpdateFromOps(const CChatOpsCtrl& opsCtrl);

    // 更新当前输入内容及输入控件提供的 token 光标位置。
    // 实现时将输入按光标位置拆分为光标行、光标前行、光标后行。
    void UpdateInput(const std::wstring& fullContent, int caretTokenPos);

    // 清空上下文中保存的所有数据。
    void Clear();

    // 设置 embedding 相似代码片段查询结果。
    void SetSimilarChunks(std::vector<EmbeddingSimilarChunk> chunks);

    const std::string& GetChatOpsContent() const;
    uint32_t GetChatOpsContentVersion() const;

    const std::wstring& GetCaretLine() const;
    const std::wstring& GetBeforeCaretLines() const;
    const std::wstring& GetAfterCaretLines() const;

    int GetCaretTokenPos() const;
    int GetCaretPlainPos() const;

    const std::vector<EmbeddingSimilarChunk>& GetSimilarChunks() const;


private:
    // 最近一次全量生成 _chatOpsContent 所对应的 CChatOpsCtrl 版本。
    uint32_t _chatOpsContentVersion = 0;

    // 从当前有效聊天记录中提取出的聊天操作内容。
    std::string _chatOpsContent;

    // 当前输入框内容按光标拆分后的三部分：
    //   _caretLine        光标所在行（含光标标记 \x2038）
    //   _beforeCaretLines 光标行之前的所有行（用 '\n' 连接，可能为空）
    //   _afterCaretLines  光标行之后的所有行（用 '\n' 连接，可能为空）
    std::wstring _caretLine;
    std::wstring _beforeCaretLines;
    std::wstring _afterCaretLines;

    // 输入控件坐标系中的光标位置，以及 plainContent 坐标系中的光标位置。
    int _caretTokenPos = -1;
    int _caretPlainPos = -1;

    // embedding 相似代码片段查询结果（由 CChatTask_EmbeddingQuery 写入）。
    std::vector<EmbeddingSimilarChunk> _similarChunks;


};
