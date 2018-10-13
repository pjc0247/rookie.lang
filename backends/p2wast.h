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
        for (const char *ptr : allocs)
            delete[] ptr;
    }

    void append(const char *str) {
        ropes.push_back(str);
    }
    void append_fmt(const char * format, ...) {
        char *buffer = new char[256];
        va_list args;
        va_start(args, format);
        vsprintf(buffer, format, args);
        va_end(args);

        allocs.push_back(buffer);
        append(buffer);
    }
    void append(char c) {
        auto buf = new char[2] { c, 0 };
        allocs.push_back(buf);
        append(buf);
    }

    void read(const std::function<void(const char *)> &callback) {
        for (const char *chunk : ropes)
            callback(chunk);
    }

private:
    std::list<const char*> allocs;
    std::list<const char*> ropes;
};

struct wastmapping {
    opcode opcode;
    const char *instruction;
    char stackitem;

    wastmapping(opcode_t opcode, const char *instruction, char stackitem) :
        opcode(opcode),
        instruction(instruction), stackitem(stackitem){
    }
};
#define _wastmapitem(opcode, inst, stackitem) wastmapping(opcode, inst##" ", stackitem),
wastmapping _wastmap[] = {
    //           OPCODE      WASMOP               STACKITEMS
    _wastmapitem(op_nop    , "nop"              , 0)

    _wastmapitem(op_ldi    , "i32.const %d"     , 0)
    _wastmapitem(op_ldloc  , "get_local %d"     , 0)

    _wastmapitem(op_add    , "i32.add"            , 2)
    _wastmapitem(op_sub    , "i32.sub"            , 2)
    _wastmapitem(op_mul    , "i32.mul"            , 2)
    _wastmapitem(op_div    , "i32.div"            , 2)
    _wastmapitem(op_g      , "i32.gt_s"            , 2)
    _wastmapitem(op_ge     , "i32.ge_s"            , 2)
    _wastmapitem(op_l      , "i32.lt_s"            , 2)
    _wastmapitem(op_le     , "i32.le_s"            , 2)
};

// Converts rookie program into WebAssemblyTextFormat.
class p2wast {
public:
    void convert(const program &p) {
        emit_program(p);
    }
    void dump() {
        out.read([](const char *chunk) {
            printf("%s", chunk);
        });
    }

private:
    void emit_program(const program &p) {
        out.append("(module \r\n");
        out.append("(memory 1)\r\n");
        out.append("(export \"rdata\" memory)\r\n");

        out.append("(data (i32.const 0) \"");
        for (int i = 0; i < p.header.rdata_len; i++)
            out.append(p.rdata[i]);
        out.append("\")\r\n");

        for (int i = 0; i < p.header.entry_len; i++)
            emit_entry(p, p.entries[i]);

        out.append(")");
    }
    void emit_entry(const program &p, const program_entry &entry) {
        out.append_fmt("(func $%s ", entry.signature);

        for (int i = 0; i < entry.params; i++)
            out.append(" (param i32) ");

        if (entry.ret != ptype::t_none) {
            if (entry.ret == ptype::t_integer)
                out.append(" (result i32) ");
            if (entry.ret == ptype::t_string)
                out.append(" (result i32) ");
        }
        out.append("\r\n");
        emit_body(p, entry);
        out.append(")\r\n");
        out.append_fmt("(export \"%s\" (func $%s)\n",
            entry.signature, entry.signature);
    }

    void emit_body(const program &p, const program_entry &entry) {
        int stackdepth = 0;

        for (int i = entry.entry; i < entry.entry + entry.codesize; i++) {
            auto &inst = p.code[i];

            if (inst.opcode == opcode::op_nop)
                continue;

            switch (inst.opcode) {
            case opcode::op_ldloc:
            case opcode::op_ldi:
            case opcode::op_ldnull:
            case opcode::op_ldstate:
                stackdepth++;
                break;
            case opcode::op_ret:
            case opcode::op_stloc:
            case opcode::op_ststate:
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

                out.append("\r\n");

                if (istack.empty() == false)
                    printf("BUG COMPILER \n");
            }
        }
    }

#define _emit_front() do { emit_instruction(p, istack.top(), depth + 1); istack.pop(); } while(0)

    void emit_instruction(const program &p, const instruction &inst, int depth = 0) {
        for (int i = 0; i < depth * 2; i++)
            out.append(' ');
        out.append("\n (");

        if (inst.opcode == opcode::op_ldi)
            out.append_fmt("i32.const %d", inst.operand);
        else if (inst.opcode == opcode::op_ldloc)
            out.append_fmt("get_local %d", inst.operand);

        // uses 1 stackitem
        else if (inst.opcode == opcode::op_ret)
            _emit_front();
        else if (inst.opcode == opcode::op_stloc) {
            out.append_fmt("set_local %d", inst.operand);
            _emit_front();
        }

        // uses 2 stackitems
        else if (inst.opcode == opcode::op_g) {
            out.append("i32.gt_s ");
            _emit_front(); _emit_front();
        }

        else if (inst.opcode == opcode::op_add) {
            out.append("i32.add ");
            _emit_front(); _emit_front();
        }
        else if (inst.opcode == opcode::op_sub) {
            out.append("i32.sub ");
            _emit_front(); _emit_front();
        }
        else if (inst.opcode == opcode::op_div) {
            out.append("i32.div ");
            _emit_front(); _emit_front();
        }
        else if (inst.opcode == opcode::op_mul) {
            out.append("i32.mul ");
            _emit_front(); _emit_front();
        }

        else if (inst.opcode == opcode::op_call) {
            out.append_fmt("call ");
        }

        else {
            out.append_fmt(";; missing instruction, %s \n", to_string((opcode_t)inst.opcode));
        }

        out.append(")");
    }

private:
    rope out;

    std::stack<instruction> istack;
};