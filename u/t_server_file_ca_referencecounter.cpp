/**
  *  \file u/t_server_file_ca_referencecounter.cpp
  *  \brief Test for server::file::ca::ReferenceCounter
  */

#include "server/file/ca/referencecounter.hpp"

#include "t_server_file_ca.hpp"

/** Interface test. */
void
TestServerFileCaReferenceCounter::testInterface()
{
    class Tester : public server::file::ca::ReferenceCounter {
     public:
        virtual void set(const server::file::ca::ObjectId& /*id*/, int32_t /*value*/)
            { }
        virtual bool modify(const server::file::ca::ObjectId& /*id*/, int32_t /*delta*/, int32_t& /*result*/)
            { return false; }
    };
    Tester t;
}

