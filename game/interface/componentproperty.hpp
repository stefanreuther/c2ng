/**
  *  \file game/interface/componentproperty.hpp
  *  \brief Enum game::interface::ComponentProperty
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
        icpDescription,
        icpId
    };

    /** Get property of a component.
        \param comp Component to query
        \param icp  Property to query
        \param list Ship list
        \return Newly-allocated value */
    afl::data::Value* getComponentProperty(const game::spec::Component& comp,
                                           ComponentProperty icp,
                                           const game::spec::ShipList& list);

    /** Set component property.
        \param comp  Component
        \param icp   Property
        \param value Value, owned by caller
        \param list  Ship list (for change signalisation)
        \throw interpreter::Error if property is not modifiable */
    void setComponentProperty(game::spec::Component& comp,
                              ComponentProperty icp,
                              const afl::data::Value* value,
                              game::spec::ShipList& list);

} }

#endif
