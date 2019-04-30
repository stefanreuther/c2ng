/**
  *  \file u/t_server_play_packer.cpp
  *  \brief Test for server::play::Packer
  */

#include "server/play/packer.hpp"

#include "t_server_play.hpp"

/** Interface test. */
void
TestServerPlayPacker::testInterface()
{
    class Tester : public server::play::Packer {
     public:
        virtual server::Value_t* buildValue() const
            { return 0; }
        virtual String_t getName() const
            { return String_t(); }
    };
    Tester t;
}

