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

	// 保存 _majorChatApi
	reg.WriteString(mainSection, "majorChatApi", _majorChatApi.c_str());

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

	// 加载 _majorChatApi
	_majorChatApi = reg.ReadString(mainSection, "majorChatApi", "");
	if (_majorChatApi.empty())
	{
		for (int i = 0;i < _apis.size();i++)
		{
			for(int j = 0;j < _apis[i].purpose.size();j++)
			{
				if(_apis[i].purpose[j] == LlmApiPurpose::MajorChat)
				{
					_majorChatApi = _apis[i].name;
					break;
				}
			}
			if (!_majorChatApi.empty())
				break;
		}
	}

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

	// 检查 db root folder 下是否有 llm.ini，如果没有则从 llm_default.ini 复制
	{
		extern const char* GetCurModuleFolderPath_utf8();
		std::string dbFolder = Utils::GetDBRootFolder_utf8();
		std::string dbLlmIni = dbFolder + "\\llm.ini";

		if (!Utils::IsFileExist(dbLlmIni.c_str()))
		{
			// 文件不存在，从 CurModuleFolder 的 llm_default.ini 复制
			std::string moduleFolder = GetCurModuleFolderPath_utf8();
			std::string defaultLlmIni = moduleFolder + "\\llm_default.ini";

			Utils::CopyFile(defaultLlmIni.c_str(), dbLlmIni.c_str());
		}

		// 读取 db root folder 下的 llm.ini
		CLlmLibLoader::LoadInto(_providers, _apis, dbLlmIni.c_str());
		_llmIniLastTick = Utils::GetFileTick(dbLlmIni.c_str());
	}

	// 从注册表加载动态数据
	_Load(g_reg);

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

bool CLlmLib::LoadLlmSetting(LlmSessionSetting& setting, LlmApiPurpose purpose, LlmApiProviderTypeName providerTypeName, bool allowUnavailable, const char* ruleName)
{
	// 查找具有指定用途且价格最低的API
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

		for (const auto& apiPurpose : api.purpose)
		{
			if (apiPurpose == purpose)
			{
				// 比较价格，选择价格更低的API
				if (api.priceInputToken < lowestPrice)
				{
					lowestPrice = api.priceInputToken;
					cheapestApi = &api;
				}
				break;
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

bool CLlmLib::LoadLlmSetting(LlmSessionSetting& setting, LlmApiPurpose purpose, const char* ruleName)
{
	return LoadLlmSetting(setting, purpose, LlmApiProviderTypeName(), false, ruleName);
}

bool CLlmLib::LoadLlmSetting(LlmSessionSetting& setting, const std::string& apiName, const char* ruleName)
{
	if (!IsApiAvailable(apiName))
		return false;
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
	bool hasMainChat = false;
	bool hasMinorChat = false;


	// 遍历所有API，检查可用的API类型
	for (const auto& api : _apis)
	{
		// 检查提供商是否可用
		const LlmApiProvider* provider = GetProvider(api.providerTypeName);
		if (!provider || !provider->IsAvailable())
		{
			continue; // 跳过不可用的提供商
		}

		// 检查API的用途
		for (const auto& purpose : api.purpose)
		{
			switch (purpose)
			{
			case LlmApiPurpose::MajorChat:
				hasMainChat = true;
				break;
			case LlmApiPurpose::MinorChat:
				hasMinorChat = true;
				break;
			}
		}
	}

	// 根据可用API确定工作能力
	if (hasMainChat && hasMinorChat)
	{
		// 能找到 MainChat, MinorChat这三种api
		_cap = WorkingCapability::Full;
	}
	else if (!hasMainChat)
	{
		// 无法找到 MainChat
		_cap = WorkingCapability::CannotWork;
	}
	else
	{
		// 其余情况
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

bool CLlmLib::UpdateReload()
{
	// 检查 db root folder 下的 llm.ini 是否变化
	std::string dbFolder = Utils::GetDBRootFolder_utf8();
	std::string dbLlmIni = dbFolder + "\\llm.ini";
	AbsTick tick1 = Utils::GetFileTick(dbLlmIni.c_str());

	// 如果文件发生变化，则重新加载
	if (tick1 != _llmIniLastTick)
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
	// 验证apiName是否在可用的MajorChat API列表中
	bool found = false;
	for (const auto& api : _apis)
	{
		if (api.name == apiName)
		{
			// 检查是否有MajorChat用途
			for (const auto& purpose : api.purpose)
			{
				if (purpose == LlmApiPurpose::MajorChat)
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
			if (found) break;
		}
	}

	if (found)
	{
		_majorChatApi = apiName;
		// 保存设置到注册表
		SaveSettings();
	}
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

