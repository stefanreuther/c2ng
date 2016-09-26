/**
  *  \file interpreter/arguments.hpp
  */
#ifndef C2NG_INTERPRETER_ARGUMENTS_HPP
#define C2NG_INTERPRETER_ARGUMENTS_HPP

#include "afl/data/segment.hpp"
#include "afl/base/types.hpp"

namespace interpreter {

    /** Arguments to a function call.
        This describes a slice of a data segment used as parameters to a function call or array indexing operation.

        Its main use is to pack all necessary information conveniently together
        so we don't have to pass around three args all the time,
        and it provides useful functions to query the arguments. */
    class Arguments {
     public:
        /** Constructor.
            \param data    [in] Data segment
            \param index   [in] Index of first argument
            \param numArgs [in] Number of argument */
        Arguments(const afl::data::Segment& data, size_t index, size_t numArgs);

        /** Get next argument.
            If there are no more arguments, returns 0, corresponding to an "empty" argument. */
        afl::data::Value* getNext();

        /** Get number of (remaining) arguments.
            Each getNext() will reduce this number. */
        size_t getNumArgs() const;

        /** Check that there are exactly \c need arguments.
            If not, throws an appropriate Error. */
        void checkArgumentCount(size_t need);

        /** Check that there are \c min to \c max arguments.
            If not, throws an appropriate Error. */
        void checkArgumentCount(size_t min, size_t max);

        /** Check that there are at least \c min arguments.
            If not, throws an appropriate Error. */
        void checkArgumentCountAtLeast(size_t min);
     private:
        const afl::data::Segment& m_data;
        size_t m_index;
        size_t m_numArgs;
    };

    /** Check argument count.
        \param have Number of arguments we have
        \param min  Minimum number required
        \param max  Maximum number accepted
        \throw Error if constraint violated */
    void checkArgumentCount(size_t have, size_t min, size_t max);

    /** Check integer argument.
        Note that this also accepts float arguments, like PCC 1.x's EvalArgs().
        \param out   [out] Result will be placed here
        \param value [in] Value given by user
        \return true if value was specified, false if value was null (out not changed)
        \throw Error if value is invalid */
    bool checkIntegerArg(int32_t& out, afl::data::Value* value);

    /** Check integer argument with range.
        \param out   [out] Result will be placed here
        \param value [in] Value given by user
        \param min   [in] Minimum accepted value
        \param min   [in] Maximum accepted value
        \return true if value was specified, false if value was null (out not changed)
        \throw Error if value is invalid */
    bool checkIntegerArg(int32_t& out, afl::data::Value* value, int32_t min, int32_t max);

    /** Check boolean argument.
        \param out   [out] Result will be placed here
        \param value [in] Value given by user
        \return true if value was specified, false if value was null (out not changed)
        \throw Error if value is invalid (can currently not happen) */
    bool checkBooleanArg(bool& out, afl::data::Value* value);

    /** Check string argument.
        \param out   [out] Result will be placed here
        \param value [in] Value given by user
        \return true if value was specified, false if value was null (out not changed)
        \throw Error if value is invalid (can currently not happen) */
    bool checkStringArg(String_t& out, afl::data::Value* value);

    /** Check flag argument.
        Users specify flags as a string containing latin letters.
        Optionally, if valueOut is given, an additional integer argument can be specified.
        The set of possible flags is specified using a list of upper-case letters.
        The result will have the "1<<n" bit set if flag "tpl[n]" was specified.

        Note that valueOut is not changed when no value is specified; this is
        required (at least) for IFSelectionLoad/IFSelectionSave.

        \param flagOut  [out] Flag result will be placed here
        \param valueOut [out,optional] Value result will be placed here
        \param value    [in] Value given by user
        \param tpl      [in] Template used to parse user's input
        \return true if value was specified, false if value was null (flagOut, valueOut not changed)
        \throw Error if value is invalid */
    bool checkFlagArg(int32_t& flagOut, int32_t* valueOut, afl::data::Value* value, const char* tpl);

}

#endif
