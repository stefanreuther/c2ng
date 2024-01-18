/**
  *  \file test/server/file/internalfileservertest.cpp
  *  \brief Test for server::file::InternalFileServer
  */

#include "server/file/internalfileserver.hpp"

#include "afl/test/testrunner.hpp"
#include "server/interface/baseclient.hpp"
#include "server/interface/filebaseclient.hpp"

/** Simple test. */
AFL_TEST("server.file.InternalFileServer", a)
{
    server::file::InternalFileServer testee;

    // Must work with FileBaseClient
    AFL_CHECK_SUCCEEDS(a("01. createDirectory"), server::interface::FileBaseClient(testee).createDirectory("x"));
    a.checkEqual("02. getFileInformation", server::interface::FileBaseClient(testee).getFileInformation("x").type, server::interface::FileBase::IsDirectory);

    // Must work with BaseClient
    a.checkEqual("11. ping", server::interface::BaseClient(testee).ping(), "PONG");
}
