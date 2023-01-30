/**
  *  \file game/test/root.hpp
  *  \brief Test Root
  */
#ifndef C2NG_GAME_TEST_ROOT_HPP
#define C2NG_GAME_TEST_ROOT_HPP

#include "afl/base/ref.hpp"
#include "game/hostversion.hpp"
#include "game/registrationkey.hpp"
#include "game/root.hpp"

namespace game { namespace test {

    /** Make a Root for testing.

        This used to be a class; however, Root has no virtual destructor,
        so deriving from it and deleting it through the base-class pointer violates C++.

        @param host    Host version
        @param status  Registration key status
        @param maxTech Maximum tech level
        @return Newly-allocated Root */
    afl::base::Ref<Root> makeRoot(HostVersion host, RegistrationKey::Status status = RegistrationKey::Unknown, int maxTech = 6);

} }

#endif
