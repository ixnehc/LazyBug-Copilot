# CWebViewControl FileEdit 内嵌窗口功能

## 功能概述

为 `CWebViewControl` 类添加了 FileEdit 内嵌窗口功能，允许在 AI 消息的 bubble 中添加文件编辑窗口。每个窗口包含：

- **标题栏**：显示文件名，支持点击折叠/展开
- **自定义按钮**：可在标题栏右侧添加多个功能按钮
- **大小调整按钮**：最右侧的按钮，用于调整显示区域高度
- **显示区域**：以代码风格显示文件内容，支持滚动

## 主要类和结构

### 新增数据结构

```cpp
// FileEdit 窗口按钮结构
struct FileEditButton {
    std::wstring text;      // 按钮文字
    std::wstring action;    // 按钮动作标识
    std::wstring id;        // 按钮唯一ID
};

// FileEdit 窗口结构
struct FileEditWindow {
    std::wstring id;                          // 窗口唯一ID
    std::wstring title;                       // 标题栏文字
    std::wstring content;                     // 显示区域内容
    std::wstring messageId;                   // 所属的AI消息ID
    std::vector<FileEditButton> buttons;      // 标题栏按钮列表
    int height;                               // 显示区域高度（像素）
    bool isCollapsed;                         // 是否折叠
};
```

## 公共API接口

### 核心方法

```cpp
// 在指定AI消息中添加FileEdit窗口
std::wstring AddFileEditToAIMessage(
    const std::wstring& messageId,     // AI消息ID
    const std::wstring& title = L"",   // 窗口标题
    const std::wstring& content = L"", // 显示内容
    int height = 200                   // 窗口高度
);

// 设置FileEdit窗口标题
void SetFileEditTitle(const std::wstring& fileEditId, const std::wstring& title);

// 设置FileEdit窗口显示内容
void SetFileEditContent(const std::wstring& fileEditId, const std::wstring& content);

// 设置FileEdit窗口高度
void SetFileEditHeight(const std::wstring& fileEditId, int height);
```

### 按钮管理

```cpp
// 添加FileEdit窗口标题栏按钮
std::wstring AddFileEditButton(
    const std::wstring& fileEditId,    // FileEdit窗口ID
    const std::wstring& buttonText,    // 按钮文字
    const std::wstring& buttonAction   // 按钮动作标识
);
```

### 窗口控制

```cpp
// 折叠/展开FileEdit窗口
void ToggleFileEditCollapse(const std::wstring& fileEditId);
```

## 使用示例

### 基本用法

```cpp
// 1. 开始AI消息
std::wstring aiMessageId = webViewControl.StartStreamingAIMessage();

// 2. 添加AI响应内容
webViewControl.AddStreamingAIMessage(aiMessageId, L"我为您创建了一个文件：\n\n");

// 3. 添加FileEdit窗口
std::wstring fileEditId = webViewControl.AddFileEditToAIMessage(
    aiMessageId,
    L"main.cpp",
    L"#include <iostream>\n\nint main() {\n    return 0;\n}",
    250
);

// 4. 添加按钮
webViewControl.AddFileEditButton(fileEditId, L"保存", L"save");
webViewControl.AddFileEditButton(fileEditId, L"复制", L"copy");

// 5. 完成AI消息
webViewControl.CompleteStreamingAIMessage(aiMessageId);
```

### 事件处理

```cpp
// 设置消息回调来处理用户交互
webViewControl.SetWebMessageReceivedCallback([&](const std::wstring& message) {
    if (message.find(L"\"action\":\"fileEditButtonClicked\"") != std::wstring::npos) {
        if (message.find(L"\"buttonAction\":\"save\"") != std::wstring::npos) {
            // 处理保存操作
        }
        else if (message.find(L"\"buttonAction\":\"copy\"") != std::wstring::npos) {
            // 处理复制操作
        }
    }
    else if (message.find(L"\"action\":\"fileEditResized\"") != std::wstring::npos) {
        // 处理大小调整事件
    }
    else if (message.find(L"\"action\":\"fileEditToggled\"") != std::wstring::npos) {
        // 处理折叠/展开事件
    }
});
```

## 用户界面特性

### 交互功能

1. **标题栏点击**：点击标题栏左侧（文件名区域）可折叠/展开窗口
2. **自定义按钮**：点击标题栏右侧的自定义按钮会触发相应事件
3. **大小调整**：点击最右侧的调整按钮（⚏）在预设尺寸间切换
4. **滚动支持**：内容区域支持垂直滚动，适用于长文件

### 视觉设计

1. **深色主题**：默认使用深色主题，与现有聊天界面一致
2. **代码风格**：显示区域使用等宽字体，适合代码显示
3. **悬停效果**：按钮和标题栏有悬停高亮效果
4. **折叠指示器**：▼ 箭头指示窗口状态，折叠时旋转90度

## 技术实现

### CSS 样式类

- `.file-edit-window`：窗口容器
- `.file-edit-titlebar`：标题栏
- `.file-edit-title`：文件名显示
- `.file-edit-buttons`：按钮容器
- `.file-edit-button`：自定义按钮
- `.file-edit-resize-btn`：大小调整按钮
- `.file-edit-content`：内容显示区域
- `.file-edit-collapsed`：折叠状态

### JavaScript 通信

前端 JavaScript 与 C++ 后端通过 WebView2 消息机制通信：

**C++ → JavaScript 消息类型：**
- `addFileEdit`：创建新窗口
- `updateFileEditTitle`：更新标题
- `updateFileEditContent`：更新内容
- `updateFileEditHeight`：调整高度
- `updateFileEditButtons`：更新按钮
- `toggleFileEditCollapse`：切换折叠状态

**JavaScript → C++ 消息类型：**
- `fileEditButtonClicked`：按钮点击事件
- `fileEditResized`：大小调整事件
- `fileEditToggled`：折叠切换事件

## 文件修改清单

### 头文件修改 (WebViewControl.h)

1. 添加了新的数据结构：`FileEditButton`、`FileEditWindow`
2. 添加了公共方法声明
3. 添加了私有成员变量和辅助方法声明

### 实现文件修改 (WebViewControl.cpp)

1. 构造函数初始化新的计数器
2. `InitializeChatUI()` 和 `ClearChat()` 初始化/清理 FileEdit 数据
3. 添加了完整的 FileEdit 功能实现
4. 更新了 HTML 模板，添加 CSS 样式和 JavaScript 函数
5. 扩展了消息事件处理逻辑

### 新建示例文件

1. `FileEditExample.cpp`：使用示例和最佳实践
2. `FileEdit_README.md`：功能文档和说明

## 注意事项

1. **消息ID管理**：确保在正确的AI消息中添加FileEdit窗口
2. **事件处理**：建议使用JSON库来解析前端发送的事件消息
3. **内存管理**：FileEdit窗口数据会在聊天清空时自动清理
4. **UI响应**：窗口操作会自动触发滚动到底部以保持用户体验
5. **多窗口支持**：单个AI消息可以包含多个FileEdit窗口

## 扩展建议

1. **语法高亮**：可集成代码高亮库（如Prism.js）
2. **编辑功能**：可添加内容编辑能力（目前为只读显示）
3. **文件格式支持**：可根据文件扩展名提供不同的显示样式
4. **拖拽支持**：可实现窗口拖拽排序功能
5. **主题切换**：可扩展到支持亮色主题 