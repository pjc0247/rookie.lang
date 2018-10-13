#pragma once

#include <map>
#include <string>
#include <functional>

#include "value_object.h"

#define _bind(signature, lambda) \
	map[signature] = lambda

class stack_provider {
public:
	stack_provider(std::deque<value> &stack) :
		stackref(stack) {
	}

	void push(const value &v) {
		stackref.push_back(v);
	}
	value pop() {
		auto item = stackref.back();
		stackref.pop_back();
		return item;
	}

private:
	std::deque<value> &stackref;
};

class binding {
public:
	typedef std::map<std::string, std::function<void(stack_provider&)>> bindmap;

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

	template <typename T>
	binding &import() {
		printf("[import] %s\n", typeid(T).name());
		T().import(*this);
		return *this;
	}

	const bindmap &bindings() const {
		return map;
	}

private:
	bindmap map;
};