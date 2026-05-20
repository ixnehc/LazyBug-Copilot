// 简单的FileEdit功能测试
#include "WebViewControl.h"

void TestBasicFunctionality(CWebViewControl& webView)
{
    // 测试基本的聊天功能
    webView.AddSystemMessage(L"开始测试...");
    
    // 测试AI消息
    std::wstring aiMsgId = webView.StartStreamingAIMessage();
    webView.AddStreamingAIMessage(aiMsgId, L"测试AI消息正常工作。");
    webView.CompleteStreamingAIMessage(aiMsgId);
    
    // 测试用户消息
    webView.AddUserMessage(L"测试用户消息");
    
    // 测试FileEdit功能
    std::wstring aiMsgId2 = webView.StartStreamingAIMessage();
    webView.AddStreamingAIMessage(aiMsgId2, L"现在测试FileEdit功能：\n\n");
    
    std::wstring fileEditId = webView.AddFileEditToAIMessage(
        aiMsgId2,
        L"test.cpp",
        L"// 测试文件\n#include <iostream>\n\nint main() {\n    std::cout << \"Hello, World!\" << std::endl;\n    return 0;\n}",
        200
    );
    
    // 添加测试按钮
    webView.AddFileEditButton(fileEditId, L"测试", L"test");
    
    webView.AddStreamingAIMessage(aiMsgId2, L"\n如果您能看到上面的文件编辑窗口，说明功能正常工作。");
    webView.CompleteStreamingAIMessage(aiMsgId2);
} 