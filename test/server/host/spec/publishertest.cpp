/**
  *  \file test/server/host/spec/publishertest.cpp
  *  \brief Test for server::host::spec::Publisher
  */

#include "server/host/spec/publisher.hpp"

#include <stdexcept>
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("server.host.spec.Publisher")
{
    class Tester : public server::host::spec::Publisher {
     public:
        virtual afl::data::Hash::Ref_t getSpecificationData(String_t /*pathName*/, String_t /*flakPath*/, const afl::data::StringList_t& /*keys*/)
            { throw std::runtime_error("no ref"); }
    };
    Tester t;
}
