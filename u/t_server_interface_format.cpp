/**
  *  \file u/t_server_interface_format.cpp
  *  \brief Test for server::interface::Format
  */

#include "server/interface/format.hpp"

#include "t_server_interface.hpp"

/** Interface test. */
void
TestServerInterfaceFormat::testIt()
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

