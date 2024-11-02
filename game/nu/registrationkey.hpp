/**
  *  \file game/nu/registrationkey.hpp
  *  \brief Class game::nu::RegistrationKey
  */
#ifndef C2NG_GAME_NU_REGISTRATIONKEY_HPP
#define C2NG_GAME_NU_REGISTRATIONKEY_HPP

#include "afl/data/access.hpp"
#include "game/registrationkey.hpp"

namespace game { namespace nu {

    /** RegistrationKey for Nu.
        Publishes the content of an "account" object. */
    class RegistrationKey : public game::RegistrationKey {
     public:
        /** Constructor.
            @param accountObject   "account" object (from /account/load endpoint) */
        explicit RegistrationKey(afl::data::Access accountObject);

        /** Destructor. */
        ~RegistrationKey();

        // RegistrationKey:
        virtual Status getStatus() const;
        virtual String_t getLine(Line which) const;
        virtual bool setLine(Line which, String_t value);
        virtual int getMaxTechLevel(TechLevel area) const;

     private:
        Status m_status;
        String_t m_line1;
        String_t m_line2;
    };

} }

#endif
