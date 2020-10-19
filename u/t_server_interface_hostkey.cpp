/**
  *  \file u/t_server_interface_hostkey.cpp
  *  \brief Test for server::interface::HostKey
  */

#include "server/interface/hostkey.hpp"

#include "t_server_interface.hpp"

/** Interface test. */
void
TestServerInterfaceHostKey::testInterface()
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

