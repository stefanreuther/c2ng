/**
  *  \file test/server/interface/hostkeytest.cpp
  *  \brief Test for server::interface::HostKey
  */

#include "server/interface/hostkey.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("server.interface.HostKey")
{
    class Tester : public server::interface::HostKey {
     public:
        virtual void listKeys(Infos_t& /*out*/)
            { }
        virtual String_t getKey(String_t /*keyId*/)
            { return String_t(); }
    };
    Tester t;
}
