#include "stdafx.h"

#include <string>
#include <regex>
#include <list>
#include <stack>
#include <vector>
#include <map>

#include "code_gen.h"
#include "compiler.h"
#include "p2wast.h"
#include "program_io.h"
#include "runner.h"

#include <windows.h>

int main()
{
    program p;
    std::vector<compile_error> errors;

    if (compiler().compile(
        "class a { def _ctor(a, c) { a = 1; b = 4; c = 5; } def foo() { print(1234, 56); } }",
        p, errors)) {

		//p.dump();

		//program_writer::write("a.rky", p);
		/*
		auto p2 = p2wast();
		p2.convert(p);
		p2.dump();
		*/

		//runner().execute(p);
    }
    else {
        for (auto &err : errors)
            printf("%s\n", err.message.c_str());
    }

    return 0;
}

