/**
  *  \file test/server/file/itemtest.cpp
  *  \brief Test for server::file::Item
  */

#include "server/file/item.hpp"
#include "afl/test/testrunner.hpp"

/** Test the base class. */
AFL_TEST("server.file.Item", a)
{
    server::file::Item testee("xyz");
    a.checkEqual("01. getName", testee.getName(), "xyz");
}
