/**
  *  \file test/server/file/ca/internalreferencecountertest.cpp
  *  \brief Test for server::file::ca::InternalReferenceCounter
  */

#include "server/file/ca/internalreferencecounter.hpp"
#include "afl/test/testrunner.hpp"

/** Simple test. */
AFL_TEST("server.file.ca.InternalReferenceCounter", a)
{
    using server::file::ca::ObjectId;

    server::file::ca::InternalReferenceCounter testee;

    ObjectId id = ObjectId::fromHex("12345");

    // Initially empty
    int32_t v;
    a.check("01", !testee.modify(id, +1, v));
    a.check("02", !testee.modify(id, -1, v));

    // Set it
    testee.set(id, 1);
    a.check("11", testee.modify(id, 0, v));
    a.checkEqual("12", v, 1);
    a.check("13", testee.modify(id, 2, v));
    a.checkEqual("14", v, 3);
    a.check("15", testee.modify(id, -3, v));
    a.checkEqual("16", v, 0);

    // It's now zero, and should no longer be modifiable
    a.check("21", !testee.modify(id, 1, v));

    // Set it again
    testee.set(id, 1);
    a.check("31", testee.modify(id, 0, v));
    a.checkEqual("32", v, 1);
}
