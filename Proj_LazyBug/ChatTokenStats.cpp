#include "stdh.h"
#include "ChatTokenStats.h"
#include "Utils_Context.h"
#include "Utils_File.h"
#include "Utils_CliWhitelist.h"
#include "ChatDialogA.h"
#include "ChatInputHistory.h"
#include "ChatInputTag.h"
#include "LlmLib.h"
#include "LlmTools.h"
#include "LlmSkills.h"
#include "ChatOpsCtrl.h"
#include <functional>
#include <set>

// 外部全局变量声明
extern CLlmLib g_llmLib;
extern CLlmTools g_llmTools;
extern CLlmSkills g_llmSkills;

// ============================================================================
// HistoryTokenProvider 实现
// ============================================================================

HistoryTokenProvider::HistoryTokenProvider(const ChatTokenStatsContext& ctx)
	: _ctx(ctx)
{
}

uint64_t HistoryTokenProvider::GetVersion() const
{
	if (!_ctx.opsCtrl)
		return 0;

	// 使用 CChatOpsCtrl 的数据版本号：每次写入 Op 后递增，可直接作为变更检测依据
	return static_cast<uint64_t>(_ctx.opsCtrl->GetVer());
}

int HistoryTokenProvider::CalculateTokens()
{
	if (!_ctx.opsCtrl)
		return 0;

	return _ctx.opsCtrl->GetEstimateTokens();
}

int HistoryTokenProvider::GetUncompressedTokens() const
{
	if (!_ctx.opsCtrl)
		return 0;

	return _ctx.opsCtrl->GetUncompressedEstimateTokens();
}

// ============================================================================
// InputTokenProvider 实现
// ============================================================================

InputTokenProvider::InputTokenProvider(const ChatTokenStatsContext& ctx)
	: _ctx(ctx)
{
}

uint64_t InputTokenProvider::GetVersion() const
{
	if (!_ctx.chatDialog)
		return 0;

	// 获取当前输入内容（plain text），使用哈希值作为版本号
	const std::wstring& content = _ctx.chatDialog->GetInputHistory().GetCurrentContent();
	std::wstring plainText = ExtractPlainText(content);
	
	if (plainText.empty())
		return 0;

	// 使用 std::hash 计算字符串哈希值作为版本号
	return std::hash<std::wstring>{}(plainText);
}

int InputTokenProvider::CalculateTokens()
{
	if (!_ctx.chatDialog)
		return 0;

	// 获取当前输入内容（plain text）
	const std::wstring& content = _ctx.chatDialog->GetInputHistory().GetCurrentContent();
	std::wstring plainText = ExtractPlainText(content);
	
	if (plainText.empty())
		return 0;

	// 使用 Utils::EstimateTokenCount 估算 token 数
	return Utils::EstimateTokenCount(plainText);
}

// ============================================================================
// AttachmentTokenProvider 实现
// ============================================================================

AttachmentTokenProvider::AttachmentTokenProvider(const ChatTokenStatsContext& ctx)
	: _ctx(ctx)
{
}

uint64_t AttachmentTokenProvider::GetVersion() const
{
	if (!_ctx.chatDialog)
		return 0;

	// 使用 set 去重（存储文件路径）
	std::set<std::wstring> filePaths;

	// 1. 获取标签栏中的可见文件标签
	const std::vector<ChatInputTag>& tags = _ctx.chatDialog->GetChatInput().GetTags();
	for (const auto& tag : tags)
	{
		if (!tag.visible)
			continue;
		// 只统计 file 和 image 类型的标签
		if (tag.type != L"file" && tag.type != L"image")
			continue;
		filePaths.insert(tag.path);
	}

	// 2. 获取输入内容中的 inline tags，提取 image 类型
	const std::wstring& content = _ctx.chatDialog->GetInputHistory().GetCurrentContent();
	std::vector<ChatInputTag> inlineTags;
	ParseInlineTags(content, inlineTags);
	for (const auto& tag : inlineTags)
	{
		// 只统计 image 类型的 inline tag
		if (tag.type == L"image" && !tag.path.empty())
			filePaths.insert(tag.path);
	}

	// 计算版本号：组合所有文件路径的状态（路径 + 文件修改时间）
	// 使用滚动哈希保证顺序敏感
	uint64_t version = 0;
	for (const auto& path : filePaths)
	{
		uint64_t pathHash = std::hash<std::wstring>{}(path);
		FILETIME fileTime = Utils::GetFileTime(path.c_str());
		uint64_t fileTimeVal = (static_cast<uint64_t>(fileTime.dwHighDateTime) << 32) | fileTime.dwLowDateTime;

		version = version * 1000003 + pathHash;
		version = version * 1000003 + fileTimeVal;
	}

	return version;
}

int AttachmentTokenProvider::CalculateTokens()
{
	if (!_ctx.chatDialog)
		return 0;

	// 使用 set 去重（存储文件路径）
	std::set<std::wstring> filePaths;

	// 1. 获取标签栏中的可见文件标签
	const std::vector<ChatInputTag>& tags = _ctx.chatDialog->GetChatInput().GetTags();
	for (const auto& tag : tags)
	{
		if (!tag.visible)
			continue;
		// 只统计 file 和 image 类型的标签
		if (tag.type != L"file" && tag.type != L"image")
			continue;
		filePaths.insert(tag.path);
	}

	// 2. 获取输入内容中的 inline tags，提取 image 类型
	const std::wstring& content = _ctx.chatDialog->GetInputHistory().GetCurrentContent();
	std::vector<ChatInputTag> inlineTags;
	ParseInlineTags(content, inlineTags);
	for (const auto& tag : inlineTags)
	{
		// 只统计 image 类型的 inline tag
		if (tag.type == L"image" && !tag.path.empty())
			filePaths.insert(tag.path);
	}

	// 计算所有文件的 token 数
	int totalTokens = 0;
	for (const auto& path : filePaths)
	{
		std::string pathUtf8 = widechar_to_utf8(path.c_str());
		totalTokens += Utils::EstimateFileTokenCount(pathUtf8.c_str());
	}

	return totalTokens;
}

// ============================================================================
// SystemPromptProvider 实现
// ============================================================================

SystemPromptProvider::SystemPromptProvider(const ChatTokenStatsContext& ctx)
	: _ctx(ctx)
{
}

// 获取当前 Agent API 对应的 rule 文件路径
static std::string _GetApiRuleFilePath()
{
	// 获取当前 Agent API
	std::string currentApiName = g_llmLib.GetMajorChatApi();
	if (currentApiName.empty())
		return "";

	// 获取 API 信息
	const LlmApi* api = g_llmLib.GetApi(currentApiName);
	if (!api || api->rule.empty())
		return "";

	// 构建 rule 文件完整路径: {ModulePath}\rules\{rule}.txt
	extern const char* GetCurModuleFolderPath_utf8();
	std::string rulePath = GetCurModuleFolderPath_utf8();
	rulePath += "\\rules\\";
	rulePath += api->rule;
	rulePath += ".txt";

	return rulePath;
}

uint64_t SystemPromptProvider::GetVersion() const
{
	// 获取 global_rules.md、project_rules.md 的文件路径
	std::string globalRulesPath = Utils::GetGlobalRulesFilePath();
	std::string projectRulesPath = Utils::GetProjectRulesFilePath();
	std::string apiRulePath = _GetApiRuleFilePath();

	// 计算版本号：用滚动哈希组合各文件路径 + 修改时间
	// 每一项都参与有序混合，避免路径哈希碰撞后互相抵消
	uint64_t version = 0;

	auto mixFile = [&](const std::string& path)
	{
		version = version * 1000003 + std::hash<std::string>{}(path);
		if (!path.empty() && Utils::IsFileExist(path.c_str()))
		{
			FILETIME ft = Utils::GetFileTime(path.c_str());
			version = version * 1000003 + ((static_cast<uint64_t>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime);
		}
	};

	mixFile(globalRulesPath);
	mixFile(projectRulesPath);
	mixFile(apiRulePath);  // rule 文件修改时也需要感知

	// 加入当前 API 名字（切换 API 时版本号变化）
	version = version * 1000003 + std::hash<std::string>{}(g_llmLib.GetMajorChatApi());

	return version;
}

int SystemPromptProvider::CalculateTokens()
{
	// 获取 global_rules.md、project_rules.md 和 API rule 文件的文件路径
	std::string globalRulesPath = Utils::GetGlobalRulesFilePath();
	std::string projectRulesPath = Utils::GetProjectRulesFilePath();
	std::string apiRulePath = _GetApiRuleFilePath();

	int totalTokens = 0;

	// 估算 global_rules.md 的 token
	if (!globalRulesPath.empty() && Utils::IsFileExist(globalRulesPath.c_str()))
	{
		totalTokens += Utils::EstimateFileTokenCount(globalRulesPath.c_str());
	}

	// 估算 project_rules.md 的 token
	if (!projectRulesPath.empty() && Utils::IsFileExist(projectRulesPath.c_str()))
	{
		totalTokens += Utils::EstimateFileTokenCount(projectRulesPath.c_str());
	}

	// 估算 API rule 文件的 token
	if (!apiRulePath.empty() && Utils::IsFileExist(apiRulePath.c_str()))
	{
		totalTokens += Utils::EstimateFileTokenCount(apiRulePath.c_str());
	}

	return totalTokens;
}


// ============================================================================
// ToolsTokenProvider 实现
// ============================================================================

ToolsTokenProvider::ToolsTokenProvider(const ChatTokenStatsContext& ctx)
	: _ctx(ctx)
{
}

uint64_t ToolsTokenProvider::GetVersion() const
{
	// 获取当前 Agent API 名字
	std::string currentApiName = g_llmLib.GetMajorChatApi();

	// 获取 API 信息
	const LlmApi* api = g_llmLib.GetApi(currentApiName);
	if (!api)
		return 0;

	// 计算版本号：组合 API 名字 + tools 列表 + LLM Lib 版本号
	// 使用 API 名字哈希作为基础
	uint64_t version = std::hash<std::string>{}(currentApiName);

	// 组合 tools 列表的哈希（tools 变化时版本号变化）
	for (const auto& tool : api->tools)
	{
		version = version * 31 + static_cast<uint64_t>(tool);
	}

	// 组合 LLM Lib 版本号（配置重载时版本号变化）
	version ^= static_cast<uint64_t>(g_llmLib.GetVer());

	return version;
}

int ToolsTokenProvider::CalculateTokens()
{
	// 获取当前 Agent API 名字
	std::string currentApiName = g_llmLib.GetMajorChatApi();

	// 获取 API 信息
	const LlmApi* api = g_llmLib.GetApi(currentApiName);
	if (!api || api->tools.empty())
		return 0;

	// 估算所有启用工具的 token 数
	int totalTokens = 0;

	for (const auto& toolType : api->tools)
	{
		// 获取工具定义
		const CLlmTools::ToolDefinition* toolDef = g_llmTools.GetToolDefinition(toolType);
		if (!toolDef)
			continue;

		// 估算工具名称的 token
		totalTokens += Utils::EstimateTokenCount(toolDef->name);

		// 估算工具描述的 token
		totalTokens += Utils::EstimateTokenCount(toolDef->description);

		// 估算参数的 token
		for (const auto& param : toolDef->params)
		{
			totalTokens += Utils::EstimateTokenCount(param.name);
			totalTokens += Utils::EstimateTokenCount(param.type);
			totalTokens += Utils::EstimateTokenCount(param.description);
		}

		// 估算必需参数列表的 token
		for (const auto& requiredParam : toolDef->required_params)
		{
			totalTokens += Utils::EstimateTokenCount(requiredParam);
		}
	}

	return totalTokens;
}

// ============================================================================
// SkillsTokenProvider 实现
// ============================================================================

SkillsTokenProvider::SkillsTokenProvider(const ChatTokenStatsContext& ctx)
	: _ctx(ctx)
{
}

uint64_t SkillsTokenProvider::GetVersion() const
{
	// 计算版本号：用滚动哈希组合所有启用的 skill 的状态
	// 同时感知：skill 列表顺序、名称、类型、skill.md 修改时间
	uint64_t version = 0;

	for (const auto& skill : g_llmSkills._skills)
	{
		// 跳过 name 为空的 skill（目录节点，非实际 skill）
		if (skill.name.empty())
			continue;

		if (!skill.enable)
			continue;

		version = version * 1000003 + std::hash<std::string>{}(skill.name);
		version = version * 1000003 + static_cast<uint64_t>(skill.tp);

		// 组合 skill.md 文件的修改时间
		std::string skillMdPath = skill.folderPath + "\\skill.md";
		if (Utils::IsFileExist(skillMdPath.c_str()))
		{
			FILETIME ft = Utils::GetFileTime(skillMdPath.c_str());
			version = version * 1000003 + ((static_cast<uint64_t>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime);
		}
	}

	return version;
}

int SkillsTokenProvider::CalculateTokens()
{
	// 获取所有启用的 skill 的汇总字符串
	std::string skillStr;
	g_llmSkills.Dump(skillStr);

	if (skillStr.empty())
		return 0;

	// 使用 Utils::EstimateTokenCount 估算 token 数
	return Utils::EstimateTokenCount(skillStr);
}

// ============================================================================
// TokenStats::TotalResult 实现
// ============================================================================

int TokenStats::TotalResult::GetSectionTokens(SectionType type) const
{
	auto it = sections.find(type);
	if (it != sections.end())
		return it->second.tokenCount;
	return 0;
}

// ============================================================================
// CChatTokenStats 实现
// ============================================================================

CChatTokenStats::CChatTokenStats()
	: _historyProvider(_ctx)
	, _inputProvider(_ctx)
	, _attachmentProvider(_ctx)
	, _systemPromptProvider(_ctx)
	, _toolsProvider(_ctx)
	, _skillsProvider(_ctx)
{
}

CChatTokenStats::~CChatTokenStats()
{
}

void CChatTokenStats::Initialize(const ChatTokenStatsContext& ctx)
{
	_ctx = ctx;
	_InitSections();
}

void CChatTokenStats::_InitSections()
{
	_sections.clear();

	// 建立映射：SectionType -> Provider
	_sections[TokenStats::SectionType::History].provider = &_historyProvider;
	_sections[TokenStats::SectionType::Input].provider = &_inputProvider;
	_sections[TokenStats::SectionType::Attachments].provider = &_attachmentProvider;
	_sections[TokenStats::SectionType::SystemPrompt].provider = &_systemPromptProvider;
	_sections[TokenStats::SectionType::Tools].provider = &_toolsProvider;
	_sections[TokenStats::SectionType::Skills].provider = &_skillsProvider;

	// 初始化所有缓存为脏状态，确保首次 Update 时重新计算
	for (auto& pair : _sections)
	{
		pair.second.lastVersion = 0;
		pair.second.cachedTokens = 0;
		pair.second.hasChanged = false;
		pair.second.isDirty = true;
	}
}

void CChatTokenStats::Update()
{
	for (auto& pair : _sections)
	{
		CachedSection& section = pair.second;
		if (!section.provider)
			continue;

		// 获取当前版本号
		uint64_t currentVersion = section.provider->GetVersion();

		// 检测是否有变化：版本号变化 或 被标记为脏
		bool changed = (currentVersion != section.lastVersion) || section.isDirty;

		if (changed)
		{
			// 重新计算 token 数
			section.cachedTokens = section.provider->CalculateTokens();
			section.lastVersion = currentVersion;
			section.hasChanged = true;
			section.isDirty = false;
		}
	}
}

int CChatTokenStats::GetTotalTokens() const
{
	int total = 0;
	for (const auto& pair : _sections)
	{
		total += pair.second.cachedTokens;
	}
	return total;
}

int CChatTokenStats::GetTotalUncompressedTokens() const
{
	int total = 0;
	for (const auto& pair : _sections)
	{
		total += pair.second.cachedTokens;
	}
	
	// History 部分使用未压缩值替换已缓存值
	const CachedSection* historySection = _GetSection(TokenStats::SectionType::History);
	if (historySection)
	{
		total -= historySection->cachedTokens;
		total += _historyProvider.GetUncompressedTokens();
	}
	
	return total;
}

int CChatTokenStats::GetSectionTokens(TokenStats::SectionType type) const
{
	const CachedSection* section = _GetSection(type);
	return section ? section->cachedTokens : 0;
}

TokenStats::TotalResult CChatTokenStats::GetTotalResult() const
{
	TokenStats::TotalResult result;
	result.totalTokens = GetTotalTokens();

	for (const auto& pair : _sections)
	{
		TokenStats::SectionResult sr;
		sr.tokenCount = pair.second.cachedTokens;
		sr.isDirty = pair.second.hasChanged;
		result.sections[pair.first] = sr;
	}

	return result;
}

bool CChatTokenStats::HasChanged(TokenStats::SectionType type) const
{
	const CachedSection* section = _GetSection(type);
	return section ? section->hasChanged : false;
}

bool CChatTokenStats::HasAnyChanged() const
{
	for (const auto& pair : _sections)
	{
		if (pair.second.hasChanged)
			return true;
	}
	return false;
}

void CChatTokenStats::ClearAllChanged()
{
	for (auto& pair : _sections)
	{
		pair.second.hasChanged = false;
	}
}

void CChatTokenStats::MarkSectionDirty(TokenStats::SectionType type)
{
	CachedSection* section = _GetSection(type);
	if (section)
		section->isDirty = true;
}

void CChatTokenStats::MarkAllDirty()
{
	for (auto& pair : _sections)
	{
		pair.second.isDirty = true;
	}
}

CChatTokenStats::CachedSection* CChatTokenStats::_GetSection(TokenStats::SectionType type)
{
	auto it = _sections.find(type);
	return (it != _sections.end()) ? &it->second : nullptr;
}

const CChatTokenStats::CachedSection* CChatTokenStats::_GetSection(TokenStats::SectionType type) const
{
	auto it = _sections.find(type);
	return (it != _sections.end()) ? &it->second : nullptr;
}

// ============================================================================
// Token 矫正机制实现
// ============================================================================

void CChatTokenStats::BeginCalibration()
{
	_tokenCalibrate.BeginCalibration(GetTotalTokens());
}

void CChatTokenStats::ApplyCalibration(int actualTokens)
{
	_tokenCalibrate.ApplyCalibration(actualTokens);
}

int CChatTokenStats::GetCalibratedTokens() const
{
	return static_cast<int>(GetTotalTokens() * _tokenCalibrate.GetCalibrationFactor());
}

int CChatTokenStats::GetUncompressedCalibratedTokens() const
{
	return static_cast<int>(GetTotalUncompressedTokens() * _tokenCalibrate.GetCalibrationFactor());
}

float CChatTokenStats::GetCalibrationFactor() const
{
	return _tokenCalibrate.GetCalibrationFactor();
}
