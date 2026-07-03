/*
 * ChatInput原生自动补全窗口功能使用示例
 * 
 * 功能特性：
 * - 自动补全列表现在显示为独立的原生Windows窗口
 * - 不受WebView边界限制，可以显示在ChatInput窗口外部
 * - 支持鼠标点击和键盘导航
 * - 具有现代化的GitHub深色主题
 * - 自动定位到合适位置，避免超出屏幕边界
 */

#include "stdh.h"
#include "ChatInput.h"

class CNativeACExampleDialog : public CDialog
{
public:
    CNativeACExampleDialog(CWnd* pParent = nullptr) 
        : CDialog(IDD_EXAMPLE_DIALOG, pParent)
        , m_chatInput()
    {
    }

protected:
    virtual BOOL OnInitDialog() override
    {
        CDialog::OnInitDialog();

        // 创建ChatInput控件
        CRect rect(10, 10, 610, 110);
        m_chatInput.Create(rect, this, IDC_CHAT_INPUT);

        // 设置自动补全请求回调
        m_chatInput.SetAutoCompleteRequestCallback([this](const std::wstring& query) {
            OnAutoCompleteRequest(query);
        });

        // 设置发送回调
        m_chatInput.SetSendCallback([this](const std::wstring& content, const std::wstring& plainText) {
            OnSendMessage(content, plainText);
        });

        return TRUE;
    }

    void OnAutoCompleteRequest(const std::wstring& query)
    {
        // 创建候选项列表
        std::vector<ChatInputACItem> items;

        // 用户候选项
        if (query.empty() || query.find(L"user") != std::wstring::npos)
        {
            ChatInputACItem userItem1;
            userItem1.id = L"user_001";
            userItem1.text = L"张三";
            userItem1.value = L"zhangsan";
            userItem1.description = L"产品经理 - 产品部";
            userItem1.icon = L"👤";
            userItem1.type = L"user";
            userItem1.data = L"{\"userId\":\"001\",\"department\":\"产品部\"}";
            items.push_back(userItem1);

            ChatInputACItem userItem2;
            userItem2.id = L"user_002";
            userItem2.text = L"李四";
            userItem2.value = L"lisi";
            userItem2.description = L"高级开发工程师 - 研发部";
            userItem2.icon = L"👨‍💻";
            userItem2.type = L"user";
            userItem2.data = L"{\"userId\":\"002\",\"department\":\"研发部\"}";
            items.push_back(userItem2);

            ChatInputACItem userItem3;
            userItem3.id = L"user_003";
            userItem3.text = L"王五";
            userItem3.value = L"wangwu";
            userItem3.description = L"UI设计师 - 设计部";
            userItem3.icon = L"🎨";
            userItem3.type = L"user";
            userItem3.data = L"{\"userId\":\"003\",\"department\":\"设计部\"}";
            items.push_back(userItem3);
        }

        // 文件候选项
        if (query.empty() || query.find(L"file") != std::wstring::npos)
        {
            ChatInputACItem fileItem1;
            fileItem1.id = L"file_001";
            fileItem1.text = L"需求文档.docx";
            fileItem1.value = L"requirement.docx";
            fileItem1.description = L"产品需求文档 - 2024年1月";
            fileItem1.icon = L"📄";
            fileItem1.type = L"file";
            fileItem1.data = L"{\"filePath\":\"D:\\\\Documents\\\\requirement.docx\"}";
            items.push_back(fileItem1);

            ChatInputACItem fileItem2;
            fileItem2.id = L"file_002";
            fileItem2.text = L"项目计划.xlsx";
            fileItem2.value = L"project_plan.xlsx";
            fileItem2.description = L"2024年Q1项目计划表";
            fileItem2.icon = L"📊";
            fileItem2.type = L"file";
            fileItem2.data = L"{\"filePath\":\"D:\\\\Documents\\\\project_plan.xlsx\"}";
            items.push_back(fileItem2);
        }

        // 命令候选项
        if (query.empty() || query.find(L"cmd") != std::wstring::npos)
        {
            ChatInputACItem cmdItem1;
            cmdItem1.id = L"cmd_001";
            cmdItem1.text = L"/help";
            cmdItem1.value = L"/help";
            cmdItem1.description = L"显示帮助信息";
            cmdItem1.icon = L"❓";
            cmdItem1.type = L"command";
            cmdItem1.data = L"{\"command\":\"help\"}";
            items.push_back(cmdItem1);

            ChatInputACItem cmdItem2;
            cmdItem2.id = L"cmd_002";
            cmdItem2.text = L"/clear";
            cmdItem2.value = L"/clear";
            cmdItem2.description = L"清空当前对话";
            cmdItem2.icon = L"🗑️";
            cmdItem2.type = L"command";
            cmdItem2.data = L"{\"command\":\"clear\"}";
            items.push_back(cmdItem2);
        }

        // 设置候选项到自动补全列表
        m_chatInput.SetAutoCompleteItems(items);
    }

    void OnSendMessage(const std::wstring& content, const std::wstring& plainText)
    {
        // 处理发送的消息
        CString msg;
        msg.Format(_T("发送消息:\n内容: %s\n纯文本: %s"), 
            content.c_str(), plainText.c_str());
        
        AfxMessageBox(msg);
        
        // 清空输入
        m_chatInput.ClearInput();
    }

private:
    CChatInput m_chatInput;
    
    enum { IDC_CHAT_INPUT = 1001 };
    enum { IDD_EXAMPLE_DIALOG = 1000 };
};

/*
 * 使用说明：
 * 
 * 1. 原生自动补全窗口特性：
 *    - 自动补全列表现在以独立的原生Windows窗口形式显示
 *    - 不再受WebView边界限制，可以完全显示在ChatInput控件外部
 *    - 窗口具有现代化的GitHub深色主题，与WebView样式保持一致
 * 
 * 2. 自动定位功能：
 *    - 窗口会自动定位到ChatInput控件下方
 *    - 如果空间不足，会自动调整位置避免超出屏幕边界
 *    - 支持多显示器环境的正确定位
 * 
 * 3. 交互方式：
 *    - 鼠标点击：直接点击候选项进行选择
 *    - 鼠标悬停：悬停时会高亮显示当前项
 *    - 键盘导航：上下箭头键选择，Enter确认，Esc取消
 * 
 * 4. 窗口行为：
 *    - 失去焦点时自动隐藏
 *    - 支持阴影效果和边框样式
 *    - 窗口大小根据内容自动调整
 * 
 * 5. 性能优势：
 *    - 原生GDI绘制，比WebView中的HTML/CSS渲染更高效
 *    - 直接的Windows消息处理，响应更快
 *    - 内存占用更小
 * 
 * 6. 示例中的候选项类型：
 *    - 用户：👤 显示用户名和部门信息
 *    - 文件：📄📊 显示文件名和描述
 *    - 命令：❓🗑️ 显示命令和用途说明
 */ 