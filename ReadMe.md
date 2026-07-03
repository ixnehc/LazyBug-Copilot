# LazyBug Copilot - Visual Studio AI Coding Assistant Extension

[![Visual Studio Marketplace](https://img.shields.io/badge/VS%20Marketplace-Install-orange?logo=visual-studio)](https://marketplace.visualstudio.com/items?itemName=IxSoftware.lazybug2026)
[![Version](https://img.shields.io/badge/version-0.20.1-blue)](https://github.com/ixnehc/LazyBug-Copilot-Publish/blob/main/patchnotes.md)
[![Visual Studio 2022](https://img.shields.io/badge/Visual%20Studio-2022-purple?logo=visual-studio)](https://marketplace.visualstudio.com/items?itemName=IxSoftware.lazybug2026)

> 📖 [中文版](doc/ReadMe_cn.md)

## Product Overview

LazyBug Copilot is a "Cursor-like" intelligent coding assistant extension designed specifically for Visual Studio. It integrates Large Language Model (LLM) capabilities to provide developers with intelligent code creation, refactoring, and Q&A experiences. The extension supports multiple mainstream AI service providers, enabling developers to enjoy AI-assisted programming within their familiar IDE environment.

![screenshot](./media/screenshot3.gif)

---

## Version 0.20.1 Release Notes

- Fixed an issue where the GLM model would sometimes interrupt unexpectedly

---

## Version 0.20 Release Notes

- Added copy/paste buttons in the Provider & API settings page
- Added symbol search support for more languages (`*.html`, `*.css`, `*.js`, `*.java`, `*.py`, `*.ts`)

_See [patchnotes.md](https://github.com/ixnehc/LazyBug-Copilot-Publish/blob/main/patchnotes.md) for full version history._

---

## Core Features

- **Multi-Turn Intelligent Chat** — Markdown-based chat content. Session history stored per VS solution.
- **Session Management** — Switch between historical sessions, one-click rollback to any previous state, session favorites management, and cost statistics per session.
- **Symbol Link Recognition** — Clickable symbol links in the chat window for quick navigation to definitions.
- **Smart Code Editing** — AI directly modifies project files with multi-file support, before/after Diff View, modification tracking, undo/redo , and file backup.
- **Automatic Code Database** — Automatically builds a code database from all files in your solution with incremental updates.
- **Codebase Search** — Fast text search for ultra-large projects (million-line scale). Significantly faster than ripgrep, especially in large codebases. 
- **Symbol Search** — Fast symbol search for C/C++/C#/JavaScript/Java/Python/TypeScript codes. Works out of the box — no LSP configuration required.
- **Smart Input Box** — Tag-based file attachment system with `@` auto-completion, input history (`PageUp`/`PageDown`), quick model switching.

![chatinput](./media/chatinput.jpg)

- **Image Attachment** — Paste images directly into the chat input to send to vision-capable LLMs.

![image support](./media/image_support2.jpg)

- **Multi-Model Support** — Customizable API endpoints. Supports mainstream LLMs: OpenAI, Anthropic, Google Gemini, OpenRouter, Moonshot (Kimi), z.ai (GLM), DeepSeek and more. Also supports local LLMs (Ollama, LM Studio).
- **Multi-API Format** — Supports three API formats: OpenAI-compatible, Anthropic, and Gemini.

![provider](./media/provider.jpg)

- **Skill System** — Browse, create, rename, and toggle skills via a management panel. Supports BuiltIn, Global, and Project-level skills. Allow using AI to edit or create new skills.

![skill](./media/skill.jpg)

- **Custom Prompts** — `global_rules.txt` and `project_rules.txt` for customized prompts; 

![settings](./media/settings2.jpg)

- **CLI Tool Integration** — Execute cmd.exe, bash.exe, python.exe scripts directly from the chat, extending capabilities beyond coding.

<img src="./media/output_10fps.gif" width="500" />

- **Context Usage Control** — Real-time context usage display with 5 context levels. Allow keeping context under relatively low level (< 30k tokens) even in extremely long conversations while maintaining high response quality. Automatic compression/decompression when context level changes.

![contextlevel](./media/contextlevel.jpg)

- **MCP Support** — Model Context Protocol server support via stdio and URL, with a built-in UI to manage them.

![mcp](./media/createmcp.gif)



## Usage Scenarios

- **Code Review and Refactoring**: Let AI analyze your code and suggest refactoring improvements.
- **New Feature Development**: Describe your requirements, and AI will assist in generating code frameworks and implementations.
- **Bug Fixing**: Describe the issue symptoms, and AI will help locate and fix the bug.
- **Code Explanation**: Ask about complex code logic, and AI will provide detailed explanations.

---

## Usage Tips

- **Open the Chat Panel**: Open the chat panel via the Visual Studio menu `View` → `Other Windows` → `LazyBug Chat`. It is recommended to bind a shortcut to the `View.LazyBugChat` command for quick access.
- **Database and Disk Space**: The LazyBug chat database is independent of the project and is stored centrally on the C drive. Please ensure sufficient free space on your C drive (for ultra-large projects, more than 10 GB may be required).
- **Adding File Attachments**:
  - Select a file in the **Solution Explorer**, right-click, and choose **Add to LazyBug Chat** to attach it to the current conversation.
  - You can also do the same by right-clicking the file's tab above the code editor.
- **Symbol Database Construction (C/C++)**: When you open a C/C++ project for the first time, LazyBug will automatically build the symbol database in the background. For ultra-large projects (e.g., 3 million lines of code), this process may take a significant amount of time (approximately 30 minutes to 1 hour). Symbol query results may not be fully accurate until the build is complete.
- **Code Comparison (Diff View)**:
  - Click the title of a file editing label in the chat panel to display the Diff view in the main editor; press `Space` to hide it.
  - Clicking the title repeatedly allows you to quickly jump between different diff hunks.
- **Avoid Editing Conflicts**: Please do not manually edit files while AI is working, especially when it is modifying file contents.
- **UI Scaling**: When the mouse focus is inside the LazyBug chat window, hold `Ctrl` and scroll the mouse wheel to freely zoom the interface and text size.
- **Cost Statistics**: The cost for each chat turn is calculated based on the unit price entered in the LLM api setting. When the LLM provider uses a complex billing model (such as a subscription plan), the statistics are approximate and for reference only.
- **Task Breakdown Strategy**: LazyBug is not designed to handle extremely large and complex tasks in a single pass. Break your development tasks into smaller, clearly defined sub-tasks for the best AI-assisted experience.
- **Code Database Updating**: After adding a new file to the solution, save the solution file so that the new file is included in the code database.
- **Skill Usage**:
  - There are 3 types of skills:
    - **BuiltIn**: Verified skills bundled with the extension. These should generally not be modified.
    - **Global**: Skills shared across all projects.
    - **Project**: Skills specific to the currently opened project.
  - Activating a skill does not mean its full content is loaded into the context. To force-load a skill, copy its path and paste it into the file attachment list.
  - Many existing skills exist in the ecosystem and may have compatibility issues with LazyBug. You may need to tweak them until they work smoothly.
  - You are always encouraged to use AI to edit existing skills or create new ones.
  - Install the necessary environments (Node.js, Python, GIT, etc.) to support various CLI commands.
- **Compression Model Selection**: Use the **Evaluate** button to assess the speed, reliability, and compression quality of the currently selected compression model, helping you choose the most suitable one.

![evaluate](./media/evaluate.jpg)

- **How Context Level Works**:
  - When context usage reaches a level's upper limit (threshold), LazyBug compresses the context down to the target lower limit. This keeps conversations sustainable without unbounded token growth.

    | Level   | Threshold (upper) | Target (lower) |
    |---------|-------------------|----------------|
    | Level 1 | 30k tokens        | 10k tokens     |
    | Level 2 | 50k tokens        | 30k tokens     |
    | Level 3 | 100k tokens       | 50k tokens     |
    | Level 4 | 200k tokens       | 100k tokens    |
    | Level 5 | 500k tokens       | 200k tokens    |

- **Context Level Settings Consideration**:
  - Higher context levels consume more tokens and incur higher costs, especially on expensive models.
  - However, higher context levels reduce the frequency of compression, which improves cache hit rates and can also lower costs to some extent.
  - Currently, context is not compressed within a single Q&A turn to avoid cache miss.
  - Compression is not unlimited — it will always preserve a baseline answer quality. So even if you set a low context level, sometimes context usage may still significantly exceed the upper limit.
  - That said, compression will more or less degrade the model's answer quality.
  - **Overall recommendation**: For expensive models (Anthropic, GPT), set the lowest context level (Level 1, < 30k tokens), unless you notice a significant drop in answer quality.

---

## Report an Issue

If you encounter any bugs or have suggestions for improvement, please report [here](https://github.com/ixnehc/LazyBug-Copilot-Publish/issues). Your feedback is the driving force behind our continuous improvement!
