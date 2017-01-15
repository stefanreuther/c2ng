/**
  *  \file game/interface/friendlycodeproperty.hpp
  */
#ifndef C2NG_GAME_INTERFACE_FRIENDLYCODEPROPERTY_HPP
#define C2NG_GAME_INTERFACE_FRIENDLYCODEPROPERTY_HPP

#include "afl/data/value.hpp"
#include "game/spec/friendlycode.hpp"

namespace game { namespace interface {

    enum FriendlyCodeProperty {
        ifpName,                // Name:Str
        ifpDescription,         // Description:Str
        ifpFlags,               // Flags:Str
        ifpRaces                // Races$:Int
    };

    afl::data::Value* getFriendlyCodeProperty(const game::spec::FriendlyCode& fc, FriendlyCodeProperty ifp, const PlayerList& players);

} }

#endif
