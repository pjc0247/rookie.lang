#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <cstring>
#include <string>
#include <algorithm>

#if _DEBUG
#define DEBUG 1
#else 
#define DEBUG 0
#endif

#define rklog(fmt, ...) do { if (DEBUG) printf(fmt, ##__VA_ARGS__); } while (0)

#ifdef _MSC_VER
#define stdinvoke std::invoke
#else
#define __forceinline inline
#define stdinvoke std::__invoke
#endif

#if defined(__EMSCRIPTEN__) || defined(__clang__)
#define RK_ENV_WEB
#define RK_HALT_ON_LONG_EXECUTION
#else
#define RK_ENV_NATIVE
#endif

/* Some specifiers cannot be used in em++. */
#ifdef RK_ENV_WEB
#define CONSTEXPR
#define THREAD_LOCAL

#define _wfopen(a, b) nullptr
#else
#define CONSTEXPR constexpr
#define THREAD_LOCAL thread_local
#endif

#include "special_name.h"