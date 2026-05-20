# LazyBug - Visual Studio AI Coding Assistant Extension

## Product Overview

**LazyBug** is a "cursor-like" intelligent coding assistant extension designed specifically for Visual Studio, integrating Large Language Model (LLM) capabilities to provide developers with intelligent code creation, refactoring, and Q&A experiences. The extension supports multiple mainstream AI service providers, allowing developers to enjoy the convenience of AI-assisted programming within their familiar IDE environment.

---

## Core Features

### 1. Intelligent Chat System

- Supports Markdown-based multi-turn chat display, featuring rich media presentations like code highlighting and Diff views, with adjustable font sizes.
- Supports creating new sessions and switching between historical sessions.
- One-click rollback to any historical state of the session, with the ability to cancel the rollback.
- Displays session cost statistics.
- Visual progress bar for context usage rate.
- Chat history is stored per VS solution and saved persistently.

### 2. Smart Code Editing

- AI can directly modify file contents within the project, supporting simultaneous modifications of multiple files.
- View before-and-after comparisons (Diff View) in the code editing window.
- File modification tracking and rollback.
- File backup system ensures user's original files can be recovered in case of rollback failures (e.g., system bugs).

### 3. Codebase Search

- Efficient search based on Visual Studio project files.
- Supports fast text search for ultra-large projects.
- Supports fast symbol queries for ultra-large projects (currently C/C++ only).
- AI automatically reads related file contents to acquire context.

### 4. Smart Input Box

- Uses a tag system to manage attached files.
- Auto-completion: trigger file/symbol auto-completion using the `@` symbol.
- Fast browsing of input history using PageUp/PageDown.
- Quickly switch between Large Language Models.

### 5. Multi-Model Support

- Default API Providers:
	- OpenAI (GPT-5, etc.)
	- Anthropic (Claude series)
	- Google (Gemini series)
	- OpenRouter (Multi-model aggregation)
	- Moonshot AI (Kimi)
	- z.ai (GLM)
- Modifiable configuration files to support custom API endpoints.
- Supports three API formats: OpenAI-compatible, Anthropic, and Gemini.

---

## Usage Scenarios

- **Code Review and Refactoring**: Let AI analyze code and suggest refactoring improvements.
- **New Feature Development**: Describe your requirements, and AI will assist in generating code frameworks and implementations.
- **Bug Fixing**: Describe the issue symptoms, and AI will help locate and fix the bug.
- **Code Explanation**: Ask about complex code logic, and AI will provide detailed explanations.

---

## Tips for Usage

- **Open the Chat Panel**: You can open the chat panel via the Visual Studio menu `View` -> `Other Windows` -> `LazyBug Chat`. It is recommended to bind a shortcut to this command (`View.LazyBugChat`) for quick access.
- **Database and Disk Space**: The LazyBug Chat database is independent of the project and is centrally stored on the C drive. Please ensure you have sufficient free space on your C drive (for ultra-large projects, you may need to reserve more than 10GB of space).
- **View Backup and Database Directory**: Hovering over the "Settings" button in the top right corner of the chat panel for a moment will reveal a directory icon. Click it to browse the current project's database directory. Files recently modified by AI will be automatically backed up in the `_backup` folder within this directory.
- **Adding File Attachments**:
  - Select a file in the `Solution Explorer` panel, right-click, and choose `Add to LazyBug Chat` to add it as an attachment to the current conversation.
  - You can also perform the same action by right-clicking the current file's tab above the code editor.
- **Automatic Search and Context**: Attachments within the chat window are fully submitted to the LLM. However, in most cases, you do not need to manually attach code files. Just provide relevant keywords in the chat, and AI will automatically search for and read the necessary file contents.
- **Symbol Database Construction (C/C++)**: When you open a C/C++ project for the first time, LazyBug will automatically build the symbol database in the background. For ultra-large projects (e.g., 3 million lines of code), this process may take a considerable amount of time (around 30 minutes to 1 hour). Symbol query results may not be entirely accurate until the build is complete.
- **Code Comparison (Diff View)**:
  - Click the title of the file editing block in the chat panel to display the Diff view of the changes in the main editing area; press the Spacebar to hide it.
  - Continuously clicking the title of the file editing block allows you to quickly jump between different Diff modification paragraphs.
- **Avoid Editing Conflicts**: Please do not manually edit files while AI is working (especially when it is modifying file contents).
- **UI Scaling**: When the mouse focus is inside the LazyBug chat window, hold the `Ctrl` key and scroll the mouse wheel to freely zoom the interface and text size.
- **Custom API and Prompt Cache**: When configuring custom LLM APIs, if the provider supports it, prioritize using the native API format (especially for Anthropic models) to ensure the Prompt Cache feature functions correctly.
- **Cost Statistics Explanation**: The cost for each chat turn is calculated based on the unit price you enter in the configuration file. When the LLM provider uses a complex billing model (such as a subscription plans), this statistical result is not so accurate and for reference only.
- **Context Progress Bar and Limits**:
  - **Progress Bar Meaning**: It reflects the context usage of the most recent chat turn. When the progress bar turns red, it indicates that the context is nearly exhausted, and the output quality of the LLM may degrade (e.g., "hallucinations" may occur).
  - **Upper Limit Setting**: The upper limit of the progress bar can be set in the LLM's API configuration file, representing the maximum context length at which the model can maintain high-quality output (usually set slightly below the model's nominal maximum capacity).
  - **Usage Recommendation**: When the progress bar turns red, try to avoid continuing the conversation in the current session (the extension currently lacks a context compression feature) and consider starting a new session.
- **Task Breakdown Strategy**: LazyBug is not designed to handle ultra-large and complex tasks in one go. Please break down your development tasks into smaller, clearly defined sub-tasks to achieve the best AI-assisted experience.

---

## Contact Us

If you encounter any bugs during use or have any suggestions for improvement, please feel free to contact us via email. Your feedback is the driving force behind our continuous product improvement!

- 📧 **Contact Email**: [ixnehc1974@gmail.com](mailto:ixnehc1974@gmail.com)

**LazyBug** is committed to seamlessly integrating AI capabilities into the development workflow, significantly improving coding efficiency while ensuring you maintain full control over your code. We strive to become an indispensable assistant for Visual Studio developers.
