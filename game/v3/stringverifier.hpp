/**
  *  \file game/v3/stringverifier.hpp
  *  \brief Class game::v3::StringVerifier
  */
#ifndef C2NG_GAME_V3_STRINGVERIFIER_HPP
#define C2NG_GAME_V3_STRINGVERIFIER_HPP

#include <memory>
#include "game/stringverifier.hpp"
#include "afl/charset/charset.hpp"

namespace game { namespace v3 {

    /** Implementation of game::StringVerifier for v3.

        v3 rules:
        - specific maximum lengths
        - characters must be part of game character set
        - for messages, characters must be compatible with rot13 encoding
          (encoded character >242 crashes planets.exe).
        - friendly codes allow US-ASCII only */
    class StringVerifier : public game::StringVerifier {
     public:
        /** Constructor.
            \param cs Game character set. Must not be null. */
        StringVerifier(std::auto_ptr<afl::charset::Charset> cs);
        ~StringVerifier();

        virtual bool isValidString(Context ctx, const String_t& text) const;
        virtual bool isValidCharacter(Context ctx, afl::charset::Unichar_t ch) const;
        virtual size_t getMaxStringLength(Context ctx) const;
        virtual StringVerifier* clone() const;

     private:
        std::auto_ptr<afl::charset::Charset> m_charset;
    };

} }

#endif
