#pragma once

#include <string>
#include <vector>

#include "token.h"
#include "program.h"
#include "errors.h"

class compile_context {
public:
    void push_error(const compile_error &err) {
        errors.push_back(err);
    }

	void fin() {
	}

public:
    std::vector<compile_error> errors;
};