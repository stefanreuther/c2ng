/**
  *  \file interpreter/taskpredictor.hpp
  *  \brief Class interpreter::TaskPredictor
  */
#ifndef C2NG_INTERPRETER_TASKPREDICTOR_HPP
#define C2NG_INTERPRETER_TASKPREDICTOR_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/types.hpp"
#include "afl/string/string.hpp"

namespace interpreter {

    class Arguments;
    class BaseTaskEditor;

    /** Auto Task Prediction.
        A TaskPredictor operates on the format provided by a TaskEditor to predict future states of the object.
        A derived class provides an implementation of the predictInstruction method which computes the actual effects;
        this class contains control and parsing.

        This interprets a small subset of CCScript, namely just the "command arg,arg,arg" syntax for simple, literal arguments.
        This is what the GUI produces.

        Technically, a much larger subset can be used (e.g. variables).
        Also, users can write their own auto task commands which we don't know anything about.

        Note that PCC2 refuses structural commands ('If', 'For') in auto tasks, cf. TaskEditor::isValidCommand(),
        so we don't have to deal with them here. */
    class TaskPredictor : public afl::base::Deletable {
     public:
        /** Predict one instruction.
            This function must store prediction results as a side-effect.
            \param name Name of command, in upper-case (e.g. "MOVETO")
            \param args Arguments of command
            \retval true successful interpretation, continue
            \retval false error, stop */
        virtual bool predictInstruction(const String_t& name, Arguments& args) = 0;

        /** Predict auto task.
            Starts interpreting at the current program counter, and ends at endPC, if specified.
            - interprets only regular commands with simple arguments; no structure commands, no computations
            - performs at most one RESTART loop

            \param editor editor
            \param endPC stop predicting before this instruction. If not given, predict to end. */
        void predictTask(const BaseTaskEditor& editor, size_t endPC = size_t(-1));

        /** Predict single statement.
            If the specified program counter points to a valid instruction, calls predictInstruction for it, otherwise does nothing.
            Use this to parse single instructions.
            \param editor editor
            \param pc instruction to interpret */
        void predictStatement(const BaseTaskEditor& editor, size_t pc);

        /** Predict single statement.
            \param statement Statement */
        void predictStatement(const String_t& statement);
    };

}

#endif
