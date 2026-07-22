# LazyBug Version History

## Version 0.22

- Support background context compression, no more wait when starting a chat
- Add a new AddMcpServer tool to allow LLM to set up MCP servers dynamically
- Clicking symbol link now shows the symbol centered in the editor

---

## Version 0.21

- Added input hint feature, showing candidate completions while typing in the chat input
- Fixed a bug where writing would fail when modifying multiple files simultaneously

---

## Version 0.20.1

- Fixed an issue where the GLM model would sometimes disconnect unexpectedly

---

## Version 0.20

- Added copy/paste buttons in the Provider & API settings page
- Added symbol search support for more languages (`*.html`, `*.css`, `*.js`, `*.java`, `*.py`, `*.ts`)

---

## Version 0.19.1

- Added a **Report Issue** command in the settings menu

---

## Version 0.19 Release Notes

**Features**

- Added MCP support (stdio and URL)
- Added a UI to manage MCPs
- Added cache rate in the cost statistics label

**Bug Fixes**

- Fix an issue that sometimes the reasoning_content is not correctly sent back to LLM for Kimi and DeepSeek, which results in significant performance degrade in complex tasks
- Fix a potential crash when outputing log.
- Disabled the bash tool when bash is not available on current system
- Enabled streaming tool calls for the GLM API
- Fixed an issue where starting a new conversation after rolling back all previous conversations would reset the chat title
- Fixed an issue where the chat input window could not regain focus when the Visual Studio window was brought back to the foreground
- Restored the cursor to its previous position after the chat input window regains focus
- Fixed an issue where token statistics would become excessively large when using certain endpoints
- Fixed an issue that finding keywords in files will fail when the files are too big


## Version 0.18.1

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

## Version 0.17.1

- Add skill tip display
- New UI for the file edit control
- Disable certain types of context compression that degraded LLM output quality
- Fix an issue in the input window where the cursor would disappear when moving between pasted lines
- Add favorites management for chat sessions

---

## Version 0.17

**Features**

- Add a badge to show real-time context usage
- Use context levels to control maximum context usage within a reasonable range, with automatic compress/decompress
- Remove legacy `llm.ini` usage; replaced with `llm.json` and a new UI for setting up LLM providers and APIs

**Bug Fixes**

- Fix a CLI tool output crash issue
- Fix an issue where the stop button on the CLI display window was not shown for whitelisted commands
- Limit CLI tool output size
- Improved non-ASCII support in CLI output
- Fix an issue where the code database service would sometimes fail to detect a modified file
- Fix an LLM response exception
- Fix an issue where skills were not being loaded

---

## Version 0.16

**Features**

- Add skill support and a panel to manage skills
- Add new CLI tool (cmd.exe, bash.exe, python.exe) to support script calling
- Add new `AskQuestion` tool
- Add `global_rule.md` and `project_rule.md` for writing customized prompts

**Bug Fixes**

- Fix an issue connecting to the LM Studio API
- Fix the cost statistics reset issue
- Remove dependency on MFC DLLs
- Improved support for file paths containing non-ASCII characters
- Fix an issue where chat title auto-generation would sometimes fail

---

## Version 0.15.1

- Tweaked the font of the chat history menu
- Fixed some minor UI issues
- New LazyBug icon
- Chat input window tag-related bug fixes
- Fix a Gemini API tool-call naming issue
- Fix a code database service stalling issue
- Fix an issue where thinking mode could never be enabled for Anthropic models

---

## Version 0.15

- C# symbol support added. LLM can now query C# symbols (function names, class names, etc.)
- Added new `SearchFilePath` tool
- Display thumbnails for image tags
- Support symbol link recognition in the chat window
- Add DeepSeek V4 support

---

## Version 0.14

- Image attachment support: paste an image or image file directly into the chat input window
- Chat input window improvements and bug fixes (tag copy-paste, undo-redo)
- Fix a symbol parsing issue that caused function bodies to not be fully retrieved. Note: this fix will trigger a full symbol database rebuild.

---

## Version 0.11 ~ 0.13.2

- Support multiple keywords in text search and symbol queries
- User messages now show the tag instead of plain text
- Fix the "thought signature" issue with the Gemini API format
- Fix a cost statistics error for the OpenAI-compatible LLM API format
- Add a "modified files" summary frame at the end of each session