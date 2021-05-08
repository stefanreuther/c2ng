/**
  *  \file game/interface/engineproperty.hpp
  */
#ifndef C2NG_GAME_INTERFACE_ENGINEPROPERTY_HPP
#define C2NG_GAME_INTERFACE_ENGINEPROPERTY_HPP

#include "game/spec/engine.hpp"
#include "game/spec/shiplist.hpp"
#include "afl/data/value.hpp"

namespace game { namespace interface {

    // /** Property for a Engine. */
    enum EngineProperty {
        iepEfficientWarp,
        iepFuelFactor
    };

    afl::data::Value* getEngineProperty(const game::spec::Engine& e,
                                        EngineProperty iep);

    void setEngineProperty(game::spec::Engine& e,
                           EngineProperty iep,
                           const afl::data::Value* value,
                           game::spec::ShipList& list);

} }

#endif
