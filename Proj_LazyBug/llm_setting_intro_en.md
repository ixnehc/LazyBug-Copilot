# LLM Configuration File Manual

## Overview

This configuration file is used to set the relevant parameters for LLM (Large Language Model) API providers and API instances. The configuration file uses the INI format and supports multiple `Provider` and `Api` sections.

## Configuration File Format

The configuration file is in INI format and supports the following sections:
- `[Provider1]` / `[Provider2]` / `[Provider3]` ... - Define API providers
- `[Api1]` / `[Api2]` / `[Api3]` ... - Define specific API instances

---

## Provider Section

Provider is used to define the basic information of the LLM API service provider.

### Field Description

| Field Name | Required | Default Value | Description |
|--------|---------|--------|------|
| `name` | **Yes** | - | Unique identifier name for the Provider, cannot be empty |
| `endpoint` |  **Yes** | - | API endpoint address |
| `apiFormat` | No | `OpenAI` | API format type, see available values below |

### apiFormat Available Values

| Value | Description |
|-----|------|
| `OpenAI` | OpenAI format |
| `Anthropic` | Anthropic Claude format |
| `Gemini` | Google Gemini format |
| `OpenRouter` | OpenRouter format |
| `Kimi` | Moonshot Kimi format |
| `GLM` | Zhipu GLM format |
| `Minimax` | Minimax format |
| `DeepSeek` | DeepSeek format |

### Provider Configuration Example

```ini
[Provider]
name=OpenAI
endpoint=https://api.openai.com/v1/chat/completions
apiFormat=OpenAI

[Provider]
name=Anthropic
endpoint=https://api.anthropic.com/v1/message
apiFormat=Anthropic

[Provider]
name=Google AI
endpoint=https://generativelanguage.googleapis.com/v1beta
apiFormat=Gemini

```

---

## Api Section

Api is used to define specific LLM model instances, which need to be linked to a Provider.

### Field Description

| Field Name | Required | Default Value | Description |
|--------|---------|--------|------|
| `name` | **Yes** | - | Unique identifier name for the API instance, cannot be empty |
| `model` | **Yes** | - | Model name (e.g., `gpt-4`, `claude-3-opus`, etc.) |
| `provider` | **Yes** | - | Linked Provider name |
| `purpose` | **Yes** | - | API purpose, multiple purposes are separated by commas |
| `maxToken` | No | 0 | Maximum number of generated tokens (0 indicates using model default) |
| `contextCapacity` | No | 131072 (128K) | The maximum context capacity the model can handle while ensuring high-quality output (in tokens) |
| `priceInputToken` | No | 0.0 | Input token unit price (per 1M tokens) |
| `priceOutputToken` | No | 0.0 | Output token unit price (per 1M tokens) |
| `priceCacheRead` | No | `priceInputToken` | Cache read unit price (per 1M tokens) |
| `priceCacheWrite` | No | `priceOutputToken` | Cache write unit price (per 1M tokens) |
| `thinkingMode` | No | `Auto` | Thinking mode, available values: `Auto`, `Enable`, `Disable` |
| `cacheControl` | No | `Auto` | Cache control type, available values: `Auto`, `Anthropic` |
| `openRouter_only` | No | Empty | OpenRouter specific routing constraints, multiple values are separated by commas |

### Purpose Available Values

| Value | Description |
|-----|------|
| `MajorChat` | Major chat (primary chat function) |
| `MinorChat` | Minor chat (auxiliary chat functions, such as chat summary) |

### ThinkingMode Available Values

| Value | Description |
|-----|------|
| `Auto` | Auto mode |
| `Enable` | Enable thinking mode |
| `Disable` | Disable thinking mode |

### CacheControl Available Values

| Value | Description |
|-----|------|
| `Auto` | Automatically select the cache control method |
| `Anthropic` | Use Anthropic style cache control, only for some LLM aggregator sites like OpenRouter |

### Api Configuration Example

```ini
[Api]
name=claude opus 4.6
model=claude-opus-4-6
provider=Anthropic
priceInputToken=5.0
priceCacheRead=0.5
priceCacheWrite=6.25
priceOutputToken=25.0
purpose=MajorChat

[Api]
name=claude-4.5(openrouter)
model=anthropic/claude-sonnet-4.5
provider=OpenRouter
maxToken=32000
priceInputToken=3.0
priceOutputToken=15.0
priceCacheRead=0.3
priceCacheWrite=3.75
purpose=MajorChat
cacheControl=Anthropic

[Api]
name=llama-3.3-70b(openrouter)
model=meta-llama/llama-3.3-70b-instruct
provider=OpenRouter
maxToken=65536
priceInputToken=0.1
priceOutputToken=0.32
purpose=MinorChat
openRouter_only=novita/bf16,parasail/int8

```

---

## Notes

1. **Name Uniqueness**: The `name` field of Provider and Api must be unique, duplicated names will overwrite previous configurations.

2. **Provider Linkage**: The `provider` field of an Api must match the `name` of an already defined Provider.

3. **Comments**: Comment lines starting with `;` can be used in the configuration file.

4. **List Format**: List fields such as `purpose`, `openRouter_only` use commas to separate multiple values.

5. **Auto Reload**: The configuration file will be automatically reloaded after saving.

---

## System Capability Judgment

The system automatically judges the capability level based on the API configuration:

- **Full (Full Capability)**: Configured with APIs for both `MajorChat` and `MinorChat` purposes.
- **Partial (Partial Capability)**: Not configured with `MinorChat`.
- **CannotWork (Cannot Work)**: No API configured.