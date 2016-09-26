/**
  *  \file u/t_game_interface_userinterfacepropertyaccessor.cpp
  *  \brief Test for game::interface::UserInterfacePropertyAccessor
  */

#include "game/interface/userinterfacepropertyaccessor.hpp"

#include "t_game_interface.hpp"

/** Interface test. */
void
TestGameInterfaceUserInterfacePropertyAccessor::testIt()
{
    class Tester : public game::interface::UserInterfacePropertyAccessor {
     public:
        virtual bool get(game::interface::UserInterfaceProperty /*prop*/, std::auto_ptr<afl::data::Value>& /*result*/)
            { return false; }
        virtual bool set(game::interface::UserInterfaceProperty /*prop*/, afl::data::Value* /*p*/)
            { return false; }
    };
    Tester t;
}

