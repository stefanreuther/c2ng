/**
  *  \file game/interface/friendlycodeproperty.hpp
  *  \brief Enum game::interface::FriendlyCodeProperty
  */
#ifndef C2NG_GAME_INTERFACE_FRIENDLYCODEPROPERTY_HPP
#define C2NG_GAME_INTERFACE_FRIENDLYCODEPROPERTY_HPP

#include "afl/data/value.hpp"
#include "afl/string/translator.hpp"
#include "game/spec/friendlycode.hpp"

namespace game { namespace interface {

    /** Friendly code property definition. */
    enum FriendlyCodeProperty {
        ifpName,                // Name:Str
        ifpDescription,         // Description:Str
        ifpFlags,               // Flags:Str
        ifpRaces                // Races$:Int
    };

    /** Get property of a friendly code definition.
        @param fc      Friendly code definition
        @param ifp     Property
        @param players Player list (for formatting descriptions)
        @param tx      Translator
        @return Newly-allocated value */
    afl::data::Value* getFriendlyCodeProperty(const game::spec::FriendlyCode& fc, FriendlyCodeProperty ifp, const PlayerList& players, afl::string::Translator& tx);

} }

#endif
