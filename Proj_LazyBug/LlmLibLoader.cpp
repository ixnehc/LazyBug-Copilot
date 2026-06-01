#include "stdh.h"
#include "LlmLibLoader.h"
#include "LlmLib.h"

#include "Utils_File.h"
#include <sstream>
#include <fstream>

// UTF-8 到宽字符转换
extern std::wstring utf8_to_widechar(const char* utf8_str);

namespace
{
	// 从旧的purpose字符串解析并转换为LlmApiRole
	// 如果包含MajorChat则返回Agent，否则返回Auxiliary
	LlmApiRole ParsePurposeToRole(const char* purposeStr)
	{
		if (!purposeStr || strlen(purposeStr) == 0)
			return LlmApiRole::Auxiliary;

		std::string str = purposeStr;
		std::stringstream ss(str);
		std::string item;

		while (std::getline(ss, item, ','))
		{
			// 去除空白字符
			size_t start = item.find_first_not_of(" \t");
			size_t end = item.find_last_not_of(" \t");
			if (start != std::string::npos)
			{
				item = item.substr(start, end - start + 1);

				if (item == "MajorChat")
					return LlmApiRole::Agent;
			}
		}
		return LlmApiRole::Auxiliary;
	}

	// 从JSON的purpose数组解析并转换为LlmApiRole
	LlmApiRole ParsePurposeArrayToRole(const nlohmann::json& jPurpose)
	{
		if (!jPurpose.is_array())
			return LlmApiRole::Auxiliary;

		for (const auto& elem : jPurpose)
		{
			if (elem.is_string())
			{
				std::string s = elem.get<std::string>();
				if (s == "MajorChat")
					return LlmApiRole::Agent;
			}
		}
		return LlmApiRole::Auxiliary;
	}

	// 解析工具字符串，多个工具用逗号分隔
	void ParseToolsString(const char* toolsStr, std::vector<LlmToolType>& tools)
	{
		tools.clear();
		if (!toolsStr || strlen(toolsStr) == 0)
			return;

		std::string str = toolsStr;
		std::stringstream ss(str);
		std::string item;

		while (std::getline(ss, item, ','))
		{
			// 去除空白字符
			size_t start = item.find_first_not_of(" \t");
			size_t end = item.find_last_not_of(" \t");
			if (start != std::string::npos)
			{
				item = item.substr(start, end - start + 1);

				if (item == "ReplaceInFile")
					tools.push_back(LlmToolType::ReplaceInFile);
				else if (item == "FindSymbolDefine")
					tools.push_back(LlmToolType::FindSymbolDefine);
				else if (item == "FindInFiles")
					tools.push_back(LlmToolType::FindInFiles);
				else if (item == "ReadFile")
					tools.push_back(LlmToolType::ReadFile);
				else if (item == "SearchFile")
					tools.push_back(LlmToolType::SearchFile);
				else if (item == "CLI_Cmd")
					tools.push_back(LlmToolType::CLI_Cmd);
				else if (item == "CLI_Bash")
					tools.push_back(LlmToolType::CLI_Bash);
				else if (item == "CLI_RunScript")
					tools.push_back(LlmToolType::CLI_RunScript);
				else if (item == "Question")
					tools.push_back(LlmToolType::Question);
				else if (item == "QueryFinish")
					tools.push_back(LlmToolType::QueryFinish);
			}
		}
	}

	// 解析缓存控制类型
	LlmApiCacheControlType ParseCacheControlType(const char* str)
	{
		if (!str)
			return LlmApiCacheControlType::Auto;

		if (_stricmp(str, "Anthropic") == 0)
			return LlmApiCacheControlType::Anthropic_;

		return LlmApiCacheControlType::Auto;
	}

	// 解析工具格式
	LlmApiFormat ParseApiFormat(const char* str)
	{
		if (!str)
			return LlmApiFormat::Unknown;

		if (_stricmp(str, "OpenAI") == 0)
			return LlmApiFormat::OpenAI_;
		else if (_stricmp(str, "Anthropic") == 0)
			return LlmApiFormat::Anthropic_;
		else if (_stricmp(str, "Gemini") == 0)
			return LlmApiFormat::Gemini_;
		else if (_stricmp(str, "OpenRouter") == 0)
			return LlmApiFormat::OpenRouter;
		else if (_stricmp(str, "Kimi") == 0)
			return LlmApiFormat::Kimi;
		else if (_stricmp(str, "GLM") == 0)
			return LlmApiFormat::GLM;
		else if (_stricmp(str, "Minimax") == 0)
			return LlmApiFormat::Minimax;
		else if (_stricmp(str, "DeepSeek") == 0)
			return LlmApiFormat::DeepSeek;

		return LlmApiFormat::Unknown;
	}

	// 解析思考模式
	LlmThinkingMode ParseThinkingMode(const char* str)
	{
		if (!str)
			return LlmThinkingMode::Auto;

		if (_stricmp(str, "Auto") == 0)
			return LlmThinkingMode::Auto;
		else if (_stricmp(str, "Enable") == 0)
			return LlmThinkingMode::Enable;
		else if (_stricmp(str, "Disable") == 0)
			return LlmThinkingMode::Disable;

		return LlmThinkingMode::Auto;
	}

	// 枚举转字符串函数
	std::string ApiFormatToString(LlmApiFormat fmt)
	{
		switch (fmt)
		{
		case LlmApiFormat::OpenAI_: return "OpenAI";
		case LlmApiFormat::Anthropic_: return "Anthropic";
		case LlmApiFormat::Gemini_: return "Gemini";
		case LlmApiFormat::OpenRouter: return "OpenRouter";
		case LlmApiFormat::Kimi: return "Kimi";
		case LlmApiFormat::GLM: return "GLM";
		case LlmApiFormat::Minimax: return "Minimax";
		case LlmApiFormat::DeepSeek: return "DeepSeek";
		default: return "Unknown";
		}
	}

	std::string ProviderStatusToString(LlmApiProvider::Status status)
	{
		switch (status)
		{
		case LlmApiProvider::Status::Ok: return "Ok";
		case LlmApiProvider::Status::Unavailable: return "Unavailable";
		default: return "Unknown";
		}
	}

	std::string RoleToString(LlmApiRole role)
	{
		switch (role)
		{
		case LlmApiRole::Agent: return "Agent";
		case LlmApiRole::Auxiliary: return "Auxiliary";
		default: return "None";
		}
	}

	std::string CacheControlTypeToString(LlmApiCacheControlType type)
	{
		switch (type)
		{
		case LlmApiCacheControlType::Anthropic_: return "Anthropic";
		case LlmApiCacheControlType::None_: return "None";
		default: return "Auto";
		}
	}

	std::string ThinkingModeToString(LlmThinkingMode mode)
	{
		switch (mode)
		{
		case LlmThinkingMode::Enable: return "Enable";
		case LlmThinkingMode::Disable: return "Disable";
		default: return "Auto";
		}
	}

	std::string ToolTypeToString(LlmToolType tool)
	{
		switch (tool)
		{
		case LlmToolType::ReplaceInFile: return "ReplaceInFile";
		case LlmToolType::FindSymbolDefine: return "FindSymbolDefine";
		case LlmToolType::FindInFiles: return "FindInFiles";
		case LlmToolType::SearchFile: return "SearchFile";
		case LlmToolType::ReadFile: return "ReadFile";
		case LlmToolType::CLI_Cmd: return "CLI_Cmd";
		case LlmToolType::CLI_Bash: return "CLI_Bash";
		case LlmToolType::CLI_RunScript: return "CLI_RunScript";
		case LlmToolType::Question: return "Question";
		case LlmToolType::QueryFinish: return "QueryFinish";
		case LlmToolType::CreateSkill: return "CreateSkill";
		default: return "None";
		}
	}

	// 从字符串解析单个 Role
	LlmApiRole StringToRole(const std::string& str)
	{
		if (str == "Agent") return LlmApiRole::Agent;
		if (str == "Auxiliary") return LlmApiRole::Auxiliary;
		return LlmApiRole::None;
	}

	// 从字符串解析单个 ToolType
	LlmToolType StringToToolType(const std::string& str)
	{
		if (str == "ReplaceInFile") return LlmToolType::ReplaceInFile;
		if (str == "FindSymbolDefine") return LlmToolType::FindSymbolDefine;
		if (str == "FindInFiles") return LlmToolType::FindInFiles;
		if (str == "SearchFile") return LlmToolType::SearchFile;
		if (str == "ReadFile") return LlmToolType::ReadFile;
		if (str == "CLI_Cmd") return LlmToolType::CLI_Cmd;
		if (str == "CLI_Bash") return LlmToolType::CLI_Bash;
		if (str == "CLI_RunScript") return LlmToolType::CLI_RunScript;
		if (str == "Question") return LlmToolType::Question;
		if (str == "QueryFinish") return LlmToolType::QueryFinish;
		if (str == "CreateSkill") return LlmToolType::CreateSkill;
		return LlmToolType::None;
	}

	// 去除字符串首尾空白
	std::string Trim(const std::string& str)
	{
		size_t start = str.find_first_not_of(" \t\r\n");
		if (start == std::string::npos)
			return "";
		size_t end = str.find_last_not_of(" \t\r\n");
		return str.substr(start, end - start + 1);
	}

	// 解析ini文件中的某个section数据
	// 使用命名section方式来区分多个同名section: [Provider], [Provider2], [Provider3]...
	struct SectionData
	{
		std::string name;
		std::map<std::string, std::string> keyValues;
	};

	std::vector<SectionData> ParseIniSections(const char* iniPath, const char* sectionPrefix)
	{
		std::vector<SectionData> sections;
		std::ifstream file;
		Utils::OpenIFStream(file, iniPath);
		if (!file.is_open())
			return sections;

		SectionData* currentSection = nullptr;
		std::string line;
		std::string sectionBaseName = sectionPrefix;

		while (std::getline(file, line))
		{
			std::string trimmedLine = Trim(line);

			// 跳过空行和注释行
			if (trimmedLine.empty() || trimmedLine[0] == ';')
				continue;

			// 检查是否是section行
			if (trimmedLine.size() >= 2 && trimmedLine[0] == '[' && trimmedLine[trimmedLine.size() - 1] == ']')
			{
				std::string sectionName = trimmedLine.substr(1, trimmedLine.size() - 2);
				std::string sectionNameTrimmed = Trim(sectionName);

				// 检查是否是目标section（精确匹配或以数字结尾）
				bool isTargetSection = false;
				if (_stricmp(sectionNameTrimmed.c_str(), sectionPrefix) == 0)
				{
					isTargetSection = true;
				}
				else if (_strnicmp(sectionNameTrimmed.c_str(), sectionPrefix, strlen(sectionPrefix)) == 0)
				{
					// 检查剩余部分是否都是数字
					std::string suffix = sectionNameTrimmed.substr(strlen(sectionPrefix));
					bool allDigits = !suffix.empty();
					for (char c : suffix)
					{
						if (!isdigit(c))
						{
							allDigits = false;
							break;
						}
					}
					if (allDigits)
						isTargetSection = true;
				}

				if (isTargetSection)
				{
					SectionData newSection;
					newSection.name = sectionNameTrimmed;
					sections.push_back(newSection);
					currentSection = &sections.back();
				}
				else
				{
					currentSection = nullptr;
				}
			}
			else if (currentSection)
			{
				// 解析key=value
				size_t equalPos = trimmedLine.find('=');
				if (equalPos != std::string::npos)
				{
					std::string key = Trim(trimmedLine.substr(0, equalPos));
					std::string value = Trim(trimmedLine.substr(equalPos + 1));
					currentSection->keyValues[key] = value;
				}
			}
		}

		return sections;
	}
}

void CLlmLibLoader::LoadInto(std::vector<LlmApiProvider>& providers, std::vector<LlmApi>& apis, const char* iniPath)
{
	if (!iniPath || strlen(iniPath) == 0)
		return;

	// 检查文件是否存在
	if (!Utils::IsFileExist(iniPath))
		return;

	// 加载Providers
	std::vector<SectionData> providerSections = ParseIniSections(iniPath, "Provider");
	for (const auto& section : providerSections)
	{
		// 读取name（必填）
		auto itName = section.keyValues.find("name");
		if (itName == section.keyValues.end() || itName->second.empty())
		{
			// name为空，跳过此Provider
			continue;
		}

		LlmApiProvider provider;
		provider.name = itName->second;

		// 读取其他字段
		auto it = section.keyValues.find("desc");
		if (it != section.keyValues.end())
			provider.desc = it->second;

		it = section.keyValues.find("url");
		if (it != section.keyValues.end())
			provider.url = it->second;

		it = section.keyValues.find("endpoint");
		if (it != section.keyValues.end())
			provider.endpoint = it->second;

		it = section.keyValues.find("key");
		if (it != section.keyValues.end())
			provider.key = it->second;

		// 读取toolFormat
		it = section.keyValues.find("apiFormat");
		if (it != section.keyValues.end())
			provider.format = ParseApiFormat(it->second.c_str());
		else
			provider.format = LlmApiFormat::OpenAI_;

		provider.status = LlmApiProvider::Status::Unknown;

		// 检查是否已存在同名Provider，如果存在则覆盖，否则追加
		bool found = false;
		for (size_t i = 0; i < providers.size(); ++i)
		{
			if (providers[i].name == provider.name)
			{
				providers[i] = provider;
				found = true;
				break;
			}
		}
		if (!found)
		{
			providers.push_back(provider);
		}
	}

	// 加载Apis
	std::vector<SectionData> apiSections = ParseIniSections(iniPath, "Api");
	for (const auto& section : apiSections)
	{
		// 读取name（必填）
		auto itName = section.keyValues.find("name");
		if (itName == section.keyValues.end() || itName->second.empty())
		{
			// name为空，跳过此Api
			continue;
		}

		LlmApi api;

		// 读取name
		api.name = itName->second;

		// 读取desc
		auto it = section.keyValues.find("desc");
		if (it != section.keyValues.end())
			api.desc = it->second;
		else
			api.desc = "";

		// 读取model（必填）
		it = section.keyValues.find("model");
		if (it != section.keyValues.end())
			api.model = it->second;
		else
			api.model = "";

		// 读取rule
		it = section.keyValues.find("rule");
		if (it != section.keyValues.end())
			api.rule = it->second;
		else
			api.rule = "chatrules_usingtools";

		// 读取provider
		it = section.keyValues.find("provider");
		if (it != section.keyValues.end())
			api.providerTypeName = it->second;

		// 读取maxToken
		it = section.keyValues.find("maxToken");
		if (it != section.keyValues.end())
			api.maxToken = atoi(it->second.c_str());
		else
			api.maxToken = 0;

		// 读取contextCapacity
		it = section.keyValues.find("contextCapacity");
		if (it != section.keyValues.end())
			api.contextCapacity = atoi(it->second.c_str());
		else
			api.contextCapacity = 128 * 1024;

		// 读取priceInputToken
		it = section.keyValues.find("priceInputToken");
		if (it != section.keyValues.end())
			api.priceInputToken = (float)atof(it->second.c_str());
		else
			api.priceInputToken = 0.0f;

		// 读取priceOutputToken
		it = section.keyValues.find("priceOutputToken");
		if (it != section.keyValues.end())
			api.priceOutputToken = (float)atof(it->second.c_str());
		else
			api.priceOutputToken = 0.0f;

		// 读取priceCacheRead
		it = section.keyValues.find("priceCacheRead");
		if (it != section.keyValues.end())
			api.priceCacheRead = (float)atof(it->second.c_str());
		else
			api.priceCacheRead = api.priceInputToken;

		// 读取priceCacheWrite
		it = section.keyValues.find("priceCacheWrite");
		if (it != section.keyValues.end())
			api.priceCacheWrite = (float)atof(it->second.c_str());
		else
			api.priceCacheWrite = api.priceOutputToken;

		// 读取thinkingMode
		it = section.keyValues.find("thinkingMode");
		if (it != section.keyValues.end())
			api.thinkingMode = ParseThinkingMode(it->second.c_str());
		else
			api.thinkingMode = LlmThinkingMode::Auto;

		// 读取purpose(旧格式，转换为role)
		it = section.keyValues.find("purpose");
		if (it != section.keyValues.end())
			api.role = ParsePurposeToRole(it->second.c_str());

		// 读取role(新格式，优先于purpose)
		it = section.keyValues.find("role");
		if (it != section.keyValues.end())
		{
			if (it->second == "Agent")
				api.role = LlmApiRole::Agent;
			else if (it->second == "Auxiliary")
				api.role = LlmApiRole::Auxiliary;
		}

		// 读取cacheControl
		it = section.keyValues.find("cacheControl");
		if (it != section.keyValues.end())
			api.cacheControlType = ParseCacheControlType(it->second.c_str());
		else
			api.cacheControlType = LlmApiCacheControlType::Auto;

		// 读取tools
		it = section.keyValues.find("tools");
		if (it != section.keyValues.end())
			ParseToolsString(it->second.c_str(), api.tools);
		else
		{
			api.tools =
			{
				LlmToolType::ReplaceInFile,
				LlmToolType::FindSymbolDefine,
				LlmToolType::FindInFiles,
				LlmToolType::ReadFile ,
				LlmToolType::SearchFile,
				LlmToolType::CLI_Cmd,
				LlmToolType::CLI_Bash,
				LlmToolType::CLI_RunScript,
				LlmToolType::Question
				//				LlmToolType::CreateSkill 
			};
		}

		// 读取openRouterOptions.disableReasoning
		it = section.keyValues.find("disableReasoning");
		if (it != section.keyValues.end())
			api.openRouterOptions.disableReasoning = (atoi(it->second.c_str()) != 0);
		else
			api.openRouterOptions.disableReasoning = false;

		// 读取openRouterOptions.only
		it = section.keyValues.find("openRouter_only");
		if (it != section.keyValues.end())
		{
			std::stringstream ss(it->second);
			std::string item;
			while (std::getline(ss, item, ','))
			{
				std::string trimmed = Trim(item);
				if (!trimmed.empty())
				{
					api.openRouterOptions.only.push_back(trimmed);
				}
			}
		}

		// 检查是否已存在同名Api，如果存在则覆盖，否则追加
		bool found = false;
		for (size_t i = 0; i < apis.size(); ++i)
		{
			if (apis[i].name == api.name)
			{
				apis[i] = api;
				found = true;
				break;
			}
		}
		if (!found)
		{
			apis.push_back(api);
		}
	}
}

void CLlmLibLoader::SaveJsonFile(CLlmLib& lib, const char* jsonFilePath)
{
	if (!jsonFilePath || strlen(jsonFilePath) == 0)
		return;

	json j;

	j["providers"] = json::array();
	for (const auto& provider : lib._providers)
	{
		json jProvider;
		jProvider["name"] = provider.name;
		jProvider["desc"] = provider.desc;
		jProvider["url"] = provider.url;
		jProvider["endpoint"] = provider.endpoint;
		jProvider["format"] = ApiFormatToString(provider.format);
		jProvider["key"] = provider.key;
		jProvider["status"] = ProviderStatusToString(provider.status);
		j["providers"].push_back(jProvider);
	}

	j["apis"] = json::array();
	for (const auto& api : lib._apis)
	{
		json jApi;
		jApi["name"] = api.name;
		jApi["desc"] = api.desc;
		jApi["model"] = api.model;
		jApi["rule"] = api.rule;
		jApi["maxToken"] = api.maxToken;
		jApi["contextCapacity"] = api.contextCapacity;
		jApi["priceInputToken"] = api.priceInputToken;
		jApi["priceOutputToken"] = api.priceOutputToken;
		jApi["priceCacheRead"] = api.priceCacheRead;
		jApi["priceCacheWrite"] = api.priceCacheWrite;
		jApi["thinkingMode"] = ThinkingModeToString(api.thinkingMode);
		jApi["role"] = RoleToString(api.role);
		jApi["providerTypeName"] = api.providerTypeName;
		jApi["cacheControl"] = CacheControlTypeToString(api.cacheControlType);
		jApi["enable"] = api.enable;
		jApi["tools"] = json::array();
		for (auto tool : api.tools)
			jApi["tools"].push_back(ToolTypeToString(tool));
		jApi["openRouterOptions"]["disableReasoning"] = api.openRouterOptions.disableReasoning;
		jApi["openRouterOptions"]["only"] = json::array();
		for (const auto& s : api.openRouterOptions.only)
			jApi["openRouterOptions"]["only"].push_back(s);
		j["apis"].push_back(jApi);
	}

	std::string content = j.dump(2);
	Utils::SaveFileContent(jsonFilePath, content);
}

void CLlmLibLoader::LoadJsonFile(CLlmLib& lib, const char* jsonFilePath)
{
	if (!jsonFilePath || strlen(jsonFilePath) == 0)
		return;

	if (!Utils::IsFileExist(jsonFilePath))
		return;

	std::string content;
	if (!Utils::LoadFileContent(jsonFilePath, content))
		return;

	try
	{
		json j = json::parse(content);

		lib._providers.clear();
		if (j.contains("providers") && j["providers"].is_array())
		{
			for (const auto& jProvider : j["providers"])
			{
				LlmApiProvider provider;
				provider.name = jProvider.value("name", "");
				if (provider.name.empty())
					continue;
				provider.desc = jProvider.value("desc", "");
				provider.url = jProvider.value("url", "");
				provider.endpoint = jProvider.value("endpoint", "");
				provider.key = jProvider.value("key", "");
				std::string formatStr = jProvider.value("format", "");
				if (formatStr.empty())
					provider.format = LlmApiFormat::OpenAI_;
				else
					provider.format = ParseApiFormat(formatStr.c_str());
				std::string statusStr = jProvider.value("status", "");
				if (statusStr == "Ok")
					provider.status = LlmApiProvider::Status::Ok;
				else if (statusStr == "Unavailable")
					provider.status = LlmApiProvider::Status::Unavailable;
				else
					provider.status = LlmApiProvider::Status::Unknown;
				lib._providers.push_back(provider);
			}
		}

		lib._apis.clear();
		if (j.contains("apis") && j["apis"].is_array())
		{
			for (const auto& jApi : j["apis"])
			{
				LlmApi api;
				api.name = jApi.value("name", "");
				if (api.name.empty())
					continue;
				api.desc = jApi.value("desc", "");
				api.model = jApi.value("model", "");
				api.rule = jApi.value("rule", "chatrules_usingtools");
				api.providerTypeName = jApi.value("providerTypeName", "");
				api.maxToken = jApi.value("maxToken", 0);
				api.contextCapacity = jApi.value("contextCapacity", 128 * 1024);
				api.priceInputToken = jApi.value("priceInputToken", 0.0f);
				api.priceOutputToken = jApi.value("priceOutputToken", 0.0f);
				api.priceCacheRead = jApi.value("priceCacheRead", api.priceInputToken);
				api.priceCacheWrite = jApi.value("priceCacheWrite", api.priceOutputToken);
				api.thinkingMode = ParseThinkingMode(jApi.value("thinkingMode", "").c_str());
				api.cacheControlType = ParseCacheControlType(jApi.value("cacheControl", "").c_str());
				api.enable = jApi.value("enable", true);  // 默认为true

				// 读取role(新格式)
				if (jApi.contains("role") && jApi["role"].is_string())
				{
					api.role = StringToRole(jApi["role"].get<std::string>());
				}
				// 兼容旧格式：如果存在purpose数组，则转换为role
				else if (jApi.contains("purpose") && jApi["purpose"].is_array())
				{
					api.role = ParsePurposeArrayToRole(jApi["purpose"]);
				}

// 				if (jApi.contains("tools") && jApi["tools"].is_array())
// 				{
// 					for (const auto& jTool : jApi["tools"])
// 						api.tools.push_back(StringToToolType(jTool.get<std::string>()));
// 				}
// 				else
				{
					api.tools =
					{
						LlmToolType::ReplaceInFile,
						LlmToolType::FindSymbolDefine,
						LlmToolType::FindInFiles,
						LlmToolType::ReadFile,
						LlmToolType::SearchFile,
						LlmToolType::CLI_Cmd,
						LlmToolType::CLI_Bash,
						LlmToolType::CLI_RunScript,
						LlmToolType::Question
					};
				}

				if (jApi.contains("openRouterOptions") && jApi["openRouterOptions"].is_object())
				{
					auto& jOpt = jApi["openRouterOptions"];
					api.openRouterOptions.disableReasoning = jOpt.value("disableReasoning", false);
					if (jOpt.contains("only") && jOpt["only"].is_array())
					{
						for (const auto& jOnly : jOpt["only"])
							api.openRouterOptions.only.push_back(jOnly.get<std::string>());
					}
				}

				lib._apis.push_back(api);
			}
		}
	}
	catch (...)
	{
		// JSON解析失败，静默处理
	}
}
