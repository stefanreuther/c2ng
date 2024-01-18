/**
  *  \file test/game/interface/userinterfacepropertyaccessortest.cpp
  *  \brief Test for game::interface::UserInterfacePropertyAccessor
  */

#include "game/interface/userinterfacepropertyaccessor.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("game.interface.UserInterfacePropertyAccessor")
{
    class Tester : public game::interface::UserInterfacePropertyAccessor {
     public:
        virtual bool get(game::interface::UserInterfaceProperty /*prop*/, std::auto_ptr<afl::data::Value>& /*result*/)
            { return false; }
        virtual bool set(game::interface::UserInterfaceProperty /*prop*/, const afl::data::Value* /*p*/)
            { return false; }
    };
    Tester t;
}
