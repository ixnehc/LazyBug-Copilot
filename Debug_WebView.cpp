// WebView 调试辅助文件
#include "WebViewControl.h"
#include <windows.h>

// 调试助手类
class WebViewDebugger {
public:
    static void DebugWebView(CWebViewControl& webView) {
        // 设置消息接收回调来监控所有JavaScript消息
        webView.SetWebMessageReceivedCallback([](const std::wstring& message) {
            // 输出所有从JavaScript收到的消息
            OutputDebugStringW(L"[WebView Debug] Received from JS: ");
            OutputDebugStringW(message.c_str());
            OutputDebugStringW(L"\n");
        });
        
        // 执行一些基本的JavaScript来测试连接
        webView.ExecuteScript(L"console.log('WebView Debug: JavaScript is working'); window.chrome.webview.postMessage({action: 'debug', message: 'JavaScript to C++ communication test'});", 
            [](const std::wstring& result) {
                OutputDebugStringW(L"[WebView Debug] Script execution result: ");
                OutputDebugStringW(result.c_str());
                OutputDebugStringW(L"\n");
            });
    }
    
    static void TestBasicFunctionality(CWebViewControl& webView) {
        OutputDebugStringW(L"[WebView Debug] Starting basic functionality test...\n");
        
        // 先等待一点时间确保WebView初始化完成
        Sleep(1000);
        
        // 测试系统消息
        OutputDebugStringW(L"[WebView Debug] Testing system message...\n");
        webView.AddSystemMessage(L"Debug: Testing system message");
        
        Sleep(500);
        
        // 测试用户消息
        OutputDebugStringW(L"[WebView Debug] Testing user message...\n");
        webView.AddUserMessage(L"Debug: Testing user message");
        
        Sleep(500);
        
        // 测试AI流式消息
        OutputDebugStringW(L"[WebView Debug] Testing AI streaming message...\n");
        std::wstring aiMsgId = webView.StartStreamingAIMessage();
        webView.AddStreamingAIMessage(aiMsgId, L"Debug: This is a test AI message. ");
        webView.AddStreamingAIMessage(aiMsgId, L"It should appear incrementally. ");
        webView.CompleteStreamingAIMessage(aiMsgId);
        
        Sleep(1000);
        
        // 测试FileEdit功能
        OutputDebugStringW(L"[WebView Debug] Testing FileEdit functionality...\n");
        std::wstring aiMsgId2 = webView.StartStreamingAIMessage();
        webView.AddStreamingAIMessage(aiMsgId2, L"Debug: Now testing FileEdit:\n\n");
        
        std::wstring fileEditId = webView.AddFileEditToAIMessage(
            aiMsgId2,
            L"debug.cpp",
            L"// Debug test file\n#include <iostream>\n\nint main() {\n    std::cout << \"Debug test\" << std::endl;\n    return 0;\n}",
            150
        );
        
        webView.AddFileEditButton(fileEditId, L"Debug", L"debug");
        webView.CompleteStreamingAIMessage(aiMsgId2);
        
        OutputDebugStringW(L"[WebView Debug] All tests completed!\n");
    }
    
    static void CheckWebViewState(CWebViewControl& webView) {
        // 执行JavaScript来检查当前DOM状态
        webView.ExecuteScript(L"const chatContainer = document.getElementById('chat-container'); window.chrome.webview.postMessage({action: 'debug_dom', chatContainerExists: !!chatContainer, childCount: chatContainer ? chatContainer.children.length : 0, bodyContent: document.body.innerHTML.substring(0, 500)});", 
            [](const std::wstring& result) {
                OutputDebugStringW(L"[WebView Debug] DOM Check result: ");
                OutputDebugStringW(result.c_str());
                OutputDebugStringW(L"\n");
            });
    }
}; 