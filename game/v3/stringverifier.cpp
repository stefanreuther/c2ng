/**
  *  \file game/v3/stringverifier.cpp
  *  \brief Class game::v3::StringVerifier
  */

#include "game/v3/stringverifier.hpp"
#include "afl/charset/utf8.hpp"

game::v3::StringVerifier::StringVerifier(std::auto_ptr<afl::charset::Charset> cs)
    : m_charset(cs)
{ }

game::v3::StringVerifier::~StringVerifier()
{ }

bool
game::v3::StringVerifier::isValidString(Context ctx, const String_t& text) const
{
    return defaultIsValidString(ctx, text);
}

bool
game::v3::StringVerifier::isValidCharacter(Context ctx, afl::charset::Unichar_t ch) const
{
    switch (ctx) {
     case Unknown:
     case ShipName:
     case PlanetName:
     case PlayerLongName:
     case PlayerShortName:
     case PlayerAdjectiveName:
        // Encode in character set. Result must be 8-bit number.
     {
         String_t utf;
         afl::charset::Utf8().append(utf, ch);
         afl::base::GrowableBytes_t encoded = m_charset->encode(afl::string::toMemory(utf));
         String_t decoded = m_charset->decode(encoded);
         return encoded.size() == 1
             && utf == decoded;
     }

     case FriendlyCode:
        // Friendly codes allow printable 7-bit ASCII
        return (ch >= 0x20 && ch < 0x7F);

     case Message:
     {
         String_t utf;
         afl::charset::Utf8().append(utf, ch);
         afl::base::GrowableBytes_t encoded = m_charset->encode(afl::string::toMemory(utf));
         String_t decoded = m_charset->decode(encoded);
         return encoded.size() == 1
             && utf == decoded
             && *encoded.at(0) < 0x100 - 13;
     }
    }
    return false;
}

size_t
game::v3::StringVerifier::getMaxStringLength(Context ctx) const
{
    switch (ctx) {
     case Unknown:
        return 1000;
     case ShipName:
     case PlanetName:
        return 20;
     case PlayerLongName:
        return 30;
     case PlayerShortName:
        return 20;
     case PlayerAdjectiveName:
        return 12;
     case FriendlyCode:
        return 3;
     case Message:
        return 1000;
    }
    return 0;
}

game::v3::StringVerifier*
game::v3::StringVerifier::clone() const
{
    return new StringVerifier(std::auto_ptr<afl::charset::Charset>(m_charset->clone()));
}
