/**
  *  \file u/t_server_host_spec_publisher.cpp
  *  \brief Test for server::host::spec::Publisher
  */

#include "server/host/spec/publisher.hpp"

#include "t_server_host_spec.hpp"

/** Interface test. */
void
TestServerHostSpecPublisher::testInterface()
{
    class Tester : public server::host::spec::Publisher {
     public:
        virtual afl::data::Hash::Ref_t getSpecificationData(String_t /*pathName*/, String_t /*flakPath*/, const afl::data::StringList_t& /*keys*/)
            { throw std::runtime_error("no ref"); }
    };
    Tester t;
}

