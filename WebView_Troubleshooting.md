# WebView FileEdit 功能故障排除指南

## 问题诊断步骤

### 1. 编译问题检查

确保代码能够正常编译：

```cpp
// 检查头文件包含
#include "stdh.h"
#include "WebViewControl.h"
#include <fstream>
#include <algorithm>  // 这个必须包含
```

**已修复的编译错误：**
- ✅ 添加了 `#include <algorithm>` 来修复 `std::find_if` 错误
- ✅ 修复了JSON布尔值构造错误

### 2. 运行时检查

**步骤1：检查WebView2是否正确初始化**

```cpp
// 在您的代码中添加调试输出
void CheckWebViewInitialization(CWebViewControl& webView) {
    // 设置消息回调来监控状态
    webView.SetWebMessageReceivedCallback([](const std::wstring& message) {
        OutputDebugStringW(L"Received: ");
        OutputDebugStringW(message.c_str());
        OutputDebugStringW(L"\n");
    });
    
    // 检查JavaScript是否正常工作
    webView.ExecuteScript(L"window.chrome.webview.postMessage({action: 'test', message: 'JS working'});");
}
```

**步骤2：测试基本聊天功能**

```cpp
void TestBasicChat(CWebViewControl& webView) {
    // 等待WebView完全初始化
    Sleep(1000);
    
    // 测试系统消息
    webView.AddSystemMessage(L"测试系统消息");
    
    // 测试用户消息
    webView.AddUserMessage(L"测试用户消息");
    
    // 测试AI消息
    std::wstring aiMsgId = webView.StartStreamingAIMessage();
    webView.AddStreamingAIMessage(aiMsgId, L"测试AI消息");
    webView.CompleteStreamingAIMessage(aiMsgId);
}
```

**步骤3：测试FileEdit功能**

```cpp
void TestFileEdit(CWebViewControl& webView) {
    Sleep(2000); // 确保基本功能工作正常
    
    std::wstring aiMsgId = webView.StartStreamingAIMessage();
    webView.AddStreamingAIMessage(aiMsgId, L"测试FileEdit窗口：\n\n");
    
    std::wstring fileEditId = webView.AddFileEditToAIMessage(
        aiMsgId,
        L"test.cpp",
        L"#include <iostream>\nint main() { return 0; }",
        200
    );
    
    if (!fileEditId.empty()) {
        OutputDebugStringW(L"FileEdit created successfully\n");
        webView.AddFileEditButton(fileEditId, L"测试", L"test");
    } else {
        OutputDebugStringW(L"FileEdit creation failed\n");
    }
    
    webView.CompleteStreamingAIMessage(aiMsgId);
}
```

### 3. 常见问题及解决方案

**问题1：没有任何消息显示**

可能原因：
- WebView2运行时未安装
- HTML模板加载失败
- JavaScript错误阻止了页面渲染

解决方案：
```cpp
// 检查HTML模板是否正确加载
webView.ExecuteScript(L"console.log('Document loaded:', document.readyState); console.log('Chat container:', !!document.getElementById('chat-container'));");
```

**问题2：基本消息显示但FileEdit不工作**

可能原因：
- JavaScript函数定义有错误
- CSS样式加载失败
- 消息传递格式错误

解决方案：
```cpp
// 检查FileEdit相关的JavaScript函数
webView.ExecuteScript(L"console.log('createFileEditWindow function:', typeof createFileEditWindow);");
```

**问题3：FileEdit窗口显示但按钮不工作**

可能原因：
- 事件监听器设置错误
- 按钮点击事件处理有问题

解决方案：
```cpp
// 设置详细的消息监控
webView.SetWebMessageReceivedCallback([](const std::wstring& message) {
    if (message.find(L"fileEdit") != std::wstring::npos) {
        OutputDebugStringW(L"FileEdit event: ");
        OutputDebugStringW(message.c_str());
        OutputDebugStringW(L"\n");
    }
});
```

### 4. 调试工具使用

**使用Debug_WebView.cpp中的调试工具：**

```cpp
#include "Debug_WebView.cpp"

// 在您的主程序中
void DebugWebViewIssues(CWebViewControl& webView) {
    // 启用详细调试
    WebViewDebugger::DebugWebView(webView);
    
    // 检查DOM状态
    WebViewDebugger::CheckWebViewState(webView);
    
    // 运行完整功能测试
    WebViewDebugger::TestBasicFunctionality(webView);
}
```

### 5. 检查列表

在报告问题之前，请确认：

- [ ] 代码能够成功编译（无编译错误）
- [ ] WebView2运行时已安装
- [ ] 基本聊天功能（系统消息、用户消息、AI消息）正常工作
- [ ] JavaScript控制台没有错误（使用F12开发者工具检查）
- [ ] HTML模板正确加载（可以看到聊天容器）
- [ ] 消息回调函数正确设置

### 6. 获取详细日志

```cpp
// 添加到您的代码中以获取详细日志
void EnableDetailedLogging(CWebViewControl& webView) {
    webView.SetWebMessageReceivedCallback([](const std::wstring& message) {
        // 记录所有消息到文件
        std::wofstream logFile(L"webview_debug.log", std::ios::app);
        if (logFile.is_open()) {
            logFile << L"[" << GetTickCount() << L"] " << message << std::endl;
            logFile.close();
        }
        
        // 同时输出到调试控制台
        OutputDebugStringW(message.c_str());
        OutputDebugStringW(L"\n");
    });
}
```

### 7. 如果问题仍然存在

请提供以下信息：
1. 编译错误信息（如果有）
2. 运行时错误信息或崩溃信息
3. 调试输出日志
4. WebView2版本信息
5. 操作系统版本 