/**
  *  \file test/server/format/packertest.cpp
  *  \brief Test for server::format::Packer
  */

#include "server/format/packer.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("server.format.Packer")
{
    class Tester : public server::format::Packer {
     public:
        virtual String_t pack(afl::data::Value* /*data*/, afl::charset::Charset& /*cs*/)
            { return String_t(); }
        virtual afl::data::Value* unpack(const String_t& /*data*/, afl::charset::Charset& /*cs*/)
            { return 0; }
    };
    Tester t;
}
