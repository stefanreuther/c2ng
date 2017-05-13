/**
  *  \file u/t_server_interface_base.cpp
  *  \brief Test for server::interface::Base
  */

#include "server/interface/base.hpp"

#include "t_server_interface.hpp"

/** Interface test. */
void
TestServerInterfaceBase::testInterface()
{
    class Tester : public server::interface::Base {
     public:
        virtual String_t ping()
            { return String_t(); }
        virtual void setUserContext(String_t /*user*/)
            { }
    };
    Tester t;
}

