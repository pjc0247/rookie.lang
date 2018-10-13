#pragma once

#include <string>
#include <map>
#include <deque>
#include <exception>

#include "program.h"
#include "errors.h"

#include "value_object.h"
#include "gc.h"
#include "binding.h"

#define _invalid_stackitem \
    throw new invalid_program_exception("invalid stackitem, expected integer");

#define _ensure_int(a) \
    if (a.type != value_type::integer) \
        _invalid_stackitem;
#define _ensure_int(a,b) \
    if (a.type != value_type::integer || \
        b.type != value_type::integer) \
        _invalid_stackitem;

#define _pop1_int(a) \
    autoa = pop(); _ensure_int(a);
#define _pop2_int(a, b) \
    auto a = pop(); auto b = pop(); _ensure_int(a,b);

class runner {
public:
	runner(binding &binding) :
		binding(binding) {

		for (auto &b : binding.bindings()) {
			syscalls.table.push_back(b.second);
		}
	}

    void execute(const program &p) {
        if (p.header.entry_len == 0)
            throw new std::invalid_argument("program does not have any entries.");

        execute(p, &p.entries[0]);
    }
    void execute(const program &p, program_entry *_entry) {
        printf("===execute===\n");

        current_entry = _entry;
        pc = _entry->entry;
        bp = 1;

        push_callframe(*_entry);

        while (true) {
            if (pc >= p.header.code_len)
                throw new invalid_program_exception("unexpected end of program.");

            auto inst = p.code[pc++];

            printf("%s\n", to_string((opcode_t)inst.opcode));

			if (inst.opcode == opcode::op_nop);
			else if (inst.opcode == opcode::op_ldi)
				stack.push_back(value::mkinteger(inst.operand));
			else if (inst.opcode == opcode::op_ldstr)
				stack.push_back(value::mkstring(p.rdata + inst.operand));
            else if (inst.opcode == opcode::op_ldstate)
                stack.push_back(value::mkstring(p.rdata + inst.operand));

            else if (inst.opcode == opcode::op_l) {
                _pop2_int(left, right);
                push(value::mkinteger(left.integer > right.integer));
            }
            else if (inst.opcode == opcode::op_g) {
                _pop2_int(left, right);
                push(value::mkinteger(left.integer < right.integer));
            }
            else if (inst.opcode == opcode::op_le) {
                _pop2_int(left, right);
                push(value::mkinteger(left.integer >= right.integer));
            }
            else if (inst.opcode == opcode::op_ge) {
                _pop2_int(left, right);
                push(value::mkinteger(left.integer <= right.integer));
            }

            else if (inst.opcode == opcode::op_add) {
                _pop2_int(left, right);
                left.integer += right.integer;
                push(left);
            }
            else if (inst.opcode == opcode::op_sub) {
                _pop2_int(left, right);
                left.integer -= right.integer;
                push(left);
            }
            else if (inst.opcode == opcode::op_div) {
                _pop2_int(left, right);
                // TODO: check right is zero
                left.integer /= right.integer;
                push(left);
            }
            else if (inst.opcode == opcode::op_mul) {
                _pop2_int(left, right);
                left.integer *= right.integer;
                push(left);
            }

            else if (inst.opcode == opcode::op_newobj) {
                auto objref = new object();
                push(value::mkobjref(objref));

                gc.add_object(objref);
            }

            else if (inst.opcode == opcode::op_call) {
				if (inst.cs.lookup_type == callsite_lookup::cs_method) {
					auto entry = p.entries[inst.cs.index];
					push_callframe(entry);
					pc = entry.entry;
					bp = stack.size() - entry.params;
					current_entry = &entry;
				}
				else if (inst.cs.lookup_type == callsite_lookup::cs_syscall) {
					printf("SYSCALL %d\n", inst.cs.index);
					auto sp = stack_provider(stack);
					syscalls.table[inst.cs.index](sp);
				}
            }
            else if (inst.opcode == opcode::op_ret) {
                auto callframe = pop_callframe(*current_entry);
                pc = callframe->pc;
                bp = callframe->bp;
                current_entry = callframe->entry;
                delete callframe;

                if (stack.empty()) break;
            }

            else if (inst.opcode == opcode::op_ldloc)
                stack.push_back(stack[bp + inst.operand]);
            else if (inst.opcode == opcode::op_stloc) {
                stack[bp + inst.operand] = stack.back();
                stack.pop_back();
            }

            else if (inst.opcode == opcode::op_jmp_true) {
                if (pop().integer != 0)
                    pc = inst.operand;
            }
            else if (inst.opcode == opcode::op_jmp_false) {
                if (pop().integer == 0)
                    pc = inst.operand;
            }

            else
                throw new invalid_program_exception("unknown instruction.");
        }
    }

private:
    value pop() {
        if (stack.empty())
            throw new invalid_program_exception("stack underflow");
        auto item = stack.back();
        stack.pop_back();
        return item;
    }
    void push(const value &v) {
        stack.push_back(v);
    }

    void push_callframe(program_entry &entry) {
        stack.push_back(value::mkcallframe(pc, bp, current_entry));
        for (int i = 0; i < entry.locals - entry.params; i++)
            stack.push_back(value());
    }
    callframe *pop_callframe(program_entry &entry) {
        for (int i = 0; i < entry.locals; i++)
            stack.pop_back();

        auto callframe = stack.back();
        if (callframe.type != value_type::callframe)
            throw new invalid_program_exception("unexpected stack item.");
        stack.pop_back();

        return callframe.cframe;
    }

private:
	binding &binding;
	syscalltable syscalls;

    gc gc;

    program_entry *current_entry;
    short pc; // program counter
    short bp; // base stack pointer

    std::deque<value> stack;
};