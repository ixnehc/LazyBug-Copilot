# 快速入门指南

## 第一步：从 Visual Studio Marketplace 下载安装

从 [Visual Studio Marketplace](https://marketplace.visualstudio.com/items?itemName=IxSoftware.lazybug2026) 下载并安装 **LazyBug Copilot**。

安装完成后，重启 Visual Studio 以激活扩展。

## 第二步：打开 LazyBug 聊天面板

在 Visual Studio 菜单栏中选择：

```
视图 → 其他窗口 → LazyBug Chat
```

LazyBug 聊天面板将会出现，通常停靠在 IDE 的右侧。

> **提示：** 你也可以将 `View.LazyBugChat` 命令绑定到快捷键以便快速访问：
> 1. 前往 `工具 → 选项 → 环境 → 键盘`。
> 2. 在**显示命令包含**框中输入 `View.LazyBugChat`。
> 3. 选中该命令并分配你喜欢的快捷键（例如 `Ctrl+Shift+L`）。
> 4. 点击**分配**，然后点击**确定**。

## 第三步：验证已有 Provider 的 API Key

LazyBug 预置了多个 Provider。要验证或更新已有 Provider 的 API key：

1. 在 LazyBug 聊天面板中打开**设置**菜单。
2. 选择 **Provider & API**。
3. 选择你要验证的 Provider。
4. 粘贴你的 API key，并按**回车**。
5. 如果验证成功，Provider 名称前面的红色指示灯会变为绿色。

![验证已有 Provider](../media/verify_existing_provider.gif)

## 第四步：添加新的 Provider

要添加自定义 LLM Provider：

1. 在 LazyBug 聊天面板中打开**设置**菜单。
2. 选择 **Provider & API**。
3. 点击所有 Provider 下方的 **"+"** 按钮来新增一个 Provider。
4. 填写：**名称**、**API 端点**，并选择 **API 格式**（如果你不知道选哪个，默认选 **OpenAI**）。
5. 你需要先在该 Provider 下至少添加一个 API，才能验证 API key。
   - 点击 **APIs** 右侧的 **"+"** 按钮来添加 API。
   - 至少输入 **API 名称**和**模型标识符**，以确保它可用。
6. 最后粘贴你的 API key 并按**回车**，开始验证。如果验证成功，Provider 名称前的红色指示灯会变为绿色。

![添加新 Provider](../media/add_new_provider.gif)

---

配置完成后，你就可以在聊天面板中选择该 Provider 下的 API，开始与 AI 助手对话了。