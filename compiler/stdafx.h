#pragma once

struct IUnknown; // Workaround for "combaseapi.h(229): error C2187: syntax error: 'identifier' was unexpected here" when using /permissive-

#include "rookie.h"

std::wstring str2wstr(const char* text);
std::string  wstr2str(const wchar_t* text);