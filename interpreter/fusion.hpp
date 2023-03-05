/**
  *  \file interpreter/fusion.hpp
  *  \brief Interpreter: Instruction Fusion
  */
#ifndef C2NG_INTERPRETER_FUSION_HPP
#define C2NG_INTERPRETER_FUSION_HPP

namespace interpreter {

    class BytecodeObject;

    /** Fuse instructions.

        To avoid unnecessary creation of temporary values, we can fuse instructions together.
        The idea is to detect patterns, and mark fusable instructions with a special major opcode.
        The interpreter then executes these major opcodes in one cycle, saving temporaries where possible.

        Instruction fusion is an internal optimisation.
        Fused instructions are not saved to the VM file; see interpreter::Opcode::getExternalMajor().

        No changes are done to the instruction stream other than the major opcode.
        Thus, all information can still be found at the usual places.
        In particular, if a jump exists into the middle of a fused instruction,
        it will simply proceed by executing the original unfused instruction.

        \param bco [in/out] BytecodeObject */
    void fuseInstructions(BytecodeObject& bco);

    /** Unfuse instructions.
        Undoes the transformation done by fuseInstructions() and restores the original instructions.
        \param bco [in/out] BytecodeObject */
    void unfuseInstructions(BytecodeObject& bco);

}

#endif
