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
        static void import(binding &b) {
#define _end_rookie_library \
    } };
#define _rookie_function(name, body) \
    b.function(name, [] body );

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

	__forceinline value &get(int n) {
		return stackref[stackref.size() - 1 - n];
	}
	
	template <int N>
	__forceinline void drop() {
		stackref.pop_back();
		drop<N - 1>();
	}
	template <>
	__forceinline void drop<1>() {
		stackref.pop_back();
	}

	template <int N>
	__forceinline void replace(const value &v) {
		stackref[stackref.size() - 1 - N] = v;
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
        const std::function<value()> &function) {

        _bind(signature, [function](stack_provider &sp) {
            sp.push(function());
        });
        return *this;
    }
    type_builder &method(const std::string &signature,
        const std::function<value(value&)> &function) {

        _bind(signature, [function](stack_provider &sp) {
			sp.replace<0>(function(sp.get(0)));
        });
        return *this;
    }
	type_builder &method(const std::string &signature,
		const std::function<value(value&, value&)> &function) {

		_bind(signature, [function](stack_provider &sp) {
			sp.replace<1>(function(sp.get(1), sp.get(0)));
			sp.drop<1>();
		});
		return *this;
	}
	type_builder &method(const std::string &signature,
		const std::function<value(value&, value&, value&)> &function) {

		_bind(signature, [function](stack_provider &sp) {
			sp.replace<2>(function(sp.get(2), sp.get(1), sp.get(0)));
			sp.drop<2>();
		});
		return *this;
	}
	type_builder &method(const std::string &signature,
		const std::function<value(value&, value&, value&, value&)> &function) {

		_bind(signature, [function](stack_provider &sp) {
			sp.replace<3>(function(sp.get(3), sp.get(2), sp.get(1), sp.get(0)));
			sp.drop<3>();
		});
		return *this;
	}
	type_builder &method(const std::string &signature,
		const std::function<value(value&, value&, value&, value&, value&)> &function) {

		_bind(signature, [function](stack_provider &sp) {
			sp.replace<4>(function(sp.get(4), sp.get(3), sp.get(2), sp.get(1), sp.get(0)));
			sp.drop<4>();
		});
		return *this;
	}
	type_builder &method(const std::string &signature,
		const std::function<value(value&, value&, value&, value&, value&, value&)> &function) {

		_bind(signature, [function](stack_provider &sp) {
			sp.replace<5>(function(sp.get(5), sp.get(4), sp.get(3), sp.get(2), sp.get(1), sp.get(0)));
			sp.drop<5>();
		});
		return *this;
	}
	type_builder &method(const std::string &signature,
		const std::function<value(value&, value&, value&, value&, value&, value&, value&)> &function) {

		_bind(signature, [function](stack_provider &sp) {
			sp.replace<6>(function(sp.get(6), sp.get(5), sp.get(4), sp.get(3), sp.get(2), sp.get(1), sp.get(0)));
			sp.drop<6>();
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

    void function(const std::string &signature,
        const std::function<value()> &function) {

        _bind(signature, [function](stack_provider &sp) {
            sp.push(function());
        });
    }
    void function(const std::string &signature,
        const std::function<value(value&)> &function) {

        _bind(signature, [function](stack_provider &sp) {
			sp.replace<0>(function(sp.get(0)));
        });
    }
	void function(const std::string &signature,
		const std::function<value(value&,value&)> &function) {

		_bind(signature, [function](stack_provider &sp) {
			sp.replace<1>(function(sp.get(1), sp.get(0)));
			sp.drop<1>();
		});
	}
	void function(const std::string &signature,
		const std::function<value(value&, value&, value&)> &function) {

		_bind(signature, [function](stack_provider &sp) {
			sp.replace<2>(function(sp.get(2), sp.get(1), sp.get(0)));
			sp.drop<2>();
		});
	}
	void function(const std::string &signature,
		const std::function<value(value&, value&, value&, value&)> &function) {

		_bind(signature, [function](stack_provider &sp) {
			sp.replace<3>(function(sp.get(3), sp.get(2), sp.get(1), sp.get(0)));
			sp.drop<3>();
		});
	}
	void function(const std::string &signature,
		const std::function<value(value&, value&, value&, value&, value&)> &function) {

		_bind(signature, [function](stack_provider &sp) {
			sp.replace<4>(function(sp.get(4), sp.get(3), sp.get(2), sp.get(1), sp.get(0)));
			sp.drop<4>();
		});
	}
	void function(const std::string &signature,
		const std::function<value(value&, value&, value&, value&, value&, value&)> &function) {

		_bind(signature, [function](stack_provider &sp) {
			sp.replace<5>(function(sp.get(5), sp.get(4), sp.get(3), sp.get(2), sp.get(1), sp.get(0)));
			sp.drop<5>();
		});
	}
	void function(const std::string &signature,
		const std::function<value(value&, value&, value&, value&, value&, value&, value&)> &function) {

		_bind(signature, [function](stack_provider &sp) {
			sp.replace<6>(function(sp.get(6), sp.get(5), sp.get(4), sp.get(3), sp.get(2), sp.get(1), sp.get(0)));
			sp.drop<6>();
		});
	}

    template <typename T>
    binding &import() {
        printf("[import] %s\n", typeid(T).name());
        T::import(*this);
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

template <typename T>
class rkobject : public object {
public:
	static value create_instance() {
		return value::mkobjref(new T());
	}

	static void method(type_builder &type,
		const char *name, value(T::*function)()) {

		type.method(name, [function](value &_this) {
			auto obj = ((T*)_this.objref);
			return std::invoke(function, obj);
		});
	}
	static void method(type_builder &type,
		const char *name, value(T::*function)(value&)) {

		type.method(name, [function](value &_this, value &a) {
			auto obj = ((T*)_this.objref);
			return std::invoke(function, obj, a);
		});
	}
	static void method(type_builder &type,
		const char *name, value(T::*function)(value&, value&)) {

		type.method(name, [function](value &_this, value &a, value &b) {
			auto obj = ((T*)_this.objref);
			return std::invoke(function, obj, a, b);
		});
	}
	static void method(type_builder &type,
		const char *name, value(T::*function)(value&, value&, value&)) {

		type.method(name, [function](value &_this, value &a, value &b, value &c) {
			auto obj = ((T*)_this.objref);
			return std::invoke(function, obj, a, b, c);
		});
	}
	static void method(type_builder &type,
		const char *name, value(T::*function)(value&, value&, value&, value&)) {

		type.method(name, [function](value &_this, value &a, value &b, value &c, value &d) {
			auto obj = ((T*)_this.objref);
			return std::invoke(function, obj, a, b, c, d);
		});
	}
	static void method(type_builder &type,
		const char *name, value(T::*function)(value&, value&, value&, value&, value&)) {

		type.method(name, [function](value &_this, value &a, value &b, value &c, value &d, value &e) {
			auto obj = ((T*)_this.objref);
			return std::invoke(function, obj, a, b, c, d, e);
		});
	}
	static void method(type_builder &type,
		const char *name, value(T::*function)(value&, value&, value&, value&, value&, value&)) {

		type.method(name, [function](value &_this, value &a, value &b, value &c, value &d, value &e, value &f) {
			auto obj = ((T*)_this.objref);
			return std::invoke(function, obj, a, b, c, d, e, f);
		});
	}
};