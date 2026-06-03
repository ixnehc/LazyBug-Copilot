#include "stdh.h"
#include "LlmLib.h"
#include "LlmLibLoader.h"
#include "LlmSession.h"
#include "Registry/Registry.h"

#include "Utils.h"
#include <fstream>

CLlmLib g_llmLib;

// 简单的字符串加密/解密函数
std::string EncryptString(const std::string& plaintext)
{
	if (plaintext.empty())
		return "";

	// 使用固定密钥进行XOR加密
	const std::string key = "LazyBug2024SecretKey";
	std::string encrypted;
	encrypted.reserve(plaintext.length() * 2); // 预分配空间，因为要转换为十六进制

	for (size_t i = 0; i < plaintext.length(); ++i)
	{
		// XOR加密
		char encryptedChar = plaintext[i] ^ key[i % key.length()];

		// 转换为十六进制字符串
		char hex[3];
		sprintf_s(hex, "%02X", (unsigned char)encryptedChar);
		encrypted += hex;
	}

	return encrypted;
}

std::string DecryptString(const std::string& encrypted)
{
	if (encrypted.empty())
		return "";

	// 检查加密字符串长度是否为偶数
	if (encrypted.length() % 2 != 0)
		return ""; // 无效的加密字符串

	const std::string key = "LazyBug2024SecretKey";
	std::string plaintext;
	plaintext.reserve(encrypted.length() / 2);

	for (size_t i = 0; i < encrypted.length(); i += 2)
	{
		// 从十六进制字符串转换回字节
		std::string hexByte = encrypted.substr(i, 2);
		char byte = (char)strtol(hexByte.c_str(), nullptr, 16);

		// XOR解密
		char decryptedChar = byte ^ key[(i / 2) % key.length()];
		plaintext += decryptedChar;
	}

	return plaintext;
}

// 处理provider名称为Registry兼容的section名称
std::string MakeRegistrySafeName(const std::string& name)
{
	std::string safeName = name;
	// 替换空格和特殊字符为下划线
	for (char& c : safeName)
	{
		if (c == ' ' || c == ':' || c == '\\' || c == '/' || c == '*' || c == '?' || c == '"' || c == '<' || c == '>' || c == '|')
		{
			c = '_';
		}
	}
	return safeName;
}

// CLlmLib 类实现
void CLlmLib::_Save(CCurrentUserRegistry &reg)
{
	const char* mainSection = "LazyBugLlmLib";

	// 保存 _majorChatApi, _briefApi 和 _summarizeApi
	reg.WriteString(mainSection, "majorChatApi", _majorChatApi.c_str());
	reg.WriteString(mainSection, "briefApi", _briefApi.c_str());
	reg.WriteString(mainSection, "summarizeApi", _summarizeApi.c_str());

	// 保存API提供商的动态数据到注册表
	for (int i = 0; i < (int)_providers.size(); i++)
	{
		const LlmApiProvider& provider = _providers[i];

		// 跳过没有名称的provider
		if (provider.name.empty())
			continue;

		// 使用 "提供商名称_key", "提供商名称_status" 作为键名
		std::string safeName = MakeRegistrySafeName(provider.name);
		std::string keyName_key = safeName + "_key";
		std::string keyName_status = safeName + "_status";

		// 保存加密的key
		std::string encryptedKey = EncryptString(provider.key);
		reg.WriteString(mainSection, keyName_key.c_str(), encryptedKey.c_str());

		// 保存status
		reg.WriteInt(mainSection, keyName_status.c_str(), (int)provider.status);
	}
}

void CLlmLib::_Load(CCurrentUserRegistry& reg)
{
	const char* mainSection = "LazyBugLlmLib";

	// 从注册表加载 _majorChatApi, _briefApi 和 _summarizeApi
	_majorChatApi = reg.ReadString(mainSection, "majorChatApi", "");
	_briefApi = reg.ReadString(mainSection, "briefApi", "");
	_summarizeApi = reg.ReadString(mainSection, "summarizeApi", "");

	// 从注册表读取API提供商的动态数据
	for (int i = 0; i < (int)_providers.size(); i++)
	{
		LlmApiProvider& provider = _providers[i];

		// 跳过没有名称的provider
		if (provider.name.empty())
			continue;

		// 使用 "提供商名称_key", "提供商名称_status" 作为键名
		std::string safeName = MakeRegistrySafeName(provider.name);
		std::string keyName_key = safeName + "_key";
		std::string keyName_status = safeName + "_status";

		// 读取并解密key
		std::string encryptedKey = reg.ReadString(mainSection, keyName_key.c_str(), "");
		if (!encryptedKey.empty())
		{
			provider.key = DecryptString(encryptedKey);
		}

		// 读取status
		int statusValue = reg.ReadInt(mainSection, keyName_status.c_str(), (int)LlmApiProvider::Status::Unknown);
		provider.status = (LlmApiProvider::Status)statusValue;
	}

	// 确保默认API被设置
	_EnsureDefApis();
}

void CLlmLib::_LoadLlmSessionSetting(LlmSessionSetting& setting, const LlmApi &api, const char* ruleName)
{
	// 获取对应的提供商信息
	const LlmApiProvider* provider = GetProvider(api.providerTypeName);

	// 设置基本信息
	setting.api = api;
	setting.apiKey = provider ? provider->key : "";
	setting.apiEndpoint_ = provider ? provider->endpoint : "";
	setting.apiFormat = provider ? provider->format : LlmApiFormat::OpenAI_;
	setting.apiCacheControlType = GetApiCacheControlType(api.name);

	extern const char* GetCurModuleFolderPath_utf8();

	setting.rulesFiles.resize(1);
	setting.rulesFiles[0] = GetCurModuleFolderPath_utf8();
	setting.rulesFiles[0] += "\\rules\\";

	// 设置规则文件路径
	if (ruleName && strlen(ruleName) > 0)
		setting.rulesFiles[0] += ruleName;
	else
		setting.rulesFiles[0] += api.rule;
	setting.rulesFiles[0] += ".txt";

	// 设置默认超时时间
	setting.timeoutSeconds = 600;
}

extern CCurrentUserRegistry g_reg;

void CLlmLib::Init()
{
	// 初始化API提供商数组和API列表

	// 确保 llm.json 文件存在，如果不存在则从 ini 文件迁移
	EnsureJson();

	// 读取 db root folder 下的 llm.json
	{
		std::string dbFolder = Utils::GetDBRootFolder_utf8();
		std::string jsonPath = dbFolder + "\\llm.json";
		CLlmLibLoader::LoadJsonFile(*this, jsonPath.c_str());
		_llmFileLastTick = Utils::GetFileTick(jsonPath.c_str());
	}

	// 从注册表加载动态数据
	_Load(g_reg);
	_Save(g_reg);//_Load(..)后可能会修正,重新存一下

	// 验证工作能力
	ValidateCap();
}

void CLlmLib::Clear()
{

	// 清空提供商数组
	_providers.clear();

	// 清空API列表
	_apis.clear();

	// 重置工作能力
	_cap = WorkingCapability::CannotWork;
}

void CLlmLib::_EnsureDefApis()
{
	// 辅助lambda：检查api是否有效（存在、provider有效、角色匹配）
	auto IsApiValid = [this](const std::string& apiName, LlmApiRole expectedRole) {
		if (apiName.empty()) return false;
		const LlmApi* api = GetApi(apiName);
		if (!api) return false;
		if (api->role != expectedRole) return false;
		return IsApiAvailable(apiName);
	};

	// 辅助lambda：从指定role的有效api中获取第一个
	auto FindFirstValidApi = [this](LlmApiRole role) -> std::string {
		for (const auto& api : _apis)
		{
			if (api.role == role && IsApiAvailable(api.name))
			{
				return api.name;
			}
		}
		return "";
	};

	// 确保 _majorChatApi：从Agent里取，如果原值无效则重新查找
	if (!IsApiValid(_majorChatApi, LlmApiRole::Agent))
	{
		_majorChatApi = FindFirstValidApi(LlmApiRole::Agent);
	}

	// 确保 _briefApi：优先从Auxiliary取，其次从Agent取
	if (!IsApiValid(_briefApi, LlmApiRole::Auxiliary) && !IsApiValid(_briefApi, LlmApiRole::Agent))
	{
		_briefApi = FindFirstValidApi(LlmApiRole::Auxiliary);
		if (_briefApi.empty())
		{
			_briefApi = FindFirstValidApi(LlmApiRole::Agent);
		}
	}

	// 确保 _summarizeApi：优先从Auxiliary取，其次从Agent取
	if (!IsApiValid(_summarizeApi, LlmApiRole::Auxiliary) && !IsApiValid(_summarizeApi, LlmApiRole::Agent))
	{
		if((_summarizeApi!= SUMMARIZE_API_AUTO)&& (_summarizeApi != SUMMARIZE_API_DISABLE))
			_summarizeApi = SUMMARIZE_API_AUTO;
// 		if (_summarizeApi.empty())
// 		{
// 			_summarizeApi = FindFirstValidApi(LlmApiRole::Agent);
// 		}
	}
}

void CLlmLib::EnsureJson()
{
	std::string dbFolder = Utils::GetDBRootFolder_utf8();
	std::string jsonPath = dbFolder + "\\llm.json";
	
	// 如果llm.json已存在，什么都不做
	if (Utils::IsFileExist(jsonPath.c_str()))
		return;
	
	CLlmLib tempLib;
	
	// 检查是否有llm.ini文件
	std::string iniPath = dbFolder + "\\llm.ini";
	if (Utils::IsFileExist(iniPath.c_str()))
	{
		// 从llm.ini加载
		CLlmLibLoader::LoadInto(tempLib._providers, tempLib._apis, iniPath.c_str());
	}
	else
	{
		// 从llm_default.ini加载
		extern const char* GetCurModuleFolderPath_utf8();
		std::string moduleFolder = GetCurModuleFolderPath_utf8();
		std::string defaultIniPath = moduleFolder + "\\llm_default.ini";
		CLlmLibLoader::LoadInto(tempLib._providers, tempLib._apis, defaultIniPath.c_str());
	}
	
	// 保存为llm.json
	CLlmLibLoader::SaveJsonFile(tempLib, jsonPath.c_str());
}

bool CLlmLib::LoadLlmSetting(LlmSessionSetting& setting, LlmApiRole role, LlmApiProviderTypeName providerTypeName, bool allowUnavailable, const char* ruleName)
{
	// 查找具有指定角色且价格最低的API
	const LlmApi* cheapestApi = nullptr;
	float lowestPrice = FLT_MAX;

	for (const auto& api : _apis)
	{
		// 获取提供商信息
		const LlmApiProvider* provider = GetProvider(api.providerTypeName);
		if (!provider)
			continue;
		if (!providerTypeName.empty())
		{
			if (providerTypeName != api.providerTypeName)
				continue;
		}
		if (!allowUnavailable)
		{
			if (!provider->IsAvailable())
			{
				continue; // 跳过不可用的提供商
			}
		}

		if (api.role == role)
		{
			// 比较价格，选择价格更低的API
			if (api.priceInputToken < lowestPrice)
			{
				lowestPrice = api.priceInputToken;
				cheapestApi = &api;
			}
		}
	}

	if (cheapestApi)
	{
		// 找到价格最低的API，加载设置
		_LoadLlmSessionSetting(setting, *cheapestApi, ruleName);
		return true;
	}

	return false; // 未找到合适的API
}

bool CLlmLib::LoadLlmSetting(LlmSessionSetting& setting, LlmApiRole role, const char* ruleName)
{
	return LoadLlmSetting(setting, role, LlmApiProviderTypeName(), false, ruleName);
}

bool CLlmLib::LoadLlmSetting(LlmSessionSetting& setting, const std::string& apiName, bool allowUnavailable, const char* ruleName)
{
	if (!allowUnavailable)
	{
		if (!IsApiAvailable(apiName))
			return false;
	}
	LlmApi* api = _FindApi(apiName.c_str());
	if (api)
	{
		_LoadLlmSessionSetting(setting, *api, ruleName);
		return true;
	}
	return false;
}

void CLlmLib::ValidateCap()
{
	bool hasAgent = false;
	bool hasAuxiliary = false;

	// 遍历所有API，检查可用的API类型
	for (const auto& api : _apis)
	{
		// 检查提供商是否可用
		const LlmApiProvider* provider = GetProvider(api.providerTypeName);
		if (!provider || !provider->IsAvailable())
		{
			continue; // 跳过不可用的提供商
		}

		// 检查API的角色
		if (api.role == LlmApiRole::Agent)
			hasAgent = true;
		else if (api.role == LlmApiRole::Auxiliary)
			hasAuxiliary = true;
	}

	// 根据可用API确定工作能力
	if (hasAgent && hasAuxiliary)
	{
		_cap = WorkingCapability::Full;
	}
	else if (!hasAgent)
	{
		_cap = WorkingCapability::CannotWork;
	}
	else
	{
		_cap = WorkingCapability::Partial;
	}
}

// 获取Provider信息的方法实现
const LlmApiProvider* CLlmLib::GetProvider(int index) const
{
	if (index >= 0 && index < (int)_providers.size())
	{
		return &_providers[index];
	}
	return nullptr;
}

const LlmApiProvider* CLlmLib::GetProvider(const LlmApiProviderTypeName& name) const
{
	for (const auto& provider : _providers)
	{
		if (provider.name == name)
		{
			return &provider;
		}
	}
	return nullptr;
}

const LlmApiProvider* CLlmLib::GetProviderByApiName(const std::string& apiName) const
{
	// 先找到对应的API
	const LlmApi* api = nullptr;
	for (const auto& a : _apis)
	{
		if (a.name == apiName)
		{
			api = &a;
			break;
		}
	}
	if (!api)
		return nullptr;

	// 再通过API的providerTypeName找到Provider
	for (const auto& provider : _providers)
	{
		if (provider.name == api->providerTypeName)
		{
			return &provider;
		}
	}
	return nullptr;
}

LlmApiCacheControlType CLlmLib::GetApiCacheControlType(const std::string& apiName)
{
	const LlmApi* api = GetApi(apiName);
	const LlmApiProvider* provider = GetProviderByApiName(apiName);
	if (api && provider)
	{
		if ((api->cacheControlType == LlmApiCacheControlType::Anthropic_) ||
			(provider->format == LlmApiFormat::Anthropic_))
			return LlmApiCacheControlType::Anthropic_;
	}
	return LlmApiCacheControlType::None_;
}



bool CLlmLib::SetProviderKey(const LlmApiProviderTypeName& name, const std::string& key)
{
	for (auto& provider : _providers)
	{
		if (provider.name == name)
		{
			provider.key = key;
			// 重新验证工作能力
			ValidateCap();
			return true;
		}
	}
	return false;
}

bool CLlmLib::SetProviderStatus(const LlmApiProviderTypeName& name, LlmApiProvider::Status status)
{
	for (auto& provider : _providers)
	{
		if (provider.name == name)
		{
			provider.status = status;
			// 重新验证工作能力
			ValidateCap();
			_EnsureDefApis();
			return true;
		}
	}
	return false;
}


void CLlmLib::SaveSettings()
{
	extern CCurrentUserRegistry g_reg;
	_Save(g_reg);
}

bool CLlmLib::SetProviderName(const LlmApiProviderTypeName& oldName, const LlmApiProviderTypeName& newName)
{
	if (oldName.empty() || newName.empty() || oldName == newName)
		return false;
	for (auto& p : _providers)
	{
		if (p.name == oldName)
		{
			p.name = newName;
			// 同步更新所有引用该 provider 的 api
			for (auto& api : _apis)
				if (api.providerTypeName == oldName)
					api.providerTypeName = newName;
			return true;
		}
	}
	return false;
}

bool CLlmLib::SetProviderEndpoint(const LlmApiProviderTypeName& name, const std::string& endpoint)
{
	for (auto& p : _providers)
	{
		if (p.name == name)
		{
			p.endpoint = endpoint;
			return true;
		}
	}
	return false;
}

bool CLlmLib::SetProviderFormat(const LlmApiProviderTypeName& name, LlmApiFormat format)
{
	for (auto& p : _providers)
	{
		if (p.name == name)
		{
			p.format = format;
			return true;
		}
	}
	return false;
}

bool CLlmLib::SetApiName(const std::string& oldName, const std::string& newName)
{
	if (oldName.empty() || newName.empty() || oldName == newName)
		return false;
	for (auto& api : _apis)
	{
		if (api.name == oldName)
		{
			api.name = newName;
			return true;
		}
	}
	return false;
}

LlmApi* CLlmLib::GetApiMutable(const std::string& apiName)
{
	for (auto& api : _apis)
		if (api.name == apiName)
			return &api;
	return nullptr;
}

bool CLlmLib::UpdateReload()
{
	// 检查 db root folder 下的 llm.json 是否变化
	std::string dbFolder = Utils::GetDBRootFolder_utf8();
	std::string dbLlmJson = dbFolder + "\\llm.json";
	AbsTick tick1 = Utils::GetFileTick(dbLlmJson.c_str());

	// 如果文件发生变化，则重新加载
	if (tick1 != _llmFileLastTick)
	{
		// 清空现有数据
		Clear();

		// 重新初始化
		Init();

		// 版本号+1
		_version++;

		return true;
	}

	return false;
}

void CLlmLib::SetMajorChatApi(const std::string& apiName)
{
	// 验证apiName是否在可用的Agent API列表中
	bool found = false;
	for (const auto& api : _apis)
	{
		if (api.name == apiName)
		{
			// 检查是否有Agent角色
			if (api.role == LlmApiRole::Agent)
			{
				// 检查提供商是否可用
				const LlmApiProvider* provider = GetProvider(api.providerTypeName);
				if (provider && provider->IsAvailable())
				{
					found = true;
					break;
				}
			}
		}
	}

	if (found)
	{
		_majorChatApi = apiName;
		_version++;
		// 保存设置到注册表
		SaveSettings();
	}
}

void CLlmLib::SetBriefApi(const std::string& apiName)
{
	// 验证apiName是否在可用的API列表中
	bool found = false;
	for (const auto& api : _apis)
	{
		if (api.name == apiName)
		{
			// 检查提供商是否可用
			const LlmApiProvider* provider = GetProvider(api.providerTypeName);
			if (provider && provider->IsAvailable())
			{
				found = true;
				break;
			}
		}
	}

	if (found)
	{
		_briefApi = apiName;
		_version++;
		// 保存设置到注册表
		SaveSettings();
	}
}

void CLlmLib::SetSummarizeApi(const std::string& apiName)
{
	// 特殊值 <disable> 和 <auto> 直接接受，无需验证
	if (apiName == SUMMARIZE_API_DISABLE || apiName == SUMMARIZE_API_AUTO)
	{
		_summarizeApi = apiName;
		_version++;
		// 保存设置到注册表
		SaveSettings();
		return;
	}

	// 验证apiName是否在可用的API列表中
	bool found = false;
	for (const auto& api : _apis)
	{
		if (api.name == apiName)
		{
			// 检查提供商是否可用
			const LlmApiProvider* provider = GetProvider(api.providerTypeName);
			if (provider && provider->IsAvailable())
			{
				found = true;
				break;
			}
		}
	}

	if (found)
	{
		_summarizeApi = apiName;
		_version++;
		// 保存设置到注册表
		SaveSettings();
	}
}

LlmApiProvider* CLlmLib::AddProvider(const LlmApiProviderTypeName& name)
{
	if (name.empty())
		return nullptr;
	// 检查是否已存在同名 provider
	for (const auto& p : _providers)
	{
		if (p.name == name)
			return nullptr;
	}
	// 创建新 provider
	LlmApiProvider newProvider;
	newProvider.name = name;
	newProvider.desc = name;
	newProvider.status = LlmApiProvider::Status::Unknown;
	newProvider.format = LlmApiFormat::OpenAI_;
	_providers.push_back(newProvider);
	return &_providers.back();
}

bool CLlmLib::DeleteProvider(const LlmApiProviderTypeName& name)
{
	if (name.empty())
		return false;
	
	// 找到要删除的 provider
	auto it = std::find_if(_providers.begin(), _providers.end(),
		[&name](const LlmApiProvider& p) { return p.name == name; });
	
	if (it == _providers.end())
		return false;
	
	// 删除该 provider 下的所有 API
	_apis.erase(std::remove_if(_apis.begin(), _apis.end(),
		[&name](const LlmApi& api) { return api.providerTypeName == name; }), _apis.end());
	
	// 删除 provider
	_providers.erase(it);
	
	return true;
}

LlmApi* CLlmLib::AddApi(const LlmApiProviderTypeName& providerName, const std::string& apiName)
{
	if (providerName.empty() || apiName.empty())
		return nullptr;
	
	// 检查 provider 是否存在
	bool providerExists = false;
	for (const auto& p : _providers)
	{
		if (p.name == providerName)
		{
			providerExists = true;
			break;
		}
	}
	if (!providerExists)
		return nullptr;
	
	// 检查 API 名称是否已存在
	for (const auto& api : _apis)
	{
		if (api.name == apiName)
			return nullptr;
	}
	
	// 创建新 API
	LlmApi newApi;
	newApi.name = apiName;
	newApi.desc = apiName;
	newApi.providerTypeName = providerName;
	newApi.model = "";
	newApi.maxToken = 0;
	newApi.contextCapacity = 128000;
	newApi.priceInputToken = 0;
	newApi.priceOutputToken = 0;
	newApi.priceCacheRead = 0;
	newApi.priceCacheWrite = 0;
	newApi.thinkingMode = LlmThinkingMode::Auto;
	newApi.role = LlmApiRole::Agent;
	_apis.push_back(newApi);
	return &_apis.back();
}

bool CLlmLib::DeleteApi(const std::string& name)
{
	if (name.empty())
		return false;
	
	// 找到并删除 API
	auto it = std::find_if(_apis.begin(), _apis.end(),
		[&name](const LlmApi& api) { return api.name == name; });
	
	if (it == _apis.end())
		return false;
	
	_apis.erase(it);
	return true;
}

LlmApi* CLlmLib::_FindApi(const char* apiName)
{
	for (int i = 0;i < _apis.size();i++)
	{
		if (_apis[i].name == std::string(apiName))
		{
			return &_apis[i];
		}
	}
	return nullptr;
}


bool CLlmLib::IsApiAvailable(const std::string& apiName)
{
	LlmApi* api = _FindApi(apiName.c_str());
	if (api)
	{
		// enable为false的API视为不可用
		if (!api->enable)
			return false;
		
		if (!api->providerTypeName.empty())
		{
			const LlmApiProvider* provider = GetProvider(api->providerTypeName);
			if (provider && provider->IsAvailable())
				return true;
		}
	}
	return false;
}

const LlmApi* CLlmLib::GetApi(const std::string& apiName)
{
	return _FindApi(apiName.c_str());
}


std::string CLlmLib::FindApiToValidateApiKey(const LlmApiProviderTypeName& name)
{
	std::string auxiliaryApi;
	std::string otherApi;

	for (const auto& api : _apis)
	{
		// 检查是否属于指定provider
		if (api.providerTypeName != name)
			continue;

		// 检查model是否非空
		if (api.model.empty())
			continue;

		// 优先找Auxiliary角色
		if (api.role == LlmApiRole::Auxiliary)
		{
			auxiliaryApi = api.name;
			break; // 找到Auxiliary，直接退出
		}
		else if (otherApi.empty())
		{
			otherApi = api.name;
		}
	}

	// 优先返回Auxiliary，否则返回其他符合条件的api
	return auxiliaryApi.empty() ? otherApi : auxiliaryApi;
}
