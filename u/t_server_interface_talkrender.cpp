/**
  *  \file u/t_server_interface_talkrender.cpp
  *  \brief Test for server::interface::TalkRender
  */

#include "server/interface/talkrender.hpp"

#include "t_server_interface.hpp"

/** Interface test. */
void
TestServerInterfaceTalkRender::testIt()
{
    class Tester : public server::interface::TalkRender {
     public:
        virtual void setOptions(const Options& /*opts*/)
            { }
        virtual String_t render(const String_t& /*text*/, const Options& /*opts*/)
            { return String_t(); }
    };
    Tester t;
}

