#include "stdh.h"
#include "TokenCalibrate.h"
#include "LlmLib.h"
#include "registry/registry.h"
#include <string>

// 外部全局变量声明
extern CLlmLib g_llmLib;
extern CCurrentUserRegistry g_reg;

// ============================================================================
// Token 矫正器实现
// ============================================================================

CTokenCalibrate::CTokenCalibrate()
{
}

CTokenCalibrate::~CTokenCalibrate()
{
}

void CTokenCalibrate::BeginCalibration(int estimatedTokens)
{
	_estimatedTokens = estimatedTokens;
	_calibrationApiName = g_llmLib.GetMajorChatApi();
}

void CTokenCalibrate::ApplyCalibration(int actualTokens)
{
	// 避免无效数据
	if (_estimatedTokens <= 0 || actualTokens <= 100)
		return;

	if (_calibrationApiName.empty())
		return;

	// 读取累计值
	uint64_t totalEstimated = 0;
	uint64_t totalActual = 0;
	_LoadAccumulated(_calibrationApiName, totalEstimated, totalActual);

	totalEstimated = totalEstimated * 9 / 10;
	totalActual = totalActual * 9 / 10;

	// 累加
	totalEstimated += static_cast<uint64_t>(_estimatedTokens);
	totalActual += static_cast<uint64_t>(actualTokens);

	// 保存到注册表
	_SaveAccumulated(_calibrationApiName, totalEstimated, totalActual);

	// 重置会话数据
	_estimatedTokens = 0;
	_calibrationApiName.clear();
}

float CTokenCalibrate::GetCalibrationFactor()
{
	std::string apiName = g_llmLib.GetMajorChatApi();
	if (apiName.empty())
		return 1.0f;

	uint64_t totalEstimated = 0;
	uint64_t totalActual = 0;
	_LoadAccumulated(apiName, totalEstimated, totalActual);

	if (totalEstimated == 0)
		return 1.0f;

	return static_cast<float>(totalActual) / static_cast<float>(totalEstimated);
}

void CTokenCalibrate::_LoadAccumulated(const std::string& apiName, uint64_t& estimated, uint64_t& actual)
{
	std::string keyEstimated = apiName + "_estimated";
	std::string keyActual = apiName + "_actual";

	std::string valueEstimated = g_reg.ReadString(REG_SECTION, keyEstimated.c_str(), "0");
	std::string valueActual = g_reg.ReadString(REG_SECTION, keyActual.c_str(), "0");

	try
	{
		estimated = std::stoull(valueEstimated);
		actual = std::stoull(valueActual);
	}
	catch (...)
	{
		estimated = 0;
		actual = 0;
	}
}

void CTokenCalibrate::_SaveAccumulated(const std::string& apiName, uint64_t estimated, uint64_t actual)
{
	std::string keyEstimated = apiName + "_estimated";
	std::string keyActual = apiName + "_actual";

	g_reg.WriteString(REG_SECTION, keyEstimated.c_str(), std::to_string(estimated).c_str());
	g_reg.WriteString(REG_SECTION, keyActual.c_str(), std::to_string(actual).c_str());
}
