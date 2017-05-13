/**
  *  \file u/t_server_file_item.cpp
  *  \brief Test for server::file::Item
  */

#include "server/file/item.hpp"

#include "t_server_file.hpp"

/** Test the base class. */
void
TestServerFileItem::testInterface()
{
    server::file::Item testee("xyz");
    TS_ASSERT_EQUALS(testee.getName(), "xyz");
}
