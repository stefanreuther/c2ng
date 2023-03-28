/**
  *  \file game/interface/hullproperty.hpp
  *  \brief Enum game::interface::HullProperty
  */
#ifndef C2NG_GAME_INTERFACE_HULLPROPERTY_HPP
#define C2NG_GAME_INTERFACE_HULLPROPERTY_HPP

#include "afl/data/value.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/spec/hull.hpp"
#include "game/spec/shiplist.hpp"

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

    /** Get hull property.
        @param h    Hull
        @param isp  Property index
        @param list Ship list
        @param config Host configuration
        @return newly-allocated property value; can be null */
    afl::data::Value* getHullProperty(const game::spec::Hull& h, HullProperty isp, const game::spec::ShipList& list, const game::config::HostConfiguration& config);

    /** Set hull property.
        @param h    Hull
        @param isp  Property index
        @param value New value; owned by caller
        @param list  Ship list, for change signalisation
        @throw interpreter::Error if property is not modifiable */
    void setHullProperty(game::spec::Hull& h, HullProperty isp, const afl::data::Value* value, game::spec::ShipList& list);

} }

#endif
