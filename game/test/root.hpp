/**
  *  \file game/test/root.hpp
  *  \brief Class game::test::Root
  */
#ifndef C2NG_GAME_TEST_ROOT_HPP
#define C2NG_GAME_TEST_ROOT_HPP

#include "game/root.hpp"
#include "game/hostversion.hpp"
#include "game/registrationkey.hpp"

namespace game { namespace test {

    /** Test support: Root.
        A Root, with all components instantiated from the test versions. */
    class Root : public game::Root {
     public:
        /** Constructor.
            \param host Host version
            \param status Registration key status
            \param maxTech Maximum tech level */
        Root(HostVersion host,
             RegistrationKey::Status status = RegistrationKey::Unknown,
             int maxTech = 6);
    };

} }

#endif
