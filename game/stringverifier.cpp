/**
  *  \file game/stringverifier.cpp
  */

#include "game/stringverifier.hpp"
#include "afl/charset/utf8reader.hpp"

bool
game::StringVerifier::defaultIsValidString(Context ctx, const String_t& text)
{
    size_t length = getMaxStringLength(ctx);
    afl::charset::Utf8Reader rdr(afl::string::toBytes(text), 0);
    while (rdr.hasMore()) {
        // Check length
        if (length == 0) {
            return false;
        }
        --length;

        // Check character
        afl::charset::Unichar_t ch = rdr.eat();
        if (!isValidCharacter(ctx, ch)) {
            return false;
        }
    }
    return true;
}
