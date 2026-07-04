// ChatInputExample.cpp - CChatInput使用示例

#include "stdh.h"
#include "ChatInput.h"

class CChatInputExample : public CDialog
{
public:
    CChatInputExample(CWnd* pParent = nullptr);
    virtual ~CChatInputExample();

protected:
    virtual BOOL OnInitDialog() override;
    virtual void DoDataExchange(CDataExchange* pDX) override;

    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnDestroy();

    DECLARE_MESSAGE_MAP()

private:
    CChatInput m_chatInput;
    
    // 回调函数
    void OnSendMessage(const std::wstring& content);
    void OnToolButtonClicked(const std::wstring& buttonId, const std::wstring& action);
    void OnContentChanged(const std::wstring& content);
    void OnTagRemoved(const std::wstring& tagId);
};

BEGIN_MESSAGE_MAP(CChatInputExample, CDialog)
    ON_WM_SIZE()
    ON_WM_DESTROY()
END_MESSAGE_MAP()

CChatInputExample::CChatInputExample(CWnd* pParent)
    : CDialog(IDD_CHATINPUT_EXAMPLE, pParent) // 假设有这个对话框资源
{
}

CChatInputExample::~CChatInputExample()
{
}

void CChatInputExample::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BOOL CChatInputExample::OnInitDialog()
{
    CDialog::OnInitDialog();

    // 获取客户区域
    CRect rect;
    GetClientRect(&rect);
    rect.DeflateRect(10, 10); // 留一些边距

    // 创建ChatInput控件
    if (m_chatInput.Create(rect, this, 1001))
    {
        // 设置回调函数
        m_chatInput.SetSendCallback([this](const std::wstring& content) {
            OnSendMessage(content);
        });

        m_chatInput.SetToolButtonClickedCallback([this](const std::wstring& buttonId, const std::wstring& action) {
            OnToolButtonClicked(buttonId, action);
        });

        m_chatInput.SetContentChangedCallback([this](const std::wstring& content) {
            OnContentChanged(content);
        });

        m_chatInput.SetTagRemovedCallback([this](const std::wstring& tagId) {
            OnTagRemoved(tagId);
        });

        // 添加一些示例标签
        m_chatInput.AddTag(L"示例文件.txt", L"file", L"C:\\example\\file.txt");
        m_chatInput.AddTag(L"重要", L"info", L"", L"#ff6b6b", true);

        // 添加一些工具按钮
        m_chatInput.AddToolButton(L"添加文件", L"📁", L"addFile", L"选择文件添加到输入");
        m_chatInput.AddToolButton(L"插入标签", L"🏷️", L"insertTag", L"在光标位置插入标签");
        m_chatInput.AddToolButton(L"格式化", L"✨", L"format", L"格式化代码");
        m_chatInput.AddToolButton(L"清空", L"🗑️", L"clear", L"清空输入内容");

        // 设置占位符
        m_chatInput.SetPlaceholder(L"请输入您的问题或描述...");
    }

    return TRUE;
}

void CChatInputExample::OnSize(UINT nType, int cx, int cy)
{
    CDialog::OnSize(nType, cx, cy);

    if (m_chatInput.GetSafeHwnd())
    {
        CRect rect;
        GetClientRect(&rect);
        rect.DeflateRect(10, 10);
        m_chatInput.MoveWindow(&rect);
    }
}

void CChatInputExample::OnDestroy()
{
    CDialog::OnDestroy();
}

void CChatInputExample::OnSendMessage(const std::wstring& content)
{
    // 处理发送的消息
    CString message;
    message.Format(L"发送消息: %s", content.c_str());
    AfxMessageBox(message);

    // 可以在这里处理实际的消息发送逻辑
    // 例如：发送到AI服务器、保存到数据库等

    // 发送完成后可以清空输入
    m_chatInput.ClearInput();
    
    // 或者添加一个状态标签
    m_chatInput.AddTag(L"已发送", L"info", L"", L"#28a745", true);
}

void CChatInputExample::OnToolButtonClicked(const std::wstring& buttonId, const std::wstring& action)
{
    if (action == L"addFile")
    {
        // 打开文件选择对话框
        CFileDialog dlg(TRUE, NULL, NULL, 
                       OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
                       L"所有文件 (*.*)|*.*||");
        
        if (dlg.DoModal() == IDOK)
        {
            CString filePath = dlg.GetPathName();
            CString fileName = dlg.GetFileName();
            
            // 添加文件标签到标签栏
            m_chatInput.AddTag(fileName.GetString(), L"file", filePath.GetString());
            
            // 在光标位置插入内联文件标签
            m_chatInput.InsertInlineTag(fileName.GetString(), L"file", filePath.GetString(), true);
        }
    }
    else if (action == L"insertTag")
    {
        // 插入一个示例标签到光标位置
        static int tagCounter = 1;
        CString tagText;
        tagText.Format(L"标签%d", tagCounter++);
        
        m_chatInput.InsertInlineTag(tagText.GetString(), L"info", L"示例数据", true);
    }
    else if (action == L"format")
    {
        // 格式化输入内容（示例）
        m_chatInput.GetInputContent([this](const std::wstring& content) {
            if (!content.empty())
            {
                // 简单的格式化：去除多余空行
                std::wstring formatted = content;
                // 这里可以添加更复杂的格式化逻辑
                
                m_chatInput.SetInputContent(formatted);
                AfxMessageBox(L"内容已格式化");
            }
        });
    }
    else if (action == L"clear")
    {
        // 清空输入内容
        if (AfxMessageBox(L"确定要清空输入内容吗？", MB_YESNO | MB_ICONQUESTION) == IDYES)
        {
            m_chatInput.ClearInput();
            m_chatInput.ClearTags();
        }
    }

    CString message;
    message.Format(L"工具按钮点击: %s (动作: %s)", buttonId.c_str(), action.c_str());
    OutputDebugString(message);
}

void CChatInputExample::OnContentChanged(const std::wstring& content)
{
    // 根据内容长度启用/禁用发送按钮
    bool hasContent = !content.empty() && content.find_first_not_of(L" \t\r\n") != std::wstring::npos;
    m_chatInput.SetSendButtonEnabled(hasContent);

    // 可以在这里添加其他逻辑，如自动保存草稿等
    if (content.length() > 1000)
    {
        m_chatInput.AddTag(L"内容较长", L"info", L"", L"#ffc107", true);
    }
}

void CChatInputExample::OnTagRemoved(const std::wstring& tagId)
{
    CString message;
    message.Format(L"标签已移除: %s", tagId.c_str());
    OutputDebugString(message);
}

// 使用示例：
// 
// void CMainFrame::OnShowChatInput()
// {
//     CChatInputExample dlg;
//     dlg.DoModal();
// } 