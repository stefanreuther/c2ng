/**
  *  \file game/v3/stringverifier.hpp
  */
#ifndef C2NG_GAME_V3_STRINGVERIFIER_HPP
#define C2NG_GAME_V3_STRINGVERIFIER_HPP

#include <memory>
#include "game/stringverifier.hpp"
#include "afl/charset/charset.hpp"

namespace game { namespace v3 {

    class StringVerifier : public game::StringVerifier {
     public:
        StringVerifier(std::auto_ptr<afl::charset::Charset> cs);
        ~StringVerifier();
        virtual bool isValidString(Context ctx, const String_t& text);
        virtual bool isValidCharacter(Context ctx, afl::charset::Unichar_t ch);
        virtual size_t getMaxStringLength(Context ctx);
        virtual StringVerifier* clone() const;

     private:
        std::auto_ptr<afl::charset::Charset> m_charset;
    };

} }

#endif
