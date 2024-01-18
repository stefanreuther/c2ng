/**
  *  \file test/server/interface/formattest.cpp
  *  \brief Test for server::interface::Format
  */

#include "server/interface/format.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("server.interface.Format")
{
    class Tester : public server::interface::Format {
     public:
        virtual afl::data::Value* pack(String_t /*formatName*/, afl::data::Value* /*data*/, afl::base::Optional<String_t> /*format*/, afl::base::Optional<String_t> /*charset*/)
            { return 0; }
        virtual afl::data::Value* unpack(String_t /*formatName*/, afl::data::Value* /*data*/, afl::base::Optional<String_t> /*format*/, afl::base::Optional<String_t> /*charset*/)
            { return 0; }
    };
    Tester t;
}
