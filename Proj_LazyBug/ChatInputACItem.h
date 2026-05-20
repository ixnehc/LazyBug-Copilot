#pragma once
#include <string>

#include "timer/wuid.h"


// 自动补全项结构
struct ChatInputACItem
{
	WUID id;
    std::string text;        // 显示文本
	std::string description; // 描述信息
	std::string fullPath;		// 完整路径
    std::string type;        // 类型（用于样式）
	int kind;
	SingleLineLoc loc;
};
