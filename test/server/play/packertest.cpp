/**
  *  \file test/server/play/packertest.cpp
  *  \brief Test for server::play::Packer
  */

#include "server/play/packer.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("server.play.Packer")
{
    class Tester : public server::play::Packer {
     public:
        virtual server::Value_t* buildValue() const
            { return 0; }
        virtual String_t getName() const
            { return String_t(); }
    };
    Tester t;
}
