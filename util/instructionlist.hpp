/**
  *  \file util/instructionlist.hpp
  *  \brief Class util::InstructionList
  */
#ifndef C2NG_UTIL_INSTRUCTIONLIST_HPP
#define C2NG_UTIL_INSTRUCTIONLIST_HPP

#include <vector>
#include "afl/base/types.hpp"

namespace util {

    /** Instruction list.
        This class can be used to serialize and later replay a sequence of function calls,
        represented as a list of instructions similar to a "bytecode".
        Instructions are represented as a 16-bit integer with a 16-bit parameter count.
        Parameters can be 32-bit integers.

        To write,
        - call addInstruction() to add an instruction
        - call addParameter() to add a parameter
        - repeat as needed.

        To read,
        - call read() to obtain an iterator
        - call readInstruction() to read the next instruction, if any
        - call readParameter() to read the next parameter of the current instruction, if any
        - repeat as needed until end is reached

        The instruction sequence is correctly reproduced even when the number of
        addParameter() and readParameter() calls do not match exactly.

        A user will typically implement their own custom parameter reading functions.
        If you have string parameters, use StringInstructionList. */
    class InstructionList {
     public:
        /** Typedef for an instruction. */
        typedef uint16_t Instruction_t;

        /** Typedef for a parameter. */
        typedef int32_t Parameter_t;

        /** Iterator for reading. */
        class Iterator {
         public:
            /** Constructor.
                \param parent InstructionList to read. Lifetime must exceed that of the iterator. */
            Iterator(const InstructionList& parent);

            /** Read an instruction.
                \param [out] insn Instruction returned here
                \return true if instruction read; false if no more instructions */
            bool readInstruction(Instruction_t& insn);

            /** Read a parameter.
                \param [out] insn Parameter returned here
                \return true if parameter read; false if no more parameters for this instruction */
            bool readParameter(Parameter_t& param);

         private:
            const InstructionList& m_parent;
            size_t m_nextInstruction;
            size_t m_nextParameter;
        };

        /** Default constructor.
            Make empty list. */
        InstructionList();

        /** Destructor. */
        ~InstructionList();

        /** Add instruction.
            \param insn Instruction op-code
            \return *this */
        InstructionList& addInstruction(Instruction_t insn);

        /** Add parameter.
            Must be called after addInstruction(); ignored otherwise.
            \param param Parameter
            \return *this */
        InstructionList& addParameter(Parameter_t param);

        /** Clear this InstructionList. */
        void clear();

        /** Append copy of another InstructionList.
            Reading this InstructionList will reproduce its old content,
            followed by the content of \c other.
            \param other Other InstructionList */
        void append(const InstructionList& other);

        /** Get size of InstructionList (for informational purposed).
            \return Number of words occupied by instructions and parameters */
        size_t size() const;

        /** Read instructions.
            \return Iterator */
        Iterator read() const;

        /** Swap with other InstructionList.
            \param [in,out] other Other list */
        void swap(InstructionList& other);

     private:
        std::vector<Parameter_t> m_data;
        size_t m_lastInstruction;
    };

}

#endif
