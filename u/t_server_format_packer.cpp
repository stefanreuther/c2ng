/**
  *  \file u/t_server_format_packer.cpp
  *  \brief Test for server::format::Packer
  */

#include "server/format/packer.hpp"

#include "t_server_format.hpp"

/** Interface test. */
void
TestServerFormatPacker::testInterface()
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

