/**
  *  \file game/nu/stringverifier.cpp
  */

#include "game/nu/stringverifier.hpp"

            // Unknown,
            // ShipName,
            // PlanetName,
            // PlayerLongName,
            // PlayerShortName,
            // PlayerAdjectiveName,
            // FriendlyCode,
            // Message

namespace {
    /*
     *  Character escaping seems to be a lowlight of planets.nu.
     *
     *  (a) everything that is posted through the turn file interface will have substitutions
     *  '&' -> '|||', '=' -> ':::', making '&', '|', '=', ':' unsafe to use.
     *
     *  (b) some user input (ship names, fcodes) goes through a "cleanUserInput" function that
     *      - parses the input as HTML
     *      - replaces "&" by "and" in the remainder
     *  This makes '<', '>', '&' unsafe to use.
     */
    const char* getBlacklist(game::StringVerifier::Context ctx)
    {
        if (ctx == game::StringVerifier::Message) {
            return "<>&";
        } else {
            return "|=:<>&";
        }
    }
}


game::nu::StringVerifier::StringVerifier()
{ }

game::nu::StringVerifier::~StringVerifier()
{ }

bool
game::nu::StringVerifier::isValidString(Context ctx, const String_t& text)
{
    return text.find_first_of(getBlacklist(ctx)) == String_t::npos;
}

bool
game::nu::StringVerifier::isValidCharacter(Context ctx, afl::charset::Unichar_t ch)
{
    return !(ch > 0 && ch < 127 && std::strchr(getBlacklist(ctx), char(ch)) != 0);
}

size_t
game::nu::StringVerifier::getMaxStringLength(Context ctx)
{
    switch (ctx) {
     case Unknown:
        return 1000;
     case ShipName:
        return 50;              // taken from web interface
     case PlanetName:
        return 50;              // analogous to ship name
     case PlayerLongName:
        return 30;              // NO SOURCE. Taking v3 value.
     case PlayerShortName:
        return 20;              // NO SOURCE. Taking v3 value.
     case PlayerAdjectiveName:
        return 12;              // NO SOURCE. Taking v3 value.
     case FriendlyCode:
        return 3;               // taken from web interface
     case Message:
        return 1000;            // NO SOURCE.
    }
    return 1000;
}

game::nu::StringVerifier*
game::nu::StringVerifier::clone() const
{
    return new StringVerifier();
}
