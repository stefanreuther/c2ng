/**
  *  \file test/server/interface/talksyntaxtest.cpp
  *  \brief Test for server::interface::TalkSyntax
  */

#include "server/interface/talksyntax.hpp"

#include "afl/test/testrunner.hpp"
#include <stdexcept>

/** Interface test. */
AFL_TEST_NOARG("server.interface.TalkSyntax")
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
