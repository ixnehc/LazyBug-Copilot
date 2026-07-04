// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include "framework.h"

#include <GdiPlus.h>

using namespace Gdiplus;

#pragma comment( lib, "gdiplus.lib" )

#include "resource.h"

#include "commondefines/general.h"

#include "math/imath_all.h"
#include "timer/timer.h"

#include "nlohmann/json.hpp"
using json = nlohmann::ordered_json;

#define always_assert(condition) \
    do { \
        if (!(condition)) { \
            *((volatile int*)0) = 0;  /* 故意访问空指针 */ \
        } \
    } while(0)

// 替换 TRACE 的宏
#define MY_TRACE(fmt, ...) do { \
    static wchar_t buf[64*1024]; \
    swprintf_s(buf, fmt, __VA_ARGS__); \
    OutputDebugStringW(buf); \
} while(0)

//#define LOCAL_SOLUTIONDB_SERVICE

#include "../LazyBugSource/CoreDefines.h"

#include "../LazyBugSource/LazyBugConfig.h"

#define DISABLE_WEBVIEW_CONTEXTMENU

#endif //PCH_H
