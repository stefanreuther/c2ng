/**
  *  \file u/t_server_interface_talksyntax.cpp
  *  \brief Test for server::interface::TalkSyntax
  */

#include "server/interface/talksyntax.hpp"

#include "t_server_interface.hpp"

/** Interface test. */
void
TestServerInterfaceTalkSyntax::testInterface()
{
    class Tester : public server::interface::TalkSyntax {
     public:
        virtual String_t get(String_t /*key*/)
            { return String_t(); }
        virtual afl::base::Ref<afl::data::Vector> mget(afl::base::Memory<const String_t> /*keys*/)
            { throw std::runtime_error("no ref"); }
    };
    Tester t;
}

