// ChatInputACExample.cpp - 自动补全功能使用示例
// 本文件展示如何在ChatInput中使用自动补全功能

#include "stdh.h"
#include "ChatInput.h"
#include "ChatInputACList.h"

// 示例：如何在你的窗口或对话框类中使用ChatInput的自动补全功能
class CExampleDialog : public CDialog
{
public:
    CExampleDialog() : m_chatInput() {}

protected:
    CChatInput m_chatInput;
    std::vector<ChatInputACItem> m_autoCompleteItems;

    void InitializeChatInput()
    {
        // 创建ChatInput控件
        CRect rect(10, 10, 400, 200);
        m_chatInput.Create(rect, this, 1001);

        // 设置自动补全请求回调
        m_chatInput.SetAutoCompleteRequestCallback([this](const std::wstring& query) {
            OnAutoCompleteRequest(query);
        });

        // 准备自动补全数据
        PrepareAutoCompleteData();
    }

    void PrepareAutoCompleteData()
    {
        // 示例：添加一些用户、文件、命令等候选项
        m_autoCompleteItems.clear();

        // 用户候选项
        ChatInputACItem userItem1;
        userItem1.id = L"user_001";
        userItem1.text = L"张三";
        userItem1.value = L"zhangsan";
        userItem1.description = L"产品经理";
        userItem1.icon = L"👤";
        userItem1.type = L"user";
        userItem1.data = L"{\"userId\":\"001\",\"department\":\"产品部\"}";
        m_autoCompleteItems.push_back(userItem1);

        ChatInputACItem userItem2;
        userItem2.id = L"user_002";
        userItem2.text = L"李四";
        userItem2.value = L"lisi";
        userItem2.description = L"开发工程师";
        userItem2.icon = L"👨‍💻";
        userItem2.type = L"user";
        userItem2.data = L"{\"userId\":\"002\",\"department\":\"研发部\"}";
        m_autoCompleteItems.push_back(userItem2);

        // 文件候选项
        ChatInputACItem fileItem1;
        fileItem1.id = L"file_001";
        fileItem1.text = L"项目设计文档.docx";
        fileItem1.value = L"design_doc";
        fileItem1.description = L"最后修改: 2024-01-15";
        fileItem1.icon = L"📄";
        fileItem1.type = L"file";
        fileItem1.data = L"{\"filePath\":\"D:\\\\Documents\\\\项目设计文档.docx\"}";
        m_autoCompleteItems.push_back(fileItem1);

        // 命令候选项
        ChatInputACItem cmdItem1;
        cmdItem1.id = L"cmd_001";
        cmdItem1.text = L"help";
        cmdItem1.value = L"help";
        cmdItem1.description = L"显示帮助信息";
        cmdItem1.icon = L"❓";
        cmdItem1.type = L"command";
        cmdItem1.data = L"{\"command\":\"help\"}";
        m_autoCompleteItems.push_back(cmdItem1);

        ChatInputACItem cmdItem2;
        cmdItem2.id = L"cmd_002";
        cmdItem2.text = L"status";
        cmdItem2.value = L"status";
        cmdItem2.description = L"查看当前状态";
        cmdItem2.icon = L"📊";
        cmdItem2.type = L"command";
        cmdItem2.data = L"{\"command\":\"status\"}";
        m_autoCompleteItems.push_back(cmdItem2);
    }

    void OnAutoCompleteRequest(const std::wstring& query)
    {
        // 当用户输入@后，系统会调用这个回调函数
        // query参数包含@符号后的查询字符串

        TRACE(L"自动补全请求: %s\n", query.c_str());

        // 根据查询字符串过滤候选项
        std::vector<ChatInputACItem> filteredItems;
        
        for (const auto& item : m_autoCompleteItems)
        {
            // 简单的匹配逻辑：检查文本或值是否包含查询字符串
            std::wstring lowerQuery = query;
            std::wstring lowerText = item.text;
            std::wstring lowerValue = item.value;
            
            std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::towlower);
            std::transform(lowerText.begin(), lowerText.end(), lowerText.begin(), ::towlower);
            std::transform(lowerValue.begin(), lowerValue.end(), lowerValue.begin(), ::towlower);
            
            if (lowerText.find(lowerQuery) != std::wstring::npos || 
                lowerValue.find(lowerQuery) != std::wstring::npos)
            {
                filteredItems.push_back(item);
            }
        }

        // 将过滤后的候选项发送给自动补全列表
        m_chatInput.SetAutoCompleteItems(filteredItems);
    }

    // 可选：处理发送消息事件
    void OnChatInputSend(const std::wstring& contentJson, const std::wstring& plainText)
    {
        TRACE(L"发送消息 - 纯文本: %s\n", plainText.c_str());
        TRACE(L"发送消息 - 完整内容: %s\n", contentJson.c_str());

        // 解析消息中的标签
        std::vector<ChatInputTag> tags;
        m_chatInput.ParseInlineTags(contentJson, tags);
        
        for (const auto& tag : tags)
        {
            TRACE(L"消息包含标签: %s (类型: %s, 数据: %s)\n", 
                  tag.text.c_str(), tag.type.c_str(), tag.data.c_str());
        }

        // 清空输入框
        m_chatInput.ClearInput();
    }
};

/* 
使用说明：

1. 基本设置：
   - 创建CChatInput实例
   - 调用Create()创建控件
   - 设置SetAutoCompleteRequestCallback()回调函数

2. 准备候选数据：
   - 创建ChatInputACItem数组
   - 设置每个项目的id、text、value、description、icon、type、data等属性
   - type可以是"user"、"file"、"command"等，用于区分不同类型的候选项

3. 处理自动补全请求：
   - 在回调函数中根据query参数过滤候选项
   - 调用SetAutoCompleteItems()更新候选列表

4. 用户交互：
   - 用户输入@符号后，自动补全列表会显示
   - 用户可以通过键盘上下箭头选择，回车确认
   - 选中的项目会作为InlineTag插入到输入框中

5. 配置选项：
   - SetAutoCompleteEnabled()：启用/禁用自动补全
   - GetAutoCompleteList()->SetMaxVisibleItems()：设置最大显示项数
   - GetAutoCompleteList()->SetListWidth()：设置列表宽度

6. 高级功能：
   - 可以通过ParseInlineTags()解析消息中的标签
   - 标签的data字段可以存储JSON格式的额外信息
   - 支持自定义图标和样式类型
*/ 