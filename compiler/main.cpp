#include "stdafx.h"

#include <string>
#include <regex>
#include <list>
#include <stack>
#include <vector>
#include <map>

#include "code_gen.h"
#include "compiler.h"

int main()
{
    //auto tokens = lexer.lex("a = 1; b = 4; if (4) a = 1;");

    /*
    for (auto &token : tokens)
        printf("%s, %d\n", token.raw.c_str(), token.type);
        */

    program p;
    std::vector<compile_error> errors;

    if (compiler().compile(
        "class a { def _ctor() { a = 1; b = 4; c = 5; } }",
        p, errors)) {

    }
    else {
        for (auto &err : errors)
            printf("%s\n", err.message.c_str());
    }

    return 0;
}

