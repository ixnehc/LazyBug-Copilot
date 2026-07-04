<div align="center">

![LazyBug Copilot](../media/icon2.png)

# LazyBug Copilot - Visual Studio AI 编程助手扩展

[![Visual Studio Marketplace](https://img.shields.io/badge/VS%20Marketplace-Download-orange?logo=visual-studio)](https://marketplace.visualstudio.com/items?itemName=IxSoftware.lazybug2026)
[![Version](https://img.shields.io/badge/version-0.20.1-blue)](patchnotes.md)
[![Visual Studio 2022](https://img.shields.io/badge/Visual%20Studio-2022-purple?logo=visual-studio)](https://marketplace.visualstudio.com/items?itemName=IxSoftware.lazybug2026)

</div>

> 📖 [English](../ReadMe.md)

## 产品概述

LazyBug Copilot 是一款专为 Visual Studio 打造的"类 Cursor"智能编程助手扩展。它集成了大语言模型（LLM）能力，为开发者提供智能代码创建、重构和问答体验。该扩展支持多种主流 AI 服务提供商，让开发者能够在熟悉的 IDE 环境中享受 AI 辅助编程。

![screenshot](../media/screenshot3.gif)

---

## Version 0.20.1 更新说明

- 修正了 GLM 模型有时会意外中断的问题

---

## Version 0.20 更新说明

- 在 Provider & API 设置页面添加了复制/粘贴按钮
- 新增更多语言的符号搜索支持（`*.html`、`*.css`、`*.js`、`*.java`、`*.py`、`*.ts`）

_完整版本历史请参见 [patchnotes.md](patchnotes.md)_

---

## 核心功能

- **多轮智能对话** — 基于 Markdown 的聊天内容。会话历史按 VS 解决方案独立存储。
- **会话管理** — 切换历史会话、一键回滚到任意历史状态、会话收藏管理、每次会话的费用统计。
- **符号链接识别** — 聊天窗口中可点击的符号链接，快速导航到定义处。
- **智能代码编辑** — AI 直接修改项目文件，支持多文件编辑、前后对比（Diff View）、修改追踪、撤销/重做以及文件备份。
- **自动代码数据库** — 自动从解决方案中的所有文件构建代码数据库，支持增量更新。
- **代码库搜索** — 对超大型项目（百万行规模）进行快速文本搜索。速度远超 ripgrep，在大代码库中尤为明显。
- **符号搜索** — 快速符号搜索，支持 C/C++/C#/JavaScript/Java/Python/TypeScript。开箱即用，无需 LSP 配置。
- **智能输入框** — 基于标签的文件附件系统，支持 `@` 自动补全、输入历史遍历（`PageUp`/`PageDown`）、快速切换模型。

![chatinput](../media/chatinput.jpg)

- **图片附件** — 直接将图片粘贴到聊天输入框中，发送给支持视觉的 LLM。

![image support](../media/image_support2.jpg)

- **多模型支持** — 可自定义 API 端点。支持主流 LLM：OpenAI、Anthropic、Google Gemini、OpenRouter、Moonshot（Kimi）、z.ai（GLM）、DeepSeek 等。同时支持本地 LLM（Ollama、LM Studio）。
- **多 API 格式** — 支持三种 API 格式：OpenAI 兼容格式、Anthropic 和 Gemini。

![provider](../media/provider.jpg)

- **技能系统** — 通过管理面板浏览、创建、重命名和开关技能。支持内置、全局和项目级技能。允许使用 AI 编辑或创建新技能。

![skill](../media/skill.jpg)

- **自定义提示词** — `global_rules.txt` 和 `project_rules.txt` 用于编写自定义提示词。

![settings](../media/settings2.jpg)

- **CLI 工具集成** — 直接在聊天中执行 cmd.exe、bash.exe、python脚本，扩展编程之外的能力。

<img src="../media/output_10fps.gif" width="500" />

- **上下文使用控制** — 实时显示上下文使用量，提供 5 个上下文级别。即使在极长对话中也能将上下文保持在较低水平（< 30k tokens），同时保持较高的回答质量。上下文级别变化时自动压缩/解压。

![contextlevel](../media/contextlevel.jpg)

- **MCP 支持** — 通过 stdio 和 URL 支持 Model Context Protocol 服务器，内置管理界面。

![mcp](../media/createmcp.gif)




## 使用场景

- **代码审查与重构**：让 AI 分析你的代码并提出重构改进建议。
- **新功能开发**：描述你的需求，AI 将协助生成代码框架和实现。
- **Bug 修复**：描述问题症状，AI 将帮助定位并修复 Bug。
- **代码解释**：询问复杂代码逻辑，AI 将提供详细解释。

---

## 使用技巧

- **打开聊天面板**：通过 Visual Studio 菜单 `视图` → `其他窗口` → `LazyBug Chat` 打开聊天面板。建议为 `View.LazyBugChat` 命令绑定快捷键以便快速访问。
- **数据库与磁盘空间**：LazyBug 聊天数据库独立于项目，集中存储在 C 盘。请确保 C 盘有足够的可用空间（对于超大型项目，可能需要 10 GB 以上）。
- **添加文件附件**：
  - 在**解决方案资源管理器**中选择文件，右键点击选择**添加到 LazyBug Chat**，将其附加到当前对话中。
  - 也可以通过右键点击代码编辑器上方的文件标签页来执行相同操作。
- **符号数据库构建（C/C++）**：首次打开 C/C++ 项目时，LazyBug 将在后台自动构建符号数据库。对于超大型项目（如 300 万行代码），此过程可能需要较长时间（约 30 分钟到 1 小时）。在构建完成之前，符号查询结果可能不完全准确。
- **代码对比（Diff View）**：
  - 点击聊天面板中文件编辑标签的标题，可在主编辑器中显示 Diff 视图；按 `Space` 键隐藏。
  - 重复点击标题可在不同的差异块之间快速跳转。
- **避免编辑冲突**：AI 工作时请勿手动编辑文件，尤其是在 AI 正在修改文件内容时。
- **界面缩放**：当鼠标焦点在 LazyBug 聊天窗口内时，按住 `Ctrl` 并滚动鼠标滚轮，自由缩放界面和文字大小。
- **费用统计**：每次对话的费用根据 LLM API 设置中输入的单价计算。当 LLM 提供商使用复杂的计费模式（如订阅计划）时，统计数据仅为近似值，仅供参考。
- **任务拆分策略**：LazyBug 不适合一次性处理超大复杂任务。请将开发任务拆分为更小、定义清晰的子任务，以获得最佳的 AI 辅助体验。
- **代码数据库更新**：向解决方案添加新文件后，请保存解决方案文件，以便新文件被纳入代码数据库。
- **技能使用**：
  - 技能分为 3 种类型：
    - **内置（BuiltIn）**：扩展自带的已验证技能，通常不应修改。
    - **全局（Global）**：在所有项目间共享的技能。
    - **项目（Project）**：特定于当前打开项目的技能。
  - 激活技能并不意味着其全部内容已加载到上下文中。要强制加载某个技能，请复制其路径并粘贴到文件附件列表中。
  - 生态系统中存在许多现有技能，可能与 LazyBug 存在兼容性问题。你可能需要调整它们直到运行顺畅。
  - 鼓励你使用 AI 编辑现有技能或创建新技能。
  - 安装必要的运行环境（Node.js、Python、GIT 等）以支持各种 CLI 命令。
- **压缩模型选择**：使用 **Evaluate** 按钮来评估当前选择的压缩模型的速度、可靠性和压缩质量，帮助你选择合适的压缩模型。

![evaluate](../media/evaluate.jpg)

- **上下文等级工作机制**：
  - 当上下文使用量达到某个等级的上限（阈值）时，LazyBug 会将上下文压缩到目标下限。这使对话可持续进行，避免 token 无限增长。

    | 级别    | 阈值（上限） | 目标（下限） |
    |---------|-------------|-------------|
    | Level 1 | 30k tokens  | 10k tokens  |
    | Level 2 | 50k tokens  | 30k tokens  |
    | Level 3 | 100k tokens | 50k tokens  |
    | Level 4 | 200k tokens | 100k tokens |
    | Level 5 | 500k tokens | 200k tokens |

- **上下文等级设置注意事项**：
  - 较高的上下文等级会消耗更多 token，产生更多费用，在昂贵模型上尤为明显。
  - 但较高的上下文等级会减少压缩频率，从而提高缓存命中率，从某种程度上也能降低成本。
  - 目前同一次问答中不会对上下文进行压缩，以避免缓存失效。
  - 压缩并非无限制 — 它会始终保留基本的回答质量。因此即使设置了较低的上下文等级，有时上下文使用量仍可能大幅超出上限。
  - 压缩或多或少会降低模型的回答质量。
  - **总体建议**：对于昂贵模型（Anthropic、GPT），建议设置最低的上下文等级（Level 1，< 30k tokens），除非你发现回答质量明显下降。

---

## 报告问题

如果你遇到任何 Bug 或有改进建议，请在[此处](https://github.com/ixnehc/LazyBug-Copilot-Publish/issues)报告。你的反馈是我们持续改进的动力！