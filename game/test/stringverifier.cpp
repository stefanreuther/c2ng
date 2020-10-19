/**
  *  \file game/test/stringverifier.cpp
  *  \brief Class game::test::StringVerifier
  */

#include "game/test/stringverifier.hpp"

bool
game::test::StringVerifier::isValidString(Context /*ctx*/, const String_t& /*text*/) const
{
    return true;
}

bool
game::test::StringVerifier::isValidCharacter(Context /*ctx*/, afl::charset::Unichar_t /*ch*/) const
{
    return true;
}

size_t
game::test::StringVerifier::getMaxStringLength(Context /*ctx*/) const
{
    return 1000;
}

game::test::StringVerifier*
game::test::StringVerifier::clone() const
{
    return new StringVerifier();
}
