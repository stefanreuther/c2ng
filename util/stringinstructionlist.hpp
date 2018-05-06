/**
  *  \file util/stringinstructionlist.hpp
  */
#ifndef C2NG_UTIL_STRINGINSTRUCTIONLIST_HPP
#define C2NG_UTIL_STRINGINSTRUCTIONLIST_HPP

#include <vector>
#include "util/instructionlist.hpp"
#include "afl/string/string.hpp"

namespace util {

    class StringInstructionList : public InstructionList {
     public:
        class Iterator : public InstructionList::Iterator {
         public:
            Iterator(const StringInstructionList& parent);

            // bool readInstruction(Instruction_t& insn);
            // bool readParameter(Parameter_t& param);
            bool readStringParameter(String_t& param);

         private:
            const StringInstructionList& m_parent;
        };

        StringInstructionList();
        ~StringInstructionList();

        StringInstructionList& addInstruction(Instruction_t insn);
        StringInstructionList& addParameter(Parameter_t param);
        StringInstructionList& addStringParameter(const String_t& s);
        void clear();
        // FIXME: cannot do that yet! void append(const StringInstructionList& other);

        Iterator read() const;

        void swap(StringInstructionList& other);
     private:
        std::vector<String_t> m_strings;
    };

}

#endif
