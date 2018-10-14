#pragma once

#include <map>
#include <string>
#include <functional>

#include "value_object.h"

#define _bind(signature, lambda) \
    methods[signature] = lambda

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

typedef std::map<std::string, std::function<void(stack_provider&)>> bindmap;

class type_builder {
public:
	type_builder(const std::string &name) :
		name(name) {

	}

	type_builder &method(const std::string &signature,
		const std::function<void()> &function) {

		_bind(signature, [function](stack_provider &sp) {
			function();
		});
		return *this;
	}
	type_builder & method(const std::string &signature,
		const std::function<void(value)> &function) {

		_bind(signature, [function](stack_provider &sp) {
			function(sp.pop());
		});
		return *this;
	}
	
	const std::string &get_name() const {
		return name;
	}
	const bindmap &get_methods() const {
		return methods;
	}

private:
	std::string name;
	bindmap methods;
};

class binding {
public:
	void add_type(type_builder &type) {
		types.push_back(type);
	}

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

	const std::vector<type_builder> &get_types() const {
		return types;
	}
    const bindmap &get_methods() const {
        return methods;
    }

private:
	std::vector<type_builder> types;

    bindmap methods;
};