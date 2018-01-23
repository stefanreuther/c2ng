/**
  *  \file game/registrationkey.hpp
  */
#ifndef C2NG_GAME_REGISTRATIONKEY_HPP
#define C2NG_GAME_REGISTRATIONKEY_HPP

#include "afl/base/deletable.hpp"
#include "afl/string/string.hpp"
#include "game/types.hpp"

namespace game {

    class RegistrationKey : public afl::base::Deletable {
     public:
        enum Status {
            Unknown,
            Unregistered,
            Registered
        };
        enum Line {
            Line1,
            Line2,
            Line3,
            Line4
        };

        virtual Status getStatus() const = 0;

        virtual String_t getLine(Line which) const = 0;

        virtual bool setLine(Line which, String_t value) = 0;

        virtual int getMaxTechLevel(TechLevel area) const = 0;
    };

}

#endif
