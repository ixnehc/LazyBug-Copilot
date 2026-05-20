---
name: WebFetch
description: Provides web content fetching capability.
---

## Installation

```
npm install -g mcp-fetch-server
```

## Usage

**Important: You must use the Cmd tool to execute mcp-fetch commands. Do not use the Bash tool.**

## CLI Usage

```
mcp-fetch <command> <url> [flags]
```

### Commands

| Command | Description |
|---------|-------------|
| `html` | Fetch a URL and return raw HTML |
| `markdown` | Fetch a URL and return Markdown |
| `readable` | Fetch a URL and return article content as Markdown (via Readability) |
| `txt` | Fetch a URL and return plain text |
| `json` | Fetch a URL and return JSON |
| `youtube` | Fetch a YouTube video transcript |

### Flags

| Flag | Description |
|------|-------------|
| `--max-length <N>` | Maximum characters to return |
| `--start-index <N>` | Start from this character index |
| `--proxy <URL>` | Proxy URL |
| `--lang <code>` | Language code for YouTube transcripts (default: `en`) |
| `--help` | Show help message |
| `--version` | Show version |

### Examples

**Note: The following commands need to be executed via the Cmd tool.**

```bash
# Fetch a page as markdown
mcp-fetch markdown https://example.com

# Extract article content without boilerplate
mcp-fetch readable https://example.com/blog/post

# Get a YouTube transcript in Spanish
mcp-fetch youtube https://www.youtube.com/watch?v=dQw4w9WgXcQ --lang es

# Fetch with a length limit
mcp-fetch html https://example.com --max-length 10000

# Fetch through a proxy
mcp-fetch json https://api.example.com/data --proxy http://proxy:8080
```