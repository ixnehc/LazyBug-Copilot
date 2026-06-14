# LazyBug - Visual Studio AI Coding Assistant Extension

## Product Overview

LazyBug is a "Cursor-like" intelligent coding assistant extension designed specifically for Visual Studio. It integrates Large Language Model (LLM) capabilities to provide developers with intelligent code creation, refactoring, and Q&A experiences. The extension supports multiple mainstream AI service providers, enabling developers to enjoy AI-assisted programming within their familiar IDE environment.

![screenshot__1.gif](screenshot__1.gif)

---

## Version 0.18 Release Notes

- Improved context compression quality
- Log the context compression results
- Added an "Evaluate" button for the selected "Compress & Summarize" API on the settings page
- Fixed an issue where showing the code diff would remove breakpoints and bookmarks
- Added a stripe on the left side of the code window to indicate that the editor is currently in code diff mode
- In the chat window, collapsed all exploration operations (file reading, keyword searching, etc.)
- Fixed an issue where chat title generation would occasionally fail
- Improved symbol link recognition in the chat window (more links are now recognized)
- Fixed a crash that occurred when displaying skill tips
- Improved CLI tool command-line error tolerance

---

_See below for full version history_

## Core Features

**1. Intelligent Chat System**

- Supports Markdown-based multi-turn chat display with rich media presentations including code highlighting and Diff views, with adjustable font sizes.
- Switch between historical sessions with ease.
- One-click rollback to any historical session state, with the ability to cancel the rollback.
- Displays session cost statistics.
- Chat history is stored per VS solution and persisted to disk.

**2. Smart Code Editing**

- AI can directly modify file contents within the project, supporting simultaneous edits across multiple files.
- View before-and-after comparisons (Diff View) in the code editor window.
- File modification tracking and rollback support.
- A file backup system ensures original files can be recovered in case of rollback failures (e.g., due to system bugs).

**3. Codebase Search**

- Builds a fast search index based on solution file symbols and text content.
- Supports fast text search for ultra-large projects.
- Supports fast symbol queries for ultra-large projects (C/C++/C# currently supported).
- AI automatically reads related file contents to acquire the necessary context.

**4. Smart Input Box**

![chatinput2.jpg](chatinput2.jpg)

- Uses a tag system to manage attached files.
- Auto-completion: trigger file/symbol auto-completion using the `@` symbol.
- Browse input history quickly using `PageUp`/`PageDown`.
- Quickly switch between Large Language Models.
- Paste an image directly into the input window and send it to the LLM.

![image_support2__1.jpg](image_support2__1.jpg)

**5. Multi-Model Support**

![provider.jpg](provider.jpg)

- Default API Providers:
  - OpenAI (GPT-5, etc.)
  - Anthropic (Claude series)
  - Google (Gemini series)
  - OpenRouter (multi-model aggregation)
  - Moonshot AI (Kimi)
  - z.ai (GLM)
  - DeepSeek (DeepSeek V4 series)

- Supports modifiable configuration files for custom API endpoints.
- Local LLM services supported (Ollama, LM Studio).
- Supports three API formats: OpenAI-compatible, Anthropic, and Gemini.

---

**6. Custom Prompts**

- Skill system supported, with a panel to browse, create, rename, and toggle skills.

![skill.jpg](skill.jpg)

- `global_rules.txt` and `project_rules.txt` for writing global and project-specific prompts.
- `cli_whitelist.ini` for adding trusted CLI commands.

![settings2__1.jpg](settings2__1.jpg)

**7. CLI Tool**

- CLI tool supported, with skills to extend capabilities beyond coding.

![output_10fps.gif](output_10fps.gif)

**8. Context Usage Control**

- Displays real-time context usage.
- Set different context levels (5 levels total) to control the context size range.

![contextlevel.jpg](contextlevel.jpg)

- If context usage exceeds the current level's upper bound, context compression is triggered automatically.
- Context decompression is triggered when a higher context level is set.


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
- **Automatic Search and Context**: Attachments in the chat window are fully submitted to the LLM. However, in most cases you do not need to manually attach files — just provide relevant keywords and AI will automatically search for and read the necessary file contents.
- **Symbol Database Construction (C/C++)**: When you open a C/C++ project for the first time, LazyBug will automatically build the symbol database in the background. For ultra-large projects (e.g., 3 million lines of code), this process may take a significant amount of time (approximately 30 minutes to 1 hour). Symbol query results may not be fully accurate until the build is complete.
- **Code Comparison (Diff View)**:
  - Click the title of a file editing block in the chat panel to display the Diff view in the main editor; press `Space` to hide it.
  - Clicking the title repeatedly allows you to quickly jump between different diff hunks.
- **Avoid Editing Conflicts**: Please do not manually edit files while AI is working, especially when it is modifying file contents.
- **UI Scaling**: When the mouse focus is inside the LazyBug chat window, hold `Ctrl` and scroll the mouse wheel to freely zoom the interface and text size.
- **Custom API and Prompt Cache**: When configuring custom LLM APIs, if the provider supports it, prefer using the native API format (especially for Anthropic models) to ensure the Prompt Cache feature works correctly.
- **Cost Statistics**: The cost for each chat turn is calculated based on the unit price entered in the configuration file. When the LLM provider uses a complex billing model (such as a subscription plan), the statistics are approximate and for reference only.
- **Context Progress Bar and Limits**:
  - **Progress Bar**: Reflects the context usage of the most recent chat turn. When the bar turns red, the context is nearly exhausted and LLM output quality may degrade (e.g., hallucinations may occur).
  - **Upper Limit**: The upper limit of the progress bar can be set in the LLM API configuration file, representing the maximum context length at which the model maintains high-quality output (typically set slightly below the model's nominal maximum capacity).
  - **Recommendation**: When the progress bar turns red, avoid continuing the conversation in the current session and consider starting a new one.
- **Task Breakdown Strategy**: LazyBug is not designed to handle extremely large and complex tasks in a single pass. Break your development tasks into smaller, clearly defined sub-tasks for the best AI-assisted experience.
- **Code Database Updating**: After adding a new file to the solution, save the solution file so that the new file is included in the code database.
- **Skill Usage**:
  - There are 3 types of skills:
    - **BuiltIn**: Verified skills bundled with the extension. These should generally not be modified.
    - **Global**: Skills shared across all projects.
    - **Project**: Skills specific to the currently opened project.
  - Install the necessary environments (Node.js, Python, etc.) to support various CLI commands.
  - Activating a skill does not mean its full content is loaded into the context. To force-load a skill, copy its path and paste it into the file attachment list.
  - Many existing skills exist in the ecosystem and may have compatibility issues with LazyBug. You may need to tweak them until they work smoothly.
  - You are always encouraged to use AI to edit existing skills or create new ones.

---

## Contact Us

If you encounter any bugs or have suggestions for improvement, please feel free to reach out via email. Your feedback is the driving force behind our continuous improvement!

- 📧 **Contact Email**: [ixnehc1974@gmail.com](mailto:ixnehc1974@gmail.com)

## Version History

**0.11 ~ 0.13.2**

- Support multiple keywords in text search and symbol queries
- User messages now show the tag instead of plain text
- Fix the "thought signature" issue with the Gemini API format
- Fix a cost statistics error for the OpenAI-compatible LLM API format
- Add a "modified files" summary frame at the end of each session

**0.14**

- Image attachment support: paste an image or image file directly into the chat input window
- Chat input window improvements and bug fixes (tag copy-paste, undo-redo)
- Fix a symbol parsing issue that caused function bodies to not be fully retrieved. Note: this fix will trigger a full symbol database rebuild.

**0.15**

- C# symbol support added. LLM can now query C# symbols (function names, class names, etc.)
- Added new `SearchFilePath` tool
- Display thumbnails for image tags
- Support symbol link recognition in the chat window
- Add DeepSeek V4 support

**0.15.1**

- Tweaked the font of the chat history menu
- Fixed some minor UI issues
- New LazyBug icon

```ini
[Provider]
name=DeepSeek
endpoint=https://api.deepseek.com/chat/completions
apiFormat=DeepSeek
```

- Chat input window tag-related bug fixes
- Fix a Gemini API tool-call naming issue
- Fix a code database service stalling issue
- Fix an issue where thinking mode could never be enabled for Anthropic models

---

**0.16**

*Features*

- Add skill support and a panel to manage skills
- Add new CLI tool (cmd.exe, bash.exe, python.exe) to support script calling
- Add new `AskQuestion` tool
- Add `global_rule.md` and `project_rule.md` for writing customized prompts

*Bug Fixes*

- Fix an issue connecting to the LM Studio API
- Fix the cost statistics reset issue
- Remove dependency on MFC DLLs
- Improved support for file paths containing non-ASCII characters
- Fix an issue where chat title auto-generation would sometimes fail

---

**0.17**

*Features*

- Add a badge to show real-time context usage
- Use context levels to control maximum context usage within a reasonable range, with automatic compress/decompress
- Remove legacy `llm.ini` usage; replaced with `llm.json` and a new UI for setting up LLM providers and APIs

*Bug Fixes*

- Fix a CLI tool output crash issue
- Fix an issue where the stop button on the CLI display window was not shown for whitelisted commands
- Limit CLI tool output size
- Improved non-ASCII support in CLI output
- Fix an issue where the code database service would sometimes fail to detect a modified file
- Fix an LLM response exception
- Fix an issue where skills were not being loaded

---

**0.17.1**

- Add skill tip display
- New UI for the file edit control
- Disable certain types of context compression that degraded LLM output quality
- Fix an issue in the input window where the cursor would disappear when moving between pasted lines
- Add favorites management for chat sessions

---
