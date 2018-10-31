#include "stdafx.h"

#include "conout.h"
#include "runner.h"

void debugger::on_begin_program(runner &_r, const program &p) {
    con::setColor(CON_WHITE);
    rklog("[%3s] %15s    %12s    %s\n", "PC", "OPCODE", "OPERAND", "CODE");
    rklog("[%3s] %15s    %12s    %s\n", "---", "-----", "-----", "-----");
    con::setColor(CON_LIGHTGRAY);

    r = &_r;
}
void debugger::on_pre_exec(runner &r, const instruction &inst) {
    con::setColor(CON_LIGHTGRAY);
    rklog("[");
    con::setColor(CON_YELLOW);
    rklog("%3d", r.pc);
    con::setColor(CON_LIGHTGRAY);
    rklog("] %15S    ", to_string((opcode_t)inst.opcode));
    if (inst.opcode == opcode::op_vcall ||
        inst.opcode == opcode::op_call ||
        inst.opcode == opcode::op_syscall ||
        inst.opcode == opcode::op_ldprop ||
        inst.opcode == opcode::op_stprop ||
        inst.opcode == opcode::op_newobj) {

        con::setColor(CON_LIGHTMAGENTA);
        rklog("%12S", pdb.get_name(inst.operand).c_str());
    }
    else {
        con::setColor(CON_WHITE);
        rklog("%12d", inst.operand);
    }
    rklog("    ");

    bool ignoring = true;
    int quote = 0;
    int cnt = -1;

    std::vector<std::wstring> keywords({
        L"for", L"if", L"class", L"def", L"static", L"this", L"in"
    });

    con::setColor(CON_LIGHTGRAY);
    if (pdb._pdb.inst_data[r.pc].codeindex != -1) {
        for (int i = pdb._pdb.inst_data[r.pc-1].codeindex;; i++) {
            auto ch = pdb._pdb.code[i];

            if (pdb._pdb.code[i + 1] == 0) break;
            if (ignoring && ch == ' ')
                continue;
            else
                ignoring = false;

            int targetColor = -1;
            if (ch == '"') {
                if (!quote) con::setColor(CON_BROWN);
                else targetColor = CON_LIGHTGRAY;
                quote ^= 1;
            }

            if (quote == 0) {
                for (auto &kw : keywords) {
                    bool match = true;
                    for (uint32_t j = i; j < i + kw.size(); j++) {
                        if (pdb._pdb.code[j] != kw[j - i]) {
                            match = false;
                            break;
                        }
                    }

                    if (match) {
                        cnt = kw.size();
                        break;
                    }
                }

                if (cnt > 0) con::setColor(CON_LIGHTCYAN);
                else if (ch == '(') con::setColor(CON_DARKGRAY);
                else if (ch == ')') con::setColor(CON_DARKGRAY);
                else if (ch >= '0' && ch <= '9') con::setColor(CON_LIGHTGREEN);
                else con::setColor(CON_LIGHTGRAY);
            }

            wprintf(L"%c", ch);

            if (targetColor != -1)
                con::setColor(targetColor);
            if (cnt > 0) cnt--;
            if (cnt == 0) {
                con::setColor(CON_LIGHTGRAY);
                cnt = -1;
            }
        }
    }
    con::setColor(CON_LIGHTGRAY);

    rklog("\n");
}

void debugger::dumpstack() {
    if (r == nullptr) {
        printf("No active runner.\n");
        return;
    }

    con::setColor(CON_WHITE);
    printf("\n\n");
    printf("[[DUMPSTACK]=====================\n");
    for (int i= r->stack.size()-1;i>=0;i--) {
        auto &item = r->stack[i];

        if (r->callee_ptr == &item) {
            con::setColor(CON_YELLOW);
            printf("  > ");
        }
        else {
            con::setColor(CON_LIGHTCYAN);
            printf("  * ");
        }

        con::setColor(CON_LIGHTMAGENTA);
        printf("%2d  ", i);

        con::setColor(CON_WHITE);
        if (item.type == value_type::integer)
            printf("%8s, %15d", "INT", item.integer);
        else if (item.type == value_type::boolean)
            printf("%8s, %15s", "BOOL", item.integer == 1 ? "true" : "false");
        else if (item.type == value_type::null)
            printf("%8s, ", "NULL");
        else if (item.type == value_type::empty)
            printf("%8s, ", "EMPTY");
        else if (item.type == value_type::object)
            printf("%8s, %15S", "OBJECT", pdb.get_name(item.objref->sighash).c_str());
        
        printf("\n");
    }
    printf("\n\n");
}