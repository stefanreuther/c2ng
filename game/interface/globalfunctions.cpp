/**
  *  \file game/interface/globalfunctions.cpp
  *  \brief Global Functions
  */

#include <cmath>
#include "game/interface/globalfunctions.hpp"
#include "afl/base/staticassert.hpp"
#include "afl/data/floatvalue.hpp"
#include "afl/data/scalarvalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/string/format.hpp"
#include "game/config/booleanvalueparser.hpp"
#include "game/game.hpp"
#include "game/interface/taskeditorcontext.hpp"
#include "game/map/circularobject.hpp"
#include "game/root.hpp"
#include "game/spec/shiplist.hpp"
#include "game/turn.hpp"
#include "interpreter/context.hpp"
#include "interpreter/error.hpp"
#include "interpreter/process.hpp"
#include "interpreter/values.hpp"

using interpreter::checkBooleanArg;
using interpreter::checkIntegerArg;
using interpreter::checkStringArg;
using interpreter::makeBooleanValue;
using interpreter::makeFloatValue;
using interpreter::makeIntegerValue;
using interpreter::makeStringValue;

namespace {
    afl::data::Value* makeScalarValue(int32_t value, const game::config::ValueParser& parser)
    {
        if ((value == 0 || value == 1) && (dynamic_cast<const game::config::BooleanValueParser*>(&parser) != 0)) {
            return makeBooleanValue(value);
        } else {
            return makeIntegerValue(value);
        }
    }

    afl::data::Value* getConfigValue(game::Session& session, game::config::Configuration& config, const String_t& optName, int player, bool isHostConfig)
    {
        // Fetch option
        // (Unlike PCC2, resolve the alias first, so we automatically deal with badly-configured aliases.)
        const game::config::ConfigurationOption* opt = config.getOptionByName(optName);
        if (const game::config::AliasOption* alias = dynamic_cast<const game::config::AliasOption*>(opt)) {
            opt = alias->getForwardedOption();
        }
        if (!opt) {
            throw interpreter::Error(isHostConfig
                                     ? "Invalid first argument to \"Cfg\""
                                     : "Invalid first argument to \"Pref\"");
        }
        const char*const fn = isHostConfig ? "Cfg" : "Pref";

        if (const game::config::GenericIntegerArrayOption* bopt = dynamic_cast<const game::config::GenericIntegerArrayOption*>(opt)) {
            // Integers; optional player
            if (player == 0) {
                /* Possible limits are
                   2    NewNativesPopulationRange
                   4    WraparoundRectangle
                   8    MeteorShowerOreRanges
                   9    NewNativesRaceRate
                   10   ConfigExpOption, e.g. EModBayRechargeRate
                   11   ConfigStdOption, e.g. RaceMiningRate
                   \change c2ng has MAX_PLAYERS instead of 11, but otherwise, the logic remains the same. */
                game::Game* g = session.getGame().get();
                if (isHostConfig && bopt->getArray().size() == size_t(game::MAX_PLAYERS) && g != 0) {
                    player = g->getViewpointPlayer();
                } else {
                    throw interpreter::Error::tooFewArguments(fn);
                }
            }
            if (const int32_t* p = bopt->getArray().at(player - 1)) {
                return makeScalarValue(*p, bopt->parser());
            } else {
                throw interpreter::Error::rangeError();
            }
        } else if (const game::config::IntegerOption* intopt = dynamic_cast<const game::config::IntegerOption*>(opt)) {
            // single int, no player. Example: NumShips
            if (player != 0) {
                throw interpreter::Error::tooManyArguments(fn);
            }
            return makeScalarValue((*intopt)(), intopt->parser());
        } else if (const game::config::CostArrayOption* costopt = dynamic_cast<const game::config::CostArrayOption*>(opt)) {
            // Array of costs. Example: StarbaseCost
            if (player == 0) {
                game::Game* g = session.getGame().get();
                if (isHostConfig && g != 0) {
                    player = g->getViewpointPlayer();
                } else {
                    throw interpreter::Error::tooFewArguments(fn);
                }
            }
            return makeStringValue((*costopt)(player).toCargoSpecString());
        } else {
            // Anything else (including StringOption): just return the value.
            // FIXME: PCC 1.x splits ExperienceLevelNames
            if (player != 0) {
                throw interpreter::Error::tooManyArguments(fn);
            }
            return makeStringValue(opt->toString());
        }
    }
}

/* @q AutoTask(type:Int, Id:Int):Obj (Function)
   Access auto-task.
   The first parameter is the type of task to access:
   - 1: ship task
   - 2: planet task
   - 3: starbase task
   The second parameter is the object Id.

   The resulting object allows reading and manipulating the auto task using {@group Auto Task Property|Auto Task Properties}.
   If the unit does not have an auto task, a blank one will be created and can be populated.

   An auto task can only be accessed when it is suspended.
   Accessing the auto task will prevent it from executing.
   An auto task is blocked as long as at least one object returned by AutoTask() exists.
   Multiple distinct AutoTask() objects can exist and all show the same state.

   The <a href="pcc2:taskscreen">auto task screens</a> show a cursor which is also part of an auto task being edited.
   The cursor state is maintained as long as an AutoTask() object or the auto task screen is active.
   When all AutoTask() objects are gone, the cursor is reset.

   @since PCC2 2.40.7 */
afl::data::Value*
game::interface::IFAutoTask(game::Session& session, interpreter::Arguments& args)
{
    args.checkArgumentCount(2);

    // Parse args
    int32_t type, id;
    if (!checkIntegerArg(type, args.getNext())
        || !checkIntegerArg(id, args.getNext()))
    {
        return 0;
    }

    // Convert args
    interpreter::Process::ProcessKind k;
    switch (type) {
     case 1: k = interpreter::Process::pkShipTask;   break;
     case 2: k = interpreter::Process::pkPlanetTask; break;
     case 3: k = interpreter::Process::pkBaseTask;   break;
     default: throw interpreter::Error::rangeError();
    }

    return TaskEditorContext::create(session, k, id);
}


/* @q Cfg(key:Str, Optional player:Int):Any (Function)
   Access host configuration.
   The first parameter is the name of a configuration option as used in <tt>pconfig.src</tt>,
   such as "AllowHiss" or "UnitsPerTorpRate".
   The function returns the value of this option, an integer, boolean or string.

   If the option is an array option, the second parameter can be specified to determine which player's value to get.
   When the second parameter is not specified for an array option, the return value is the value for your race.

   @diff This function was available with a different, more complicated definition in PCC 0.98.5 up to 1.0.8,
   under the names %Cfg and %CfgL.

   @since PCC 1.0.9, PCC2 1.99.8, PCC2 2.40.1 */
afl::data::Value*
game::interface::IFCfg(game::Session& session, interpreter::Arguments& args)
{
    // ex int/if/globalif.h:IFCfgGet
    // ex ccexpr.pas:op_CFG_func
    args.checkArgumentCount(1, 2);

    // Config key
    String_t optName;
    if (!checkStringArg(optName, args.getNext())) {
        return 0;
    }

    // Player number
    int32_t player = 0;
    if (args.getNumArgs() > 0) {
        if (!checkIntegerArg(player, args.getNext(), 1, MAX_PLAYERS)) {
            return 0;
        }
    }

    // Available?
    Root* root = session.getRoot().get();
    if (!root) {
        return 0;
    }

    // Do it
    return getConfigValue(session, root->hostConfiguration(), optName, player, true);
}


/* @q Distance(obj1:Any, x2:Int, y2:Int):Num (Function)
   @noproto
   | Distance(x1:Int, y1:Int, x2:Int, y2:Int):Num
   | Distance(x1:Int, y1:Int, obj2:Any):Num
   | Distance(obj1:Any, x2:Int, y2:Int):Num
   | Distance(obj1:Any, obj2:Any):Num
   Compute distance between two points.
   Points can be specified as two integers for an X/Y coordinate pair,
   or an object which must have {Loc.X} and {Loc.Y} properties.
   Examples:
   | Distance(1000, 1000, 1200, 1200)
   | Distance(Ship(10), Planet(30))

   If a wrapped map is being used, the map seam is also considered and the shortest possible distance is reported.
   @since PCC 1.0.11, PCC2 1.99.8, PCC2 2.40.1 */
afl::data::Value*
game::interface::IFDistance(game::Session& session, interpreter::Arguments& args)
{
    // ex int/if/globalif.h:IFDistanceGet
    // ex ccexpr.pas:op_DISTANCE_func
    game::map::Point points[2];

    for (int i = 0; i < 2; ++i) {
        /* Do we have an argument? */
        if (args.getNumArgs() == 0) {
            throw interpreter::Error::tooFewArguments("Distance");
        }

        afl::data::Value* theValue = args.getNext();

        /* What is it? */
        if (theValue == 0) {
            /* Null. Result is null. */
            return 0;
        } else if (interpreter::Context* cv = dynamic_cast<interpreter::Context*>(theValue)) {
            /* Context. Must have LOC.X and LOC.Y properties. */
            interpreter::Context::PropertyIndex_t x, y;
            interpreter::Context::PropertyAccessor* xcv = cv->lookup("LOC.X", x);
            interpreter::Context::PropertyAccessor* ycv = cv->lookup("LOC.Y", y);
            if (!xcv || !ycv) {
                throw interpreter::Error("Operand doesn't have a position");
            }

            /* Fetch values */
            std::auto_ptr<afl::data::Value> vx(xcv->get(x));
            std::auto_ptr<afl::data::Value> vy(ycv->get(y));
            int32_t ix, iy;
            if (!checkIntegerArg(ix, vx.get()) || !checkIntegerArg(iy, vy.get())) {
                return 0;
            }

            points[i] = game::map::Point(ix, iy);
        } else {
            /* Possibly integer. There must be another integer. */
            if (args.getNumArgs() == 0) {
                throw interpreter::Error::tooFewArguments("Distance");
            }
            afl::data::Value* otherValue = args.getNext();

            /* Null? */
            int ix, iy;
            if (!checkIntegerArg(ix, theValue) || !checkIntegerArg(iy, otherValue)) {
                return 0;
            }

            points[i] = game::map::Point(ix, iy);
        }
    }

    if (args.getNumArgs() != 0) {
        throw interpreter::Error::tooManyArguments("Distance");
    }

    // Check game
    Game* g = session.getGame().get();
    if (g != 0) {
        return makeFloatValue(std::sqrt(double(g->mapConfiguration().getSquaredDistance(points[0], points[1]))));
    } else {
        return 0;
    }
}


/* @q Format(fmt:Str, args:Any...):Str (Function)
   Format a string.
   The format string can contain placeholders, each of which is replaced by one of the arguments,
   similar to the %sprintf function found in many programming languages.

   Some placeholders:
   - <tt>&#37;d</tt> formats an integer as a decimal number ("99")
   - <tt>&#37;e</tt> formats a fraction in exponential format ("9.99e+1")
   - <tt>&#37;f</tt> formats a fraction as regular decimal fraction ("99.9")
   - <tt>&#37;g</tt> auto-selects between <tt>&#37;e</tt> and <tt>&#37;f</tt>
   - <tt>&#37;x</tt> formats an integer as an octal number ("143")
   - <tt>&#37;s</tt> formats a string
   - <tt>&#37;x</tt> formats an integer as a hexadecimal number ("63")

   You can specify a decimal number between the percent sign and the letter
   to format the result with at least that many places.

   This function supports up to 10 arguments (plus the format string) in one call.

   @since PCC2 1.99.9, PCC2 2.40
   @see RXml */
afl::data::Value*
game::interface::IFFormat(game::Session& /*session*/, interpreter::Arguments& args)
{
    // ex int/if/globalif.h:IFFormat
    static_assert(afl::string::Format::MAX_ARGS == 10, "update documentation if this fails");
    const size_t LIMIT = afl::string::Format::MAX_ARGS;
    args.checkArgumentCount(1, LIMIT+1);

    // First, find the format string
    String_t fmt;
    if (!checkStringArg(fmt, args.getNext())) {
        return 0;
    }

    // The formatter is intended to be used as a temporary, and keeps references
    // to its arguments. So we must make sure they live sufficiently long.
    int32_t ints[LIMIT];
    double floats[LIMIT];
    String_t strings[LIMIT];
    afl::string::Format formatter(fmt.c_str());

    for (size_t i = 0, limit = args.getNumArgs(); i < limit; ++i) {
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

/* @q IsSpecialFCode(fc:Str):Bool (Function)
   Check for special friendly code.
   Returns true if the friendly code given as a parameter is a special friendly code.

   A special friendly code is one defined as special through the <tt>fcodes.cc</tt> file, or through <tt>xtrfcode.txt</tt>.
   Note that PCC2 before 2.0.8/2.40.8 does not consider <tt>xtrfcode.txt</tt>.

   @since PCC 1.1.4, PCC2 1.99.8, PCC2 2.40.1 */
afl::data::Value*
game::interface::IFIsSpecialFCode(game::Session& session, interpreter::Arguments& args)
{
    // ex int/if/globalif.h:IFIsSpecialFCodeGet
    // ex ccexpr.pas:op_ISSPECIALFCODE_func

    // Parse args
    args.checkArgumentCount(1);

    String_t str;
    if (!checkStringArg(str, args.getNext())) {
        return 0;
    }

    // Do it
    // @change If there is no ship list, return null
    const game::spec::ShipList* sl = session.getShipList().get();
    if (sl == 0) {
        return 0;
    }
    if (str.size() > 3) {
        str.erase(3);
    }
    return makeBooleanValue(sl->friendlyCodes().isSpecial(str, true));
}

/* @q ObjectIsAt(obj:Any, x:Int, y:Int):Bool (Function)
   Check whether object is at or covers a given coordinate.

   Objects that cover a single point (ships, planets) must be at that very location.
   Objects that cover an area (minefields, ion storms, Ufos) must cover that location.
   Wrap is considered.

   Returns a boolean value, or EMPTY if any parameter is EMPTY.
   @since PCC2 2.40.7 */
afl::data::Value*
game::interface::IFObjectIsAt(game::Session& session, interpreter::Arguments& args)
{
    // This is intended to implement search.
    // searchPlanets, searchShips:
    //    match = s.getPos() == query.location;
    // searchUfos, searchIonStorms:
    //    match = distanceSquared(it->getPos(), query.location) <= square(it->getRadius());
    // searchMinefields:
    //    match = distanceSquared(mf->getPos(), query.location) <= mf->getUnits();

    // Parse args
    int32_t x, y;
    args.checkArgumentCount(3);
    afl::data::Value* obj = args.getNext();
    if (!obj) {
        return 0;
    }
    if (!checkIntegerArg(x, args.getNext()) || !checkIntegerArg(y, args.getNext())) {
        return 0;
    }

    // Validate object
    interpreter::Context* ctx = dynamic_cast<interpreter::Context*>(obj);
    if (ctx == 0) {
        throw interpreter::Error::typeError(interpreter::Error::ExpectRecord);
    }

    const game::map::Object* mapObj = dynamic_cast<const game::map::Object*>(ctx->getObject());
    if (mapObj == 0) {
        throw interpreter::Error::typeError(interpreter::Error::ExpectRecord);
    }

    // Must have a current turn to access map configuration
    // @change If there is no game, return null [probably cannot happen because then we cannot create the respective Context]
    const Game* g = session.getGame().get();
    if (g == 0) {
        return 0;
    }
    const game::map::Configuration& config = g->mapConfiguration();

    // Different handling depending on object type
    game::map::Point thisPoint(x, y);
    game::map::Point objPos;
    if (!mapObj->getPosition().get(objPos)) {
        return 0;
    }
    if (const game::map::CircularObject* circObj = dynamic_cast<const game::map::CircularObject*>(mapObj)) {
        // Circular
        int32_t r2;
        if (!circObj->getRadiusSquared().get(r2)) {
            return 0;
        }
        return makeBooleanValue(config.getSquaredDistance(thisPoint, objPos) <= r2);
    } else {
        // Point
        return makeBooleanValue(config.getCanonicalLocation(thisPoint) == objPos);
    }
}


/* @q PlanetAt(x:Int, y:Int, Optional flag:Bool):Int (Function)
   Get planet by location.

   Returns the Id number of the planet at position (%x,%y).
   When %flag is True (nonzero, nonempty), returns the planet whose gravity wells are in effect at that place;
   when %flag is False or not specified at all, returns only exact matches.
   If there is no such planet, it returns zero.
   @since PCC 1.0.18, PCC2 1.99.9, PCC2 2.40.1 */
afl::data::Value*
game::interface::IFPlanetAt(game::Session& session, interpreter::Arguments& args)
{
    // ex int/if/globalif.h:IFPlanetAtGet
    // ex ccexpr.pas:op_PLANETAT_func
    int32_t x, y;

    // Fetch x,y parameters
    args.checkArgumentCount(2, 3);
    if (!checkIntegerArg(x, args.getNext(), -32767, 32767) || !checkIntegerArg(y, args.getNext(), -32767, 32767)) {
        return 0;
    }

    // Fetch optional flag argument
    bool flag = false;
    if (args.getNumArgs() > 0) {
        if (!checkBooleanArg(flag, args.getNext())) {
            return 0;
        }
    }

    // Generate result
    Root* root = session.getRoot().get();
    Game* game = session.getGame().get();
    if (root == 0 || game == 0) {
        return 0;
    }

    return makeIntegerValue(game->currentTurn().universe().findPlanetAt(game::map::Point(x, y), flag, game->mapConfiguration(), root->hostConfiguration(), root->hostVersion()));
}

/* @q Pref(key:Str, Optional index:Int):Any (Function)
   Access user configuration (preferences).
   The first parameter is the name of a preference setting, such as "Backup.Turn" or "Label.Ship".
   The function returns the value of this option, an integer, boolean or string.

   If the option is an array option, the second parameter must be specified as the index into the array,
   starting at 1.

   @since PCC2 2.40.1 */
afl::data::Value*
game::interface::IFPref(game::Session& session, interpreter::Arguments& args)
{
    args.checkArgumentCount(1, 2);

    // Config key
    String_t optName;
    if (!checkStringArg(optName, args.getNext())) {
        return 0;
    }

    // Index
    int32_t index = 0;
    if (args.getNumArgs() > 0) {
        if (!checkIntegerArg(index, args.getNext(), 1, 100)) {
            return 0;
        }
    }

    // Available?
    Root* root = session.getRoot().get();
    if (!root) {
        return 0;
    }

    // Do it
    return getConfigValue(session, root->userConfiguration(), optName, index, false);
}

/* @q Quote(val:Any):Str (Function)
   Convert to string and quote a value.
   The value is formatted in a way such that Eval() will produce the value again.
   This function is guaranteed to work for scalars:
   - strings;
   - integers;
   - booleans;
   - floats (except for possible precision issues);
   - EMPTY.
   It will also work for some non-scalars where possible, e.g. <tt>Quote(Planet(3))</tt> will produce <tt>Planet(3)</tt>.

   @since PCC2 2.40.12 */
afl::data::Value*
game::interface::IFQuote(game::Session& /*session*/, interpreter::Arguments& args)
{
    args.checkArgumentCount(1);
    return makeStringValue(interpreter::toString(args.getNext(), true));
}

/* @q Random(a:Int, Optional b:Int):Int (Function)
   Generate random number.
   With one parameter, generates a random number in range [0,a) (i.e. including zero, not including %a).
   With two parameters, generates a random number in range [a,b) (i.e. including %a, not including %b).

   For example, <tt>Random(10)</tt> generates random numbers between 0 and 9, as does <tt>Random(0, 10)</tt>.

   <tt>Random(1,500)</tt> generates random numbers between 1 and 499,
   <tt>Random(500,1)</tt> generates random numbers between 2 and 500
   (the first parameter always included in the range, the second one is not).

   The maximum value for either parameter is 32767 (=15 bit).

   @since PCC 1.0.7, PCC2 1.99.9, PCC2 2.40 */
afl::data::Value*
game::interface::IFRandom(game::Session& session, interpreter::Arguments& args)
{
    // ex int/if/globalif.h:IFRandom
    // ex ccexpr.pas:op_RANDOM_func
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
        return makeIntegerValue(lo + session.rng()(uint16_t(hi - lo)));
    } else if (lo > hi) {
        return makeIntegerValue(lo - session.rng()(uint16_t(lo - hi)));
    } else {
        return makeIntegerValue(lo);
    }
}

/* @q RandomFCode():Str (Function)
   Generate a random friendly code.
   The friendly code will not have a special meaning.
   @since PCC 1.1.11, PCC2 1.99.8, PCC2 2.40.1 */
afl::data::Value*
game::interface::IFRandomFCode(game::Session& session, interpreter::Arguments& args)
{
    // ex int/if/globalif.h:IFRandomFCode
    // ex ccexpr.pas:op_RANDOMFCODE_func
    args.checkArgumentCount(0);

    game::spec::ShipList* shipList = session.getShipList().get();
    Root* root = session.getRoot().get();
    if (shipList != 0 && root != 0) {
        return makeStringValue(shipList->friendlyCodes().generateRandomCode(session.rng(), root->hostVersion()));
    } else {
        return 0;
    }
}

/* @q Translate(str:Str):Str (Function)
   Translate a string.
   Uses PCC's internal language database to reproduce the English string given as parameter
   in the user's preferred language.
   If the string is not contained in the language database, returns the original string.
   @since PCC2 1.99.9, PCC2 2.40 */
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

/* @q Truehull(slot:Int, Optional player:Int):Int (Function)
   Access per-player hull assignments.
   Returns the Id of the %slot'th hull number the specified %player can build.
   If the %player parameter is omitted, uses your player slot.
   If the specified slot does not contain a buildable hull, returns 0.
   @since PCC 1.0.12, PCC2 1.99.8, PCC2 2.40 */
afl::data::Value*
game::interface::IFTruehull(game::Session& session, interpreter::Arguments& args)
{
    // ex int/if/globalif.h:IFTruehull
    // ex ccexpr.pas:op_TRUEHULL_func
    int32_t slot, player;

    args.checkArgumentCount(1, 2);
    if (!checkIntegerArg(slot, args.getNext())) {
        return 0;
    }

    if (args.getNumArgs() > 0) {
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
