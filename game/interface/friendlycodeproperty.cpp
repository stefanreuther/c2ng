/**
  *  \file game/interface/friendlycodeproperty.cpp
  */

#include "game/interface/friendlycodeproperty.hpp"
#include "interpreter/values.hpp"

using interpreter::makeIntegerValue;
using interpreter::makeStringValue;

namespace {
    String_t convertFlags(game::spec::FriendlyCode::FlagSet_t flags)
    {
        using game::spec::FriendlyCode;
        String_t result;
        if (flags.contains(FriendlyCode::ShipCode)) {
            result += 's';
        }
        if (flags.contains(FriendlyCode::PlanetCode)) {
            result += 'p';
        }
        if (flags.contains(FriendlyCode::StarbaseCode)) {
            result += 'b';
        }
        if (flags.contains(FriendlyCode::CapitalShipCode)) {
            result += 'c';
        }
        if (flags.contains(FriendlyCode::AlchemyShipCode)) {
            result += 'a';
        }
        if (flags.contains(FriendlyCode::RegisteredCode)) {
            result += 'r';
        }
        if (flags.contains(FriendlyCode::UnspecialCode)) {
            result += 'u';
        }
        if (flags.contains(FriendlyCode::PrefixCode)) {
            result += 'x';
        }
        return result;
    }
}

afl::data::Value*
game::interface::getFriendlyCodeProperty(const game::spec::FriendlyCode& fc, FriendlyCodeProperty ifp, const PlayerList& players, afl::string::Translator& tx)
{
    switch (ifp) {
     case ifpName:
        /* @q Name:Str (Friendly Code Property)
           Friendly code.
           @since PCC2 2.40.1 */
        return makeStringValue(fc.getCode());
     case ifpDescription:
        /* @q Description:Str (Friendly Code Property)
           Description.
           A one-liner describing the friendly code.
           @since PCC2 2.40.1 */
        return makeStringValue(fc.getDescription(players, tx));
     case ifpFlags:
        /* @q Flags:Str (Friendly Code Property)
           Flags. Contains options specified for the friendly code:
           - "s": code is valid for ships
           - "p": code is valid for planets
           - "b": code is valid for starbases
           - "c": code is valid for capital ships
           - "a": code is valid for alchemy ships
           - "r": this is a registered-only code
           - "u": this is not a special friendly code
           - "x": this is a prefix, not a code (since PCC2 2.40.9)
           @since PCC2 2.40.1 */
        return makeStringValue(convertFlags(fc.getFlags()));
     case ifpRaces:
        /* @q Races$:Int (Friendly Code Property)
           Races.
           A bitfield with the "2^N" bit set if race N can use this code.
           @since PCC2 2.40.1 */
        return makeIntegerValue(fc.getRaces().toInteger());
    }
    return 0;
}
