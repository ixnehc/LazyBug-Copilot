// TitlebarTest.cpp - WebView 标题栏功能使用示例

#include "stdh.h" 
#include "WebViewControl.h"

// 使用示例类
class CTitlebarTestDlg : public CDialogEx
{
public:
    CTitlebarTestDlg(CWnd* pParent = nullptr) : CDialogEx(IDD_DIALOG, pParent) {}

    virtual BOOL OnInitDialog() override
    {
        CDialogEx::OnInitDialog();
        
        // 创建WebView控件
        CRect rect(10, 10, 800, 600);
        m_webViewControl.Create(rect, this, IDC_WEBVIEW);
        
        // 设置Web消息回调，处理来自JavaScript的消息
        m_webViewControl.SetWebMessageReceivedCallback([this](const std::wstring& message) {
            OnWebMessageReceived(message);
        });
        
        // 设置导航完成回调，在页面加载完成后设置标题栏
        m_webViewControl.SetNavigationCompletedCallback([this](bool success) {
            if (success) {
                SetupTitlebar();
            }
        });
        
        return TRUE;
    }

private:
    CWebViewControl m_webViewControl;
    std::wstring m_saveButtonId;
    std::wstring m_clearButtonId;
    
    // 设置标题栏
    void SetupTitlebar()
    {
        // 设置标题栏标题
        m_webViewControl.SetWebViewTitle(L"AI 聊天助手 - LazyBug");
        
        // 添加标题栏按钮
        m_saveButtonId = m_webViewControl.AddTitlebarButton(L"保存", L"save_chat");
        m_clearButtonId = m_webViewControl.AddTitlebarButton(L"清空", L"clear_chat");
        std::wstring settingsButtonId = m_webViewControl.AddTitlebarButton(L"设置", L"open_settings");
        
        // 添加一些测试消息
        m_webViewControl.AddSystemMessage(L"欢迎使用 AI 聊天助手！");
        m_webViewControl.AddUserMessage(L"你好！");
        
        std::wstring aiMsgId = m_webViewControl.StartStreamingAIMessage();
        m_webViewControl.AddStreamingAIMessage(aiMsgId, L"您好！我是您的AI助手。");
        m_webViewControl.AddStreamingAIMessage(aiMsgId, L"我可以帮助您解答问题和完成各种任务。");
        m_webViewControl.CompleteStreamingAIMessage(aiMsgId);
        
        // 添加一个FileEdit示例
        std::wstring fileEditId = m_webViewControl.AddFileEditToAIMessage(aiMsgId, L"示例代码.cpp", 
            L"#include <iostream>\n\nint main() {\n    std::cout << \"Hello, World!\" << std::endl;\n    return 0;\n}", 200);
        m_webViewControl.AddFileEditButton(fileEditId, L"编译", L"compile_code");
        m_webViewControl.AddFileEditButton(fileEditId, L"运行", L"run_code");
    }
    
    // 处理来自WebView的消息
    void OnWebMessageReceived(const std::wstring& message)
    {
        // 这里应该解析JSON消息，为简化起见直接打印
        OutputDebugStringW((L"收到WebView消息: " + message + L"\n").c_str());
        
        // 简单的消息处理示例（实际应该用JSON解析器）
        if (message.find(L"titlebarClicked") != std::wstring::npos) {
            AfxMessageBox(L"标题栏被点击了！");
        }
        else if (message.find(L"titlebarButtonClicked") != std::wstring::npos) {
            if (message.find(L"save_chat") != std::wstring::npos) {
                OnSaveChat();
            }
            else if (message.find(L"clear_chat") != std::wstring::npos) {
                OnClearChat();
            }
            else if (message.find(L"open_settings") != std::wstring::npos) {
                OnOpenSettings();
            }
        }
        else if (message.find(L"fileEditButtonClicked") != std::wstring::npos) {
            if (message.find(L"compile_code") != std::wstring::npos) {
                AfxMessageBox(L"编译代码功能！");
            }
            else if (message.find(L"run_code") != std::wstring::npos) {
                AfxMessageBox(L"运行代码功能！");
            }
        }
    }
    
    // 保存聊天记录
    void OnSaveChat()
    {
        AfxMessageBox(L"保存聊天记录功能！");
        // 这里可以实现保存聊天记录的逻辑
    }
    
    // 清空聊天
    void OnClearChat()
    {
        if (AfxMessageBox(L"确定要清空所有聊天记录吗？", MB_YESNO) == IDYES) {
            m_webViewControl.ClearChat();
        }
    }
    
    // 打开设置
    void OnOpenSettings()
    {
        AfxMessageBox(L"打开设置对话框！");
        // 这里可以打开设置对话框
        
        // 示例：动态修改标题栏
        m_webViewControl.SetWebViewTitle(L"AI 聊天助手 - 设置模式");
        
        // 示例：临时移除保存按钮
        m_webViewControl.RemoveTitlebarButton(m_saveButtonId);
        
        // 5秒后恢复
        SetTimer(1, 5000, nullptr);
    }
    
    // 定时器处理
    virtual void OnTimer(UINT_PTR nIDEvent) override
    {
        if (nIDEvent == 1) {
            KillTimer(1);
            // 恢复标题和按钮
            m_webViewControl.SetWebViewTitle(L"AI 聊天助手 - LazyBug");
            m_saveButtonId = m_webViewControl.AddTitlebarButton(L"保存", L"save_chat");
        }
        CDialogEx::OnTimer(nIDEvent);
    }
};

/*
使用说明：

1. 基本标题栏设置：
   - SetWebViewTitle() - 设置标题栏标题
   - AddTitlebarButton() - 添加按钮（返回按钮ID）
   - RemoveTitlebarButton() - 移除指定按钮
   - ClearTitlebarButtons() - 清空所有按钮

2. 事件处理：
   需要在SetWebMessageReceivedCallback回调中处理两种事件：
   - titlebarClicked - 标题栏被点击
   - titlebarButtonClicked - 标题栏按钮被点击（包含buttonId和buttonAction）

3. 布局特点：
   - 标题栏固定在顶部，不会滚动
   - 聊天内容在标题栏下方的独立滚动区域
   - 深色主题，与现有UI风格一致
   - 支持悬停效果和过渡动画

4. 集成方式：
   - 完全向后兼容，不影响现有聊天和FileEdit功能
   - 可以动态修改标题和按钮
   - 支持多个按钮，自动排列
   - 按钮点击事件与标题栏点击事件独立

*/ 