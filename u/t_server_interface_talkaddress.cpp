/**
  *  \file u/t_server_interface_talkaddress.cpp
  *  \brief Test for server::interface::TalkAddress
  */

#include "server/interface/talkaddress.hpp"

#include "t_server_interface.hpp"

/** Interface test. */
void
TestServerInterfaceTalkAddress::testInterface()
{
    class Tester : public server::interface::TalkAddress {
     public:
        virtual void parse(afl::base::Memory<const String_t> /*in*/, afl::data::StringList_t& /*out*/)
            { }
        virtual void render(afl::base::Memory<const String_t> /*in*/, afl::data::StringList_t& /*out*/)
            { }
    };
    Tester t;
}

