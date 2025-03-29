/**
  *  \file game/element.cpp
  *  \brief Class game::Element
  */

#include "game/element.hpp"
#include "game/spec/torpedo.hpp"
#include "game/spec/shiplist.hpp"

game::Element::Type
game::Element::fromTorpedoType(int torpedoType)
{
    // ex getCargoTypeFromTorpType
    return Type(FirstTorpedo + (torpedoType - 1));
}

bool
game::Element::isTorpedoType(Type t, int& torpedoType)
{
    // ex getTorpTypeFromCargoType, isCargoTorpedo
    if (t >= FirstTorpedo) {
        torpedoType = (t - FirstTorpedo) + 1;
        return true;
    } else {
        return false;
    }
}

// Get name of an element type.
String_t
game::Element::getName(Type t, afl::string::Translator& tx, const game::spec::ShipList& shipList)
{
    // ex getCargoName, transfer.pas:GoodName
    switch (t) {
     case Neutronium:
        return tx("Neutronium");
     case Tritanium:
        return tx("Tritanium");
     case Duranium:
        return tx("Duranium");
     case Molybdenum:
        return tx("Molybdenum");
     case Fighters:
        return tx("Fighters");
     case Colonists:
        return tx("Colonists");
     case Supplies:
        return tx("Supplies");
     case Money:
        return tx("Money");
     default:
        int torpedoType;
        if (isTorpedoType(t, torpedoType)) {
            if (const game::spec::TorpedoLauncher* torp = shipList.launchers().get(torpedoType)) {
                return torp->getName(shipList.componentNamer());
            }
        }
        return String_t();
    }
}

// Get unit of an element type.
String_t
game::Element::getUnit(Type t, afl::string::Translator& tx, const game::spec::ShipList& /*shipList*/)
{
    // ex getCargoUnit, transfer.pas:GoodUnit
    switch(t) {
     case Neutronium:
     case Tritanium:
     case Duranium:
     case Molybdenum:
     case Supplies:
        return tx("kt");
     case Colonists:
        return tx("clans");
     case Money:
        return tx("mc");
     case Fighters:
     default:
        return String_t();
    }
}

game::Element::Type
game::Element::end(const game::spec::ShipList& shipList)
{
    return fromTorpedoType(shipList.launchers().size() + 1);
}
