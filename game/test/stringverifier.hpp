/**
  *  \file game/test/stringverifier.hpp
  *  \brief Class game::test::StringVerifier
  */
#ifndef C2NG_GAME_TEST_STRINGVERIFIER_HPP
#define C2NG_GAME_TEST_STRINGVERIFIER_HPP

#include "game/stringverifier.hpp"

namespace game { namespace test {

    /** Test support: StringVerifier.
        This StringVerifier accepts all strings (up to 1000 characters). */
    class StringVerifier : public game::StringVerifier {
     public:
        virtual bool isValidString(Context ctx, const String_t& text);
        virtual bool isValidCharacter(Context ctx, afl::charset::Unichar_t ch);
        virtual size_t getMaxStringLength(Context ctx);
        virtual StringVerifier* clone() const;
    };

} }

#endif
