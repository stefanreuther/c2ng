/**
  *  \file game/interface/componentproperty.cpp
  *  \brief Enum game::interface::ComponentProperty
  */

#include "game/interface/componentproperty.hpp"
#include "interpreter/values.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"

using interpreter::makeIntegerValue;
using interpreter::makeStringValue;
using interpreter::checkStringArg;

afl::data::Value*
game::interface::getComponentProperty(const game::spec::Component& comp,
                                      ComponentProperty icp,
                                      const game::spec::ShipList& list)
{
    // ex int/if/specif.h:getComponentProperty
    switch (icp) {
     case icpMass:
        /* @q Mass:Int (Beam Property, Torpedo Property)
           Mass of this component, in kt. */
        return makeIntegerValue(comp.getMass());
     case icpTech:
        /* @q Tech:Int (Hull Property, Engine Property, Beam Property, Torpedo Property)
           @q Tech.Engine:Int (Engine Property)
           @q Tech.Beam:Int (Beam Property)
           @q Tech.Torpedo:Int (Torpedo Property)
           Tech level of this component. */
        /* @q Tech.Hull:Int (Hull Property, Ship Property)
           Hull tech level. */
        return makeIntegerValue(comp.getTechLevel());
     case icpCostT:
        /* @q Cost.T:Int (Hull Property, Engine Property, Beam Property, Torpedo Property)
           Tritanium cost of this component. */
        return makeIntegerValue(comp.cost().get(game::spec::Cost::Tritanium));
     case icpCostD:
        /* @q Cost.D:Int (Hull Property, Engine Property, Beam Property, Torpedo Property)
           Duranium cost of this component. */
        return makeIntegerValue(comp.cost().get(game::spec::Cost::Duranium));
     case icpCostM:
        /* @q Cost.M:Int (Hull Property, Engine Property, Beam Property, Torpedo Property)
           Molybdenum cost of this component. */
        return makeIntegerValue(comp.cost().get(game::spec::Cost::Molybdenum));
     case icpCostMC:
        /* @q Cost.MC:Int (Hull Property, Engine Property, Beam Property, Torpedo Property)
           Money cost of this component. */
        return makeIntegerValue(comp.cost().get(game::spec::Cost::Money));
     case icpCostSup:
        // FIXME: not referenced!
        return makeIntegerValue(comp.cost().get(game::spec::Cost::Supplies));
     case icpCostStr:
        /* @q Cost.Str:Cargo (Hull Property, Engine Property, Beam Property, Torpedo Property)
           Cost of this component, as a string. */
        return makeStringValue(comp.cost().toCargoSpecString());
     case icpName:
        /* @q Name:Str (Hull Property, Engine Property, Beam Property, Torpedo Property)
           Name of this component.
           @assignable */
        /* @q Hull:Str (Ship Property)
           Name of the ship's hull. */
        return makeStringValue(comp.getName(list.componentNamer()));
     case icpNameShort:
        /* @q Name.Short:Str (Hull Property, Engine Property, Beam Property, Torpedo Property)
           Short name of this component.
           @assignable */
        /* @q Hull.Short:Str (Ship Property)
           Short name of the hull. */
        return makeStringValue(comp.getShortName(list.componentNamer()));
     case icpDescription:
        /* @q Description:Str (Hull Property, Engine Property, Beam Property, Torpedo Property)
           Description,
           @assignable
           @since PCC2 2.41.2 */
        return makeStringValue(comp.getDescription());
     case icpId:
        /* @q Id:Str (Hull Property, Engine Property, Beam Property, Torpedo Property)
           Component Id. */
        /* @q Hull$:Str (Ship Property)
           Hull Id. */
        return makeIntegerValue(comp.getId());
    }
    return 0;
}

void
game::interface::setComponentProperty(game::spec::Component& comp,
                                      ComponentProperty icp,
                                      const afl::data::Value* value,
                                      game::spec::ShipList& list)
{
    String_t n;
    switch (icp) {
     case icpName:
        if (checkStringArg(n, value)) {
            comp.setName(n);
            list.sig_change.raise();
        }
        break;
     case icpNameShort:
        if (checkStringArg(n, value)) {
            comp.setShortName(n);
            list.sig_change.raise();
        }
        break;
     case icpDescription:
        if (checkStringArg(n, value)) {
            comp.setDescription(n);
            list.sig_change.raise();
        }
        break;
     default:
        throw interpreter::Error::notAssignable();
    }
}
