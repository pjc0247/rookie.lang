#pragma once

#include <stdio.h>
#include <tchar.h>


#if _DEBUG
#define DEBUG 1
#else 
#define DEBUG 0
#endif

#define rklog(fmt, ...) do { if (DEBUG) printf(fmt, __VA_ARGS__); } while (0)