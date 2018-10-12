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

	//
    if (compiler::default_compiler().compile(
        "class a { def _ctor(a, b) { for (i=0;i<10;i++;) foo(); } def foo() { a = 4 >= 4; } def bar(a) { } }",
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

