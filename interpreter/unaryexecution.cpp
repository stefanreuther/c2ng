/**
  *  \file interpreter/unaryexecution.cpp
  */

#include <cmath>
#include <cstdlib>
#include <cstring>
#include "interpreter/unaryexecution.hpp"
#include "afl/base/countof.hpp"
#include "interpreter/values.hpp"
#include "interpreter/error.hpp"
#include "afl/data/scalarvalue.hpp"
#include "afl/data/floatvalue.hpp"
#include "util/math.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/charset/utf8reader.hpp"
#include "afl/charset/utf8.hpp"
#include "interpreter/world.hpp"
#include "interpreter/keymapvalue.hpp"
#include "afl/sys/loglistener.hpp"
#include "interpreter/callablevalue.hpp"
#include "interpreter/filevalue.hpp"
#include "afl/data/visitor.hpp"

using interpreter::Error;
using interpreter::getBooleanValue;
using interpreter::makeBooleanValue;
using interpreter::makeIntegerValue;
using interpreter::makeSizeValue;
using interpreter::makeFloatValue;
using interpreter::makeStringValue;

namespace {
    /** Arithmetic status. */
    enum Arithmetic {
        ariBad,                 ///< Bad argument combination.
        ariNull,                ///< Either argument is null.
        ariInt,                 ///< Use integer arithmetic.
        ariFloat                ///< Use float arithmetic.
    };

    /** Arithmetic argument. */
    struct ArithmeticArg {
        int ia;
        double fa;
    };

    /** Classify argument for arithmetic operations.
        \param arg [out] Argument for arithmetic
        \param a [in] User-supplied argument
        \return status */
    Arithmetic checkArithmetic(ArithmeticArg& arg, const afl::data::Value* a)
    {
        // Visitor is faster than traditional type switch
        class V : public afl::data::Visitor {
         public:
            V(ArithmeticArg& arg)
                : m_result(ariBad),
                  m_arg(arg)
                { }
            virtual void visitString(const String_t& /*str*/)
                { m_result = ariBad; }
            virtual void visitInteger(int32_t iv)
                { m_result = ariInt; m_arg.ia = iv; }
            virtual void visitFloat(double fv)
                { m_result = ariFloat; m_arg.fa = fv; }
            virtual void visitBoolean(bool bv)
                { m_result = ariInt; m_arg.ia = bv; }
            virtual void visitHash(const afl::data::Hash& /*hv*/)
                { m_result = ariBad; }
            virtual void visitVector(const afl::data::Vector& /*vv*/)
                { m_result = ariBad; }
            virtual void visitOther(const afl::data::Value& /*other*/)
                { m_result = ariBad; }
            virtual void visitNull()
                { m_result = ariNull; }
            virtual void visitError(const String_t& /*source*/, const String_t& /*str*/)
                { m_result = ariBad; }
            Arithmetic get() const
                { return m_result; }
         private:
            Arithmetic m_result;
            ArithmeticArg& m_arg;
        };
        V v(arg);
        v.visit(a);
        return v.get();
    }

    /** Prepare for trigonometry. Checks whether the argument is of the correct
        type, has reasonable range, and converts it from degrees to radians.
        \param arg user-supplied argument
        \return argument to pass to C++ math library
        \throws Error if bad user-supplied argument */
    double prepareTrig(const afl::data::Value* arg)
    {
        // Fetch value
        double value;
        if (const afl::data::ScalarValue* iv = dynamic_cast<const afl::data::ScalarValue*>(arg))
            value = iv->getValue();
        else if (const afl::data::FloatValue* fv = dynamic_cast<const afl::data::FloatValue*>(arg))
            value = fv->getValue();
        else
            throw Error::typeError(Error::ExpectNumeric);

        // Range check
        if (std::fabs(value) > 1.0E+6)
            throw Error::rangeError();

        return value * (util::PI / 180.0);
    }

    afl::data::Value* FNot(interpreter::World& /*world*/, const afl::data::Value* arg)
    {
        // logical not
        int value = getBooleanValue(arg);
        if (value >= 0) {
            value ^= 1;
        }
        return makeBooleanValue(value);
    }

    afl::data::Value* FBool(interpreter::World& /*world*/, const afl::data::Value* arg)
    {
        // conversion to bool, i.e. double not
        return makeBooleanValue(getBooleanValue(arg));
    }

    afl::data::Value* FNeg(interpreter::World& /*world*/, const afl::data::Value* arg)
    {
        // arithmetic negation
        ArithmeticArg a;
        switch (checkArithmetic(a, arg)) {
         case ariNull:
            return 0;
         case ariInt:
            return interpreter::makeIntegerValue(-a.ia);
         case ariFloat:
            return interpreter::makeFloatValue(-a.fa);
         default:
            throw Error::typeError(Error::ExpectNumeric);
        }
    }

    afl::data::Value* FPos(interpreter::World& /*world*/, const afl::data::Value* arg)
    {
        // arithmetic "identity", i.e. just check type
        ArithmeticArg a;
        switch (checkArithmetic(a, arg)) {
         case ariNull:
            return 0;
         case ariInt:
            return makeIntegerValue(+a.ia);
         case ariFloat:
            return makeFloatValue(+a.fa);
         default:
            throw Error::typeError(Error::ExpectNumeric);
        }
    }

    afl::data::Value* FSin(interpreter::World& /*world*/, const afl::data::Value* arg)
    {
        // Sine
        if (!arg)
            return 0;
        else
            return makeFloatValue(std::sin(prepareTrig(arg)));
    }

    afl::data::Value* FCos(interpreter::World& /*world*/, const afl::data::Value* arg)
    {
        // Cosine
        if (!arg)
            return 0;
        else
            return makeFloatValue(std::cos(prepareTrig(arg)));
    }

    afl::data::Value* FTan(interpreter::World& /*world*/, const afl::data::Value* arg)
    {
        // Tangent
        if (!arg) {
            return 0;
        } else {
            double a = prepareTrig(arg);
            double s = std::sin(a);
            double c = std::cos(a);
            if (std::fabs(c) < 1.0E-06)          // FIXME?, see FDivide
                throw Error("Divide by zero");
            else
                return makeFloatValue(s/c);
        }
    }

    afl::data::Value* FZap(interpreter::World& /*world*/, const afl::data::Value* arg)
    {
        // Turn false into null
        if (!arg)
            return 0;
        else if (getBooleanValue(arg) == 0)
            return 0;
        else
            return arg->clone();
    }

    afl::data::Value* FAbs(interpreter::World& /*world*/, const afl::data::Value* arg)
    {
        // Absolute value
        ArithmeticArg a;
        switch (checkArithmetic(a, arg)) {
         case ariNull:
            return 0;
         case ariInt:
            return makeIntegerValue(std::abs(a.ia));
         case ariFloat:
            return makeFloatValue(std::fabs(a.fa));
         default:
            throw Error::typeError(Error::ExpectNumeric);
        }
    }

    afl::data::Value* FExp(interpreter::World& /*world*/, const afl::data::Value* arg)
    {
        // Exponential
        ArithmeticArg a;
        switch (checkArithmetic(a, arg)) {
         case ariNull:
            return 0;
         case ariInt:
            return makeFloatValue(std::exp(double(a.ia)));
         case ariFloat:
            return makeFloatValue(std::exp(a.fa));
         default:
            throw Error::typeError(Error::ExpectNumeric);
        }
    }

    afl::data::Value* FLog(interpreter::World& /*world*/, const afl::data::Value* arg)
    {
        // Logarithm
        ArithmeticArg a;
        switch (checkArithmetic(a, arg)) {
         case ariNull:
            return 0;
         case ariInt:
            if (a.ia <= 0)
                throw Error::rangeError();
            else
                return makeFloatValue(std::log(double(a.ia)));
         case ariFloat:
            if (a.fa <= 0)
                throw Error::rangeError();
            else
                return makeFloatValue(std::log(a.fa));
         default:
            throw Error::typeError(Error::ExpectNumeric);
        }
    }

    afl::data::Value* FBitNot(interpreter::World& /*world*/, const afl::data::Value* arg)
    {
        // Bitwise negation
        ArithmeticArg a;
        switch (checkArithmetic(a, arg)) {
         case ariNull:
            return 0;
         case ariInt:
            return makeIntegerValue(~a.ia);
         case ariFloat:
         default:
            throw Error::typeError(Error::ExpectInteger);
        }
    }

    afl::data::Value* FIsEmpty(interpreter::World& /*world*/, const afl::data::Value* arg)
    {
        // Check emptiness (null)
        return makeBooleanValue(arg == 0);
    }

    afl::data::Value* FIsNum(interpreter::World& /*world*/, const afl::data::Value* arg)
    {
        // Check numericness
        // FIXME: PCC 1.x returns False for bools
        return makeBooleanValue(arg != 0
                                && (dynamic_cast<const afl::data::ScalarValue*>(arg) != 0
                                    || dynamic_cast<const afl::data::FloatValue*>(arg) != 0));
    }

    afl::data::Value* FIsString(interpreter::World& /*world*/, const afl::data::Value* arg)
    {
        // Check string
        return makeBooleanValue(arg != 0
                                && dynamic_cast<const afl::data::StringValue*>(arg) != 0);
    }

    afl::data::Value* FAsc(interpreter::World& /*world*/, const afl::data::Value* arg)
    {
        // Get ASCII/Unicode code of (stringified) arg
        if (arg == 0)
            return 0;

        String_t s = interpreter::toString(arg, false);
        afl::charset::Utf8Reader rdr(afl::string::toBytes(s), 0);
        if (rdr.hasMore()) {
            return makeIntegerValue(rdr.eat());
        } else {
            return 0;
        }
    }

    afl::data::Value* FChr(interpreter::World& /*world*/, const afl::data::Value* arg)
    {
        // Get character from ASCII/Unicode code
        if (arg == 0) {
            return 0;
        } else if (const afl::data::ScalarValue* iv = dynamic_cast<const afl::data::ScalarValue*>(arg)) {
            String_t tmp;
            afl::charset::Utf8().append(tmp, iv->getValue());
            return makeStringValue(tmp);
        } else {
            throw Error::typeError(Error::ExpectInteger);
        }
    }

    afl::data::Value* FStr(interpreter::World& /*world*/, const afl::data::Value* arg)
    {
        // Simple stringification
        if (arg == 0)
            return 0;
        return makeStringValue(interpreter::toString(arg, false));
    }

    afl::data::Value* FSqrt(interpreter::World& /*world*/, const afl::data::Value* arg)
    {
        // Square root
        ArithmeticArg a;
        switch (checkArithmetic(a, arg)) {
         case ariNull:
            return 0;
         case ariInt:
            if (a.ia < 0)
                throw Error::rangeError();
            return makeFloatValue(std::sqrt(double(a.ia)));
         case ariFloat:
            if (a.fa < 0)
                throw Error::rangeError();
            return makeFloatValue(std::sqrt(double(a.fa)));
         default:
            throw Error::typeError(Error::ExpectNumeric);
        }
    }

    afl::data::Value* FTrunc(interpreter::World& /*world*/, const afl::data::Value* arg)
    {
        // Truncate fractional digits
        ArithmeticArg a;
        switch (checkArithmetic(a, arg)) {
         case ariNull:
            return 0;
         case ariInt:
            return makeIntegerValue(a.ia);
         case ariFloat:
            if (std::fabs(a.fa) > 2147483647.0)
                throw Error::rangeError();
            return makeIntegerValue(int32_t(a.fa));
         default:
            throw Error::typeError(Error::ExpectNumeric);
        }
    }

    afl::data::Value* FRound(interpreter::World& /*world*/, const afl::data::Value* arg)
    {
        // Round arithmetically
        ArithmeticArg a;
        switch (checkArithmetic(a, arg)) {
         case ariNull:
            return 0;
         case ariInt:
            return makeIntegerValue(a.ia);
         case ariFloat:
            if (std::fabs(a.fa) > 2147483647.0)
                throw Error::rangeError();
            if (a.fa > 0)
                a.fa += 0.5;
            else
                a.fa -= 0.5;
            return makeIntegerValue(int32_t(a.fa));
         default:
            throw Error::typeError(Error::ExpectNumeric);
        }
    }

    afl::data::Value* FLTrim(interpreter::World& /*world*/, const afl::data::Value* arg)
    {
        // Trim left whitespace
        if (arg == 0)
            return 0;
        else if (const afl::data::StringValue* sv = dynamic_cast<const afl::data::StringValue*>(arg))
            return makeStringValue(afl::string::strLTrim(sv->getValue()));
        else
            throw Error::typeError(Error::ExpectString);
    }

    afl::data::Value* FRTrim(interpreter::World& /*world*/, const afl::data::Value* arg)
    {
        // Trim right whitespace
        if (arg == 0)
            return 0;
        else if (const afl::data::StringValue* sv = dynamic_cast<const afl::data::StringValue*>(arg))
            return makeStringValue(afl::string::strRTrim(sv->getValue()));
        else
            throw Error::typeError(Error::ExpectString);
    }

    afl::data::Value* FLRTrim(interpreter::World& /*world*/, const afl::data::Value* arg)
    {
        // Trim both whitespace
        if (arg == 0)
            return 0;
        else if (const afl::data::StringValue* sv = dynamic_cast<const afl::data::StringValue*>(arg))
            return makeStringValue(afl::string::strTrim(sv->getValue()));
        else
            throw Error::typeError(Error::ExpectString);
    }

    afl::data::Value* FLength(interpreter::World& /*world*/, const afl::data::Value* arg)
    {
        // Get length of string
        // FIXME: PCC 1.x stringifies. Should we, too?
        if (arg == 0)
            return 0;
        else if (const afl::data::StringValue* sv = dynamic_cast<const afl::data::StringValue*>(arg))
            return makeSizeValue(afl::charset::Utf8().length(sv->getValue()));
        else
            throw Error::typeError(Error::ExpectString);
    }

    afl::data::Value* FVal(interpreter::World& /*world*/, const afl::data::Value* arg)
    {
        // Evaluate
        if (arg == 0) {
            return 0;
        } else if (const afl::data::StringValue* sv = dynamic_cast<const afl::data::StringValue*>(arg)) {
            String_t ssv = sv->getValue();
            if (ssv.find_first_not_of(" \t0123456789-+.") != ssv.npos) {
                // syntax error, i.e. hex
                return 0;
            } else {
                // no illegal characters, so try to convert.
                // strtod skips initial whitespace, but not trailing whitespace.
                const char* ssvp = ssv.c_str();
                char* end;
                double d = std::strtod(ssvp, &end);
                if (end == ssvp || end[std::strspn(end, " \t")] != 0)
                    return 0;

                // Int or float?
                if (ssv.find('.') == ssv.npos && std::fabs(d) <= 2147483647.0)
                    return makeIntegerValue(int32_t(d));
                else
                    return makeFloatValue(d);
            }
        } else {
            throw Error::typeError(Error::ExpectString);
        }
    }

    afl::data::Value* FTrace(interpreter::World& world, const afl::data::Value* arg)
    {
        // Debug log; print value and continue
        world.logListener().write(afl::sys::LogListener::Trace, "script", interpreter::toString(arg, true));
        return afl::data::Value::cloneOf(arg);
    }

    afl::data::Value* FNot2(interpreter::World& /*world*/, const afl::data::Value* arg)
    {
        // Negation using binary logic: t->f, f->t, e->t
        return makeBooleanValue(getBooleanValue(arg) <= 0);
    }

    afl::data::Value* FAtom(interpreter::World& world, const afl::data::Value* arg)
    {
        // String to atom
        if (arg == 0)
            return 0;
        return makeIntegerValue(world.atomTable().getAtomFromString(interpreter::toString(arg, false)));
    }

    afl::data::Value* FAtomStr(interpreter::World& world, const afl::data::Value* arg)
    {
        // Atom to string
        if (arg == 0)
            return 0;

        const afl::data::ScalarValue* iv = dynamic_cast<const afl::data::ScalarValue*>(arg);
        if (!iv)
            throw Error::typeError(Error::ExpectInteger);

        return makeStringValue(world.atomTable().getStringFromAtom(iv->getValue()));
    }

    afl::data::Value* FKeyCreate(interpreter::World& world, const afl::data::Value* arg)
    {
        if (arg == 0)
            return 0;

        const afl::data::StringValue* sv = dynamic_cast<const afl::data::StringValue*>(arg);
        if (!sv)
            throw Error::typeError(Error::ExpectString);

        util::KeymapRef_t km = world.keymaps().createKeymap(sv->getValue());
        return interpreter::makeKeymapValue(km);
    }

    afl::data::Value* FKeyLookup(interpreter::World& world, const afl::data::Value* arg)
    {
        if (arg == 0)
            return 0;

        const afl::data::StringValue* sv = dynamic_cast<const afl::data::StringValue*>(arg);
        if (!sv)
            throw Error::typeError(Error::ExpectString);

        util::KeymapRef_t km = world.keymaps().getKeymapByName(sv->getValue());
        if (km == 0)
            throw Error("No such keymap");
        return interpreter::makeKeymapValue(km);
    }

    afl::data::Value* FInc(interpreter::World& /*world*/, const afl::data::Value* arg)
    {
        // arithmetic increment
        ArithmeticArg a;
        switch (checkArithmetic(a, arg)) {
         case ariNull:
            return 0;
         case ariInt:
            return makeIntegerValue(a.ia + 1);
         case ariFloat:
            return makeFloatValue(a.fa + 1);
         default:
            throw Error::typeError(Error::ExpectNumeric);
        }
    }

    afl::data::Value* FDec(interpreter::World& /*world*/, const afl::data::Value* arg)
    {
        // arithmetic decrement
        ArithmeticArg a;
        switch (checkArithmetic(a, arg)) {
         case ariNull:
            return 0;
         case ariInt:
            return makeIntegerValue(a.ia - 1);
         case ariFloat:
            return makeFloatValue(a.fa - 1);
         default:
            throw Error::typeError(Error::ExpectNumeric);
        }
    }

    afl::data::Value* FIsProcedure(interpreter::World& /*world*/, const afl::data::Value* arg)
    {
        // check for procedure
        if (!arg)
            return 0;

        if (const interpreter::CallableValue* cv = dynamic_cast<const interpreter::CallableValue*>(arg)) {
            // Callable builtin
            return makeBooleanValue(cv->isProcedureCall());
        } else {
            // Variable
            return makeBooleanValue(false);
        }
    }

    afl::data::Value* FFileNr(interpreter::World& /*world*/, const afl::data::Value* arg)
    {
        // null
        if (!arg)
            return 0;

        // integer?
        if (const afl::data::ScalarValue* iv = dynamic_cast<const afl::data::ScalarValue*>(arg))
            return new interpreter::FileValue(iv->getValue());
        else if (const interpreter::FileValue* fv = dynamic_cast<const interpreter::FileValue*>(arg))
            return new interpreter::FileValue(fv->getFileNumber());
        else
            throw Error::typeError(Error::ExpectInteger);
    }

    afl::data::Value* FIsArray(interpreter::World& /*world*/, const afl::data::Value* arg)
    {
        if (!arg)
            return 0;
        else if (const interpreter::CallableValue* a = dynamic_cast<const interpreter::CallableValue*>(arg))
            return makeIntegerValue(a->getDimension(0));
        else
            return makeIntegerValue(0);
    }

    afl::data::Value* (*const unary_ops[])(interpreter::World&, const afl::data::Value*) = {
        FNot,
        FBool,
        FNeg,
        FPos,
        FSin,
        FCos,
        FTan,
        FZap,
        FAbs,
        FExp,
        FLog,
        FBitNot,
        FIsEmpty,
        FIsNum,
        FIsString,
        FAsc,
        FChr,
        FStr,
        FSqrt,
        FTrunc,
        FRound,
        FLTrim,
        FRTrim,
        FLRTrim,
        FLength,
        FVal,
        FTrace,
        FNot2,
        FAtom,
        FAtomStr,
        FKeyCreate,
        FKeyLookup,
        FInc,
        FDec,
        FIsProcedure,
        FFileNr,
        FIsArray,
    };
}

// /** Execute unary operation.
//     \param op Operation, IntUnaryOperation
//     \param arg User-supplied argument taken from value stack
//     \return New value to push on value stack */
afl::data::Value*
interpreter::executeUnaryOperation(World& world, uint8_t op, afl::data::Value* arg)
{
    // ex int/unary.h:executeUnaryOperation(uint8_t op, IntValue* arg)
    if (op < countof(unary_ops)) {
        return unary_ops[op](world, arg);
    } else {
        throw Error::internalError("invalid unary operation");
    }
}
