/**
  *  \file game/nu/registrationkey.hpp
  */
#ifndef C2NG_GAME_NU_REGISTRATIONKEY_HPP
#define C2NG_GAME_NU_REGISTRATIONKEY_HPP

#include "game/registrationkey.hpp"
#include "afl/data/access.hpp"

namespace game { namespace nu {

    class RegistrationKey : public game::RegistrationKey {
     public:
        RegistrationKey(afl::data::Access playerObject);
        ~RegistrationKey();

        virtual Status getStatus() const;
        virtual String_t getLine(Line which) const;
        virtual bool setLine(Line which, String_t value);
        virtual int getMaxTechLevel(TechLevel area) const;

        void setStatus(Status st);

     private:
        Status m_status;
        String_t m_line1;
        String_t m_line2;
    };

} }

#endif
