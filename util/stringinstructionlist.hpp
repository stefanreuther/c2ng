/**
  *  \file util/stringinstructionlist.hpp
  *  \brief Class util::StringInstructionList
  */
#ifndef C2NG_UTIL_STRINGINSTRUCTIONLIST_HPP
#define C2NG_UTIL_STRINGINSTRUCTIONLIST_HPP

#include <vector>
#include "util/instructionlist.hpp"
#include "afl/string/string.hpp"

namespace util {

    /** Instruction list.
        This class can be used to serialize and later replay a sequence of function calls,
        represented as a list of instructions similar to a "bytecode".
        It extends InstructionList to support string parameters.
        See there for more details.

        Mixing string and integer parameters will produce unspecified but valid results (no crash).

        This class does not currently support polymorphic usage; hence private derivation. */
    class StringInstructionList : private InstructionList {
     public:
        using InstructionList::Instruction_t;
        using InstructionList::Parameter_t;

        /** Iterator for reading. */
        class Iterator : public InstructionList::Iterator {
         public:
            /** Constructor.
                \param parent StringInstructionList to read. Lifetime must exceed that of the iterator. */
            Iterator(const StringInstructionList& parent);

            /** Read a string parameter.
                \param [out] param Result
                \return true on success, false on error or end of instruction */
            bool readStringParameter(String_t& param);

         private:
            const StringInstructionList& m_parent;
        };

        /** Default constructor.
            Make empty list. */
        StringInstructionList();

        /** Destructor. */
        ~StringInstructionList();

        /** Add instruction.
            \param insn Instruction op-code
            \return *this */
        StringInstructionList& addInstruction(Instruction_t insn);
        /** Add parameter.
            Must be called after addInstruction(); ignored otherwise.
            \param param Parameter
            \return *this */
        StringInstructionList& addParameter(Parameter_t param);

        /** Add string parameter.
            Must be called after addInstruction(); ignored otherwise.
            \param param Parameter
            \return *this */
        StringInstructionList& addStringParameter(const String_t& s);

        /** Clear this StringInstructionList. */
        void clear();

        // FIXME: cannot do that yet! void append(const StringInstructionList& other);

        using InstructionList::size;

        /** Read instructions.
            \return Iterator */
        Iterator read() const;

        /** Swap with other StringInstructionList.
            \param [in,out] other Other list */
        void swap(StringInstructionList& other);
     private:
        std::vector<String_t> m_strings;
    };

}

#endif
