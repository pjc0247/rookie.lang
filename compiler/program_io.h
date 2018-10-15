#pragma once

#include <stdio.h>
#include <string>

#include "program.h"

class program_writer {
public:
    static bool write(const std::wstring &path, const program &p) {
        FILE *fp = _wfopen(path.c_str(), L"wb");
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
    static bool read(const std::wstring &path, program &p) {
        FILE *fp = _wfopen(path.c_str(), L"rb");
        if (fp == 0) return false;

        fread(&p.header, sizeof(program_header), 1, fp);

        p.entries = (program_entry*)malloc(sizeof(program_entry) * p.header.entry_len);
        fread(p.entries, sizeof(program_entry), p.header.entry_len, fp);
        p.code = (instruction*)malloc(sizeof(instruction) * p.header.code_len);
        fread(p.code, sizeof(instruction), p.header.code_len, fp);
        p.rdata = (const wchar_t*)malloc(sizeof(wchar_t) * p.header.rdata_len);
        fread((wchar_t*)p.rdata, sizeof(wchar_t), p.header.rdata_len, fp);

        fclose(fp);
        return true;
    }
};