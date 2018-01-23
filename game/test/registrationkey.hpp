/**
  *  \file game/test/registrationkey.hpp
  *  \brief Class game::test::RegistrationKey
  */
#ifndef C2NG_GAME_TEST_REGISTRATIONKEY_HPP
#define C2NG_GAME_TEST_REGISTRATIONKEY_HPP

#include "game/registrationkey.hpp"

namespace game { namespace test {

    /** Test support: RegistrationKey.
        This key reports standard values, and refuses changes. */
    class RegistrationKey : public game::RegistrationKey {
     public:
        /** Constructor.
            \param status Status to report
            \param maxTech Tech limit to report */
        RegistrationKey(Status status, int maxTech);
        ~RegistrationKey();

        virtual Status getStatus() const;
        virtual String_t getLine(Line which) const;
        virtual bool setLine(Line which, String_t value);
        virtual int getMaxTechLevel(game::TechLevel area) const;

     private:
        Status m_status;
        int m_maxTech;
    };

} }

#endif
