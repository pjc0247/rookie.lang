#include "stdafx.h"

#include "p2wast.h"

struct wastmapping {
    opcode opcode;
    const char *instruction;
    char stackitem;

    wastmapping(opcode_t opcode, const char *instruction, char stackitem) :
        opcode(opcode),
        instruction(instruction), stackitem(stackitem) {
    }
};

#define _wastmapitem(opcode, inst, stackitem) wastmapping(opcode, inst " ", stackitem),
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