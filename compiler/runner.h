#pragma once

#include <string>
#include <exception>
#include <deque>

#include "program.h"

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

class invalid_program_exception : public std::exception {
public:
	invalid_program_exception(const char *msg) 
		: std::exception(msg) {
	}
};

enum class value_type : char {
	empty,
	callframe,
	integer, string, object
};
struct value {
	value_type type;

	union {
		int integer;
		const char *str;

		short bp_pc[2];
	};
		
	value() :
		type(value_type::empty) {
	}

	static value mkcallframe(short pc, short bp) {
		value v;
		v.type = value_type::callframe;
		v.bp_pc[0] = bp;
		v.bp_pc[1] = pc;
		return v;
	}
	static value mkinteger(int n) {
		value v;
		v.type = value_type::integer;
		v.integer = n;
		return v;
	}
	static value mkstring(const char *str) {
		value v;
		v.type = value_type::string;
		v.str = str;
		return v;
	}
};

class runner {
public:
	void execute(const program &p) {
		if (p.header.entry_len == 0)
			throw new std::invalid_argument("program does not have any entries.");

		auto main = p.entries[0];
		pc = main.entry; // program counter
		bp = 1;

		program_entry *current_entry = &main;
		push_callframe(main);

		while (true) {
			if (pc >= p.header.code_len)
				throw new invalid_program_exception("unexpected end of program.");

			auto inst = p.code[pc];

			if (inst.opcode == opcode::op_nop);
			else if (inst.opcode == opcode::op_ldi)
				stack.push_back(value::mkinteger(inst.operand));

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

			else if (inst.opcode == opcode::op_call) {
				auto entry = p.entries[inst.operand];
				push_callframe(entry);
				pc = entry.entry;
				bp = stack.size() - entry.params;
			}
			else if (inst.opcode == opcode::op_ret) {
				auto callframe = pop_callframe(*current_entry);
				bp = callframe.bp_pc[0];
				pc = callframe.bp_pc[1];
				if (stack.empty()) break;
			}

			else if (inst.opcode == opcode::op_ldloc)
				stack.push_back(stack[bp + inst.operand]);
			else if (inst.opcode == opcode::op_stloc) {
				stack[bp + inst.operand] = stack.back();
				stack.pop_back();
			}

			pc++;
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
		stack.push_back(value::mkcallframe(pc, bp));
		for (int i = 0; i < entry.locals - entry.params; i++)
			stack.push_back(value());
	}
	value pop_callframe(program_entry &entry) {
		for (int i = 0; i < entry.locals; i++)
			stack.pop_back();

		auto callframe = stack.back();
		if (callframe.type != value_type::callframe)
			throw new invalid_program_exception("unexpected stack item.");
		stack.pop_back();

		return callframe;
	}

private:
	short pc; // program counter
	short bp; // base stack pointer
	std::deque<value> stack;
};