/**
  *  \file game/interface/globalfunctions.cpp
  */

#include "game/interface/globalfunctions.hpp"
#include "interpreter/values.hpp"
#include "afl/string/format.hpp"
#include "afl/base/staticassert.hpp"
#include "interpreter/error.hpp"
#include "afl/data/scalarvalue.hpp"
#include "afl/data/floatvalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "game/spec/shiplist.hpp"
#include "game/game.hpp"
#include "game/root.hpp"

using interpreter::checkStringArg;
using interpreter::checkIntegerArg;
using interpreter::makeStringValue;
using interpreter::makeIntegerValue;

// /* @q Format(fmt:Str, args:Any...):Str (Function)
//    Format a string.
//    The format string can contain placeholders, each of which is replaced by one of the arguments,
//    similar to the %sprintf function found in many programming languages.

//    Some placeholders:
//    - <tt>&#37;d</tt> formats an integer as a decimal number ("99")
//    - <tt>&#37;e</tt> formats a fraction in exponential format ("9.99e+1")
//    - <tt>&#37;f</tt> formats a fraction as regular decimal fraction ("99.9")
//    - <tt>&#37;g</tt> auto-selects between <tt>&#37;e</tt> and <tt>&#37;f</tt>
//    - <tt>&#37;x</tt> formats an integer as an octal number ("143")
//    - <tt>&#37;s</tt> formats a string
//    - <tt>&#37;x</tt> formats an integer as a hexadecimal number ("63")

//    You can specify a decimal number between the percent sign and the letter
//    to format the result with at least that many places.

//    This function supports up to 10 arguments (plus the format string) in one call.

//    @since PCC2 1.99.9
//    @see RXml */
afl::data::Value*
game::interface::IFFormat(game::Session& /*session*/, interpreter::Arguments& args)
{
    // ex int/if/globalif.h:IFFormat
    static_assert(afl::string::Format::MAX_ARGS == 10, "update documentation if this fails");
    const size_t LIMIT = afl::string::Format::MAX_ARGS;
    args.checkArgumentCount(1, LIMIT+1);

    // First, find the format string
    String_t fmt; {
    if (!checkStringArg(fmt, args.getNext()))
        return 0;
    }

    // The formatter is intended to be used as a temporary, and keeps references
    // to its arguments. So we must make sure they live sufficiently long.
    int32_t ints[LIMIT];
    double floats[LIMIT];
    String_t strings[LIMIT];
    afl::string::Format formatter(fmt.c_str());

    for (uint32_t i = 0, limit = args.getNumArgs(); i < limit; ++i) {
        // Check and convert value
        afl::data::Value* v = args.getNext();
        if (v == 0) {
            return 0;
        }

        if (afl::data::ScalarValue* iv = dynamic_cast<afl::data::ScalarValue*>(v)) {
            ints[i] = iv->getValue();
            formatter << ints[i];
        } else if (afl::data::FloatValue* fv = dynamic_cast<afl::data::FloatValue*>(v)) {
            floats[i] = fv->getValue();
            formatter << floats[i];
        } else if (afl::data::StringValue* sv = dynamic_cast<afl::data::StringValue*>(v)) {
            strings[i] = sv->getValue();
            formatter << strings[i];
        } else {
            throw interpreter::Error::typeError(interpreter::Error::ExpectBaseType);
        }
    }

    // Format
    return makeStringValue(formatter);
}

// /* @q Random(a:Int, Optional b:Int):Int (Function)
//    Generate random number.
//    With one parameter, generates a random number in range [0,a) (i.e. including zero, not including %a).
//    With two parameters, generates a random number in range [a,b) (i.e. including %a, not including %b).

//    For example, <tt>Random(10)</tt> generates random numbers between 0 and 9, as does <tt>Random(0, 10)</tt>.

//    <tt>Random(1,500)</tt> generates random numbers between 1 and 499,
//    <tt>Random(500,1)</tt> generates random numbers between 2 and 500
//    (the first parameter always included in the range, the second one is not).

//    @since PCC 1.0.7, PCC2 1.99.9 */
afl::data::Value*
game::interface::IFRandom(game::Session& session, interpreter::Arguments& args)
{
    // ex int/if/globalif.h:IFRandom
    int32_t lo, hi;
    args.checkArgumentCount(1, 2);
    if (!checkIntegerArg(lo, args.getNext(), 0, 0x7FFF)) {
        return 0;
    }

    if (args.getNumArgs() > 0) {
        if (!checkIntegerArg(hi, args.getNext(), 0, 0x7FFF)) {
            return 0;
        }
    } else {
        hi = lo;
        lo = 0;
    }

    if (lo < hi) {
        return makeIntegerValue(lo + session.rng()(hi - lo));
    } else if (lo > hi) {
        return makeIntegerValue(lo - session.rng()(lo - hi));
    } else {
        return makeIntegerValue(lo);
    }
}

// /* @q Translate(str:Str):Str (Function)
//    Translate a string.
//    Uses PCC's internal language database to reproduce the English string given as parameter
//    in the user's preferred language.
//    If the string is not contained in the language database, returns the original string.
//    @since PCC2 1.99.9 */
afl::data::Value*
game::interface::IFTranslate(game::Session& session, interpreter::Arguments& args)
{
    // ex int/if/globalif.h:IFTranslate
    args.checkArgumentCount(1);
    String_t s;
    if (!checkStringArg(s, args.getNext())) {
        return 0;
    } else {
        return makeStringValue(session.translator().translateString(s));
    }    
}

// /* @q Truehull(slot:Int, Optional player:Int):Int (Function)
//    Access per-player hull assignments.
//    Returns the Id of the %slot'th hull number the specified %player can build.
//    If the %player parameter is omitted, uses your player slot.
//    If the specified slot does not contain a buildable hull, returns 0.
//    @since PCC 1.0.12, PCC2 1.99.8 */
afl::data::Value*
game::interface::IFTruehull(game::Session& session, interpreter::Arguments& args)
{
    // ex int/if/globalif.h:IFTruehull
    int32_t slot, player;

    args.checkArgumentCount(1, 2);
    if (!checkIntegerArg(slot, args.getNext())) {
        return 0;
    }

    if (args.getNumArgs() > 0) {
        // FIXME: this is inconsistent in that a missing argument does not equal a null argument
        if (!checkIntegerArg(player, args.getNext())) {
            return 0;
        }
    } else {
        if (Game* game = session.getGame().get()) {
            player = game->getViewpointPlayer();
        } else {
            return 0;
        }
    }

    game::spec::ShipList* list = session.getShipList().get();
    Root* root = session.getRoot().get();
    if (list != 0 && root != 0) {
        return makeIntegerValue(list->hullAssignments().getHullFromIndex(root->hostConfiguration(), player, slot));
    } else {
        return 0;
    }
}
