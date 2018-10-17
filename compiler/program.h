#pragma once

#define rooke_max_signature 24
#define rookie_max_params 7

typedef enum ptype : unsigned char {
    t_none,
    t_integer,
    t_string,
    t_object
};
typedef enum opcode : unsigned char {
    op_nop = 0,

    op_add,
    op_sub,
    op_mul,
    op_div,
    op_eq,
    op_g, op_l, op_ge, op_le,

    op_newobj, op_newarr,
    op_call, op_syscall, op_vcall,
    op_ret,

    op_dup,
    op_pop,
    op_ldthis,
    op_ldloc, op_stloc,
    op_ldprop, op_stprop,
    op_setcallee,

    op_ldi,
    op_ldstr, op_ldnull,

    op_jmp_true, op_jmp_false,

    op_endenum // not a real opcode, indicates last num
} opcode_t;

inline const wchar_t *to_string(opcode type) {
    switch (type) {
    case op_nop: return L"op_nop";
    case op_add: return L"op_add";
    case op_sub: return L"op_sub";
    case op_mul: return L"op_mul";
    case op_div: return L"op_div";
    case op_eq: return L"op_eq";
    case op_g: return L"op_g";
    case op_l: return L"op_l";
    case op_ge: return L"op_ge";
    case op_le: return L"op_le";
    case op_newobj: return L"op_newobj";
    case op_newarr: return L"op_newarr";
    case op_syscall: return L"op_syscall";
    case op_call: return L"op_call";
    case op_vcall: return L"op_vcall";
    case op_ret: return L"op_ret";
    case op_dup: return L"op_dup";
    case op_pop: return L"op_pop";
    case op_ldthis: return L"op_ldthis";
    case op_ldloc: return L"op_ldloc";
    case op_stloc: return L"op_stloc";
    case op_ldprop: return L"op_ldprop";
    case op_stprop: return L"op_stprop";
    case op_setcallee: return L"op_setcallee";
    case op_ldi: return L"op_ldi";
    case op_ldstr: return L"op_ldstr";
    case op_ldnull: return L"op_ldnull";
    case op_jmp_true: return L"op_jmp_true";
    case op_jmp_false: return L"op_jmp_false";

    default: return L"op_unknown";
    }
}

typedef enum callsite_lookup {
    cs_method,
    cs_syscall
};

#pragma pack (push, 1)
struct callsite {
    unsigned char lookup_type;
    unsigned char pushed;
    short index;

    callsite(unsigned char lookup_type, short index) :
        lookup_type(lookup_type), index(index), pushed(0) {
    }
    callsite(unsigned char lookup_type, unsigned char pushed, short index) :
        lookup_type(lookup_type), index(index), pushed(pushed) {
    }
};
struct instruction {
    uint8_t opcode;

    union {
        // full operand with 4 bytes.
        uint32_t operand;

        // encodings
        callsite cs;
        int32_t i32;
    };

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
};
struct program {
    program_header header;

    program_entry *entries;
    instruction *code;
    const wchar_t *rdata;
    typedata *types;

    program() :
        entries(nullptr), code(nullptr), rdata(nullptr) {
    }
    program(const program &other) = default;
    program(program &&other) {
        header = other.header;

        entries = other.entries;
        code = other.code;
        rdata = other.rdata;
        types = other.types;

        other.entries = nullptr;
        other.code = nullptr;
        other.rdata = nullptr;
        other.types = nullptr;
    }
    program& operator=(program &&other) {
        if (this != &other) {
            header = other.header;

            entries = other.entries;
            code = other.code;
            rdata = other.rdata;
            types = other.types;

            other.entries = nullptr;
            other.code = nullptr;
            other.rdata = nullptr;
            other.types = nullptr;
        }
        return *this;
    }
    virtual ~program() {
        if (entries != nullptr)
            free(entries);
        entries = nullptr;
    }

    void dump() {
        wprintf(L"[rookie_program]\r\n");

        wprintf(L"  [types]\n");
        for (int i = 0; i < header.types_len; i++) {
            wprintf(L"    [%s]\n", types[i].name);

            for (int j = 0; j < types[i].methods_len; j++) {
                wprintf(L"      * %s\n", types[i].methods[j].name);
            } 
        }

        for (int i = 0; i < header.entry_len; i++) {
            wprintf(L"  [%s]\n", entries[i].signature);
            wprintf(L"    * params: %d\n", entries[i].params);
            wprintf(L"    * locals: %d\n", entries[i].locals);
            wprintf(L"    * body\n");
            for (int j = entries[i].entry; j < entries[i].entry + entries[i].codesize; j++) {
                wprintf(L"      %s, %d\n", to_string((opcode)code[j].opcode), code[j].operand);
            }
        }
    }
};
#pragma pack (pop)

#pragma pack (push, 1)
struct pdb_signature {
    uint32_t sighash;
    wchar_t signature[rooke_max_signature];

    pdb_signature() { }
    pdb_signature(uint32_t sighash, const wchar_t *name) :
        sighash(sighash) {
        wcscpy(signature, name);
    }
};
struct pdb_instruction_data {
    uint32_t codeindex;
};
struct pdb {
    uint32_t program_hash;

    pdb_signature *sigtable;
    uint32_t sigtable_len;

    pdb_instruction_data *inst_data;
    uint32_t inst_data_len;

    wchar_t *code;
    uint32_t code_len;
};
#pragma pack (pop)