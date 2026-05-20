# WebView 标题栏功能文档

## 概述

WebViewControl 现在支持顶部标题栏功能，提供了更丰富的用户界面体验。标题栏包含可自定义的标题文本和多个操作按钮，支持完整的交互功能。

## 功能特性

### 🎯 核心特性
- **固定标题栏**: 始终显示在WebView顶部，不随内容滚动
- **自定义标题**: 支持动态设置和修改标题文本
- **多按钮支持**: 可添加多个自定义按钮，支持动态管理
- **深色主题**: 与现有UI风格保持一致的深色主题设计
- **交互反馈**: 支持悬停效果和点击事件处理

### 🔧 技术特性
- **完全向后兼容**: 不影响现有聊天和FileEdit功能
- **事件驱动**: 通过C++/JavaScript双向通信处理交互
- **内存安全**: 完整的错误处理和内存管理
- **响应式布局**: 自适应不同窗口大小

## API 参考

### C++ 公共方法

#### `void SetWebViewTitle(const std::wstring& title)`
设置标题栏显示的标题文本。

**参数:**
- `title`: 要显示的标题文本

**示例:**
```cpp
m_webViewControl.SetWebViewTitle(L"AI 聊天助手 - LazyBug");
```

#### `std::wstring AddTitlebarButton(const std::wstring& buttonText, const std::wstring& buttonAction)`
在标题栏添加一个按钮。

**参数:**
- `buttonText`: 按钮显示的文本
- `buttonAction`: 按钮的动作标识符（用于事件处理）

**返回值:**
- 按钮的唯一ID，用于后续操作

**示例:**
```cpp
std::wstring saveButtonId = m_webViewControl.AddTitlebarButton(L"保存", L"save_chat");
std::wstring settingsButtonId = m_webViewControl.AddTitlebarButton(L"设置", L"open_settings");
```

#### `void RemoveTitlebarButton(const std::wstring& buttonId)`
移除指定的标题栏按钮。

**参数:**
- `buttonId`: 要移除的按钮ID（由AddTitlebarButton返回）

**示例:**
```cpp
m_webViewControl.RemoveTitlebarButton(saveButtonId);
```

#### `void ClearTitlebarButtons()`
清空所有标题栏按钮。

**示例:**
```cpp
m_webViewControl.ClearTitlebarButtons();
```

## 事件处理

### JavaScript 事件类型

标题栏交互会生成以下JSON消息发送给C++端：

#### 1. 标题栏点击事件
```json
{
    "action": "titlebarClicked"
}
```

#### 2. 标题栏按钮点击事件
```json
{
    "action": "titlebarButtonClicked",
    "buttonId": "titlebar_btn_1",
    "buttonAction": "save_chat"
}
```

### C++ 事件处理示例

```cpp
// 设置消息回调
m_webViewControl.SetWebMessageReceivedCallback([this](const std::wstring& message) {
    // 解析JSON消息（建议使用JSON库）
    if (message.find(L"titlebarClicked") != std::wstring::npos) {
        OnTitlebarClicked();
    }
    else if (message.find(L"titlebarButtonClicked") != std::wstring::npos) {
        if (message.find(L"save_chat") != std::wstring::npos) {
            OnSaveChat();
        }
        else if (message.find(L"open_settings") != std::wstring::npos) {
            OnOpenSettings();
        }
    }
});
```

## 使用示例

### 基础用法

```cpp
class CMyDialog : public CDialogEx
{
    CWebViewControl m_webViewControl;
    std::wstring m_saveButtonId;
    
public:
    virtual BOOL OnInitDialog() override
    {
        // 创建WebView控件
        CRect rect(10, 10, 800, 600);
        m_webViewControl.Create(rect, this, IDC_WEBVIEW);
        
        // 设置事件回调
        m_webViewControl.SetWebMessageReceivedCallback([this](const std::wstring& msg) {
            HandleWebMessage(msg);
        });
        
        // 设置导航完成回调
        m_webViewControl.SetNavigationCompletedCallback([this](bool success) {
            if (success) {
                SetupUI();
            }
        });
        
        return TRUE;
    }
    
private:
    void SetupUI()
    {
        // 设置标题
        m_webViewControl.SetWebViewTitle(L"我的应用");
        
        // 添加按钮
        m_saveButtonId = m_webViewControl.AddTitlebarButton(L"保存", L"save");
        m_webViewControl.AddTitlebarButton(L"导出", L"export");
        m_webViewControl.AddTitlebarButton(L"设置", L"settings");
    }
    
    void HandleWebMessage(const std::wstring& message)
    {
        // 处理标题栏事件
        if (message.find(L"save") != std::wstring::npos) {
            // 处理保存操作
        }
    }
};
```

### 动态标题栏管理

```cpp
void CMyDialog::OnModeChange(int mode)
{
    switch (mode) {
        case MODE_CHAT:
            m_webViewControl.SetWebViewTitle(L"聊天模式");
            m_webViewControl.ClearTitlebarButtons();
            m_webViewControl.AddTitlebarButton(L"清空", L"clear_chat");
            m_webViewControl.AddTitlebarButton(L"导出", L"export_chat");
            break;
            
        case MODE_EDIT:
            m_webViewControl.SetWebViewTitle(L"编辑模式");
            m_webViewControl.ClearTitlebarButtons();
            m_webViewControl.AddTitlebarButton(L"保存", L"save_file");
            m_webViewControl.AddTitlebarButton(L"编译", L"compile");
            m_webViewControl.AddTitlebarButton(L"运行", L"run");
            break;
    }
}
```

## 样式和主题

### CSS 类名

标题栏使用以下CSS类，可以通过修改`WebViewControl.html`自定义样式：

- `.webview-titlebar` - 标题栏容器
- `.webview-title` - 标题文本
- `.webview-titlebar-buttons` - 按钮容器
- `.webview-titlebar-button` - 标题栏按钮
- `.webview-content` - 内容区域

### 自定义样式示例

```css
.webview-titlebar {
    background: linear-gradient(45deg, #1a1a1a, #2d2d2d);
    border-bottom: 2px solid #00796b;
}

.webview-titlebar-button {
    background-color: #00796b;
    color: white;
}

.webview-titlebar-button:hover {
    background-color: #004d40;
    transform: translateY(-1px);
}
```

## 布局结构

```
┌─────────────────────────────────────────┐
│ [标题]                    [按钮1][按钮2] │ ← 固定标题栏
├─────────────────────────────────────────┤
│                                         │
│              聊天内容区域                 │ ← 可滚动内容
│         (包含消息和FileEdit窗口)          │
│                                         │
│                                         │
└─────────────────────────────────────────┘
```

## 集成注意事项

### 1. 初始化时序
- 确保在WebView导航完成后再设置标题栏
- 建议在`NavigationCompletedCallback`中进行初始化

### 2. 事件处理
- 使用JSON解析器处理复杂消息（推荐使用nlohmann/json）
- 区分不同事件类型，避免误处理

### 3. 内存管理
- 按钮ID由系统自动生成，无需手动管理
- 移除按钮时使用返回的ID

### 4. 错误处理
- 检查WebView初始化状态
- 处理消息解析异常

## 兼容性

- **WebView2 版本**: 需要WebView2 Runtime 1.0.774.44+
- **操作系统**: Windows 10 1803+
- **编译器**: Visual Studio 2017+ (C++17)
- **MFC版本**: MFC 14.0+

## 故障排除

### 常见问题

**Q: 标题栏不显示**
A: 检查是否在导航完成后设置，确保`m_isChatInitialized`为true

**Q: 按钮点击无响应**
A: 检查`SetWebMessageReceivedCallback`是否正确设置，查看消息格式

**Q: 样式异常**
A: 检查CSS文件是否正确加载，验证网络连接（CDN资源）

**Q: 滚动异常**
A: 确保body设置了`overflow: hidden`，内容区域设置了`overflow-y: auto`

### 调试技巧

1. 使用`OutputDebugString`打印接收的消息
2. 在浏览器开发者工具中检查HTML结构
3. 验证JSON消息格式
4. 检查CSS样式应用情况

## 更新历史

- **v1.0.0** (2024-01): 初始版本，支持基础标题栏功能
- **v1.1.0** (2024-01): 添加深色主题支持
- **v1.2.0** (2024-01): 优化布局和响应式设计

## 相关文档

- [WebViewControl基础文档](./README_WebView.md)
- [FileEdit功能文档](./README_FileEdit.md)
- [事件处理指南](./README_Events.md) 