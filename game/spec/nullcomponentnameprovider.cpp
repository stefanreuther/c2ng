/**
  *  \file game/spec/nullcomponentnameprovider.cpp
  *  \brief Class game::spec::NullComponentNameProvider
  */

#include "game/spec/nullcomponentnameprovider.hpp"

String_t
game::spec::NullComponentNameProvider::getName(Type /*type*/, int /*index*/, const String_t& name) const
{
    return name;
}

String_t
game::spec::NullComponentNameProvider::getShortName(Type /*type*/, int /*index*/, const String_t& name, const String_t& shortName) const
{
    return shortName.empty() ? name : shortName;
}
