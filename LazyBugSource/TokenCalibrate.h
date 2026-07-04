#pragma once

#include <string>
#include <cstdint>

// ============================================================================
// Token 矫正器
// 根据累计的估计 token 数和实际 token 数计算矫正因子
// 每个 apiName 独立维护矫正数据，存储在注册表中
// ============================================================================
class CTokenCalibrate
{
public:
	CTokenCalibrate();
	~CTokenCalibrate();

	// 开始矫正：记录当前的估计 token 数
	void BeginCalibration(int estimatedTokens);

	// 应用矫正：传入真实 token 数，累加并保存到注册表
	void ApplyCalibration(int actualTokens);

	// 获取当前 apiName 对应的矫正因子
	// = 累计 actualTokens / 累计 estimatedTokens
	static float GetCalibrationFactor();

private:
	// 从注册表加载累计值（estimated 和 actual）
	static void _LoadAccumulated(const std::string& apiName, uint64_t& estimated, uint64_t& actual);

	// 保存累计值到注册表（estimated 和 actual）
	static void _SaveAccumulated(const std::string& apiName, uint64_t estimated, uint64_t actual);

	// 注册表相关
	static constexpr const char* REG_SECTION = "TokenCalibration";

	// 当前矫正会话数据
	int _estimatedTokens = 0;            // 本次矫正的估计 token 数
	std::string _calibrationApiName;     // BeginCalibration 时记录的 apiName
};
