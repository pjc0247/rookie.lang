#pragma once

#include <stdio.h>
#include <string>

#include "program.h"

class program_writer {
public:
	static bool write(const std::string &path, const program &p) {
		FILE *fp = fopen(path.c_str(), "wb");
		if (fp == 0) return false;

		fwrite(&p.header, sizeof(program_header), 1, fp);
		fwrite(p.entries, sizeof(program_entry), p.header.entry_len, fp);
		fwrite(p.code, sizeof(instruction), p.header.code_len, fp);
		fwrite(p.rdata, sizeof(char), p.header.rdata_len, fp);

		fclose(fp);
		return true;
	}
};
class program_reader {
public:
	static bool read(const std::string &path, program &p) {
		FILE *fp = fopen(path.c_str(), "rb");
		if (fp == 0) return false;

		fread(&p.header, sizeof(program_header), 1, fp);

		p.entries = (program_entry*)malloc(sizeof(program_entry) * p.header.entry_len);
		fread(p.entries, sizeof(program_entry), p.header.entry_len, fp);
		p.code = (instruction*)malloc(sizeof(instruction) * p.header.code_len);
		fread(p.code, sizeof(instruction), p.header.code_len, fp);
		p.rdata = (const char*)malloc(sizeof(char) * p.header.rdata_len);
		fread((char*)p.rdata, sizeof(char), p.header.rdata_len, fp);

		fclose(fp);
		return true;
	}
};