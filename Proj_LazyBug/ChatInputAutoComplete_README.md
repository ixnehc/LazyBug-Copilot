# ChatInput 自动补全功能说明

## 概述

为ChatInput控件实现了一个基于WebView的自动补全列表功能(ChatInputACList)。当用户输入"@"符号后，会弹出一个候选列表窗口，显示可选的项目。用户可以通过键盘或鼠标选择项目，选中的项目会以InlineTag的形式插入到输入框中。

## 功能特性

### 核心特性
- **触发机制**: 用户输入"@"符号后自动触发
- **实时过滤**: 随着用户继续输入，实时过滤候选项
- **键盘导航**: 支持上下箭头键选择，回车/Tab确认，ESC取消
- **鼠标操作**: 支持鼠标点击选择项目
- **智能匹配**: 支持前缀匹配、包含匹配和模糊匹配
- **标签插入**: 选中项目自动转换为InlineTag插入

### 样式和UI
- **现代化设计**: 符合GitHub深色主题风格
- **响应式布局**: 自动调整位置和大小
- **图标支持**: 支持emoji或其他图标显示
- **类型区分**: 不同类型的候选项可以有不同的样式
- **描述信息**: 支持显示项目的描述信息

## 文件结构

### 新增文件
- `ChatInputACList.h` - 自动补全列表类的头文件
- `ChatInputACList.cpp` - 自动补全列表类的实现
- `ChatInputACExample.cpp` - 使用示例和说明
- `ChatInputAutoComplete_README.md` - 本说明文档

### 修改的文件
- `ChatInput.h` - 添加了自动补全相关的接口和成员变量
- `ChatInput.cpp` - 添加了自动补全功能的实现
- `ChatInput.html` - 添加了自动补全的UI和JavaScript逻辑

## 核心类和结构

### ChatInputACItem 结构
```cpp
struct ChatInputACItem
{
    std::wstring id;          // 项目唯一ID
    std::wstring text;        // 显示文本
    std::wstring value;       // 实际值（用于插入标签）
    std::wstring description; // 描述信息
    std::wstring icon;        // 图标（可选）
    std::wstring type;        // 类型（用于样式）
    std::wstring data;        // 额外数据
};
```

### CChatInputACList 类
负责管理自动补全的显示、过滤、选择等逻辑：

#### 主要方法
- `Show()` - 显示自动补全列表
- `Hide()` - 隐藏自动补全列表
- `SetItems()` - 设置候选项列表
- `UpdateQuery()` - 更新查询字符串并过滤
- `SelectNext/Previous()` - 键盘导航
- `ConfirmSelection()` - 确认选择

#### 回调接口
- `ACItemSelectedCallback` - 项目选中回调
- `ACListCancelledCallback` - 列表取消回调
- `ACRequestItemsCallback` - 请求候选项回调

## 使用方法

### 1. 基本设置
```cpp
// 创建ChatInput控件
CChatInput chatInput;
chatInput.Create(rect, parentWnd, ID);

// 设置自动补全请求回调
chatInput.SetAutoCompleteRequestCallback([](const std::wstring& query) {
    // 处理自动补全请求
    OnAutoCompleteRequest(query);
});
```

### 2. 准备候选数据
```cpp
void PrepareAutoCompleteData()
{
    std::vector<ChatInputACItem> items;
    
    ChatInputACItem item;
    item.id = L"user_001";
    item.text = L"张三";
    item.value = L"zhangsan";
    item.description = L"产品经理";
    item.icon = L"👤";
    item.type = L"user";
    item.data = L"{\"userId\":\"001\"}";
    
    items.push_back(item);
    // ... 添加更多项目
}
```

### 3. 处理自动补全请求
```cpp
void OnAutoCompleteRequest(const std::wstring& query)
{
    // 根据query过滤候选项
    std::vector<ChatInputACItem> filteredItems = FilterItems(query);
    
    // 更新自动补全列表
    chatInput.SetAutoCompleteItems(filteredItems);
}
```

## 交互流程

### 用户操作流程
1. 用户在输入框中输入"@"符号
2. 自动补全列表弹出，显示候选项
3. 用户继续输入字符，列表实时过滤
4. 用户通过键盘上下箭头或鼠标选择项目
5. 按回车、Tab或点击确认选择
6. 选中的项目作为InlineTag插入到输入框
7. 自动补全列表隐藏

### 技术实现流程
1. JavaScript检测到"@"输入，发送`autoCompleteShow`消息
2. C++接收消息，调用`ACRequestItemsCallback`回调
3. 外部代码处理请求，调用`SetAutoCompleteItems()`
4. `CChatInputACList`过滤和排序候选项
5. 通过JavaScript更新UI显示列表
6. 用户选择后，通过回调处理选中事件
7. 将选中项目转换为InlineTag插入

## 配置选项

### 自动补全列表配置
```cpp
auto* acList = chatInput.GetAutoCompleteList();
acList->SetMaxVisibleItems(8);        // 最大显示项数
acList->SetListWidth(300);            // 列表宽度
acList->SetAutoFilter(true);          // 自动过滤
```

### 启用/禁用功能
```cpp
chatInput.SetAutoCompleteEnabled(true);  // 启用自动补全
chatInput.SetAutoCompleteEnabled(false); // 禁用自动补全
```

## 扩展和自定义

### 自定义匹配算法
可以在`CChatInputACList::_CalculateMatchScore()`中实现自定义的匹配算法。

### 自定义样式
可以在`ChatInput.html`的CSS中修改`.autocomplete-*`相关的样式。

### 支持多种触发符号
当前实现只支持"@"符号，可以扩展支持其他触发符号（如"#"、"/"等）。

### 异步数据加载
可以在`ACRequestItemsCallback`中实现异步数据加载，比如从服务器获取候选项。

## 注意事项

1. **内存管理**: `CChatInputACList`会自动管理内存，无需手动释放
2. **线程安全**: 回调函数应该在UI线程中执行
3. **性能优化**: 大量候选项时建议实现分页或虚拟滚动
4. **数据格式**: `data`字段建议使用JSON格式存储结构化数据
5. **错误处理**: 建议在回调函数中添加异常处理逻辑

## 示例项目类型

### 用户提及
- **触发**: @用户名
- **类型**: user
- **数据**: 用户ID、部门、角色等信息

### 文件引用
- **触发**: @文件名
- **类型**: file  
- **数据**: 文件路径、修改时间、大小等

### 命令执行
- **触发**: @命令
- **类型**: command
- **数据**: 命令参数、权限要求等

### 标签分类
- **触发**: @标签
- **类型**: tag
- **数据**: 标签颜色、分类、权重等

这个自动补全功能为ChatInput提供了强大的交互能力，可以大大提升用户体验和输入效率。 