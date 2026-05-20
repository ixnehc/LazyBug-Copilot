# LazyBug - AI Programming Assistant Extension for Visual Studio

## Product Overview

**LazyBug** is a cursor-like intelligent programming assistant extension designed specifically for Visual Studio. Powered by large language models (LLMs), it provides developers with intelligent code generation, refactoring, and Q&A experiences. The extension supports multiple mainstream AI service providers, allowing developers to enjoy AI-assisted programming within their familiar IDE environment.

---

## Core Features

### 1. Intelligent Chat System

- Supports Markdown-based multi-turn conversations, with rich media rendering such as code highlighting and Diff views, as well as freely adjustable font sizes
- Supports creating new conversations and switching between conversation history
- Allows one-click rollback to any previous state in a conversation, with the ability to undo the rollback
- Conversation cost statistics display
- Context usage progress bar
- Conversation history is stored by VS solution and can be persistently saved

### 2. Intelligent Code Editing

- AI can directly modify files in the project, including multiple files at once
- Supports viewing before-and-after comparisons in the code editor through Diff View
- File modification tracking and rollback
- File backup system, so even if rollback fails (for example, due to a system bug), the user's original files can still be recovered

### 3. Code Database Search

- Efficient search based on Visual Studio project files
- Supports fast text search in ultra-large projects
- Supports fast symbol lookup in ultra-large projects (currently C/C++ only)
- AI can automatically read relevant file contents to obtain context

### 4. Smart Input Box

- Uses a tagging system to manage attached files
- Autocomplete: typing `@` triggers autocomplete for files/symbols
- Quickly browse input history with PageUp/PageDown
- Quickly switch between large language models

### 5. Multi-Model Support

- Default API providers
  - OpenAI (GPT-4, GPT-3.5, etc.)
  - Anthropic (Claude series)
  - Google (Gemini series)
  - OpenRouter (multi-model aggregation)
  - Moonshot AI (Kimi)
  - z.ai (GLM)
- Configuration files can be modified to support custom API endpoints
- Supports three API formats: OpenAI-compatible, Anthropic, and Gemini

---

## Use Cases

- **Code Review and Refactoring**: Let AI analyze your code and propose refactoring suggestions
- **New Feature Development**: Describe your requirements and let AI help generate code structure and implementation
- **Bug Fixing**: Describe the issue symptoms and let AI help locate and fix bugs
- **Code Explanation**: Ask about complex code logic and receive detailed explanations from AI

---

## Tips for Use

- **Open the chat panel**: Open the chat panel through the Visual Studio menu `View` -> `Other Windows` -> `LazyBug Chat`. It is recommended to bind a shortcut key to this command (`View.LazyBugChat`) for quick access at any time.
- **Database and disk space**: The LazyBug Chat database is independent of the project and is stored centrally on drive C. Please ensure that drive C has sufficient available space (for ultra-large projects, you may need to reserve more than 10GB).
- **View backup and database directories**: Hover the mouse over the "Settings" button in the upper-right corner of the chat panel for a moment, and a directory icon will appear. Click it to browse the current project's database directory. Files recently modified by AI are automatically backed up in the `_backup` folder under that directory.
- **Add file attachments**:
  - In the `Solution Explorer` panel, select a file, right-click it, and choose `Add to LazyBug Chat` to add it as an attachment to the current conversation.
  - You can also perform the same action by right-clicking the current file tab above the code editor.
- **Automatic search and context**: Attachments in the chat window are submitted to the large model in full. However, in most cases, you do not need to manually attach code files. Simply provide relevant keywords in the conversation, and AI will automatically search for and read the required file contents.
- **Symbol database building (C/C++)**: When opening a C/C++ project for the first time, LazyBug automatically builds a symbol database in the background. For ultra-large projects (for example, 3 million lines of code), this process may take a long time (about 30 minutes to 1 hour). Before the build is complete, symbol query results may not be fully accurate.
- **Code comparison (Diff View)**:
  - Click the title of a file edit block in the chat panel to display the before-and-after Diff view in the main editor; press the space bar to cancel the display.
  - Repeatedly clicking the title of a file edit block allows you to quickly jump between different modified sections in the Diff.
- **Avoid editing conflicts**: While AI is working (especially when it is modifying file contents), do not edit files manually.
- **Interface zooming**: When the mouse focus is inside the LazyBug chat window, hold down `Ctrl` and scroll the mouse wheel to freely zoom the interface and text size.
- **Custom API and Prompt Cache**: When configuring a custom LLM API, if the provider supports it, it is recommended to prioritize the native API format (especially for Anthropic models) to ensure that Prompt Cache functions properly.
- **Cost statistics note**: The cost amount for each round of conversation is calculated according to the unit price you enter in the configuration file. When the model provider uses a complex pricing scheme (such as certain subscription plans), the statistics are for reference only.
- **Context progress bar and limits**:
  - **Meaning of the progress bar**: Reflects the context usage of the most recent conversation round. When the progress bar turns red, it means the context is nearly exhausted, and the model's output quality may decline (for example, hallucinations may occur).
  - **Limit setting**: The upper limit of the progress bar can be configured in the LLM API configuration file, representing the maximum context length at which the model can still maintain high-quality output (it should usually be set slightly lower than the model's advertised maximum capacity).
  - **Recommendation**: When the progress bar turns red, try to avoid continuing the conversation in the current session (the extension currently does not support context compression). It is recommended to start a new conversation.
- **Task decomposition strategy**: LazyBug is not designed to handle extremely large and complex tasks. Please break down your development work into smaller, well-defined subtasks to achieve the best AI-assisted experience.

---

LazyBug seamlessly integrates AI capabilities into the development workflow, preserving developers' full control over their code while significantly improving coding efficiency. It is a powerful assistant for Visual Studio developers.