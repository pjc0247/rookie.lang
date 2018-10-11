#pragma once

#include <map>
#include <string>
#include <functional>

class binding {
public:

private:
	std::map<std::string, std::function<void()>> bindings;
};