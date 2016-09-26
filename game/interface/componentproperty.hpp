/**
  *  \file game/interface/componentproperty.hpp
  */
#ifndef C2NG_GAME_INTERFACE_COMPONENTPROPERTY_HPP
#define C2NG_GAME_INTERFACE_COMPONENTPROPERTY_HPP

#include "afl/data/value.hpp"
#include "game/spec/component.hpp"
#include "game/spec/shiplist.hpp"

namespace game { namespace interface {

    /** Property for a game::spec::Component. */
    enum ComponentProperty {
        icpMass,
        icpTech,
        icpCostT,
        icpCostD,
        icpCostM,
        icpCostMC,
        icpCostSup,
        icpCostStr,
        icpName,
        icpNameShort,
        icpId
    };

    afl::data::Value* getComponentProperty(const game::spec::Component& comp,
                                           ComponentProperty icp,
                                           const game::spec::ShipList& list);

    void setComponentProperty(game::spec::Component& comp,
                              ComponentProperty icp,
                              afl::data::Value* value,
                              game::spec::ShipList& list);
    
} }

#endif
