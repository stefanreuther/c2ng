/**
  *  \file u/t_server_file_ca_referenceupdater.cpp
  *  \brief Test for server::file::ca::ReferenceUpdater
  */

#include "server/file/ca/referenceupdater.hpp"

#include "t_server_file_ca.hpp"

/** Interface test. */
void
TestServerFileCaReferenceUpdater::testInterface()
{
    class Tester : public server::file::ca::ReferenceUpdater {
     public:
        virtual void updateDirectoryReference(const String_t& /*name*/, const server::file::ca::ObjectId& /*newId*/)
            { }
    };
    Tester t;
}

