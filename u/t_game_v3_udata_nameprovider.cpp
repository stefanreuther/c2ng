/**
  *  \file u/t_game_v3_udata_nameprovider.cpp
  *  \brief Test for game::v3::udata::NameProvider
  */

#include "game/v3/udata/nameprovider.hpp"

#include "t_game_v3_udata.hpp"

/** Interface test. */
void
TestGameV3UdataNameProvider::testInterface()
{
    class Tester : public game::v3::udata::NameProvider {
     public:
        virtual String_t getName(Type /*type*/, int /*id*/) const
            { return String_t(); }
    };
    Tester t;
}

