#include "stdafx.h"

#include <codecvt>
#include <locale>

#include "code_gen.h"
#include "compiler.h"
#include "backends/p2wast.h"
#include "program_io.h"
#include "runner.h"
#include "fileio.h"

#include "libs/array.h"
#include "libs/string.h"
#include "libs/stdlib.h"

#include "extern_interface.h"

void rk_exec(const char *code) {
    binding b;

    b.import<rkstdlib>();
    b.import<rkarray>();
    b.import<rkstring>();

    auto rc = compiler::default_compiler(b);

    compile_option opts;

    auto out = rc.compile(str2wstr(code), opts);
    if (out.errors.empty()) {
        runner(out.program, b).execute();
    }
}