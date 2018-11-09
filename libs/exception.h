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

	value to_string() {
		std::wstring str;
		str = L"#<exception: " + msg + L">";
		return str2rk(str);
	}

private:
	std::wstring msg;
};