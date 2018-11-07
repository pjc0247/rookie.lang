#pragma once

#define rooke_max_signature 32
#define rookie_max_params 7

typedef enum opcode : unsigned char {
    op_nop = 0,

    op_eqtype,
    op_add,
    op_sub,
    op_mul,
    op_div,
    op_eq, op_neq,
    op_g, op_l, op_ge, op_le,
    op_and, op_or, op_not,

    op_newobj, op_newarr, op_newdic,
    op_call, op_syscall, op_vcall,
    op_ret,

    op_dup,
    op_pop,
    op_ldthis,
    op_ldtype,
    op_ldloc, op_stloc,
    op_ldprop, op_stprop,
    op_ldfld, op_stfld,
    op_setcallee,

    op_ldi, op_ldf,
    op_ldstr,
    op_ldnull,
    op_ldempty,
    op_ldtrue, op_ldfalse,

    op_jmp,
    op_jmp_true, op_jmp_false,

    op_endenum // not a real opcode, indicates last num
} opcode_t;

inline const wchar_t *to_string(opcode type) {
    switch (type) {
    case op_nop: return L"op_nop";
    case op_eqtype: return L"op_eqtype";
    case op_add: return L"op_add";
    case op_sub: return L"op_sub";
    case op_mul: return L"op_mul";
    case op_div: return L"op_div";
    case op_eq: return L"op_eq";
    case op_neq: return L"op_neq";
    case op_g: return L"op_g";
    case op_l: return L"op_l";
    case op_ge: return L"op_ge";
    case op_le: return L"op_le";
    case op_and: return L"op_and";
    case op_or: return L"op_or";
    case op_not: return L"op_not";
    case op_newobj: return L"op_newobj";
    case op_newarr: return L"op_newarr";
    case op_newdic: return L"op_newdic";
    case op_syscall: return L"op_syscall";
    case op_call: return L"op_call";
    case op_vcall: return L"op_vcall";
    case op_ret: return L"op_ret";
    case op_dup: return L"op_dup";
    case op_pop: return L"op_pop";
    case op_ldthis: return L"op_ldthis";
    case op_ldtype: return L"op_ldtype";
    case op_ldloc: return L"op_ldloc";
    case op_stloc: return L"op_stloc";
    case op_ldprop: return L"op_ldprop";
    case op_stprop: return L"op_stprop";
    case op_ldfld: return L"op_ldfld";
    case op_stfld: return L"op_stfld";
    case op_setcallee: return L"op_setcallee";
    case op_ldi: return L"op_ldi";
    case op_ldf: return L"op_ldf";
    case op_ldstr: return L"op_ldstr";
    case op_ldempty: return L"op_ldempty";
    case op_ldnull: return L"op_ldnull";
    case op_ldtrue: return L"op_ldtrue";
    case op_ldfalse: return L"op_ldfalse";
    case op_jmp: return L"op_jmp";
    case op_jmp_true: return L"op_jmp_true";
    case op_jmp_false: return L"op_jmp_false";

    default: return L"op_unknown";
    }
}

typedef enum callsite_lookup {
    cs_method,
    cs_syscall
};
typedef enum callsite_flag {
    cf_defer = 1
};

#pragma pack (push, 1)
struct callsite {
    uint8_t lookup_type;
    uint8_t flags;
    uint16_t index;

    callsite(uint8_t lookup_type, uint16_t index) :
        lookup_type(lookup_type), index(index), flags(0) {
    }
    callsite(uint8_t lookup_type, uint8_t flags , uint16_t index) :
        lookup_type(lookup_type), index(index), flags(flags) {
    }
};
struct instruction {
    uint8_t opcode;

    union {
        // full operand with 4 bytes.
        uint32_t operand;

        // encodings
        callsite cs;
        int32_t  i32;
        float    f32;
    };

    instruction() :
        opcode(opcode::op_nop), i32(0) {
    }
    instruction(opcode_t _o, uint32_t operand) :
        opcode((uint8_t)_o), operand(operand) {
    }
    instruction(opcode_t _o, const callsite &cs) :
        opcode((uint8_t)_o), cs(cs) {
    }
};

struct methoddata {
    wchar_t name[rooke_max_signature];
    uint32_t entry;
};
struct typedata {
    wchar_t name[rooke_max_signature];

    uint32_t parents_len;
    uint32_t *parents;

    uint32_t methods_len;
    methoddata *methods;
};
#pragma pack (pop)

#pragma pack (push, 1)
// program_header: 12bytes
struct program_header {
    uint32_t code_len;
    uint32_t rdata_len;
    uint32_t entry_len;
    uint32_t types_len;
    
    uint32_t main_entry;
};
// program_entry: ?????bytes
struct program_entry {
    wchar_t signature[rooke_max_signature];
    uint16_t params;
    uint16_t locals;

    uint32_t entry;
    uint32_t codesize;
    uint16_t stacksize;
};
struct program {
    program_header  header;

    program_entry   *entries;
    instruction     *code;
    const wchar_t   *rdata;
    typedata        *types;

    void dump() {
        wprintf(L"[rookie_program]\r\n");

        wprintf(L"  * main: %d\n", header.main_entry);
        wprintf(L"  * codesize: %d\n", header.code_len);

        wprintf(L"  [types]\n");
        for (uint32_t i = 0; i < header.types_len; i++) {
            wprintf(L"    [%s]\n", types[i].name);

            for (uint32_t j = 0; j < types[i].methods_len; j++) {
                wprintf(L"      * %s\n", types[i].methods[j].name);
            } 
        }

        for (uint32_t i = 0; i < header.entry_len; i++) {
            printf("  [%S, %d]\n", entries[i].signature, entries[i].entry);
            printf("    * params: %d\n", entries[i].params);
            printf("    * locals: %d\n", entries[i].locals);
            printf("    * body\n");
            for (uint32_t j = entries[i].entry; j < entries[i].entry + entries[i].codesize; j++) {
                printf("      %S, %d\n", to_string((opcode)code[j].opcode), code[j].operand);
            }
        }
    }
};
#pragma pack (pop)

#pragma pack (push, 1)
struct pdb_signature {
    uint32_t  sighash;
    wchar_t   signature[rooke_max_signature];

    pdb_signature() { }
    pdb_signature(uint32_t sighash, const wchar_t *name) :
        sighash(sighash) {
        wcscpy(signature, name);
    }
};
struct pdb_instruction {
    uint32_t codeindex;
};
struct pdb {
    uint32_t         program_hash;

    pdb_signature    *sigtable;
    uint32_t         sigtable_len;

    pdb_instruction  *inst_data;
    uint32_t         inst_data_len;

    wchar_t          *code;
    uint32_t         code_len;
};
#pragma pack (pop)