#pragma once

#include <map>
#include <string>
#include <functional>

#include "value_object.h"

#define _bind(signature, lambda) \
	map[signature] = lambda

#define _rookie_library(name) \
	class name { \
	public: \
		void import(binding &b) {
#define _end_rookie_library \
	} };
#define _rookie_function(name, body) \
	b.add(name, [] body );

#define is_rkint(v) v.type == value_type::integer
#define is_rkstr(v) v.type == value_type::string
#define rkint(v) v.integer
#define rkcstr(v) v.str

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