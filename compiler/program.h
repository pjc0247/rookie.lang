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
    op_ldloc, op_stloc,
    op_ldstate, op_ststate,
    op_setcallee,

    op_ldi,
    op_ldstr, op_ldnull,

    op_jmp_true, op_jmp_false,

    op_endenum // not a real opcode, indicates last num
} opcode_t;

inline const char *to_string(opcode type) {
    switch (type) {
    case op_nop: return "op_nop";
    case op_add: return "op_add";
    case op_sub: return "op_sub";
    case op_mul: return "op_mul";
    case op_div: return "op_div";
	case op_eq: return "op_eq";
    case op_g: return "op_g";
    case op_l: return "op_l";
    case op_ge: return "op_ge";
    case op_le: return "op_le";
    case op_newobj: return "op_newobj";
    case op_newarr: return "op_newarr";
    case op_syscall: return "op_syscall";
    case op_call: return "op_call";
    case op_vcall: return "op_vcall";
    case op_ret: return "op_ret";
    case op_dup: return "op_dup";
    case op_pop: return "op_pop";
    case op_ldloc: return "op_ldloc";
    case op_stloc: return "op_stloc";
    case op_ldstate: return "op_ldstate";
    case op_ststate: return "op_ststate";
    case op_setcallee: return "op_setcallee";
    case op_ldi: return "op_ldi";
    case op_ldstr: return "op_ldstr";
    case op_ldnull: return "op_ldnull";
    case op_jmp_true: return "op_jmp_true";
    case op_jmp_false: return "op_jmp_false";

    default: return "op_unknown";
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
    unsigned char opcode;

    union {
        // full operand with 4 bytes.
        int operand;

        callsite cs;
    };

    instruction(opcode_t _o, int operand) :
        opcode((unsigned char)_o), operand(operand) {
    }
    instruction(opcode_t _o, const callsite &cs) :
        opcode((unsigned char)_o), cs(cs) {
    }
};

struct methoddata {
    char name[rooke_max_signature];
    int entry;
};
struct typedata {
    char name[rooke_max_signature];
    int methods_len;
    methoddata *methods;
};
#pragma pack (pop)

#pragma pack (push, 1)
// program_header: 12bytes
struct program_header {
    unsigned int code_len;
    unsigned int rdata_len;
    unsigned int entry_len;
    unsigned int types_len;

    unsigned int main_entry;
};
// program_entry: ?????bytes
struct program_entry {
    char signature[rooke_max_signature];
    char params;
    char ret;
    short locals;

    int entry;
    int codesize;
};
struct program {
    program_header header;

    program_entry *entries;
    instruction *code;
    const char *rdata;
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
        printf("[rookie_program]\r\n");

        printf("  [types]\n");
        for (int i = 0; i < header.types_len; i++) {
            printf("    [%s]\n", types[i].name);

            for (int j = 0; j < types[i].methods_len; j++) {
                printf("      * %s\n", types[i].methods[j].name);
            } 
        }

        for (int i = 0; i < header.entry_len; i++) {
            printf("  [%s]\n", entries[i].signature);
            printf("    * params: %d\n", entries[i].params);
            printf("    * locals: %d\n", entries[i].locals);
            printf("    * body\n");
            for (int j = entries[i].entry; j < entries[i].entry + entries[i].codesize; j++) {
                printf("      %s, %d\n", to_string((opcode)code[j].opcode), code[j].operand);
            }
        }
    }
};
#pragma pack (pop)