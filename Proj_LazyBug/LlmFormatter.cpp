#include "stdh.h"
#include "LlmLib.h"
#include "LlmFormatter.h"
#include <map>

// 将任意字符串规范化为符合 Anthropic tool id 规范的字符串
// Anthropic 要求: ^[a-zA-Z0-9_-]{1,64}$
// 处理策略:
//   1. 将非法字符替换为 '_'
//   2. 若结果为空则使用默认前缀 "tool_"
//   3. 截断到 64 字符以内
static std::string NormalizeAnthropicToolId(const std::string& rawId)
{
	std::string result;
	result.reserve(rawId.size());

	for (char c : rawId)
	{
		if ((c >= 'a' && c <= 'z') ||
			(c >= 'A' && c <= 'Z') ||
			(c >= '0' && c <= '9') ||
			c == '_' || c == '-')
		{
			result += c;
		}
		else
		{
			result += '_';
		}
	}

	if (result.empty())
	{
		result = "tool_0";
	}

	if (result.size() > 64)
	{
		result.resize(64);
	}

	return result;
}

bool CLlmFormatter::ConvertLlmRequestToAnthoropicFormat(json& requestJson)
{
	// 将 OpenAI 格式的 image_url content block 转换为 Anthropic 格式的 image block
	// OpenAI:    { "type": "image_url", "image_url": { "url": "data:image/jpeg;base64,<data>" } }
	//            { "type": "image_url", "image_url": { "url": "https://..." } }
	// Anthropic: { "type": "image", "source": { "type": "base64", "media_type": "image/jpeg", "data": "<data>" } }
	//            { "type": "image", "source": { "type": "url", "url": "https://..." } }
	auto convertContentBlockToAnthropic = [](const json& block) -> json
	{
		if (!block.is_object())
			return block;

		std::string blockType = block.value("type", "");

		if (blockType != "image_url")
			return block; // 非图片 block 原样返回

		json imageBlock;
		imageBlock["type"] = "image";

		if (block.contains("image_url") && block["image_url"].is_object())
		{
			std::string url = block["image_url"].value("url", "");

			// 判断是 base64 data URI 还是普通 URL
			// data URI 格式: "data:<media_type>;base64,<data>"
			const std::string dataUriPrefix = "data:";
			const std::string base64Marker = ";base64,";
			if (url.rfind(dataUriPrefix, 0) == 0)
			{
				// base64 data URI
				size_t base64Pos = url.find(base64Marker);
				if (base64Pos != std::string::npos)
				{
					std::string mediaType = url.substr(dataUriPrefix.size(), base64Pos - dataUriPrefix.size());
					std::string data = url.substr(base64Pos + base64Marker.size());

					json source;
					source["type"] = "base64";
					source["media_type"] = mediaType;
					source["data"] = data;
					imageBlock["source"] = source;
				}
				else
				{
					// 格式异常，原样保留
					return block;
				}
			}
			else
			{
				// 普通 URL
				json source;
				source["type"] = "url";
				source["url"] = url;
				imageBlock["source"] = source;
			}
		}
		else
		{
			// 无法解析，原样保留
			return block;
		}

		// 保留 cache_control（如果有）
		if (block.contains("cache_control"))
		{
			imageBlock["cache_control"] = block["cache_control"];
		}

		return imageBlock;
	};

	try
	{
		json systemMessages = json::array();
		json newMessages = json::array();
		json newTools = json::array();

		// 1. 提取 system messages 并转换为 system 参数
		if (requestJson.contains("messages") && requestJson["messages"].is_array())
		{
			json& messages = requestJson["messages"];

			for (auto& msg : messages)
			{
				if (!msg.is_object() || !msg.contains("role"))
					continue;

				std::string role = msg["role"].get<std::string>();

				if (role == "system")
				{
					// 收集 system 消息
					if (msg.contains("content"))
					{
						if (msg["content"].is_string())
						{
							json systemBlock;
							systemBlock["type"] = "text";
							systemBlock["text"] = msg["content"].get<std::string>();

							// 如果有 cache_control，也添加进去
							if (msg.contains("cache_control"))
							{
								systemBlock["cache_control"] = msg["cache_control"];
							}

							systemMessages.push_back(systemBlock);
						}
						else if (msg["content"].is_array())
						{
							// 如果 content 已经是数组格式，逐块转换后添加
							for (auto& block : msg["content"])
							{
								systemMessages.push_back(convertContentBlockToAnthropic(block));
							}
						}
					}
				}
				else
				{
					// 非 system 消息保留
					// 转换 content 格式
					if (msg.contains("content"))
					{
						if (msg["content"].is_string())
						{
							// 将字符串转换为 Anthropic 的 content blocks 格式
							std::string contentStr = msg["content"].get<std::string>();
							json contentArray = json::array();

							json textBlock;
							textBlock["type"] = "text";
							textBlock["text"] = contentStr;
							contentArray.push_back(textBlock);

							msg["content"] = contentArray;
						}
						else if (msg["content"].is_array())
						{
							// 对数组中的每个 block 进行图片格式转换
							json convertedArray = json::array();
							for (auto& block : msg["content"])
							{
								convertedArray.push_back(convertContentBlockToAnthropic(block));
							}
							msg["content"] = convertedArray;
						}
					}

					// 处理 tool_calls (OpenAI) -> tool_use (Anthropic)
					if (msg.contains("tool_calls") && msg["tool_calls"].is_array())
					{
						json& toolCalls = msg["tool_calls"];
						json contentArray;

						// 如果已有 content，先获取它
						if (msg.contains("content") && msg["content"].is_array())
						{
							contentArray = msg["content"];
						}
						else
						{
							contentArray = json::array();
						}

						// 转换每个 tool_call 为 tool_use block
						for (auto& toolCall : toolCalls)
						{
							if (toolCall.contains("function"))
							{
								json toolUseBlock;
								toolUseBlock["type"] = "tool_use";
								toolUseBlock["id"] = NormalizeAnthropicToolId(toolCall.value("id", ""));
								toolUseBlock["name"] = toolCall["function"].value("name", "");

								// 解析 arguments
								std::string argsStr = toolCall["function"].value("arguments", "{}");
								try
								{
									json argsJson = json::parse(argsStr);
									toolUseBlock["input"] = argsJson;
								}
								catch (...)
								{
									toolUseBlock["input"] = json::object();
								}

								contentArray.push_back(toolUseBlock);
							}
						}

						msg["content"] = contentArray;
						msg.erase("tool_calls");
					}

					// 处理 tool_call_id (OpenAI tool result) -> tool_result (Anthropic)
					if (msg.contains("tool_call_id") && msg.contains("content"))
					{
						json toolResultBlock;
						toolResultBlock["type"] = "tool_result";
						toolResultBlock["tool_use_id"] = NormalizeAnthropicToolId(msg["tool_call_id"].get<std::string>());

						// 获取 content
						if (msg["content"].is_string())
						{
							toolResultBlock["content"] = msg["content"].get<std::string>();
						}
						else if (msg["content"].is_array())
						{
							toolResultBlock["content"] = msg["content"];
						}

						// 如果有 cache_control
						if (msg.contains("cache_control"))
						{
							toolResultBlock["cache_control"] = msg["cache_control"];
						}

						// 创建新的消息结构
						json newMsg;
						newMsg["role"] = "user";
						newMsg["content"] = json::array({ toolResultBlock });

						newMessages.push_back(newMsg);
						continue;
					}

					newMessages.push_back(msg);
				}
			}
		}

		// 2. 转换 tools 格式
		if (requestJson.contains("tools") && requestJson["tools"].is_array())
		{
			json& tools = requestJson["tools"];

			for (auto& tool : tools)
			{
				if (tool.contains("type") && tool["type"] == "function" && tool.contains("function"))
				{
					json anthropicTool;
					anthropicTool["name"] = tool["function"].value("name", "");
					anthropicTool["description"] = tool["function"].value("description", "");

					// 转换 parameters
					if (tool["function"].contains("parameters"))
					{
						anthropicTool["input_schema"] = tool["function"]["parameters"];
					}
					else
					{
						anthropicTool["input_schema"] = {
							{"type", "object"},
							{"properties", json::object()},
							{"required", json::array()}
						};
					}

					// 转换 cache_control (OpenAI format -> Anthropic format)
					if (tool.contains("cache_control"))
					{
						anthropicTool["cache_control"] = tool["cache_control"];
					}
					// 也可以在 function 内部定义 cache_control
					else if (tool["function"].contains("cache_control"))
					{
						anthropicTool["cache_control"] = tool["function"]["cache_control"];
					}

					newTools.push_back(anthropicTool);
				}
			}
		}

		// 3. 转换 tool_choice
		json toolChoiceConverted;
		bool hasToolChoice = false;
		if (requestJson.contains("tool_choice"))
		{
			auto& toolChoice = requestJson["tool_choice"];

			if (toolChoice.is_string())
			{
				std::string choice = toolChoice.get<std::string>();
				if (choice == "none")
				{
					// 不设置 tool_choice
				}
				else if (choice == "auto")
				{
					toolChoiceConverted["type"] = "auto";
					hasToolChoice = true;
				}
				else if (choice == "required")
				{
					toolChoiceConverted["type"] = "any";
					hasToolChoice = true;
				}
			}
			else if (toolChoice.is_object() && toolChoice.contains("type") && toolChoice["type"] == "function")
			{
				if (toolChoice.contains("function") && toolChoice["function"].contains("name"))
				{
					toolChoiceConverted["type"] = "tool";
					toolChoiceConverted["name"] = toolChoice["function"]["name"];
					hasToolChoice = true;
				}
			}
		}

		// 4. 处理其他参数
		// Anthropic 使用 max_tokens 而不是 max_completion_tokens
		json maxTokens;
		if (requestJson.contains("max_tokens"))
		{
			maxTokens = requestJson["max_tokens"];
		}
		else if (requestJson.contains("max_completion_tokens"))
		{
			maxTokens = requestJson["max_completion_tokens"];
		}
		else
		{
			maxTokens = 4096; // 设置默认值
		}

		// 处理 thinking 参数转换
		json thinkingConverted;
		bool hasThinking = false;
		if (requestJson.contains("thinking"))
		{
			auto& thinking = requestJson["thinking"];
			if (thinking.is_object())
			{
				std::string thinkingType = thinking.value("type", "");
				if (thinkingType == "enabled")
				{
				// Anthropic 的 thinking 参数
				thinkingConverted["type"] = "enabled";
				if (thinking.contains("budget_tokens"))
				{
					thinkingConverted["budget_tokens"] = thinking["budget_tokens"];
				}
					hasThinking = true;
				}
				else if (thinkingType == "disabled")
				{
					// Anthropic 不需要显式禁用 extended_thinking，不添加该参数即可
					// 但如果需要显式禁用，可以使用以下代码：
					// thinkingConverted["type"] = "disabled";
					// hasThinking = true;
				}
			}
		}

		// 5. 重新构建 requestJson，确保顺序为 tools -> system -> messages -> 其他参数
		json orderedJson = json::object();

		// 首先添加 model (如果存在)
		if (requestJson.contains("model"))
		{
			orderedJson["model"] = requestJson["model"];
		}

		// 然后添加 tools (如果存在)
		if (!newTools.empty())
		{
			orderedJson["tools"] = newTools;
		}

		// 然后添加 system (如果存在)
		if (!systemMessages.empty())
		{
			orderedJson["system"] = systemMessages;
		}

		// 然后添加 messages
		orderedJson["messages"] = newMessages;

		// 添加 max_tokens
		orderedJson["max_tokens"] = maxTokens;

		// 添加 tool_choice (如果存在)
		if (hasToolChoice)
		{
			orderedJson["tool_choice"] = toolChoiceConverted;
		}

		// 添加 thinking (如果存在)
		if (hasThinking)
		{
			orderedJson["thinking"] = thinkingConverted;
		}

		// 添加其他支持的参数
		const std::vector<std::string> supportedParams = {
			"temperature", "top_p", "top_k", "stop_sequences",
			"stream", "metadata"
		};

		for (const auto& param : supportedParams)
		{
			if (requestJson.contains(param))
			{
				orderedJson[param] = requestJson[param];
			}
		}

		// 用重新排序后的 JSON 替换原 JSON
		requestJson = orderedJson;

		return true;
	}
	catch (const std::exception&)
	{
		// 转换失败
		return false;
	}
}

bool CLlmFormatter::ProcessLlmResponseFromAnthropicFormat(std::deque<std::string>& inputLines, std::vector<std::string>& outputLines, const LlmApi& api)
{
	try
	{
		std::string msgId;
		std::string modelName;

		struct ToolUseInfo { std::string id; std::string name; };
		std::map<int, ToolUseInfo> toolUseBlocks;

		auto emitChunk = [&](json& chunk) {
			outputLines.push_back("data: " + chunk.dump());
		};

		auto processData = [&](const std::string& dataStr)
		{
			if (dataStr.empty() || dataStr == "[DONE]")
				return;
			try
			{
				json data = json::parse(dataStr);
				std::string type = data.value("type", "");

				if (type == "message_start")
				{
					if (data.contains("message"))
					{
						msgId = data["message"].value("id", "");
						modelName = data["message"].value("model", "");

						int inputTokens = 0;
						int outputTokens = 0;
						int originalInputTokens = 0;

						int cacheReadTokens = 0;
						int cacheWriteTokens = 0;
						int msgOutputTokens = 0;

						if (false)
							if (data["message"].contains("usage"))
							{
								originalInputTokens = data["message"]["usage"].value("input_tokens", 0);
								// 读取 cache read 和 cache write 数量
								cacheReadTokens = data["message"]["usage"].value("cache_read_input_tokens", 0);
								cacheWriteTokens = data["message"]["usage"].value("cache_creation_input_tokens", 0);

								// 读取 output_tokens（如果存在，某些场景下 message_start 可能包含）
								msgOutputTokens = data["message"]["usage"].value("output_tokens", 0);
								if (msgOutputTokens > 0)
								{
									outputTokens += msgOutputTokens;
								}

								// 将 cache read 和 cache write 都折算成标准的 input 数量
								// 使用 API 的价格数据进行换算，而不是固定值
								inputTokens = originalInputTokens;
								if (api.priceInputToken > 0.0f)
								{
									float cacheReadEquivalent = cacheReadTokens * (api.priceCacheRead / api.priceInputToken);
									float cacheWriteEquivalent = cacheWriteTokens * (api.priceCacheWrite / api.priceInputToken);
									inputTokens = originalInputTokens + static_cast<int>(cacheReadEquivalent + cacheWriteEquivalent);
								}

								originalInputTokens += cacheReadTokens + cacheWriteTokens;
							}

						json choice;
						choice["index"] = 0;
						choice["delta"] = { {"role", "assistant"}, {"content", ""} };
						choice["finish_reason"] = nullptr;

						json chunk;
						chunk["id"] = msgId;
						chunk["object"] = "chat.completion.chunk";
						chunk["created"] = 0;
						chunk["model"] = modelName;
						chunk["choices"] = json::array({ choice });

						// 添加 usage 字段，包含原始的和折算后的 token 数量
						if (false)
						{
							json usage;
							usage["prompt_tokens"] = originalInputTokens;
							usage["prompt_tokens_equivalent"] = inputTokens;
							usage["completion_tokens"] = outputTokens;
							usage["total_tokens"] = inputTokens + outputTokens;
							chunk["usage"] = usage;
						}

						emitChunk(chunk);
					}
				}
				else if (type == "content_block_start")
				{
					int index = data.value("index", 0);
					if (data.contains("content_block"))
					{
						std::string blockType = data["content_block"].value("type", "");

						if (blockType == "tool_use")
						{
							auto& block = data["content_block"];
							ToolUseInfo info;
							info.id = block.value("id", "");
							info.name = block.value("name", "");
							toolUseBlocks[index] = info;

							json toolCall;
							toolCall["index"] = index;
							toolCall["id"] = info.id;
							toolCall["type"] = "function";
							toolCall["function"] = { {"name", info.name}, {"arguments", ""} };

							json choice;
							choice["index"] = 0;
							choice["delta"] = json::object();
							choice["delta"]["tool_calls"] = json::array({ toolCall });
							choice["finish_reason"] = nullptr;

							json chunk;
							chunk["id"] = msgId;
							chunk["object"] = "chat.completion.chunk";
							chunk["created"] = 0;
							chunk["model"] = modelName;
							chunk["choices"] = json::array({ choice });
							emitChunk(chunk);
						}
						else if (blockType == "thinking")
						{
							// Anthropic extended_thinking 块的开始
							// 发送一个空的 reasoning delta 来标记 thinking 的开始
							json choice;
							choice["index"] = 0;
							choice["delta"] = { {"reasoning", ""} };
							choice["finish_reason"] = nullptr;

							json chunk;
							chunk["id"] = msgId;
							chunk["object"] = "chat.completion.chunk";
							chunk["created"] = 0;
							chunk["model"] = modelName;
							chunk["choices"] = json::array({ choice });
							emitChunk(chunk);
						}
					}
				}
				else if (type == "content_block_delta")
				{
					int index = data.value("index", 0);
					if (data.contains("delta"))
					{
						auto& delta = data["delta"];
						std::string deltaType = delta.value("type", "");

						if (deltaType == "text_delta")
						{
							json choice;
							choice["index"] = 0;
							choice["delta"] = { {"content", delta.value("text", "")} };
							choice["finish_reason"] = nullptr;

							json chunk;
							chunk["id"] = msgId;
							chunk["object"] = "chat.completion.chunk";
							chunk["created"] = 0;
							chunk["model"] = modelName;
							chunk["choices"] = json::array({ choice });
							emitChunk(chunk);
						}
						else if (deltaType == "thinking_delta")
						{
							// Anthropic extended_thinking 的增量内容
							// 转换为 OpenAI 的 reasoning 格式
							json choice;
							choice["index"] = 0;
							choice["delta"] = { {"reasoning", delta.value("thinking", "")} };
							choice["finish_reason"] = nullptr;

							json chunk;
							chunk["id"] = msgId;
							chunk["object"] = "chat.completion.chunk";
							chunk["created"] = 0;
							chunk["model"] = modelName;
							chunk["choices"] = json::array({ choice });
							emitChunk(chunk);
						}
						else if (deltaType == "input_json_delta")
						{
							json toolCall;
							toolCall["index"] = index;
							toolCall["function"] = { {"arguments", delta.value("partial_json", "")} };

							json choice;
							choice["index"] = 0;
							choice["delta"] = json::object();
							choice["delta"]["tool_calls"] = json::array({ toolCall });
							choice["finish_reason"] = nullptr;

							json chunk;
							chunk["id"] = msgId;
							chunk["object"] = "chat.completion.chunk";
							chunk["created"] = 0;
							chunk["model"] = modelName;
							chunk["choices"] = json::array({ choice });
							emitChunk(chunk);
						}
					}
				}
				else if (type == "message_delta")
				{
					std::string stopReason;

					int inputTokens = 0;
					int outputTokens = 0;
					int originalInputTokens = 0;

					int cacheReadTokens = 0;
					int cacheWriteTokens = 0;
					int msgOutputTokens = 0;

					if (data.contains("delta"))
					{
						std::string s = data["delta"].value("stop_reason", "");
						if (s == "end_turn" || s == "stop_sequence")
							stopReason = "stop";
						else if (s == "tool_use")
							stopReason = "tool_calls";
						else if (s == "max_tokens")
							stopReason = "length";
						else
							stopReason = s;
					}
					if (data.contains("usage"))
					{
						originalInputTokens = data["usage"].value("input_tokens", 0);
						// 读取 cache read 和 cache write 数量
						cacheReadTokens = data["usage"].value("cache_read_input_tokens", 0);
						cacheWriteTokens = data["usage"].value("cache_creation_input_tokens", 0);

						outputTokens = data["usage"].value("output_tokens", 0);

						// 将 cache read 和 cache write 都折算成标准的 input 数量
						// 使用 API 的价格数据进行换算，而不是固定值
						inputTokens = originalInputTokens;
						if (api.priceInputToken > 0.0f)
						{
							float cacheReadEquivalent = cacheReadTokens * (api.priceCacheRead / api.priceInputToken);
							float cacheWriteEquivalent = cacheWriteTokens * (api.priceCacheWrite / api.priceInputToken);
							inputTokens = originalInputTokens + static_cast<int>(cacheReadEquivalent + cacheWriteEquivalent);
						}

						originalInputTokens += cacheReadTokens + cacheWriteTokens;
					}

					json choice;
					choice["index"] = 0;
					choice["delta"] = json::object();
					choice["finish_reason"] = stopReason;

					json chunk;
					chunk["id"] = msgId;
					chunk["object"] = "chat.completion.chunk";
					chunk["created"] = 0;
					chunk["model"] = modelName;
					chunk["choices"] = json::array({ choice });

					chunk["usage"] = {
						{"prompt_tokens", originalInputTokens},
						{"prompt_tokens_equivalent", inputTokens},
						{"completion_tokens", outputTokens},
						{"total_tokens", inputTokens + outputTokens}
					};
					emitChunk(chunk);
				}
				else if (type == "message_stop")
				{
					outputLines.push_back("data: [DONE]");
				}
				// ping / content_block_stop: skip
			}
			catch (const json::parse_error&) {}
		};

		while (!inputLines.empty())
		{
			const std::string& front = inputLines.front();

			// consume empty lines
			if (front.empty())
			{
				inputLines.pop_front();
				continue;
			}

			// error lines: directly transfer to output
			if (front.find("{\"error\": ", 0) == 0)
			{
				outputLines.push_back(front);
				inputLines.pop_front();
				continue;
			}

			if (front.rfind("event: ", 0) == 0)
			{
				// Look ahead for the matching data: line
				size_t dataIdx = 1;
				while (dataIdx < inputLines.size())
				{
					if (!inputLines[dataIdx].empty())
						break;
					dataIdx++;
				}

				if (dataIdx >= inputLines.size())
				{
					// No data: line yet — leave the event: line and stop
					break;
				}

				const std::string& dataLine = inputLines[dataIdx];

				if (dataLine.rfind("data: ", 0) != 0)
				{
					// Next non-empty line is not data: (malformed), consume the event: line
					inputLines.pop_front();
					continue;
				}

				// Complete event: consume event line + empty lines + data line
				std::string data = dataLine.substr(6);
				for (size_t i = 0; i <= dataIdx; i++)
					inputLines.pop_front();

				processData(data);
				continue;
			}

			if (front.rfind("data: ", 0) == 0)
			{
				std::string data = front.substr(6);
				inputLines.pop_front();
				processData(data);
				continue;
			}

			// Unknown line, consume it
			inputLines.pop_front();
		}

		return true;
	}
	catch (const std::exception&)
	{
		return false;
	}
}

bool CLlmFormatter::ConvertLlmRequestToGeminiFormat(json& requestJson)
{
	try
	{
		json systemInstruction;
		json newContents = json::array();
		json newTools = json::array();

		// 1. 提取 system messages 并转换为 systemInstruction
		if (requestJson.contains("messages") && requestJson["messages"].is_array())
		{
			json& messages = requestJson["messages"];

			for (auto& msg : messages)
			{
				if (!msg.is_object() || !msg.contains("role"))
					continue;

				std::string role = msg["role"].get<std::string>();

				if (role == "system")
				{
					// 收集 system 消息
					if (msg.contains("content"))
					{
						if (msg["content"].is_string())
						{
							// Gemini 的 systemInstruction 格式
							if (!systemInstruction.contains("parts"))
							{
								systemInstruction["parts"] = json::array();
							}

							json part;
							part["text"] = msg["content"].get<std::string>();
							systemInstruction["parts"].push_back(part);
						}
					}
				}
				else
				{
					// 非 system 消息 - 转换为 Gemini 的 contents 格式
					json geminiContent;

					// 转换角色名称: assistant -> model
					if (role == "assistant")
					{
						geminiContent["role"] = "model";
					}
					else if (role == "user")
					{
						geminiContent["role"] = "user";
					}
					else if (role == "tool")
					{
						// Gemini 的工具结果也使用 user 角色
						geminiContent["role"] = "user";
					}
					else
					{
						geminiContent["role"] = role;
					}

					json parts = json::array();

					// 处理 content
					if (msg.contains("content"))
					{
						if (msg["content"].is_string() && !msg["content"].get<std::string>().empty())
						{
							json part;
							part["text"] = msg["content"].get<std::string>();
							if (msg.contains("thoughtSignature"))
							{
								part["thoughtSignature"] = msg["thoughtSignature"];
							}
							parts.push_back(part);
						}
						else if (msg["content"].is_array())
						{
							// 已经是数组格式，转换每个 block
							for (auto& block : msg["content"])
							{
								if (block.contains("type"))
								{
									std::string blockType = block["type"].get<std::string>();
									if (blockType == "text")
									{
										json part;
										part["text"] = block.value("text", "");
										if (block.contains("thoughtSignature"))
										{
											part["thoughtSignature"] = block["thoughtSignature"];
										}
										parts.push_back(part);
									}
									else if (blockType == "image_url")
									{
										if (block.contains("image_url") && block["image_url"].is_object())
										{
											std::string url = block["image_url"].value("url", "");
											const std::string dataUriPrefix = "data:";
											const std::string base64Marker = ";base64,";

											if (url.rfind(dataUriPrefix, 0) == 0)
											{
												size_t base64Pos = url.find(base64Marker);
												if (base64Pos != std::string::npos)
												{
													std::string mimeType = url.substr(dataUriPrefix.size(), base64Pos - dataUriPrefix.size());
													std::string data = url.substr(base64Pos + base64Marker.size());

													json part;
													part["inlineData"]["mimeType"] = mimeType;
													part["inlineData"]["data"] = data;
													parts.push_back(part);
												}
											}
											else
											{
												std::string mimeType = "image/jpeg";
												std::string lowerUrl = url;
												for (char& c : lowerUrl) {
													if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
												}
												if (lowerUrl.find(".png") != std::string::npos) mimeType = "image/png";
												else if (lowerUrl.find(".webp") != std::string::npos) mimeType = "image/webp";
												else if (lowerUrl.find(".gif") != std::string::npos) mimeType = "image/gif";
												else if (lowerUrl.find(".heic") != std::string::npos) mimeType = "image/heic";
												else if (lowerUrl.find(".heif") != std::string::npos) mimeType = "image/heif";

												json part;
												part["fileData"]["mimeType"] = mimeType;
												part["fileData"]["fileUri"] = url;
												parts.push_back(part);
											}
										}
									}
								}
							}
						}
					}

					// 处理 tool_calls (OpenAI) -> functionCall (Gemini)
					if (msg.contains("tool_calls") && msg["tool_calls"].is_array())
					{
						for (auto& toolCall : msg["tool_calls"])
						{
							if (toolCall.contains("function"))
							{
								json functionCallPart;

								json functionCall;
								functionCall["name"] = toolCall["function"].value("name", "");

								// 解析 arguments
								std::string argsStr = toolCall["function"].value("arguments", "{}");
								try
								{
									json argsJson = json::parse(argsStr);
									functionCall["args"] = argsJson;
								}
								catch (...)
								{
									functionCall["args"] = json::object();
								}

								functionCallPart["functionCall"] = functionCall;
								if (toolCall.contains("thoughtSignature") && !toolCall["thoughtSignature"].get<std::string>().empty())
								{
									functionCallPart["thoughtSignature"] = toolCall["thoughtSignature"];
								}
								else
								{
									functionCallPart["thoughtSignature"] = "skip_thought_signature_validator";
								}

								parts.push_back(functionCallPart);
							}
						}
					}

					// 处理 tool_call_id (OpenAI tool result) -> functionResponse (Gemini)
					if (msg.contains("tool_call_id") && msg.contains("content"))
					{
						json functionResponsePart;

						json functionResponse;
						functionResponse["name"] = msg.value("name", "");

						json response;
						if (msg["content"].is_string())
						{
							response["result"] = msg["content"].get<std::string>();
						}
						else
						{
							response["result"] = msg["content"];
						}

						functionResponse["response"] = response;
						functionResponsePart["functionResponse"] = functionResponse;

						// 将 functionResponse 添加到前一个 content 或创建新的 user content
						if (!newContents.empty() && newContents.back()["role"] == "user")
						{
							newContents.back()["parts"].push_back(functionResponsePart);
							continue;
						}
						else
						{
							json userContent;
							userContent["role"] = "user";
							userContent["parts"] = json::array({ functionResponsePart });
							newContents.push_back(userContent);
							continue;
						}
					}

					if (!parts.empty())
					{
						geminiContent["parts"] = parts;
						newContents.push_back(geminiContent);
					}
				}
			}
		}

		// 2. 转换 tools 格式
		if (requestJson.contains("tools") && requestJson["tools"].is_array())
		{
			json& tools = requestJson["tools"];

			for (auto& tool : tools)
			{
				if (tool.contains("type") && tool["type"] == "function" && tool.contains("function"))
				{
					json geminiFunction;
					geminiFunction["name"] = tool["function"].value("name", "");
					geminiFunction["description"] = tool["function"].value("description", "");

					// 转换 parameters
					if (tool["function"].contains("parameters"))
					{
						geminiFunction["parameters"] = tool["function"]["parameters"];
					}
					else
					{
						geminiFunction["parameters"] = {
							{"type", "object"},
							{"properties", json::object()},
							{"required", json::array()}
						};
					}

					json geminiTool;
					geminiTool["functionDeclarations"] = json::array({ geminiFunction });
					newTools.push_back(geminiTool);
				}
			}
		}

		// 3. 转换 tool_choice
		json toolConfigConverted;
		bool hasToolConfig = false;
		if (requestJson.contains("tool_choice"))
		{
			auto& toolChoice = requestJson["tool_choice"];

			if (toolChoice.is_string())
			{
				std::string choice = toolChoice.get<std::string>();
				if (choice == "none")
				{
					toolConfigConverted["functionCallingConfig"]["mode"] = "NONE";
					hasToolConfig = true;
				}
				else if (choice == "auto")
				{
					toolConfigConverted["functionCallingConfig"]["mode"] = "AUTO";
					hasToolConfig = true;
				}
				else if (choice == "required")
				{
					toolConfigConverted["functionCallingConfig"]["mode"] = "ANY";
					hasToolConfig = true;
				}
			}
			else if (toolChoice.is_object() && toolChoice.contains("type") && toolChoice["type"] == "function")
			{
				if (toolChoice.contains("function") && toolChoice["function"].contains("name"))
				{
					toolConfigConverted["functionCallingConfig"]["mode"] = "ANY";
					toolConfigConverted["functionCallingConfig"]["allowedFunctionNames"] = json::array({ toolChoice["function"]["name"] });
					hasToolConfig = true;
				}
			}
		}

		// 4. 处理其他参数
		json generationConfig = json::object();

		// temperature
		if (requestJson.contains("temperature"))
		{
			generationConfig["temperature"] = requestJson["temperature"];
		}

		// top_p
		if (requestJson.contains("top_p"))
		{
			generationConfig["topP"] = requestJson["top_p"];
		}

		// top_k
		if (requestJson.contains("top_k"))
		{
			generationConfig["topK"] = requestJson["top_k"];
		}

		// max_tokens
		if (requestJson.contains("max_tokens"))
		{
			generationConfig["maxOutputTokens"] = requestJson["max_tokens"];
		}

		// stop sequences
		if (requestJson.contains("stop"))
		{
			generationConfig["stopSequences"] = requestJson["stop"];
		}

		// 5. 重新构建 requestJson
		json orderedJson = json::object();

		// 添加 systemInstruction (如果存在)
		if (!systemInstruction.empty())
		{
			orderedJson["systemInstruction"] = systemInstruction;
		}

		// 添加 tools (如果存在)
		if (!newTools.empty())
		{
			// Gemini 需要将所有 functionDeclarations 合并到一个 tools 数组中
			json allFunctionDeclarations = json::array();
			for (auto& tool : newTools)
			{
				if (tool.contains("functionDeclarations") && tool["functionDeclarations"].is_array())
				{
					for (auto& func : tool["functionDeclarations"])
					{
						allFunctionDeclarations.push_back(func);
					}
				}
			}

			if (!allFunctionDeclarations.empty())
			{
				json geminiTools = json::array();
				json toolItem;
				toolItem["functionDeclarations"] = allFunctionDeclarations;
				geminiTools.push_back(toolItem);
				orderedJson["tools"] = geminiTools;
			}
		}

		// 添加 toolConfig (如果存在)
		if (hasToolConfig)
		{
			orderedJson["toolConfig"] = toolConfigConverted;
		}

		// 添加 generationConfig (如果有配置)
		if (!generationConfig.empty())
		{
			orderedJson["generationConfig"] = generationConfig;
		}

		// 添加 contents
		orderedJson["contents"] = newContents;

		// 用重新排序后的 JSON 替换原 JSON
		requestJson = orderedJson;

		return true;
	}
	catch (const std::exception&)
	{
		// 转换失败
		return false;
	}
}

bool CLlmFormatter::ProcessLlmResponseFromGeminiFormat(std::deque<std::string>& inputLines, std::vector<std::string>& outputLines, const LlmApi& api)
{
	try
	{
		std::string msgId = "gemini-" + std::to_string(std::time(nullptr));
		std::string modelName;

		auto emitChunk = [&](json& chunk) {
			outputLines.push_back("data: " + chunk.dump());
		};

		auto processData = [&](const std::string& dataStr)
		{
			if (dataStr.empty() || dataStr == "[DONE]")
				return;
			try
			{
				json data = json::parse(dataStr);

				// Gemini 响应格式
				if (data.contains("candidates") && data["candidates"].is_array() && !data["candidates"].empty())
				{
					auto& candidate = data["candidates"][0];

					if (candidate.contains("content") && candidate["content"].contains("parts"))
					{
						auto& parts = candidate["content"]["parts"];

						for (auto& part : parts)
						{
							// 处理文本内容
							if (part.contains("text"))
							{
								json choice;
								choice["index"] = 0;

								json delta = { {"content", part["text"].get<std::string>()} };
								if (part.contains("thoughtSignature"))
								{
									delta["thoughtSignature"] = part["thoughtSignature"];
								}

								choice["delta"] = delta;
								choice["finish_reason"] = nullptr;

								json chunk;
								chunk["id"] = msgId;
								chunk["object"] = "chat.completion.chunk";
								chunk["created"] = 0;
								chunk["model"] = modelName;
								chunk["choices"] = json::array({ choice });
								emitChunk(chunk);
							}
							// 处理函数调用
							else if (part.contains("functionCall"))
							{
								auto& functionCall = part["functionCall"];

								json toolCall;
								toolCall["index"] = 0;
								toolCall["id"] = "call_" + std::to_string(std::time(nullptr));
								toolCall["type"] = "function";

								json function;
								function["name"] = functionCall.value("name", "");
								function["arguments"] = functionCall.value("args", json::object()).dump();
								toolCall["function"] = function;

								if (part.contains("thoughtSignature"))
								{
									toolCall["thoughtSignature"] = part["thoughtSignature"];
								}

								json choice;
								choice["index"] = 0;
								choice["delta"] = json::object();
								choice["delta"]["tool_calls"] = json::array({ toolCall });
								choice["finish_reason"] = nullptr;

								json chunk;
								chunk["id"] = msgId;
								chunk["object"] = "chat.completion.chunk";
								chunk["created"] = 0;
								chunk["model"] = modelName;
								chunk["choices"] = json::array({ choice });
								emitChunk(chunk);
							}
						}
					}

					// 处理 finish_reason
					if (candidate.contains("finishReason"))
					{
						std::string finishReason = candidate["finishReason"].get<std::string>();
						std::string openaiFinishReason;

						if (finishReason == "STOP")
							openaiFinishReason = "stop";
						else if (finishReason == "MAX_TOKENS")
							openaiFinishReason = "length";
						else if (finishReason == "SAFETY")
							openaiFinishReason = "content_filter";
						else
							openaiFinishReason = "stop";

						json choice;
						choice["index"] = 0;
						choice["delta"] = json::object();
						choice["finish_reason"] = openaiFinishReason;

						json chunk;
						chunk["id"] = msgId;
						chunk["object"] = "chat.completion.chunk";
						chunk["created"] = 0;
						chunk["model"] = modelName;
						chunk["choices"] = json::array({ choice });
						emitChunk(chunk);
					}
				}

				// 处理 token 使用统计
				if (data.contains("usageMetadata"))
				{
					auto& usage = data["usageMetadata"];
					int originalPromptTokens = usage.value("promptTokenCount", 0);
					int completionTokens = usage.value("candidatesTokenCount", 0);
					int cacheReadTokens = usage.value("cachedContentTokenCount", 0);

					// 将 cache read 折算成标准的 input 数量
					// 使用 API 的价格数据进行换算，而不是固定值
					int promptTokens = originalPromptTokens;
					if (promptTokens > cacheReadTokens && api.priceInputToken > 0.0f)
					{
						float cacheReadEquivalent = cacheReadTokens * (api.priceCacheRead / api.priceInputToken);
						promptTokens = (promptTokens - cacheReadTokens) + static_cast<int>(cacheReadEquivalent);
					}

					json chunk;
					chunk["id"] = msgId;
					chunk["object"] = "chat.completion.chunk";
					chunk["created"] = 0;
					chunk["model"] = modelName;
					chunk["choices"] = json::array();
					chunk["usage"] = {
						{"prompt_tokens", originalPromptTokens},
						{"prompt_tokens_equivalent", promptTokens},
						{"completion_tokens", completionTokens},
						{"total_tokens", promptTokens + completionTokens},
					};
					emitChunk(chunk);
				}

				// 提取模型名称（如果存在）
				if (data.contains("modelVersion"))
				{
					modelName = data["modelVersion"].get<std::string>();
				}
			}
			catch (const json::parse_error&) {}
		};

		while (!inputLines.empty())
		{
			const std::string& front = inputLines.front();

			// consume empty lines
			if (front.empty())
			{
				inputLines.pop_front();
				continue;
			}

			// error lines: directly transfer to output
			if (front.find("{\"error\":", 0) == 0)
			{
				outputLines.push_back(front);
				inputLines.pop_front();
				continue;
			}

			if (front.rfind("data: ", 0) == 0)
			{
				std::string data = front.substr(6);
				inputLines.pop_front();
				processData(data);
				continue;
			}

			// Unknown line, consume it
			inputLines.pop_front();
		}

		// 发送结束标记
		outputLines.push_back("data: [DONE]");

		return true;
	}
	catch (const std::exception&)
	{
		return false;
	}
}


bool CLlmFormatter::ConvertLlmRequestToOpenAiCompatibleFormat(json& requestJson, LlmApiFormat fmt)
{
	try
	{
		// 遍历 messages，清除所有 thoughtSignature 字段
		if (requestJson.contains("messages") && requestJson["messages"].is_array())
		{
			json& messages = requestJson["messages"];

			for (auto& msg : messages)
			{
				if (!msg.is_object())
					continue;

				// 清除 message 级别的 thoughtSignature
				if (msg.contains("thoughtSignature"))
				{
					msg.erase("thoughtSignature");
				}

				// 处理 content 中的 thoughtSignature
				if (msg.contains("content"))
				{
					if (msg["content"].is_array())
					{
						// content 是数组格式，遍历每个 block
						for (auto& block : msg["content"])
						{
							if (block.is_object() && block.contains("thoughtSignature"))
							{
								block.erase("thoughtSignature");
							}
						}
					}
				}

				// 处理 tool_calls 中的 thoughtSignature
				if (msg.contains("tool_calls") && msg["tool_calls"].is_array())
				{
					for (auto& toolCall : msg["tool_calls"])
					{
						if (toolCall.is_object() && toolCall.contains("thoughtSignature"))
						{
							toolCall.erase("thoughtSignature");
						}
					}
				}
			}
		}

		return true;
	}
	catch (const std::exception&)
	{
		return false;
	}
}

bool CLlmFormatter::ProcessLlmResponseFromOpenAiCompatibleFormat(std::deque<std::string>& inputLines, std::vector<std::string>& outputLines, const LlmApi& api)
{
	try
	{
		auto emitChunk = [&](json& chunk) {
			outputLines.push_back("data: " + chunk.dump());
		};

		auto processData = [&](const std::string& dataStr)
		{
			if (dataStr.empty() || dataStr == "[DONE]")
			{
				outputLines.push_back("data: " + dataStr);
				return;
			}
			try
			{
				json data = json::parse(dataStr);

				json* usage = nullptr;
				if (data.contains("usage") && data["usage"].is_object())
					usage = &data["usage"];
				else if (data.contains("choices") && data["choices"].is_array())
				{
					for (auto& choice : data["choices"])
					{
						if (choice.contains("usage") && choice["usage"].is_object())
						{
							usage = &choice["usage"];
							break;
						}
					}
				}

				// 处理 usage 字段中的 cache token
				if (usage != nullptr && api.priceInputToken > 0.0f)
				{
					int originalPromptTokens = usage->value("prompt_tokens", 0);
					int cacheReadTokens = 0;

					// OpenAI / GLM / Kimi 兼容格式优先读取 usage.prompt_tokens_details.cached_tokens
					if (usage->contains("prompt_tokens_details") && (*usage)["prompt_tokens_details"].is_object())
					{
						cacheReadTokens = (*usage)["prompt_tokens_details"].value("cached_tokens", 0);
					}
					// Kimi 兼容字段: usage.cached_tokens
					else if (usage->contains("cached_tokens"))
					{
						cacheReadTokens = usage->value("cached_tokens", 0);
					}

					// 如果有 cache read tokens，进行换算并更新 prompt_tokens
					if (cacheReadTokens > 0)
					{
						float cacheReadEquivalent = cacheReadTokens * (api.priceCacheRead / api.priceInputToken);
						int promptTokensEquivalent = originalPromptTokens - cacheReadTokens + static_cast<int>(cacheReadEquivalent);
						(*usage)["prompt_tokens_equivalent"] = promptTokensEquivalent;
						(*usage)["total_tokens"] = promptTokensEquivalent + usage->value("completion_tokens", 0);
					}
					else
					{
						(*usage)["prompt_tokens_equivalent"] = originalPromptTokens;
						(*usage)["total_tokens"] = originalPromptTokens + usage->value("completion_tokens", 0);
					}
				}

				// 处理流式响应中的 usage（某些 API 在最后一个 chunk 中返回 usage）
				if (data.contains("choices") && data["choices"].is_array())
				{
					// 流式响应，直接传递
					outputLines.push_back("data: " + data.dump());
				}
				else
				{
					// 非流式响应
					emitChunk(data);
				}
			}
			catch (const json::parse_error&)
			{
				// 解析失败，直接传递原始数据
				outputLines.push_back("data: " + dataStr);
			}
		};

		while (!inputLines.empty())
		{
			const std::string& front = inputLines.front();

			// consume empty lines
			if (front.empty())
			{
				inputLines.pop_front();
				continue;
			}

			// error lines: directly transfer to output
			if (front.find("{\"error\":", 0) == 0 || front.find("{\"error\": ", 0) == 0)
			{
				outputLines.push_back(front);
				inputLines.pop_front();
				continue;
			}

			if (front.rfind("data: ", 0) == 0)
			{
				std::string data = front.substr(6);
				inputLines.pop_front();
				processData(data);
				continue;
			}

			// 非 SSE 格式的 JSON 数据（非流式响应）
			if (front.rfind("{", 0) == 0)
			{
				std::string data = front;
				inputLines.pop_front();
				processData(data);
				continue;
			}

			// Unknown line, consume it
			inputLines.pop_front();
		}

		return true;
	}
	catch (const std::exception&)
	{
		return false;
	}
}
