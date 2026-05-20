# LLM 配置文件说明书

## 概述

本配置文件用于设置 LLM（大语言模型）API 提供商和 API 实例的相关参数。配置文件采用 INI 格式，支持多个 `Provider` 和 `Api` 配置段。

## 配置文件格式

配置文件为 INI 格式，支持以下配置段：
- `[Provider1]` / `[Provider2]` / `[Provider3]` ... - 定义 API 提供商
- `[Api1]` / `[Api2]` / `[Api3]` ... - 定义具体的 API 实例

---

## Provider 配置段

Provider 用于定义 LLM API 服务提供商的基本信息。

### 字段说明

| 字段名 | 是否必填 | 默认值 | 说明 |
|--------|---------|--------|------|
| `name` | **是** | - | Provider 的唯一标识名称，不可为空 |
| `endpoint` |  **是** | - | API 端点地址 |
| `apiFormat` | 否 | `OpenAI` | API 格式类型，可选值见下方 |

### apiFormat 可选值

| 值 | 说明 |
|-----|------|
| `OpenAI` | OpenAI 格式 |
| `Anthropic` | Anthropic Claude 格式 |
| `Gemini` | Google Gemini 格式 |
| `OpenRouter` | OpenRouter 格式 |
| `Kimi` | Moonshot Kimi 格式 |
| `GLM` | 智谱 GLM 格式 |
| `Minimax` | Minimax 格式 |
| `DeepSeek` | DeepSeek 格式 |

### Provider 配置示例

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

## Api 配置段

Api 用于定义具体的 LLM 模型实例，需要关联到一个 Provider。

### 字段说明

| 字段名 | 是否必填 | 默认值 | 说明 |
|--------|---------|--------|------|
| `name` | **是** | - | API 实例的唯一标识名称，不可为空 |
| `model` | **是** | - | 模型名称（如 `gpt-4`、`claude-3-opus` 等） |
| `provider` | **是** | - | 关联的 Provider 名称 |
| `purpose` | **是** | - | API 用途，多个用途用逗号分隔 |
| `maxToken` | 否 | 0 | 最大生成 token 数（0 表示使用模型默认值） |
| `contextCapacity` | 否 | 131072 (128K) | 模型能够保证高质量输出的最大上下文容量（以 token 为单位） |
| `priceInputToken` | 否 | 0.0 | 输入 token 单价（每 1M tokens） |
| `priceOutputToken` | 否 | 0.0 | 输出 token 单价（每 1M tokens） |
| `priceCacheRead` | 否 | `priceInputToken` | 缓存读取单价（每 1M tokens） |
| `priceCacheWrite` | 否 | `priceOutputToken` | 缓存写入单价（每 1M tokens） |
| `thinkingMode` | 否 | `Auto` | 思考模式，可选值：`Auto`、`Enable`、`Disable` |
| `cacheControl` | 否 | `Auto` | 缓存控制类型，可选值：`Auto`、`Anthropic` |
| `openRouter_only` | 否 | 空 | OpenRouter 专用路由限制，多个值用逗号分隔 |

### Purpose 可选值

| 值 | 说明 |
|-----|------|
| `MajorChat` | 主要对话（主聊天功能） |
| `MinorChat` | 次要对话（辅助性的对话功能,比如聊天摘要） |

### ThinkingMode 可选值

| 值 | 说明 |
|-----|------|
| `Auto` | 自动模式 |
| `Enable` | 启用思考模式 |
| `Disable` | 禁用思考模式 |

### CacheControl 可选值

| 值 | 说明 |
|-----|------|
| `Auto` | 自动选择缓存控制方式 |
| `Anthropic` | 使用 Anthropic 风格的缓存控制,仅用于一些LLM聚合网站,比如OpenRouter |

### Api 配置示例

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

## 注意事项

1. **名称唯一性**：Provider 和 Api 的 `name` 字段必须唯一，重复的名称会覆盖之前的配置。

2. **Provider 关联**：Api 的 `provider` 字段必须与某个已定义的 Provider 的 `name` 匹配。

3. **注释**：配置文件中可以使用 `;` 开头的注释行。

4. **列表格式**：`purpose`、`openRouter_only` 等列表字段使用逗号分隔多个值。

5. **自动重载**：配置文件保存后会自动重新加载。

---

## 系统能力判定

系统根据 API 配置自动判定工作能力等级：

- **Full（完整能力）**：配置了 `MajorChat`、`MinorChat` 两种用途的 API
- **Partial（部分能力）**：未配置 `MinorChat`
- **CannotWork（无法工作）**：未配置 任何API