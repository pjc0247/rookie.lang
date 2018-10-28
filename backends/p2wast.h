#pragma once

#include <string>
#include <list>
#include <stack>
#include <functional>
#include <stdarg.h>

#include "program.h"

// rope for stricted usage
class rope {
public:
    virtual ~rope() {
        for (const wchar_t *ptr : allocs)
            delete[] ptr;
    }

    void append(const wchar_t *str) {
        ropes.push_back(str);
    }
    void append_fmt(const wchar_t *format, ...) {
        wchar_t *buffer = new wchar_t[256];
        va_list args;
        va_start(args, format);
        vswprintf(buffer, 256, format, args);
        va_end(args);

        allocs.push_back(buffer);
        append(buffer);
    }
    void append(wchar_t c) {
        auto buf = new wchar_t[2] { c, 0 };
        allocs.push_back(buf);
        append(buf);
    }

    void read(const std::function<void(const wchar_t *)> &callback) {
        for (const wchar_t *chunk : ropes)
            callback(chunk);
    }

private:
    std::list<const wchar_t*> allocs;
    std::list<const wchar_t*> ropes;
};

// Converts rookie program into WebAssemblyTextFormat.
class p2wast {
public:
    void convert(const program &p) {
        emit_program(p);
    }
    void dump() {
        out.read([](const wchar_t *chunk) {
            printf("%S", chunk);
        });
    }

private:
    void emit_program(const program &p) {
        out.append(L"(module \r\n");
        out.append(L"(memory 1)\r\n");
        out.append(L"(export \"rdata\" memory)\r\n");

        out.append(L"(data (i32.const 0) \"");
        for (uint32_t i = 0; i < p.header.rdata_len; i++)
            out.append(p.rdata[i]);
        out.append(L"\")\r\n");

        for (uint32_t i = 0; i < p.header.entry_len; i++)
            emit_entry(p, p.entries[i]);

        out.append(L")");
    }
    void emit_entry(const program &p, const program_entry &entry) {
        out.append_fmt(L"(func $%s ", entry.signature);

        for (uint32_t i = 0; i < entry.params; i++)
            out.append(L" (param i32) ");

        /*
        if (entry.ret != ptype::t_none) {
            if (entry.ret == ptype::t_integer)
                out.append(" (result i32) ");
            if (entry.ret == ptype::t_string)
                out.append(" (result i32) ");
        }
        */
        out.append(L"\r\n");
        emit_body(p, entry);
        out.append(L")\r\n");
        out.append_fmt(L"(export \"%s\" (func $%s)\n",
            entry.signature, entry.signature);
    }

    void emit_body(const program &p, const program_entry &entry) {
        int stackdepth = 0;

        for (uint32_t i = entry.entry; i < entry.entry + entry.codesize; i++) {
            auto &inst = p.code[i];

            if (inst.opcode == opcode::op_nop)
                continue;

            switch (inst.opcode) {
            case opcode::op_ldloc:
            case opcode::op_ldi:
            case opcode::op_ldnull:
            case opcode::op_ldprop:
                stackdepth++;
                break;
            case opcode::op_ret:
            case opcode::op_stloc:
            case opcode::op_stprop:
                stackdepth--;
                break;

            case opcode::op_add:
            case opcode::op_sub:
            case opcode::op_mul:
            case opcode::op_div:
                stackdepth -= 2;
                break;

            case opcode::op_call:
                stackdepth -= entry.params;
                break;
            }

            istack.push(inst);

            if (stackdepth < 0)
                printf("invalid program (stack underflow)\n");
            if (stackdepth == 0) {
                auto front = istack.top();
                istack.pop();
                emit_instruction(p, front);

                out.append(L"\r\n");

                if (istack.empty() == false)
                    printf("BUG COMPILER \n");
            }
        }
    }

#define _emit_front() do { emit_instruction(p, istack.top(), depth + 1); istack.pop(); } while(0)

    void emit_instruction(const program &p, const instruction &inst, int depth = 0) {
        for (int i = 0; i < depth * 2; i++)
            out.append(' ');
        out.append(L"\n (");

        if (inst.opcode == opcode::op_ldi)
            out.append_fmt(L"i32.const %d", inst.operand);
        else if (inst.opcode == opcode::op_ldloc)
            out.append_fmt(L"get_local %d", inst.operand);

        // uses 1 stackitem
        else if (inst.opcode == opcode::op_ret)
            _emit_front();
        else if (inst.opcode == opcode::op_stloc) {
            out.append_fmt(L"set_local %d", inst.operand);
            _emit_front();
        }

        // uses 2 stackitems
        else if (inst.opcode == opcode::op_g) {
            out.append(L"i32.gt_s ");
            _emit_front(); _emit_front();
        }

        else if (inst.opcode == opcode::op_add) {
            out.append(L"i32.add ");
            _emit_front(); _emit_front();
        }
        else if (inst.opcode == opcode::op_sub) {
            out.append(L"i32.sub ");
            _emit_front(); _emit_front();
        }
        else if (inst.opcode == opcode::op_div) {
            out.append(L"i32.div ");
            _emit_front(); _emit_front();
        }
        else if (inst.opcode == opcode::op_mul) {
            out.append(L"i32.mul ");
            _emit_front(); _emit_front();
        }

        else if (inst.opcode == opcode::op_call) {
            out.append_fmt(L"call ");
        }

        else {
            out.append_fmt(L";; missing instruction, %s \n", to_string((opcode_t)inst.opcode));
        }

        out.append(L")");
    }

private:
    rope out;

    std::stack<instruction> istack;
};