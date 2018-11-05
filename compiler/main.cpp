#include "stdafx.h"

#include <iostream>
#include <string>
#include <vector>

#include "backends/p2wast.h"
#include "fileio.h"
#include "rookie.h"

#include "thirdparty/argagg.hpp"

#ifdef _MSC_VER
#include <filesystem>
#define fs std::experimental::filesystem
#else
#include <experimental/filesystem>
#define fs std::filesytem
#endif

// Copmpiles given string into a program.
program *compile(const std::wstring &filepath) {
#ifndef RK_ENV_WEB
    wchar_t *buf = fileio::read_string(filepath);

    auto b = binding::default_binding();
    auto rc = compiler::default_compiler(b);

    compile_option opts;
    opts.generate_pdb = true;

    auto out = rc.compile(buf, opts);
    if (out.errors.empty()) {
    
#if _DEBUG
        out.program->dump();
#endif

        debugger dbg(*out.pdb);
        runner(*out.program, b)
#if _DEBUG
            .attach_debugger(dbg)
#endif
            .execute();
    }
    else {
        printf("Build failed with %d error(s):\n", out.errors.size());
        for (auto &err : out.errors) {
            wprintf(L"  * (ln: %d, col: %d): %s\n",
                err.line, err.cols,
                err.message.c_str());
        }
        printf("========BUILD: FAILURE========\n");
    }

    delete[] buf;

    return out.program;
#else
    throw rkexception("Not supported");
#endif
}

int main(int argc, char **argv) {
#ifdef RK_ENV_WEB
    printf("[rookie::webenv].\n");
#endif
#ifdef RK_ENV_NATIVE
    setlocale(LC_ALL, "");
    argagg::parser argparser{ {
        { "help", {"-h", "--help"},
          "shows the help messages", 0},
        { "out", {"-o", "--out"},
          "sets output path", 1},
        { "debug-vm",{ "--debug-vm" },
          "prints additional output for vm", 0},

        { "wasm", {"-w", "--wasm"},
          "generated wast file and write it", 1}
    }};
    argagg::parser_results args;
    try {
        args = argparser.parse(argc, argv);

        if (args["help"]) {
            printf("[rookie] build. %s\n", __DATE__);
            return 0;
        }

        if (args.pos.size() == 0) {
#if _DEBUG
            args.pos.push_back("testcode.rk");
#else
            throw rkexception("No input provided.");
#endif
        }

        program *p = nullptr;
        for (auto path : args.pos) {
            p = compile(str2wstr(path));

            auto dir = fs::path(path).parent_path();

            if (dir != L"")
                fs::current_path(dir);

            // compiling from multiple 
            // sources is not implemented yet.
            break;
        }

        if (args["out"]) {
            //program_writer::write(args["out"].all[0].arg, p);
        }

        if (args["wasm"]) {
            auto p2 = p2wast();
            p2.convert(*p);
            p2.dump();
        }

        return 0;
    }
#ifndef _DEBUG
    catch (rkexception e) {
        std::cout << e.what() << std::endl;
    }
    catch (std::exception e) {
        std::cout << e.what() << std::endl;
        return -1;
    }
#else
    catch (int n) { }
#endif
#endif
}

