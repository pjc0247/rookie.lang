#include "stdafx.h"

#include "runner.h"

void debugger::on_begin_program(runner &r, const program &p) {
    rklog("[%3s] %15s,   %12s,   %s\n", "PC", "OPCODE", "OPERAND", "CODE");
    rklog("[%3s] %15s,   %12s,   %s\n", "---", "-----", "-----", "-----");
}
void debugger::on_pre_exec(runner &r, const instruction &inst) {
    rklog("[%3d] %15S,   ", r.pc, to_string((opcode_t)inst.opcode));

    if (inst.opcode == opcode::op_vcall ||
        inst.opcode == opcode::op_call ||
        inst.opcode == opcode::op_syscall ||
        inst.opcode == opcode::op_ldprop ||
        inst.opcode == opcode::op_stprop ||
        inst.opcode == opcode::op_newobj) {

        rklog("%12S", pdb.get_name(inst.operand).c_str());
    }
    else {
        rklog("%12d", inst.operand);
    }
    rklog("    ");

    //for (int i = 0; i < pdb._pdb.code_len; i++)
        //printf("%d: %c, %d\n", i, pdb._pdb.code[i], pdb._pdb.code[i]);

    bool ignoring = true;
    if (pdb._pdb.inst_data[r.pc].codeindex != -1) {
        for (int i = pdb._pdb.inst_data[r.pc-1].codeindex;; i++) {
            if (pdb._pdb.code[i + 1] == 0) break;
            if (ignoring && pdb._pdb.code[i] == ' ')
                continue;
            else
                ignoring = false;
            wprintf(L"%c", pdb._pdb.code[i]);
        }
    }
        //rklog(" %S ", pdb._pdb.code[pdb._pdb.inst_data[r.pc].codeindex]);

    rklog("\n");
}