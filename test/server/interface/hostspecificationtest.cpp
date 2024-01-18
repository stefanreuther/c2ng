/**
  *  \file test/server/interface/hostspecificationtest.cpp
  *  \brief Test for server::interface::HostSpecification
  */

#include "server/interface/hostspecification.hpp"
#include "afl/test/testrunner.hpp"

using server::interface::HostSpecification;

/** Interface test. */
AFL_TEST_NOARG("server.interface.HostSpecification:interface")
{
    class Tester : public HostSpecification {
     public:
        virtual afl::data::Value* getShiplistData(String_t /*shiplistId*/, Format /*format*/, const afl::data::StringList_t& /*keys*/)
            { return 0; }
        virtual afl::data::Value* getGameData(int32_t /*gameId*/, Format /*format*/, const afl::data::StringList_t& /*keys*/)
            { return 0; }
    };
    Tester t;
}

/** Test formatFormat(). */
AFL_TEST("server.interface.HostSpecification:formatFormat", a)
{
    a.checkEqual("01", HostSpecification::formatFormat(HostSpecification::JsonString), "json");
    a.checkEqual("02", HostSpecification::formatFormat(HostSpecification::Direct), "direct");
}

/** Test parseFormat(). */
AFL_TEST("server.interface.HostSpecification:parseFormat", a)
{
    a.checkEqual("01", HostSpecification::parseFormat("json").isValid(), true);
    a.checkEqual("02", *HostSpecification::parseFormat("json").get(), HostSpecification::JsonString);
    a.checkEqual("03", HostSpecification::parseFormat("direct").isValid(), true);
    a.checkEqual("04", *HostSpecification::parseFormat("direct").get(), HostSpecification::Direct);

    a.checkEqual("11", HostSpecification::parseFormat("").isValid(), false);
    a.checkEqual("12", HostSpecification::parseFormat("JSON").isValid(), false);
}
