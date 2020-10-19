/**
  *  \file u/t_server_interface_filegame.cpp
  *  \brief Test for server::interface::FileGame
  */

#include "server/interface/filegame.hpp"

#include "t_server_interface.hpp"

/** Interface test. */
void
TestServerInterfaceFileGame::testInterface()
{
    class Tester : public server::interface::FileGame {
     public:
        virtual void getGameInfo(String_t /*path*/, GameInfo& /*result*/)
            { }
        virtual void listGameInfo(String_t /*path*/, afl::container::PtrVector<GameInfo>& /*result*/)
            { }
        virtual void getKeyInfo(String_t /*path*/, KeyInfo& /*result*/)
            { }
        virtual void listKeyInfo(String_t /*path*/, const Filter& /*filter*/, afl::container::PtrVector<KeyInfo>& /*result*/)
            { }
    };
    Tester t;
}

