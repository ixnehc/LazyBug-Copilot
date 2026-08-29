#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <deque>
#include <time.h>
#include <atomic>
#include "timer/timer.h"
#include "embeddingtypes.h"


namespace Utils
{

// 调用 LLM API 生成 embedding。
// enable 用于在请求过程中中止失活的 API 调用，可传 nullptr 表示不启用中止检查。
extern bool CallEmbeddingApi(const EmbedModelParam& modelParam,
                             const std::vector<std::string>& texts,
                             std::vector<std::vector<float>>& outEmbeddings,
                             const std::atomic<bool>* enable = nullptr);

}

