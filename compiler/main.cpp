#include "stdafx.h"

#include <iostream>
#include <string>
#include <vector>

#include "code_gen.h"
#include "compiler.h"
#include "backends/p2wast.h"
#include "program_io.h"
#include "runner.h"

#include "libs/stdlib.h"
#include "thirdparty/argagg.hpp"

program compile(const char *filepath) {
	FILE *fp = fopen(filepath, "rb");
	if (!fp)
		throw std::exception("no such file");

	fseek(fp, 0, SEEK_END);
	auto len = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	char *buf = new char[len+1];
	fread(buf, sizeof(char), len, fp);
	buf[len] = 0;

	binding b;

	b.import<rookie_stdlib>();

	auto rc = compiler::default_compiler(b);

	program p;
	std::vector<compile_error> errors;
	if (rc.compile(buf, p, errors)) {
	
		p.dump();
		runner(b).execute(p);

	}
	else {
		printf("Build failed with %d error(s):\n", errors.size());
		for (auto &err : errors) {
			printf("  * (ln: %d, col: %d): %s\n",
				err.line, err.cols,
				err.message.c_str());
		}
		printf("========BUILD: FAILURE========\n");
	}

	delete[] buf;
	fclose(fp);

	return p;
}

int main(int argc, char **argv) {
	argagg::parser argparser{ {
		{ "help", {"-h", "--help"},
		  "shows the help messages", 0},
		{ "out", {"-o", "--out"},
		  "sets output path", 1},
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
			throw std::exception("No input provided.");
#endif
		}

		program p;
		for (auto path : args.pos) {
			p = compile(path);

			// compiling from multiple 
			// sources is not implemented yet.
			break;
		}

		if (args["out"]) {
			program_writer::write(args["out"].all[0].arg, p);
		}

		p.dump();

		if (args["wasm"] || true) {
			auto p2 = p2wast();
			p2.convert(p);
			p2.dump();
		}

		return 0;
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return -1;
	}
}

