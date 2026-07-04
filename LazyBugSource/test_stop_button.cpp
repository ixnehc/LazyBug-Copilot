// test_stop_button.cpp - 停止按钮功能测试示例

#include "stdh.h"
#include "ChatCtrl.h"

// 使用示例类
class CStopButtonTestDlg : public CDialogEx
{
public:
    CStopButtonTestDlg(CWnd* pParent = nullptr) : CDialogEx(IDD_DIALOG, pParent) {}

    virtual BOOL OnInitDialog() override
    {
        CDialogEx::OnInitDialog();
        
        // 创建ChatCtrl控件
        CRect rect(10, 10, 800, 600);
        m_chatCtrl.Create(rect, this, IDC_CHATCTRL);
        
        // 设置导航完成回调
        m_chatCtrl.SetNavigationCompletedCallback([this](bool success) {
            if (success) {
                SetupTest();
            }
        });
        
        // 设置停止按钮点击回调
        m_chatCtrl.SetStopButtonClickedCallback([this]() {
            OnStopButtonClicked();
        });
        
        return TRUE;
    }

private:
    CChatCtrl m_chatCtrl;
    std::wstring m_currentStreamingMessageId;
    
    // 设置测试环境
    void SetupTest()
    {
        // 初始化聊天界面
        m_chatCtrl.InitializeChatUI();
        
        // 设置标题
        m_chatCtrl.SetTitle(L"停止按钮测试 - AI聊天");
        
        // 添加测试按钮
        m_chatCtrl.AddTitlebarButton(L"开始流式消息", L"start_streaming");
        m_chatCtrl.AddTitlebarButton(L"清空聊天", L"clear_chat");
        
        // 设置Web消息回调来处理按钮点击
        m_chatCtrl.SetWebMessageReceivedCallback([this](const std::wstring& message) {
            OnWebMessageReceived(message);
        });
        
        // 添加欢迎消息
        m_chatCtrl.AddSystemMessage(L"点击"开始流式消息"按钮来测试停止按钮功能");
    }
    
    // 处理来自WebView的消息
    void OnWebMessageReceived(const std::wstring& message)
    {
        // 简单的消息处理示例（实际项目中应该使用JSON解析器）
        if (message.find(L"start_streaming") != std::wstring::npos) {
            StartTestStreaming();
        }
        else if (message.find(L"clear_chat") != std::wstring::npos) {
            m_chatCtrl.ClearChat();
        }
    }
    
    // 开始测试流式消息
    void StartTestStreaming()
    {
        // 添加用户消息
        m_chatCtrl.AddUserMessage(L"请给我讲一个长故事");
        
        // 开始会话（显示停止按钮）
        m_chatCtrl.BeginSession(L"请给我讲一个长故事");
        
        // 开始AI流式消息
        m_currentStreamingMessageId = m_chatCtrl.StartStreamingAIMessage();
        
        // 模拟流式消息的发送
        SimulateStreamingResponse();
    }
    
    // 模拟流式响应
    void SimulateStreamingResponse()
    {
        // 定义故事内容片段
        static std::vector<std::wstring> storyParts = {
            L"从前有一座美丽的城堡，",
            L"城堡里住着一位善良的公主。",
            L"公主每天都会在花园里散步，",
            L"欣赏着各种美丽的花朵。",
            L"有一天，一只受伤的小鸟飞到了她的窗台上。",
            L"公主小心翼翼地照顾着这只小鸟，",
            L"给它包扎伤口，喂它食物和水。",
            L"几天后，小鸟的伤势完全好转了。",
            L"为了感谢公主的恩情，",
            L"小鸟告诉了她一个重要的秘密：",
            L"在城堡的地下室里，",
            L"藏着一个神奇的宝藏。",
            L"这个宝藏可以实现任何善良的愿望。",
            L"公主决定用这个宝藏来帮助所有需要帮助的人。",
            L"从此以后，整个王国都变得更加美好和繁荣。",
            L"这就是关于善良与回报的美丽故事。"
        };
        
        // 使用定时器模拟逐步发送
        m_currentPartIndex = 0;
        SetTimer(1, 500, nullptr); // 每500ms发送一个片段
    }
    
    // 停止按钮点击处理
    void OnStopButtonClicked()
    {
        // 停止定时器
        KillTimer(1);
        
        // 如果有正在进行的流式消息，强制完成它
        if (!m_currentStreamingMessageId.empty()) {
            m_chatCtrl.AddStreamingAIMessage(m_currentStreamingMessageId, L"\n\n[消息被用户停止]");
            m_chatCtrl.CompleteStreamingAIMessage(m_currentStreamingMessageId);
            m_currentStreamingMessageId.clear();
        }
        
        // 结束会话（隐藏停止按钮）
        m_chatCtrl.EndSession(L"[消息被用户停止]", FileChangeListUID_Invalid);
        
        // 添加系统消息通知用户
        m_chatCtrl.AddSystemMessage(L"AI消息生成已停止");
        
        AfxMessageBox(L"停止按钮被点击，AI消息生成已终止！");
    }
    
    // 定时器处理
    afx_msg void OnTimer(UINT_PTR nIDEvent)
    {
        if (nIDEvent == 1) {
            // 发送下一个故事片段
            static std::vector<std::wstring> storyParts = {
                L"从前有一座美丽的城堡，",
                L"城堡里住着一位善良的公主。",
                L"公主每天都会在花园里散步，",
                L"欣赏着各种美丽的花朵。",
                L"有一天，一只受伤的小鸟飞到了她的窗台上。",
                L"公主小心翼翼地照顾着这只小鸟，",
                L"给它包扎伤口，喂它食物和水。",
                L"几天后，小鸟的伤势完全好转了。",
                L"为了感谢公主的恩情，",
                L"小鸟告诉了她一个重要的秘密：",
                L"在城堡的地下室里，",
                L"藏着一个神奇的宝藏。",
                L"这个宝藏可以实现任何善良的愿望。",
                L"公主决定用这个宝藏来帮助所有需要帮助的人。",
                L"从此以后，整个王国都变得更加美好和繁荣。",
                L"这就是关于善良与回报的美丽故事。"
            };
            
            if (m_currentPartIndex < storyParts.size()) {
                m_chatCtrl.AddStreamingAIMessage(m_currentStreamingMessageId, storyParts[m_currentPartIndex]);
                m_currentPartIndex++;
            } else {
                // 故事发送完成
                KillTimer(1);
                m_chatCtrl.CompleteStreamingAIMessage(m_currentStreamingMessageId);
                
                // 结束会话（隐藏停止按钮）
                m_chatCtrl.EndSession(L"这就是关于善良与回报的美丽故事。", FileChangeListUID_Invalid);
                
                m_currentStreamingMessageId.clear();
            }
        }
        
        CDialogEx::OnTimer(nIDEvent);
    }
    
    int m_currentPartIndex = 0;
    
    DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP(CStopButtonTestDlg, CDialogEx)
    ON_WM_TIMER()
END_MESSAGE_MAP()

/*
使用说明：

1. **停止按钮显示/隐藏控制**：
   - 当调用 BeginSession() 时，停止按钮显示
   - 当调用 EndSession() 时，停止按钮隐藏
   - JavaScript不再自动控制停止按钮的显示/隐藏

2. **停止按钮位置和样式**：
   - 位于窗口右下角，红色圆形按钮
   - 悬停时有放大和颜色变化效果
   - 点击时有缩小动画效果

3. **事件处理**：
   - 通过 SetStopButtonClickedCallback() 设置点击回调
   - 点击后会立即隐藏按钮并触发回调函数

4. **典型用法**：
   ```cpp
   // 设置停止按钮回调
   m_chatCtrl.SetStopButtonClickedCallback([this]() {
       // 在这里处理停止逻辑
       StopAIGeneration();
   });
   
   // 开始会话（停止按钮显示）
   m_chatCtrl.BeginSession(L"用户的原始消息");
   
   // 开始流式消息
   std::wstring msgId = m_chatCtrl.StartStreamingAIMessage();
   
   // 逐步添加内容
   m_chatCtrl.AddStreamingAIMessage(msgId, L"部分内容...");
   
   // 完成消息
   m_chatCtrl.CompleteStreamingAIMessage(msgId);
   
   // 结束会话（停止按钮隐藏）
   m_chatCtrl.EndSession(L"AI的完整回复", changelistId);
   ```

5. **手动控制**（特殊情况下可选）：
   ```cpp
   // 手动显示停止按钮（通常不需要，仅用于特殊场景）
   m_chatCtrl.ShowStopButton();
   
   // 手动隐藏停止按钮（通常不需要，仅用于特殊场景）
   m_chatCtrl.HideStopButton();
   ```

6. **工作原理**：
   - C++在BeginSession()中调用ShowStopButton()显示停止按钮
   - C++在EndSession()中调用HideStopButton()隐藏停止按钮
   - JavaScript只负责UI渲染和事件处理，不自动控制按钮显示状态
   - 用户点击停止按钮时，JavaScript发送消息给C++，由C++处理停止逻辑
*/ 