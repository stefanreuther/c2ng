/**
  *  \file game/interface/hullproperty.hpp
  */
#ifndef C2NG_GAME_INTERFACE_HULLPROPERTY_HPP
#define C2NG_GAME_INTERFACE_HULLPROPERTY_HPP

#include "afl/data/value.hpp"
#include "game/spec/hull.hpp"
#include "game/spec/shiplist.hpp"
#include "game/config/hostconfiguration.hpp"

namespace game { namespace interface {

    /** Definition of hull properties. */
    enum HullProperty {
        ihpMaxBeams,
        ihpMaxCargo,
        ihpMaxFuel,
        ihpMaxCrew,
        ihpNumEngines,
        ihpNumFighterBays,
        ihpSpecial,
        ihpImage,
        ihpImage2,
        ihpMaxTorpLaunchers
    };
    
    afl::data::Value* getHullProperty(const game::spec::Hull& h, HullProperty isp, const game::spec::ShipList& list, const game::config::HostConfiguration& config);
    void setHullProperty(game::spec::Hull& h, HullProperty isp, const afl::data::Value* value, game::spec::ShipList& list);

} }

#endif
