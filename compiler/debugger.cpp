#include "stdafx.h"

#include "conout.h"
#include "runner.h"

void debugger::on_begin_program(runner &r, const program &p) {
    con::setColor(CON_WHITE);
    rklog("[%3s] %15s,   %12s,   %s\n", "PC", "OPCODE", "OPERAND", "CODE");
    rklog("[%3s] %15s,   %12s,   %s\n", "---", "-----", "-----", "-----");
    con::setColor(CON_LIGHTGRAY);
}
void debugger::on_pre_exec(runner &r, const instruction &inst) {
    con::setColor(CON_LIGHTGRAY);
    rklog("[");
    con::setColor(CON_YELLOW);
    rklog("%3d", r.pc);
    con::setColor(CON_LIGHTGRAY);
    rklog("] %15S, ", to_string((opcode_t)inst.opcode));
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

    //for (int i = 0; i < pdb._pdb.code_len; i++)
        //printf("%d: %c, %d\n", i, pdb._pdb.code[i], pdb._pdb.code[i]);

    bool ignoring = true;
    int quote = 0;
    int cnt = -1;

    std::vector<std::wstring> keywords({
        L"for", L"if", L"class", L"def", L"static"
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
                    for (int j = i; j < i + kw.size(); j++) {
                        if (ch != kw[j - i]) {
                            match = false;
                            break;
                        }
                    }

                    if (match) {
                        con::setColor(CON_LIGHTCYAN);
                        cnt = kw.size();
                        break;
                    }
                }

                if (cnt > 0);
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
        //rklog(" %S ", pdb._pdb.code[pdb._pdb.inst_data[r.pc].codeindex]);

    rklog("\n");
}