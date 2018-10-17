#pragma once

#include <string>
#include <vector>
#include <map>

#include "program.h"

class runtime_pdb {
public:
    runtime_pdb(const pdb &pdb) :
        _pdb(pdb) {

        for (int i = 0; i < pdb.sigtable_len; i++) {
            auto sig = pdb.sigtable[i];
            sigtable[sig.sighash] = sig.signature;
        }
    }

    const std::wstring &get_name(unsigned int sighash) {
        return sigtable[sighash];
    }

    const pdb &_pdb;
private:

    std::map<unsigned int, std::wstring> sigtable;
};

class debug {
public:
    debug(pdb &pdb) :
        pdb(pdb) {
    }

    const std::wstring &sig2name(unsigned int sig) {
        return pdb.get_name(sig);
    }

private:
    runtime_pdb pdb;
};