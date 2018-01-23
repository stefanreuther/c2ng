/**
  *  \file u/t_server_file_internalfileserver.cpp
  *  \brief Test for server::file::InternalFileServer
  */

#include "server/file/internalfileserver.hpp"

#include "t_server_file.hpp"
#include "server/interface/filebaseclient.hpp"
#include "server/interface/baseclient.hpp"

/** Simple test. */
void
TestServerFileInternalFileServer::testIt()
{
    server::file::InternalFileServer testee;

    // Must work with FileBaseClient
    TS_ASSERT_THROWS_NOTHING(server::interface::FileBaseClient(testee).createDirectory("x"));
    TS_ASSERT_EQUALS(server::interface::FileBaseClient(testee).getFileInformation("x").type, server::interface::FileBase::IsDirectory);

    // Must work with BaseClient
    TS_ASSERT_EQUALS(server::interface::BaseClient(testee).ping(), "PONG");
}

