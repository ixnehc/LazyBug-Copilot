#pragma once

#include <string>
#include <vector>

#include "LlmLibDefines.h"


// 前向声明
struct LlmSessionSetting;


struct LlmApiProvider
{
	LlmApiProvider()
		: name("")
		, desc("")
		, url("")
		, endpoint("")
		, key("")
		, status(Status::Unknown)
		, format(LlmApiFormat::OpenAI_)
	{

	}
	enum class Status
	{
		Unknown,
		Ok,
		Unavailable,
	};
	bool IsAvailable() const
	{
		if (endpoint.empty())
			return false;
		if (key.empty())
			return false;
		return status == Status::Ok;
	}

	//静态数据(不同用户的设置都是一样的)
	std::string name;
	std::string desc;
	std::string url;
	std::string endpoint;
	LlmApiFormat format;

	//动态数据(不同用户的设置有可能会不一样)
	std::string key;
	Status status;
};

struct LlmApi
{
	LlmApi()
		: name("")
		, desc("")
		, model("")
		, rule("")
		, maxToken(0)
		, contextCapacity(0)
		, priceInputToken(0.0f)
		, priceOutputToken(0.0f)
		, priceCacheRead(0.0f)
		, priceCacheWrite(0.0f)
		, thinkingMode(LlmThinkingMode::Auto)
		, purpose()
		, providerTypeName("")
		, cacheControlType(LlmApiCacheControlType::Auto)
	{

	}
	
	LlmApi(const std::string& _name, const std::string& _desc, const std::string& _model, const std::string& _rule,
			int _maxToken, float _priceInputToken, float _priceOutputToken, float _priceCachedRead, float _priceCachedWrite,
		   const std::vector<LlmApiPurpose>& _purpose, LlmApiProviderTypeName _providerTypeName,
			LlmApiCacheControlType _cacheControlType,
			const std::vector<LlmToolType> & _tools)
		: name(_name)
		, desc(_desc)
		, model(_model)
		, rule(_rule)
		, maxToken(_maxToken)
		, priceInputToken(_priceInputToken)
		, priceOutputToken(_priceOutputToken)
		, priceCacheRead(_priceCachedRead)
		, priceCacheWrite(_priceCachedWrite)
		, purpose(_purpose)
		, providerTypeName(_providerTypeName)
		, cacheControlType(_cacheControlType)
		, tools(_tools)
	{

	}
	std::string name;
	std::string desc;
	std::string model;
	std::string rule;
	int maxToken;
	int contextCapacity;
	float priceInputToken;
	float priceOutputToken;
	float priceCacheRead;
	float priceCacheWrite;
	LlmThinkingMode thinkingMode;
	std::vector<LlmApiPurpose> purpose;
	LlmApiProviderTypeName providerTypeName;
	LlmApiCacheControlType cacheControlType;
	std::vector<LlmToolType> tools;

	struct OpenRouterOptions
	{
		OpenRouterOptions()
		{ 
			disableReasoning = false;
		}
		std::vector<std::string> only;
		bool disableReasoning;
	};
	OpenRouterOptions openRouterOptions;

};

void SetupLlmApiProviderList(std::vector<LlmApiProvider>& providers);
void SetupLlmApiList(std::vector<LlmApi>& apis);

class CCurrentUserRegistry;

class CLlmLib
{
public:
	CLlmLib()
	{
		_cap = WorkingCapability::CannotWork;
		_version = 0;
	}
	enum class WorkingCapability
	{
		CannotWork,
		Partial,
		Full,
	};
	void Init();
	void Clear();

    bool LoadLlmSetting(LlmSessionSetting& setting, LlmApiPurpose purpose, LlmApiProviderTypeName providerTypeName, bool allowUnavailable,const char* ruleName);
	bool LoadLlmSetting(LlmSessionSetting& setting, LlmApiPurpose purpose, const char* ruleName);
	bool LoadLlmSetting(LlmSessionSetting& setting, const std::string& apiName, const char* ruleName);

	//检查可以以什么工作模式工作
	//能找到 MainChat,FastApply_Dedicated,Embedding 这三种api的为WorkingCapability::Full
	//无法找到 MainChat,FastApply_Adaptive 为WorkingCapability::CannotWork
	//其余为WorkingCapability::Partial
	void ValidateCap();
	
	// 获取当前工作能力
	WorkingCapability GetWorkingCapability() const { return _cap; }

	// 获取当前的MajorChat API
	std::string GetMajorChatApi() const { return _majorChatApi; }
	
	// 设置当前的MajorChat API
	void SetMajorChatApi(const std::string& apiName);
	
	// 获取API列表
	const std::vector<LlmApi>& GetApis() const { return _apis; }

	//API访问
	bool IsApiAvailable(const std::string& apiName);
	const LlmApi* GetApi(const std::string& apiName);
	LlmApiCacheControlType GetApiCacheControlType(const std::string& apiName);

	// 获取Provider信息的方法
	int GetProviderCount() const { return (int)_providers.size(); }
	const LlmApiProvider* GetProvider(int index) const;
	const LlmApiProvider* GetProvider(const LlmApiProviderTypeName& name) const;
	const LlmApiProvider* GetProviderByApiName(const std::string& apiName) const;
	bool SetProviderKey(const LlmApiProviderTypeName& name, const std::string& key);
	bool SetProviderStatus(const LlmApiProviderTypeName& name, LlmApiProvider::Status status);
	void SaveSettings(); // 保存设置到注册表


	// 检测ini文件是否变化，如有变化则重新加载，返回是否重新加载
	bool UpdateReload();

	// 获取当前版本号（每次reload后版本号+1）
	int GetVer() const { return _version; }

private:

	//在Registry里保存/读取各个providers的动态数据
	void _Save(CCurrentUserRegistry &reg);
	void _Load(CCurrentUserRegistry& reg);

	void _LoadLlmSessionSetting(LlmSessionSetting& setting, const LlmApi &api, const char* ruleName);
	 
	LlmApi* _FindApi(const char* apiName);

	std::vector<LlmApiProvider> _providers;
	std::vector<LlmApi> _apis;

	std::string _majorChatApi;

	WorkingCapability _cap;

	// 记录 llm.ini 文件的最后修改时间
	AbsTick _llmIniLastTick;

	// 版本号，每次reload后+1
	int _version;
};

extern CLlmLib g_llmLib;
