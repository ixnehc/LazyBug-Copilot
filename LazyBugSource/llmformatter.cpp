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

// 尝试将字符串解析为JSON，检查是否包含 "error" 字段
static bool IsJsonErrorLine(const std::string& line)
{
	try
	{
		json j = json::parse(line);
		return j.contains("error");
	}
	catch (const json::parse_error&)
	{
		return false;
	}
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
								cacheReadTokens = data["message"]["usage"].value("cache_read_input_tokens", 0);
								cacheWriteTokens = data["message"]["usage"].value("cache_creation_input_tokens", 0);

								msgOutputTokens = data["message"]["usage"].value("output_tokens", 0);
								if (msgOutputTokens > 0)
								{
									outputTokens += msgOutputTokens;
								}

								inputTokens = originalInputTokens;
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
							usage["prompt_tokens_cacheRead"] = cacheReadTokens;
							usage["prompt_tokens_cacheWrite"] = cacheWriteTokens;
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
						cacheReadTokens = data["usage"].value("cache_read_input_tokens", 0);
						cacheWriteTokens = data["usage"].value("cache_creation_input_tokens", 0);
						outputTokens = data["usage"].value("output_tokens", 0);
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
							{"prompt_tokens_cacheRead", cacheReadTokens},
							{"prompt_tokens_cacheWrite", cacheWriteTokens},
							{"completion_tokens", outputTokens},
							{"total_tokens", originalInputTokens + cacheReadTokens + cacheWriteTokens + outputTokens}
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

			// error lines: try to parse JSON and check for error field, then directly transfer to output
			if (IsJsonErrorLine(front))
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

					json chunk;
					chunk["id"] = msgId;
					chunk["object"] = "chat.completion.chunk";
					chunk["created"] = 0;
					chunk["model"] = modelName;
					chunk["choices"] = json::array();
					chunk["usage"] = {
						{"prompt_tokens", originalPromptTokens},
						{"prompt_tokens_cacheRead", cacheReadTokens},
						{"prompt_tokens_cacheWrite", 0},
						{"completion_tokens", completionTokens},
						{"total_tokens", originalPromptTokens + cacheReadTokens + completionTokens},
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

			// error lines: try to parse JSON and check for error field, then directly transfer to output
			if (IsJsonErrorLine(front))
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
				if (usage != nullptr)
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

					// prompt_tokens 包含了 cached tokens，需要减去得到实际非缓存输入量
					if (cacheReadTokens > 0)
					{
						(*usage)["prompt_tokens"] = originalPromptTokens - cacheReadTokens;
					}

					(*usage)["prompt_tokens_cacheRead"] = cacheReadTokens;
					(*usage)["prompt_tokens_cacheWrite"] = 0;
					(*usage)["total_tokens"] = usage->value("prompt_tokens", 0) + cacheReadTokens + usage->value("completion_tokens", 0);
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

			// error lines: try to parse JSON and check for error field, then directly transfer to output
			if (IsJsonErrorLine(front))
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

//////////////////////////////////////////////////////////////////////////
// OpenAI Responses API
//////////////////////////////////////////////////////////////////////////

// 将 Chat Completions 的 content block 转换为 Responses API 的 content block
static json ConvertContentBlockToResponses(const json& block, bool isAssistant)
{
	json result = json::object();
	std::string type = block.value("type", "");

	if (type == "text")
	{
		result["type"] = isAssistant ? "output_text" : "input_text";
		result["text"] = block.value("text", "");
	}
	else if (type == "image_url")
	{
		result["type"] = "input_image";
		if (block.contains("image_url") && block["image_url"].is_object())
			result["image_url"] = block["image_url"].value("url", "");
		else if (block.contains("image_url") && block["image_url"].is_string())
			result["image_url"] = block["image_url"].get<std::string>();
	}
	return result;
}

// 将 Chat Completions tools 定义转换为 Responses tools 定义
static json ConvertToolsToResponsesFormat(const json& tools)
{
	json result = json::array();
	for (const auto& tool : tools)
	{
		if (!tool.is_object())
			continue;

		std::string type = tool.value("type", "");
		if (type == "function" && tool.contains("function"))
		{
			const auto& fn = tool["function"];
			json respTool;
			respTool["type"] = "function";
			respTool["name"] = fn.value("name", "");
			respTool["description"] = fn.value("description", "");
			if (fn.contains("parameters"))
				respTool["parameters"] = fn["parameters"];
			result.push_back(respTool);
		}
		else
		{
			// 其他类型工具（如内置工具）直接透传
			result.push_back(tool);
		}
	}
	return result;
}

void CLlmFormatter::CleanupTempFields(json& requestJson)
{
	// 清理 messages 数组中的临时标记
	if (requestJson.contains("messages") && requestJson["messages"].is_array())
	{
		for (auto& msg : requestJson["messages"])
		{
			if (msg.is_object())
				msg.erase("_current_turn");
		}
	}

	// 清理 input 数组（Responses 格式）中的临时标记
	if (requestJson.contains("input") && requestJson["input"].is_array())
	{
		for (auto& item : requestJson["input"])
		{
			if (item.is_object())
				item.erase("_current_turn");
		}
	}
}

// 从 system messages 提取 instructions（两种模式共用）
static std::string ExtractResponsesInstructions(const json& messages)
{
	std::string instructions;
	for (const auto& msg : messages)
	{
		if (!msg.is_object() || msg.value("role", "") != "system")
			continue;

		if (!msg.contains("content"))
			continue;

		std::string text;
		if (msg["content"].is_string())
			text = msg["content"].get<std::string>();
		else if (msg["content"].is_array())
		{
			for (const auto& block : msg["content"])
			{
				if (block.is_object() && block.value("type", "") == "text")
				{
					if (!text.empty()) text += "\n\n";
					text += block.value("text", "");
				}
			}
		}
		if (!text.empty())
		{
			if (!instructions.empty()) instructions += "\n\n";
			instructions += text;
		}
	}
	return instructions;
}

// 公共后处理：tools 转换、max_tokens→max_output_tokens、reasoning_effort→reasoning.effort、清理 thinking
static void ApplyResponsesCommonFields(json& requestJson)
{
	// 转换 tools
	if (requestJson.contains("tools") && requestJson["tools"].is_array())
	{
		requestJson["tools"] = ConvertToolsToResponsesFormat(requestJson["tools"]);
	}

	// 重命名 max_tokens → max_output_tokens
	if (requestJson.contains("max_tokens"))
	{
		requestJson["max_output_tokens"] = requestJson["max_tokens"];
		requestJson.erase("max_tokens");
	}

	// 移除 Chat Completions 专属字段
	if (requestJson.contains("reasoning_effort"))
	{
		std::string effort = requestJson["reasoning_effort"].get<std::string>();
		if (effort != "none")
		{
			// 设置 reasoning effort
			// 注意：summary 字段可让服务端返回推理摘要的流式事件（如 response.reasoning_summary_text.delta），
			//       但目前暂不发送 summary，因为部分网关支持不稳定，且当前不需要在聊天框中显示 reasoning 内容。
			//       若后续需要开启，取消下方注释即可：
			// requestJson["reasoning"] = { {"effort", effort}, {"summary", "concise"} };
			requestJson["reasoning"] = { {"effort", effort} };
		}
		requestJson.erase("reasoning_effort");
	}

	// 清理可能残留的 Anthropic 风格 thinking 字段
	if (requestJson.contains("thinking"))
		requestJson.erase("thinking");
}

bool CLlmFormatter::ConvertLlmRequestToOpenAIResponsesFormat(json& requestJson)
{
	try
	{
		if (!requestJson.contains("messages") || !requestJson["messages"].is_array())
			return false;

		json& messages = requestJson["messages"];
		json input = json::array();

		// 提取 instructions
		std::string instructions = ExtractResponsesInstructions(messages);

		// 完整历史模式：转换所有消息
		for (const auto& msg : messages)
		{
			if (!msg.is_object())
				continue;

			std::string role = msg.value("role", "");

			if (role == "system")
			{
				// 已提取到 instructions，跳过
			}
			else if (role == "user" || role == "assistant")
			{
				json inputItem;
				inputItem["role"] = role;

				bool hasContent = false;

				if (msg.contains("content"))
				{
					if (msg["content"].is_string())
					{
						std::string text = msg["content"].get<std::string>();
						if (!text.empty())
						{
							json contentArr = json::array();
							json block;
							block["type"] = (role == "assistant") ? "output_text" : "input_text";
							block["text"] = text;
							contentArr.push_back(block);
							inputItem["content"] = contentArr;
							hasContent = true;
						}
					}
					else if (msg["content"].is_array())
					{
						json contentArr = json::array();
						for (const auto& block : msg["content"])
						{
							if (!block.is_object())
								continue;
							json converted = ConvertContentBlockToResponses(block, role == "assistant");
							if (!converted.empty())
								contentArr.push_back(converted);
						}
						if (!contentArr.empty())
						{
							inputItem["content"] = contentArr;
							hasContent = true;
						}
					}
				}

				// 仅当 assistant 消息有实际内容时才加入 input；
				// 空内容的 assistant 消息（只有 tool_calls）由 function_call 项表示
				if (hasContent || role == "user")
					input.push_back(inputItem);

				// assistant 消息中的 tool_calls 转为 function_call items
				if (role == "assistant" && msg.contains("tool_calls") && msg["tool_calls"].is_array())
				{
					for (const auto& tc : msg["tool_calls"])
					{
						if (!tc.is_object())
							continue;
						json fcItem;
						fcItem["type"] = "function_call";
						fcItem["call_id"] = tc.value("id", "");
						if (tc.contains("function"))
						{
							fcItem["name"] = tc["function"].value("name", "");
							fcItem["arguments"] = tc["function"].value("arguments", "");
						}
						input.push_back(fcItem);
					}
				}
			}
			else if (role == "tool")
			{
				// 工具结果转为 function_call_output
				json fcOutput;
				fcOutput["type"] = "function_call_output";
				fcOutput["call_id"] = msg.value("tool_call_id", "");
				if (msg.contains("content"))
				{
					if (msg["content"].is_string())
						fcOutput["output"] = msg["content"].get<std::string>();
					else if (msg["content"].is_array())
					{
						// 将 content array 转为文本
						std::string text;
						for (const auto& block : msg["content"])
						{
							if (block.is_object() && block.value("type", "") == "text")
							{
								if (!text.empty()) text += "\n";
								text += block.value("text", "");
							}
						}
						fcOutput["output"] = text;
					}
				}
				input.push_back(fcOutput);
			}
		}

		// 构建 Responses 请求
		if (!instructions.empty())
			requestJson["instructions"] = instructions;

		requestJson["input"] = input;
		requestJson.erase("messages");

		// 公共字段处理
		ApplyResponsesCommonFields(requestJson);

		// 不存储服务端对话（完整历史模式无需 previous_response_id）
		requestJson["store"] = false;

		return true;
	}
	catch (const std::exception&)
	{
		return false;
	}
}

bool CLlmFormatter::ConvertLlmRequestToOpenAIResponsesFormat(json& requestJson, const std::string& previousResponseId)
{
	try
	{
		if (!requestJson.contains("messages") || !requestJson["messages"].is_array())
			return false;

		json& messages = requestJson["messages"];
		json input = json::array();

		// 提取 instructions（续接模式也需要，previous_response_id 不恢复 instructions）
		std::string instructions = ExtractResponsesInstructions(messages);

		// 续接模式：仅发送本轮新增的 _current_turn 消息
		// 服务端通过 previous_response_id 恢复历史上下文
		for (const auto& msg : messages)
		{
			if (!msg.is_object())
				continue;

			// 只处理本轮新增的消息
			if (!msg.value("_current_turn", false))
				continue;

			std::string role = msg.value("role", "");

			if (role == "tool")
			{
				// 工具结果 → function_call_output
				json fcOutput;
				fcOutput["type"] = "function_call_output";
				fcOutput["call_id"] = msg.value("tool_call_id", "");
				if (msg.contains("content"))
				{
					if (msg["content"].is_string())
						fcOutput["output"] = msg["content"].get<std::string>();
					else if (msg["content"].is_array())
					{
						std::string text;
						for (const auto& block : msg["content"])
						{
							if (block.is_object() && block.value("type", "") == "text")
							{
								if (!text.empty()) text += "\n";
								text += block.value("text", "");
							}
						}
						fcOutput["output"] = text;
					}
				}
				input.push_back(fcOutput);
			}
			else if (role == "user")
			{
				// ReadMedia 图片 → user message (input_text + input_image)
				json inputItem;
				inputItem["role"] = "user";

				if (msg.contains("content"))
				{
					if (msg["content"].is_string())
					{
						std::string text = msg["content"].get<std::string>();
						if (!text.empty())
						{
							json contentArr = json::array();
							json block;
							block["type"] = "input_text";
							block["text"] = text;
							contentArr.push_back(block);
							inputItem["content"] = contentArr;
						}
					}
					else if (msg["content"].is_array())
					{
						json contentArr = json::array();
						for (const auto& block : msg["content"])
						{
							if (!block.is_object())
								continue;
							json converted = ConvertContentBlockToResponses(block, false);
							if (!converted.empty())
								contentArr.push_back(converted);
						}
						if (!contentArr.empty())
							inputItem["content"] = contentArr;
					}
				}
				input.push_back(inputItem);
			}
		}

		// 构建 Responses 请求
		if (!instructions.empty())
			requestJson["instructions"] = instructions;

		requestJson["input"] = input;
		requestJson.erase("messages");

		// 续接模式：设置 previous_response_id，引用上一轮响应
		requestJson["previous_response_id"] = previousResponseId;

		// 公共字段处理
		ApplyResponsesCommonFields(requestJson);

		// 存储服务端对话，以便下一轮使用 previous_response_id
		requestJson["store"] = true;

		return true;
	}
	catch (const std::exception&)
	{
		return false;
	}
}

bool CLlmFormatter::ProcessLlmResponseFromOpenAIResponsesFormat(std::deque<std::string>& inputLines, std::vector<std::string>& outputLines, const LlmApi& api)
{
	try
	{
		while (!inputLines.empty())
		{
			std::string& front = inputLines.front();

			if (front.rfind("data: ", 0) == 0)
			{
				std::string data = front.substr(6);
				inputLines.pop_front();

				if (data.empty() || data == "[DONE]")
					continue;

				json event;
				try { event = json::parse(data); }
				catch (...) { continue; }

				if (!event.is_object())
					continue;

			std::string eventType = event.value("type", "");
				json chunk;

				if (eventType == "response.output_text.delta")
				{
					chunk = {
						{"id", event.value("response_id", "")},
						{"object", "chat.completion.chunk"},
						{"created", 0},
						{"model", ""},
						{"choices", json::array({
							{{"index", 0}, {"delta", {{"role", "assistant"}, {"content", event.value("delta", "")}}}, {"finish_reason", nullptr}}
						})}
					};
				}
			else if (eventType == "response.output_item.added")
				{
					if (!event.contains("item") || !event["item"].is_object())
						continue;

					const auto& item = event["item"];
					std::string itemType = item.value("type", "");

					if (itemType == "function_call")
					{
						// function_call 开始：初始化 tool call
						int outputIndex = event.value("output_index", 0);
						std::string callId = item.value("call_id", "");
						std::string fnName = item.value("name", "");

						json toolCall;
						toolCall["index"] = outputIndex;
						toolCall["id"] = callId;
						toolCall["type"] = "function";
						toolCall["function"] = {{"name", fnName}, {"arguments", ""}};

						chunk = {
							{"id", ""},
							{"object", "chat.completion.chunk"},
							{"created", 0},
							{"model", ""},
							{"choices", json::array({
								{{"index", 0}, {"delta", {{"tool_calls", json::array({toolCall})}}}, {"finish_reason", nullptr}}
							})}
						};
					}
					else if (itemType == "reasoning")
					{
						// reasoning 开始：发送 reasoning_content 占位标记，让 UI 知道模型正在思考。
						// 由于当前网关不返回实际的推理文本（content/summary 为空），此处发送一个非空标记
						// 以激活 UI 的思考状态指示器。当 text 内容开始输出时，UI 自然切换到文本模式。
						chunk = {
							{"id", ""},
							{"object", "chat.completion.chunk"},
							{"created", 0},
							{"model", ""},
							{"choices", json::array({
								{{"index", 0}, {"delta", {{"reasoning_content", "\n"}}}, {"finish_reason", nullptr}}
							})}
						};
					}
					else
					{
						// 其他类型（如 message 等），跳过
						continue;
					}
				}
				else if (eventType == "response.function_call_arguments.delta")
				{
					int outputIndex = event.value("output_index", 0);
					std::string delta = event.value("delta", "");

					json toolCall;
					toolCall["index"] = outputIndex;
					toolCall["function"] = {{"arguments", delta}};

					chunk = {
						{"id", ""},
						{"object", "chat.completion.chunk"},
						{"created", 0},
						{"model", ""},
						{"choices", json::array({
							{{"index", 0}, {"delta", {{"tool_calls", json::array({toolCall})}}}, {"finish_reason", nullptr}}
						})}
					};
				}
				else if (eventType == "response.completed")
				{
					// 检查是否有 function_call 输出项
					bool hasToolCalls = false;
					std::string respId;
					if (event.contains("response") && event["response"].is_object())
					{
						respId = event["response"].value("id", "");
						if (event["response"].contains("output") && event["response"]["output"].is_array())
						{
							for (const auto& item : event["response"]["output"])
							{
								if (item.is_object() && item.value("type", "") == "function_call")
								{
									hasToolCalls = true;
									break;
								}
							}
						}
					}

					chunk = {
						{"id", respId},
						{"object", "chat.completion.chunk"},
						{"created", 0},
						{"model", ""},
						{"choices", json::array({
							{{"index", 0}, {"delta", json::object()}, {"finish_reason", hasToolCalls ? "tool_calls" : "stop"}}
						})}
					};
					// 转换 usage
					if (event.contains("response") && event["response"].contains("usage"))
					{
						const auto& respUsage = event["response"]["usage"];
						json usage;
						usage["prompt_tokens"] = respUsage.value("input_tokens", 0);
						usage["completion_tokens"] = respUsage.value("output_tokens", 0);
						usage["total_tokens"] = usage["prompt_tokens"].get<int>() + usage["completion_tokens"].get<int>();
						if (respUsage.contains("input_tokens_details"))
						{
							const auto& details = respUsage["input_tokens_details"];
							// 优先使用 cached_read_tokens，回退到 cached_tokens
							int cached = 0;
							if (details.contains("cached_read_tokens"))
								cached = details.value("cached_read_tokens", 0);
							else if (details.contains("cached_tokens"))
								cached = details.value("cached_tokens", 0);
							usage["prompt_tokens_cacheRead"] = cached;
						}
						chunk["usage"] = usage;
					}
				}
				else if (eventType == "response.failed" || eventType == "error")
				{
					std::string errorMsg = "Unknown error";
					if (event.contains("response") && event["response"].contains("error"))
						errorMsg = event["response"]["error"].value("message", errorMsg);
					else if (event.contains("error"))
						errorMsg = event["error"].value("message", errorMsg);

					chunk = {
						{"error", {
							{"message", errorMsg},
							{"type", "api_error"},
							{"code", 0}
						}}
					};
				}
			else if (eventType == "response.reasoning_summary_text.delta" ||
				         eventType == "response.reasoning_text.delta")
				{
					// 兼容两种 reasoning 事件：summary_text 和 reasoning_text
					chunk = {
						{"id", event.value("response_id", "")},
						{"object", "chat.completion.chunk"},
						{"created", 0},
						{"model", ""},
						{"choices", json::array({
							{{"index", 0}, {"delta", {{"reasoning_content", event.value("delta", "")}}}, {"finish_reason", nullptr}}
						})}
					};
				}
				else
				{
					// 忽略其他事件类型（如 response.created, response.in_progress,
					// response.output_item.done, response.function_call_arguments.done 等）
					continue;
				}

				outputLines.push_back("data: " + chunk.dump());
			}
			else if (front.rfind("{", 0) == 0)
			{
				// 非流式 JSON 响应
				std::string data = front;
				inputLines.pop_front();

				json resp;
				try { resp = json::parse(data); }
				catch (...) { continue; }

				if (!resp.is_object())
					continue;

			// 检查错误（Responses API 正常响应中 error 为 null）
			if (resp.contains("error") && !resp["error"].is_null())
			{
				outputLines.push_back("data: " + data);
				continue;
			}

			// 部分网关返回非标准错误格式：{ "error_code": "...", "message": "..." }
			// 直接原样透传，parseLlmResponse 已原生支持 error_code 格式
			if (resp.contains("error_code"))
			{
				outputLines.push_back("data: " + data);
				continue;
			}

				// 提取 output 内容
				std::string fullText;
				bool hasToolCalls = false;

				if (resp.contains("output") && resp["output"].is_array())
				{
					for (const auto& item : resp["output"])
					{
						if (!item.is_object()) continue;
						std::string itemType = item.value("type", "");

						if (itemType == "message" && item.contains("content"))
						{
							for (const auto& block : item["content"])
							{
								if (block.is_object() && block.value("type", "") == "output_text")
									fullText += block.value("text", "");
							}
						}
						else if (itemType == "function_call")
						{
							hasToolCalls = true;
						}
					}
				}

				json chunk;
				chunk["id"] = resp.value("id", "");
				chunk["object"] = "chat.completion";
				chunk["created"] = resp.value("created_at", 0);
				chunk["model"] = resp.value("model", "");

				json choice;
				choice["index"] = 0;
				choice["message"]["role"] = "assistant";
				choice["message"]["content"] = fullText;
				choice["finish_reason"] = hasToolCalls ? "tool_calls" : "stop";
				chunk["choices"] = json::array({ choice });

				// 转换 usage
				if (resp.contains("usage"))
				{
					const auto& respUsage = resp["usage"];
					json usage;
					usage["prompt_tokens"] = respUsage.value("input_tokens", 0);
					usage["completion_tokens"] = respUsage.value("output_tokens", 0);
					usage["total_tokens"] = usage["prompt_tokens"].get<int>() + usage["completion_tokens"].get<int>();
				if (respUsage.contains("input_tokens_details"))
				{
					const auto& details = respUsage["input_tokens_details"];
					int cached = 0;
					if (details.contains("cached_read_tokens"))
						cached = details.value("cached_read_tokens", 0);
					else if (details.contains("cached_tokens"))
						cached = details.value("cached_tokens", 0);
					usage["prompt_tokens_cacheRead"] = cached;
				}
				chunk["usage"] = usage;
				}

				outputLines.push_back("data: " + chunk.dump());
			}
			else
			{
				inputLines.pop_front();
			}
		}

		return true;
	}
	catch (const std::exception&)
	{
		return false;
	}
}
