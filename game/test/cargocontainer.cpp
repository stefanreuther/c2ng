/**
  *  \file game/test/cargocontainer.cpp
  */

#include "game/test/cargocontainer.hpp"

String_t
game::test::CargoContainer::getName(afl::string::Translator& /*tx*/) const
{
    return "<Test>";
}

String_t
game::test::CargoContainer::getInfo1(afl::string::Translator& /*tx*/) const
{
    return String_t();
}

String_t
game::test::CargoContainer::getInfo2(afl::string::Translator& /*tx*/) const
{
    return String_t();
}

game::CargoContainer::Flags_t
game::test::CargoContainer::getFlags() const
{
    return Flags_t();
}

bool
game::test::CargoContainer::canHaveElement(game::Element::Type /*type*/) const
{
    return true;
}

int32_t
game::test::CargoContainer::getMaxAmount(game::Element::Type /*type*/) const
{
    return 10000;
}

int32_t
game::test::CargoContainer::getMinAmount(game::Element::Type /*type*/) const
{
    return 0;
}

int32_t
game::test::CargoContainer::getAmount(game::Element::Type /*type*/) const
{
    return 5000;
}

void
game::test::CargoContainer::commit()
{ }
