#pragma once

namespace Utils
{
	// 估算宽字符字符串的token数量
	int EstimateTokenCount(const std::wstring& text);

	// 估算UTF-8格式字符串的token数量
	int EstimateTokenCount(const std::string& text);
	 
	// 估计文件的Token数
	// - 图片文件：根据图片尺寸估算 (粗略 Token 数 ≈ (宽 × 高) ÷ 1500)
	// - 文本文件：使用 EstimateTokenCount 估算
	// 返回值：估计的Token数，失败返回0
	int EstimateFileTokenCount(const char* filePath);

}
