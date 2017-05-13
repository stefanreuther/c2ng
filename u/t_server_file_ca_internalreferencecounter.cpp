/**
  *  \file u/t_server_file_ca_internalreferencecounter.cpp
  *  \brief Test for server::file::ca::InternalReferenceCounter
  */

#include "server/file/ca/internalreferencecounter.hpp"

#include "t_server_file_ca.hpp"

/** Simple test. */
void
TestServerFileCaInternalReferenceCounter::testIt()
{
    using server::file::ca::ObjectId;

    server::file::ca::InternalReferenceCounter testee;

    ObjectId id = ObjectId::fromHex("12345");

    // Initially empty
    int32_t v;
    TS_ASSERT(!testee.modify(id, +1, v));
    TS_ASSERT(!testee.modify(id, -1, v));

    // Set it
    testee.set(id, 1);
    TS_ASSERT(testee.modify(id, 0, v));
    TS_ASSERT_EQUALS(v, 1);
    TS_ASSERT(testee.modify(id, 2, v));
    TS_ASSERT_EQUALS(v, 3);
    TS_ASSERT(testee.modify(id, -3, v));
    TS_ASSERT_EQUALS(v, 0);

    // It's now zero, and should no longer be modifiable
    TS_ASSERT(!testee.modify(id, 1, v));

    // Set it again
    testee.set(id, 1);
    TS_ASSERT(testee.modify(id, 0, v));
    TS_ASSERT_EQUALS(v, 1);
}
