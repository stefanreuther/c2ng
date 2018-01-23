/**
  *  \file game/test/stringverifier.cpp
  *  \brief Class game::test::StringVerifier
  */

#include "game/test/stringverifier.hpp"

bool
game::test::StringVerifier::isValidString(Context /*ctx*/, const String_t& /*text*/)
{
    return true;
}

bool
game::test::StringVerifier::isValidCharacter(Context /*ctx*/, afl::charset::Unichar_t /*ch*/)
{
    return true;
}

size_t
game::test::StringVerifier::getMaxStringLength(Context /*ctx*/)
{
    return 1000;
}

game::test::StringVerifier*
game::test::StringVerifier::clone() const
{
    return new StringVerifier();
}
