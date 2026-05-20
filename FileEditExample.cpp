// FileEdit 功能使用示例
// 此文件展示了如何使用 CWebViewControl 的 FileEdit 内嵌窗口功能

#include "WebViewControl.h"

void DemoFileEditUsage(CWebViewControl& webViewControl)
{
    // 1. 首先开始一个AI消息
    std::wstring aiMessageId = webViewControl.StartStreamingAIMessage();
    
    // 2. 添加一些AI响应内容
    webViewControl.AddStreamingAIMessage(aiMessageId, L"我已经为您创建了一个文件编辑窗口：\n\n");
    
    // 3. 在AI消息中添加一个FileEdit窗口
    std::wstring fileEditId = webViewControl.AddFileEditToAIMessage(
        aiMessageId,                                    // AI消息ID
        L"main.cpp",                                   // 文件标题
        L"#include <iostream>\n\nint main() {\n    std::cout << \"Hello, World!\" << std::endl;\n    return 0;\n}",  // 文件内容
        250                                            // 窗口高度（像素）
    );
    
    // 4. 为FileEdit窗口添加一些按钮
    std::wstring saveButtonId = webViewControl.AddFileEditButton(fileEditId, L"保存", L"save");
    std::wstring copyButtonId = webViewControl.AddFileEditButton(fileEditId, L"复制", L"copy");
    std::wstring formatButtonId = webViewControl.AddFileEditButton(fileEditId, L"格式化", L"format");
    
    // 5. 继续添加AI响应内容
    webViewControl.AddStreamingAIMessage(aiMessageId, L"\n这是一个简单的C++程序。您可以：\n");
    webViewControl.AddStreamingAIMessage(aiMessageId, L"- 点击标题栏来折叠/展开窗口\n");
    webViewControl.AddStreamingAIMessage(aiMessageId, L"- 使用右侧的按钮进行操作\n");
    webViewControl.AddStreamingAIMessage(aiMessageId, L"- 点击大小调整按钮来改变窗口高度");
    
    // 6. 完成AI消息
    webViewControl.CompleteStreamingAIMessage(aiMessageId);
    
    // 7. 演示动态更新FileEdit内容
    // （通常这会在用户交互或其他事件中触发）
    
    // 更新文件标题
    webViewControl.SetFileEditTitle(fileEditId, L"main_updated.cpp");
    
    // 更新文件内容
    std::wstring newContent = L"#include <iostream>\n#include <string>\n\nint main() {\n    std::string message = \"Hello, FileEdit!\";\n    std::cout << message << std::endl;\n    return 0;\n}";
    webViewControl.SetFileEditContent(fileEditId, newContent);
    
    // 调整窗口高度
    webViewControl.SetFileEditHeight(fileEditId, 300);
}

void DemoMultipleFileEdits(CWebViewControl& webViewControl)
{
    // 演示在一个AI消息中添加多个FileEdit窗口
    
    std::wstring aiMessageId = webViewControl.StartStreamingAIMessage();
    
    webViewControl.AddStreamingAIMessage(aiMessageId, L"我为您创建了一个完整的项目文件：\n\n");
    
    // 添加头文件
    std::wstring headerFileId = webViewControl.AddFileEditToAIMessage(
        aiMessageId,
        L"MyClass.h",
        L"#pragma once\n\nclass MyClass {\npublic:\n    MyClass();\n    ~MyClass();\n    void doSomething();\n\nprivate:\n    int m_value;\n};",
        180
    );
    webViewControl.AddFileEditButton(headerFileId, L"保存", L"save");
    
    webViewControl.AddStreamingAIMessage(aiMessageId, L"\n");
    
    // 添加实现文件
    std::wstring cppFileId = webViewControl.AddFileEditToAIMessage(
        aiMessageId,
        L"MyClass.cpp",
        L"#include \"MyClass.h\"\n#include <iostream>\n\nMyClass::MyClass() : m_value(0) {\n}\n\nMyClass::~MyClass() {\n}\n\nvoid MyClass::doSomething() {\n    std::cout << \"Doing something...\" << std::endl;\n}",
        220
    );
    webViewControl.AddFileEditButton(cppFileId, L"保存", L"save");
    webViewControl.AddFileEditButton(cppFileId, L"编译", L"compile");
    
    webViewControl.AddStreamingAIMessage(aiMessageId, L"\n这是一个完整的C++类实现。您可以分别编辑头文件和实现文件。");
    
    webViewControl.CompleteStreamingAIMessage(aiMessageId);
}

// 在您的应用程序中处理FileEdit按钮点击事件
void HandleFileEditEvents(CWebViewControl& webViewControl)
{
    // 设置Web消息接收回调来处理FileEdit事件
    webViewControl.SetWebMessageReceivedCallback([&](const std::wstring& message) {
        // 解析JSON消息（这里简化处理，实际应用中建议使用JSON库）
        if (message.find(L"\"action\":\"fileEditButtonClicked\"") != std::wstring::npos) {
            // 提取按钮动作
            if (message.find(L"\"buttonAction\":\"save\"") != std::wstring::npos) {
                // 处理保存操作
                // 您可以在这里实现实际的文件保存逻辑
            }
            else if (message.find(L"\"buttonAction\":\"copy\"") != std::wstring::npos) {
                // 处理复制操作
                // 您可以在这里实现复制到剪贴板的逻辑
            }
            else if (message.find(L"\"buttonAction\":\"format\"") != std::wstring::npos) {
                // 处理代码格式化操作
                // 您可以在这里实现代码格式化逻辑
            }
        }
        else if (message.find(L"\"action\":\"fileEditResized\"") != std::wstring::npos) {
            // 处理窗口大小调整事件
            // 您可以在这里保存用户的窗口大小偏好
        }
        else if (message.find(L"\"action\":\"fileEditToggled\"") != std::wstring::npos) {
            // 处理窗口折叠/展开事件
            // 您可以在这里保存用户的折叠状态偏好
        }
    });
}

/*
使用说明：

1. 基本用法：
   - 先创建AI消息：StartStreamingAIMessage()
   - 添加FileEdit窗口：AddFileEditToAIMessage()
   - 添加按钮：AddFileEditButton()
   - 完成AI消息：CompleteStreamingAIMessage()

2. 动态更新：
   - 更新标题：SetFileEditTitle()
   - 更新内容：SetFileEditContent()
   - 调整高度：SetFileEditHeight()
   - 切换折叠：ToggleFileEditCollapse()

3. 事件处理：
   - 设置消息回调：SetWebMessageReceivedCallback()
   - 处理按钮点击、大小调整、折叠切换等事件

4. 界面特性：
   - 用户可以点击标题栏折叠/展开窗口
   - 右侧有自定义按钮和大小调整按钮
   - 支持多个FileEdit窗口在同一个AI消息中
   - 自动滚动条和代码风格显示
*/ 