/**
  *  \file util/instructionlist.hpp
  */
#ifndef C2NG_UTIL_INSTRUCTIONLIST_HPP
#define C2NG_UTIL_INSTRUCTIONLIST_HPP

#include <vector>
#include "afl/base/types.hpp"

namespace util {

    class InstructionList {
     public:
        typedef uint16_t Instruction_t;
        typedef int32_t Parameter_t;

        class Iterator {
         public:
            Iterator(const InstructionList& parent);

            bool readInstruction(Instruction_t& insn);
            bool readParameter(Parameter_t& param);

         private:
            const InstructionList& m_parent;
            size_t m_nextInstruction;
            size_t m_nextParameter;
        };

        InstructionList();
        ~InstructionList();

        InstructionList& addInstruction(Instruction_t insn);
        InstructionList& addParameter(Parameter_t param);
        void clear();
        void append(const InstructionList& other);

        size_t size() const;
        Iterator read() const;

        void swap(InstructionList& other);

     private:
        std::vector<Parameter_t> m_data;
        size_t m_lastInstruction;
    };

}

#endif
