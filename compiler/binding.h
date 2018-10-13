#pragma once

#include <map>
#include <string>
#include <functional>

#include "runner.h"
#include "value_object.h"

#define _bind(signature, lambda) \
	bindings[signature] = lambda

class binding {
public:
	void add(const std::string &signature,
		const std::function<void()> &function) {

		_bind(signature, [function](stack_provider &sp) {
			function();
		});
	}
	void add(const std::string &signature,
		const std::function<void(value)> &function) {

		_bind(signature, [function](stack_provider &sp) {
			function(sp.pop());
		});
	}

private:
    std::map<std::string, std::function<void(stack_provider&)>> bindings;
};