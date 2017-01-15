/**
  *  \file interpreter/arguments.cpp
  */

#include <cmath>
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"
#include "afl/data/scalarvalue.hpp"
#include "afl/data/floatvalue.hpp"
#include "interpreter/values.hpp"
#include "afl/string/char.hpp"
#include "afl/data/stringvalue.hpp"

interpreter::Arguments::Arguments(const afl::data::Segment& data, size_t index, size_t numArgs)
    : m_data(data),
      m_index(index),
      m_numArgs(numArgs)
{
    // ex IntArgBlock::IntArgBlock
}

afl::data::Value*
interpreter::Arguments::getNext()
{
    // ex IntArgBlock::getNext
    if (m_numArgs != 0) {
        --m_numArgs;
        return m_data[m_index++];
    } else {
        return 0;
    }
}

size_t
interpreter::Arguments::getNumArgs() const
{
    // ex IntArgBlock::getNumArgs
    return m_numArgs;
}

void
interpreter::Arguments::checkArgumentCount(size_t need)
{
    // ex IntArgBlock::checkArgumentCount
    interpreter::checkArgumentCount(m_numArgs, need, need);
}

void
interpreter::Arguments::checkArgumentCount(size_t min, size_t max)
{
    // ex IntArgBlock::checkArgumentCount
    interpreter::checkArgumentCount(m_numArgs, min, max);
}

void
interpreter::Arguments::checkArgumentCountAtLeast(size_t min)
{
    // ex IntArgBlock::checkArgumentCountAtLeast
    interpreter::checkArgumentCount(m_numArgs, min, m_numArgs);
}


void
interpreter::checkArgumentCount(size_t have, size_t min, size_t max)
{
    // ex int/args.h:checkArgumentCount
    if (have < min) {
        throw Error("Too few arguments");
    }
    if (have > max) {
        throw Error("Too many arguments");
    }
}


// Check integer argument.
bool
interpreter::checkIntegerArg(int32_t& out, afl::data::Value* value)
{
    // ex int/if/ifutil.h:checkIntArg
    if (value == 0) {
        return false;
    }

    if (afl::data::ScalarValue* iv = dynamic_cast<afl::data::ScalarValue*>(value)) {
        // Regular integer.
        out = iv->getValue();
        return true;
    } else if (afl::data::FloatValue* fv = dynamic_cast<afl::data::FloatValue*>(value)) {
        // We truncate here. This is what PCC 1.x does. Let's hope that IEEE FP is precise
        // enough that we don't lose enough precision to get off-by-one results, like HOST.EXE.
        double v = fv->getValue();
        if (std::fabs(v) > 2147483647.0)
            throw Error::rangeError();
        out = int32_t(v);
        return true;
    } else {
        throw Error::typeError(Error::ExpectInteger);
    }
}


// Check integer argument with range.
bool
interpreter::checkIntegerArg(int32_t& out, afl::data::Value* value, int32_t min, int32_t max)
{
    // ex int/if/ifutil.h:checkIntArg
    bool result = checkIntegerArg(out, value);
    if (result) {
        if (out < min || out > max) {
            throw Error::rangeError();
        }
    }
    return result;
}

// Check boolean argument.
bool
interpreter::checkBooleanArg(bool& out, afl::data::Value* value)
{
    // ex int/if/ifutil.h:checkBoolArg
    int n = getBooleanValue(value);
    if (n < 0) {
        return false;
    }
    out = (n != 0);
    return true;
}

// Check string argument.
bool
interpreter::checkStringArg(String_t& out, afl::data::Value* value)
{
    // ex int/if/ifutil.h:checkStringArg
    if (value == 0) {
        return false;
    }

    out = toString(value, false);
    return true;
}

// Check flag argument.
bool
interpreter::checkFlagArg(int32_t& flagOut, int32_t* valueOut, afl::data::Value* value, const char* tpl)
{
    // ex int/if/ifutil.h:checkFlagArg
    if (value == 0) {
        return false;
    }

    if (afl::data::ScalarValue* iv = dynamic_cast<afl::data::ScalarValue*>(value)) {
        // An integer fills the valueOut, and specifies no flags
        if (!valueOut) {
            throw Error::typeError();
        }
        *valueOut = iv->getValue();
        flagOut = 0;
        return true;
    } else if (afl::data::StringValue* sv = dynamic_cast<afl::data::StringValue*>(value)) {
        // Parse string
        const String_t& s = sv->getValue();
        uint32_t parsedValue = 0;
        enum { No, Yes, Followed } valueState = No;
        for (size_t i = 0; i < s.size(); ++i) {
            char c = afl::string::charToUpper(s[i]);
            if (const char* p = std::strchr(tpl, c)) {
                // Flag
                flagOut |= 1 << (p - tpl);
                if (valueState == Yes) {
                    valueState = Followed;
                }
            } else if (c >= '0' && c <= '9') {
                // Number
                if (valueState == Followed || !valueOut) {
                    throw Error("Invalid option");
                }
                parsedValue = 10*parsedValue + c - '0';
                *valueOut = parsedValue;
                valueState = Yes;
            } else {
                // Error
                throw Error("Invalid option");
            }
        }
        return true;
    } else {
        throw Error::typeError();
    }
}

// Check command atom argument.
bool
interpreter::checkCommandAtomArg(util::Atom_t& atomOut, afl::data::Value* value, util::AtomTable& table)
{
    if (value == 0) {
        return false;
    } else if (afl::data::StringValue* sv = dynamic_cast<afl::data::StringValue*>(value)) {
        atomOut = table.getAtomFromString(sv->getValue());
        return true;
    } else if (afl::data::ScalarValue* iv = dynamic_cast<afl::data::ScalarValue*>(value)) {
        atomOut = iv->getValue();
        return true;
    } else {
        throw Error::typeError(Error::ExpectString);
    }
}
