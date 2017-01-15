/**
  *  \file game/interface/cargofunctions.cpp
  */

#include "game/interface/cargofunctions.hpp"
#include "interpreter/values.hpp"
#include "afl/data/scalarvalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "interpreter/error.hpp"

using interpreter::makeStringValue;
using interpreter::makeIntegerValue;
using interpreter::makeBooleanValue;
using interpreter::checkIntegerArg;
using interpreter::checkStringArg;

namespace {
    /** Common back-end for CExtract and CRemove.
        \param cs [in/out] CargoSpec. On output, the result of CRemove(cs, s).
        \param total [out] On output, result of CExtract(cs, s).
        \param s [in] Type specification */
    void doCExtractRemove(game::CargoSpec& cs, int32_t& total, const String_t& s)
    {
        // ex int/if/globalif.h:doCExtractRemove
        total = 0;
        for (String_t::size_type i = 0; i < s.size(); ++i) {
            if (uint8_t(s[i]) > ' ') {
                game::CargoSpec::Type type;
                if (game::CargoSpec::charToType(s[i], type)) {
                    total += cs.get(type);
                    cs.set(type, 0);
                } else {
                    throw interpreter::Error("Invalid cargo type");
                }
            }
        }
    }
}

// /** Check cargospec argument.
//     \param out   [out] Result will be placed here
//     \param value [in] Value given by user
//     \return true if value was specified, false if value was null (out not changed)
//     \throw IntError if value is invalid (can currently not happen) */
bool
game::interface::checkCargoSpecArg(CargoSpec& out, afl::data::Value* value)
{
    // ex int/if/ifutil.h:checkCargoSpecArg
    if (value == 0) {
        return false;
    }

    out = CargoSpec(interpreter::toString(value, false), false);
    return true;
}


// /* @q CAdd(a:Cargo...):Cargo (Function)
//    Add cargo sets.
//    Returns a new cargo set containing the sum of all cargo sets given as parameter.
//    @diff PCC 1.x supports two to six arguments for this function, PCC2 supports any number from one up.
//    @since PCC 1.0.10, PCC2 1.99.9, PCC2NG 2.40.1
//    @see CSub */
afl::data::Value*
game::interface::IFCAdd(game::Session& /*session*/, interpreter::Arguments& args)
{
    // ex int/if/globalif.h:IFCAdd
    // At least one arg (PCC 1.x: at least two)
    args.checkArgumentCountAtLeast(1);

    CargoSpec sum;
    while (args.getNumArgs() != 0) {
        CargoSpec summand;
        if (!checkCargoSpecArg(summand, args.getNext())) {
            return 0;
        }
        sum += summand;
    }

    return makeStringValue(sum.toCargoSpecString());
}

// /* @q CCompare(a:Cargo, b:Cargo):Bool (Function)
//    Compare cargo sets.
//    Returns true if %a contains enough cargo to remove %b.
//    Supply sale is taken into account.
//    @since PCC 1.0.10, PCC2 1.99.9
//    @see CSub */
afl::data::Value*
game::interface::IFCCompare(game::Session& /*session*/, interpreter::Arguments& args)
{
    // ex int/if/globalif.h:IFCCompare
    args.checkArgumentCount(2);

    CargoSpec a, b;
    if (!checkCargoSpecArg(a, args.getNext())) {
        return 0;
    }
    if (!checkCargoSpecArg(b, args.getNext())) {
        return 0;
    }

    return makeBooleanValue(a.isEnoughFor(b));
}

// /* @q CDiv(a:Cargo, n:Int):Cargo (Function)
//    @noproto
//    | CDiv(a:Cargo, n:Int):Cargo
//    | CDiv(a:Cargo, b:Cargo):Int
//    Divide cargo sets.

//    In the first form, tries to divide the cargo set %a into %n equal parts, and returns the size of one part,
//    as a cargo set.

//    In the second form, tries to determine how many times %b can be removed from %a.
//    Supply sale is taken into account.

//    @since PCC 1.1.17, PCC2 1.99.9 */
afl::data::Value*
game::interface::IFCDiv(game::Session& /*session*/, interpreter::Arguments& args)
{
    // ex int/if/globalif.h:IFCDiv
    args.checkArgumentCount(2);

    CargoSpec a;
    if (!checkCargoSpecArg(a, args.getNext())) {
        return 0;
    }

    afl::data::Value* arg = args.getNext();
    if (arg == 0) {
        // Null
        return 0;
    } else if (afl::data::ScalarValue* iv = dynamic_cast<afl::data::ScalarValue*>(arg)) {
        // Divide cargospec by integer
        if (!a.divide(iv->getValue())) {
            throw interpreter::Error("Divide by zero");
        }
        return makeStringValue(a.toCargoSpecString());
    } else if (afl::data::StringValue* sv = dynamic_cast<afl::data::StringValue*>(arg)) {
        // Divide cargospec by cargospec
        CargoSpec b(sv->getValue(), false);
        int32_t result;
        if (!a.divide(b, result)) {
            throw interpreter::Error("Divide by zero");
        }
        return makeIntegerValue(result);
    } else {
        throw interpreter::Error::typeError(interpreter::Error::ExpectBaseType);
    }
}

// /* @q CExtract(a:Cargo, ele:Str):Int (Function)
//    Extract cargo.
//    %a is a cargo set, %ele is a cargo type (e.g. <tt>"n"</tt> for Neutronium).
//    This function returns the amount of that cargo type in the cargo set.
//    If multiple cargo types are given, their amounts are summed up, e.g.
//    | CExtract(e, "s$")
//    will report the total number of supplies and money in cargo set %e.
//    @since PCC 1.0.10, PCC2 1.99.9 */
afl::data::Value*
game::interface::IFCExtract(game::Session& /*session*/, interpreter::Arguments& args)
{
    // ex int/if/globalif.h:IFCExtract
    args.checkArgumentCount(2);

    // Get first arg
    CargoSpec cs; {
    if (!checkCargoSpecArg(cs, args.getNext()))
        return 0;
    }

    // Get second arg
    String_t s;
    if (!checkStringArg(s, args.getNext())) {
        return 0;
    }

    // Compute result
    int32_t result;
    doCExtractRemove(cs, result, s);

    return makeIntegerValue(result);
}

// /* @q CMul(a:Cargo, n:Int):Cargo (Function)
//    Multiply cargo set.
//    Returns a new cargo set containing %n times %a.
//    @since PCC 1.0.10, PCC2 1.99.9 */
afl::data::Value*
game::interface::IFCMul(game::Session& /*session*/, interpreter::Arguments& args)
{
    // ex int/if/globalif.h:IFCMul
    args.checkArgumentCount(2);

    // Get first arg
    CargoSpec cs;
    if (!checkCargoSpecArg(cs, args.getNext())) {
        return 0;
    }

    // Get second arg
    int32_t n;
    if (!checkIntegerArg(n, args.getNext())) {
        return 0;
    }

    cs *= n;
    return makeStringValue(cs.toCargoSpecString());
}

// /* @q CRemove(a:Cargo, ele:Str):Cargo (Function)
//    Remove cargo.
//    %a is a cargo set, %ele is a cargo type (e.g. <tt>"n"</tt> for Neutronium).
//    This function returns a cargo set with all cargo of the specified type removed.
//    %ele can also contain multiple cargo types to remove.
//    @since PCC 1.0.10, PCC2 1.99.9 */
afl::data::Value*
game::interface::IFCRemove(game::Session& /*session*/, interpreter::Arguments& args)
{
    // ex int/if/globalif.h:IFCRemove
    args.checkArgumentCount(2);

    // Get first arg
    CargoSpec cs;
    if (!checkCargoSpecArg(cs, args.getNext())) {
        return 0;
    }

    // Get second arg
    String_t s;
    if (!checkStringArg(s, args.getNext())) {
        return 0;
    }

    // Compute result
    int32_t tmp;
    doCExtractRemove(cs, tmp, s);

    return makeStringValue(cs.toCargoSpecString());
}

// /* @q CSub(a:Cargo, b:Cargo...):Cargo (Function)
//    Subtract cargo sets.
//    Removes %b and all following sets from %a, and returns the result.
//    Supply sale is taken into account.
//    @since PCC 1.0.10, PCC2 1.99.9
//    @see CCompare, CAdd */
afl::data::Value*
game::interface::IFCSub(game::Session& /*session*/, interpreter::Arguments& args)
{
    // ex int/if/globalif.h:IFCSub
    args.checkArgumentCountAtLeast(2);

    CargoSpec dif;
    if (!checkCargoSpecArg(dif, args.getNext())) {
        return 0;
    }

    while (args.getNumArgs() > 0) {
        CargoSpec subtr;
        if (!checkCargoSpecArg(subtr, args.getNext())) {
            return 0;
        }
        dif -= subtr;
    }

    dif.sellSuppliesIfNeeded();

    return makeStringValue(dif.toCargoSpecString());
}
