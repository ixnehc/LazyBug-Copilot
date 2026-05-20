#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <deque>
#include <time.h>
#include "timer/timer.h"

#include "Utils_File.h"

struct FindInFileResults;

namespace Utils
{

// ---------------------------------------------------------------------------
// 文件路径通配符匹配工具
//
// 支持的通配符：
//   ?   匹配除路径分隔符（'/' '\\'）以外的任意单个字符
//   *   匹配除路径分隔符以外的任意数量（含0个）字符（不跨目录）
//   **  匹配任意数量（含0个）任意字符（可跨目录）
//
// 说明：
//   - 匹配不区分大小写（内部自动转小写）
//   - 无通配符时退化为子串包含匹配
// ---------------------------------------------------------------------------

// 判断 pattern 中是否含有通配符（'*' 或 '?'）
bool HasWildcard(const std::string& pattern);

// 将通配符模式转换为对应的正则表达式字符串
std::string WildcardToRegexString(const std::string& pattern);

// 判断 text 是否匹配 pattern（通配符模式，不区分大小写）
// pattern 和 text 均可为原始大小写，内部会自动转换
bool MatchWildcardPath(const std::string& pattern, const std::string& text);

// 综合匹配：
//   - 含通配符 → 调用 MatchWildcardPath
//   - 不含通配符 → 子串包含匹配（与原有逻辑一致）
// lowerPattern 必须已经是小写
bool MatchFilePath(const std::string& lowerPattern, const std::string& lowerText);

}
