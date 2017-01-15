/**
  *  \file interpreter/savecontext.hpp
  */
#ifndef C2NG_INTERPRETER_SAVECONTEXT_HPP
#define C2NG_INTERPRETER_SAVECONTEXT_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/types.hpp"

namespace interpreter {

    class BytecodeObject;
    class HashData;
    class ArrayData;
    class StructureTypeData;
    class StructureValueData;
    class Process;

    class SaveContext : public afl::base::Deletable {
     public:
        virtual uint32_t addBCO(BytecodeObject& bco) = 0;
        virtual uint32_t addHash(HashData& hash) = 0;
        virtual uint32_t addArray(ArrayData& array) = 0;
        virtual uint32_t addStructureType(StructureTypeData& type) = 0;
        virtual uint32_t addStructureValue(StructureValueData& value) = 0;

        // replaces getCurrentProcess()
        // must handle p==0 and return false!
        virtual bool isCurrentProcess(Process* p) = 0;
    };

}

#endif
