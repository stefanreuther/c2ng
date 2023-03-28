/**
  *  \file game/interface/engineproperty.hpp
  *  \brief Engine Properties
  */
#ifndef C2NG_GAME_INTERFACE_ENGINEPROPERTY_HPP
#define C2NG_GAME_INTERFACE_ENGINEPROPERTY_HPP

#include "game/spec/engine.hpp"
#include "game/spec/shiplist.hpp"
#include "afl/data/value.hpp"

namespace game { namespace interface {

    /** Property for a Engine. */
    enum EngineProperty {
        iepEfficientWarp,
        iepFuelFactor
    };

    /** Get engine property.
        @param e   Engine
        @param iep Property to query
        @return newly-allocated value */
    afl::data::Value* getEngineProperty(const game::spec::Engine& e, EngineProperty iep);

    /** Set engine property.
        @param e   Engine
        @param iep Property to modify
        @param value Value
        @param list  Ship list (for change notification)
        @throw interpreter::Error if property is not assignable or value is invalid */
    void setEngineProperty(game::spec::Engine& e, EngineProperty iep, const afl::data::Value* value, game::spec::ShipList& list);

} }

#endif
