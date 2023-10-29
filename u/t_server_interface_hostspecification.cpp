/**
  *  \file u/t_server_interface_hostspecification.cpp
  *  \brief Test for server::interface::HostSpecification
  */

#include "server/interface/hostspecification.hpp"

#include "t_server_interface.hpp"

/** Interface test. */
void
TestServerInterfaceHostSpecification::testInterface()
{
    class Tester : public server::interface::HostSpecification {
     public:
        virtual afl::data::Value* getShiplistData(String_t /*shiplistId*/, Format /*format*/, const afl::data::StringList_t& /*keys*/)
            { return 0; }
        virtual afl::data::Value* getGameData(int32_t /*gameId*/, Format /*format*/, const afl::data::StringList_t& /*keys*/)
            { return 0; }
    };
    Tester t;
}

/** Test formatFormat(). */
void
TestServerInterfaceHostSpecification::testFormat()
{
    using server::interface::HostSpecification;
    TS_ASSERT_EQUALS(HostSpecification::formatFormat(HostSpecification::JsonString), "json");
    TS_ASSERT_EQUALS(HostSpecification::formatFormat(HostSpecification::Direct), "direct");
}

/** Test parseFormat(). */
void
TestServerInterfaceHostSpecification::testParse()
{
    using server::interface::HostSpecification;
    TS_ASSERT_EQUALS(HostSpecification::parseFormat("json").isValid(), true);
    TS_ASSERT_EQUALS(*HostSpecification::parseFormat("json").get(), HostSpecification::JsonString);
    TS_ASSERT_EQUALS(HostSpecification::parseFormat("direct").isValid(), true);
    TS_ASSERT_EQUALS(*HostSpecification::parseFormat("direct").get(), HostSpecification::Direct);

    TS_ASSERT_EQUALS(HostSpecification::parseFormat("").isValid(), false);
    TS_ASSERT_EQUALS(HostSpecification::parseFormat("JSON").isValid(), false);
}

