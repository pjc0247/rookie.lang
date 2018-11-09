#pragma once

#include "binding.h"
#include "object.h"

class rkexception : public rkobject<rkexception> {
public:
    TYPENAME(L"exception")

    static void import(binding &b) {
        auto type = type_builder(L"exception");

		method(type, rk_id_tostring, &rkexception::to_string);

        b.add_type(type);
    }

	rkexception(base_exception *ex) {
		assert(ex != nullptr);

		msg = str2wstr(ex->what());
	}
	rkexception(const std::wstring &msg) :
		msg(msg) {
	}

	void set_callstack(const std::deque<callframe> &cs) {
		callstack = cs;
	}

	value to_string() {
		std::wstring str;
		str = L"#<exception: " + msg + L">\r\n";
		str += L"[[STACKTRACE]]\r\n";
		for (int i=callstack.size()-1; i>= 0; i--) {
			auto &c = callstack[i];
			str += L"   * " + std::wstring(c.entry->signature) + L"\r\n";
		}
		return str2rk(str);
	}

private:
	std::wstring msg;

	std::deque<callframe> callstack;
};