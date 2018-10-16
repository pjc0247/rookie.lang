﻿#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string>

#if _DEBUG
#define DEBUG 1
#else 
#define DEBUG 0
#endif

#define rklog(fmt, ...) do { if (DEBUG) printf(fmt, __VA_ARGS__); } while (0)

#if WIN32
#define stdinvoke std::invoke
#define CONSTEXPR constexpr
#define THREAD_LOCAL thread_local
#else
#define __forceinline inline
#define stdinvoke std::__invoke
#define CONSTEXPR
#define THREAD_LOCAL
#endif


#ifdef __EMSCRIPTEN__
    #define RK_ENV_WEB
#else
    #define RK_ENV_NATIVE
#endif

std::wstring str2wstr(const char* text);