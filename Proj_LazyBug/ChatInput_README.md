# CChatInput 使用说明

CChatInput 是一个基于 WebView2 技术实现的高级输入控件，支持多行编辑、标签管理、工具栏按钮等功能。

## 主要功能

### 1. 富文本编辑
- 支持多行文本输入和富文本编辑
- 支持内联标签与文字混排
- 自动调整大小
- 支持键盘快捷键（Enter发送，Shift+Enter换行，Ctrl+Enter强制发送）
- 支持Backspace/Delete删除标签

### 2. 标签系统
- 顶部标签栏显示各种标签
- 支持在编辑区域内插入内联标签
- 标签与文字可以混排显示
- 支持不同类型的标签（文件、信息等）
- 标签可以设置颜色和是否可删除
- 内联标签支持键盘删除操作

### 3. 工具栏
- 底部工具栏支持添加各种工具按钮
- 按钮可以设置图标、文字、提示信息
- 支持启用/禁用状态
- 右下角固定显示发送按钮

### 4. 外观设计
- 采用深色主题设计
- 符合现代UI设计规范
- 优雅的视觉效果

## 基本使用

### 1. 创建控件

```cpp
CChatInput m_chatInput;

// 在对话框初始化时创建
BOOL CMyDialog::OnInitDialog()
{
    CDialog::OnInitDialog();
    
    CRect rect;
    GetClientRect(&rect);
    rect.DeflateRect(10, 10);
    
    if (m_chatInput.Create(rect, this, 1001))
    {
        // 设置回调函数
        SetupCallbacks();
        
        // 初始化界面
        InitializeUI();
    }
    
    return TRUE;
}
```

### 2. 设置回调函数

```cpp
void CMyDialog::SetupCallbacks()
{
    // 发送消息回调
    m_chatInput.SetSendCallback([this](const std::wstring& content) {
        OnSendMessage(content);
    });
    
    // 工具按钮点击回调
    m_chatInput.SetToolButtonClickedCallback([this](const std::wstring& buttonId, const std::wstring& action) {
        OnToolButtonClicked(buttonId, action);
    });
    
    // 内容变化回调
    m_chatInput.SetContentChangedCallback([this](const std::wstring& content) {
        OnContentChanged(content);
    });
    
    // 标签移除回调
    m_chatInput.SetTagRemovedCallback([this](const std::wstring& tagId) {
        OnTagRemoved(tagId);
    });
}
```

### 3. 添加标签

```cpp
void CMyDialog::InitializeUI()
{
    // 添加文件标签
    m_chatInput.AddTag(L"config.txt", L"file", L"C:\\config.txt");
    
    // 添加信息标签
    m_chatInput.AddTag(L"重要", L"info", L"", L"#ff6b6b", true);
    
    // 添加不可删除的标签
    m_chatInput.AddTag(L"系统", L"info", L"", L"#28a745", false);
}
```

### 4. 添加工具按钮

```cpp
void CMyDialog::InitializeUI()
{
    // 添加文件选择按钮
    m_chatInput.AddToolButton(L"选择文件", L"📁", L"selectFile", L"选择文件添加到输入");
    
    // 添加格式化按钮
    m_chatInput.AddToolButton(L"格式化", L"✨", L"format", L"格式化代码");
    
    // 添加清空按钮
    m_chatInput.AddToolButton(L"清空", L"🗑️", L"clear", L"清空输入内容");
}
```

## 高级功能

### 1. 动态内容管理

```cpp
// 获取输入内容
m_chatInput.GetInputContent([](const std::wstring& content) {
    // 处理获取到的内容
    ProcessContent(content);
});

// 设置输入内容
m_chatInput.SetInputContent(L"预设内容");

// 在光标位置插入文本
m_chatInput.InsertText(L"插入的文本");

// 在光标位置插入内联标签
m_chatInput.InsertInlineTag(L"文件名", L"file", L"文件路径", true);

// 清空输入
m_chatInput.ClearInput();
```

### 2. 标签管理

```cpp
// 动态添加标签
std::wstring tagId = m_chatInput.AddTag(L"新标签", L"info", L"数据");

// 检查标签是否存在
if (m_chatInput.HasTag(tagId)) {
    // 标签存在
}

// 移除标签
m_chatInput.RemoveTag(tagId);

// 清空所有标签
m_chatInput.ClearTags();
```

### 3. 工具按钮管理

```cpp
// 添加按钮
std::wstring buttonId = m_chatInput.AddToolButton(L"新按钮", L"🔧", L"newAction", L"提示信息");

// 设置按钮状态
m_chatInput.SetToolButtonEnabled(buttonId, false);

// 移除按钮
m_chatInput.RemoveToolButton(buttonId);

// 清空所有按钮
m_chatInput.ClearToolButtons();
```

### 4. 发送按钮控制

```cpp
// 设置发送按钮状态
m_chatInput.SetSendButtonEnabled(true);

// 设置发送按钮文字
m_chatInput.SetSendButtonText(L"提交");
```

### 5. 外观设置

```cpp
// 设置占位符文字
m_chatInput.SetPlaceholder(L"请输入您的问题...");

// 设置焦点
m_chatInput.FocusInput();
```

## 回调函数处理

### 1. 发送消息处理

```cpp
void CMyDialog::OnSendMessage(const std::wstring& content)
{
    // 处理发送的消息
    if (!content.empty()) {
        // 发送到服务器或处理逻辑
        SendToServer(content);
        
        // 清空输入
        m_chatInput.ClearInput();
        
        // 添加状态标签
        m_chatInput.AddTag(L"已发送", L"info", L"", L"#28a745", true);
    }
}
```

### 2. 工具按钮点击处理

```cpp
void CMyDialog::OnToolButtonClicked(const std::wstring& buttonId, const std::wstring& action)
{
    if (action == L"selectFile") {
        // 打开文件选择对话框
        CFileDialog dlg(TRUE);
        if (dlg.DoModal() == IDOK) {
            CString filePath = dlg.GetPathName();
            m_chatInput.AddTag(dlg.GetFileName(), L"file", filePath);
        }
    }
    else if (action == L"format") {
        // 格式化内容
        FormatContent();
    }
    else if (action == L"clear") {
        // 清空内容
        m_chatInput.ClearInput();
        m_chatInput.ClearTags();
    }
}
```

### 3. 内容变化处理

```cpp
void CMyDialog::OnContentChanged(const std::wstring& content)
{
    // 根据内容启用/禁用发送按钮
    bool hasContent = !content.empty() && 
                     content.find_first_not_of(L" \t\r\n") != std::wstring::npos;
    m_chatInput.SetSendButtonEnabled(hasContent);
    
    // 自动保存草稿
    if (!content.empty()) {
        SaveDraft(content);
    }
}
```

## 注意事项

1. **WebView2 依赖**: 需要确保系统已安装 WebView2 运行时
2. **文件路径**: HTML 文件路径需要正确设置
3. **内存管理**: 确保在窗口销毁时正确释放资源
4. **线程安全**: 回调函数在 UI 线程中执行，可以安全地操作界面

## 样式定制

可以通过修改 `ChatInput.html` 中的 CSS 样式来定制外观：

```css
:root {
    --primary-color: #58a6ff;      /* 主色调 */
    --border-color: #30363d;       /* 边框颜色 */
    --bg-color: #0d1117;           /* 背景色 */
    --text-color: #f0f6fc;         /* 文字颜色 */
    /* ... 更多样式变量 */
}
```

## 示例项目

参考 `ChatInputExample.cpp` 文件查看完整的使用示例。 