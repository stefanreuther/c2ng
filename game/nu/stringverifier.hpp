/**
  *  \file game/nu/stringverifier.hpp
  */
#ifndef C2NG_GAME_NU_STRINGVERIFIER_HPP
#define C2NG_GAME_NU_STRINGVERIFIER_HPP

#include "game/stringverifier.hpp"

namespace game { namespace nu {

    class StringVerifier : public game::StringVerifier {
     public:
        StringVerifier();
        ~StringVerifier();
        virtual bool isValidString(Context ctx, const String_t& text);
        virtual bool isValidCharacter(Context ctx, afl::charset::Unichar_t ch);
        virtual size_t getMaxStringLength(Context ctx);

        virtual StringVerifier* clone() const;
    };

} }

#endif
