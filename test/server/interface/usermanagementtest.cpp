/**
  *  \file test/server/interface/usermanagementtest.cpp
  *  \brief Test for server::interface::UserManagement
  */

#include "server/interface/usermanagement.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("server.interface.UserManagement")
{
    class Tester : public server::interface::UserManagement {
     public:
        virtual String_t add(String_t /*userName*/, String_t /*password*/, afl::base::Memory<const String_t> /*config*/)
            { return String_t(); }
        virtual void remove(String_t /*userId*/)
            { }
        virtual String_t login(String_t /*userName*/, String_t /*password*/)
            { return String_t(); }
        virtual String_t getUserIdByName(String_t /*userName*/)
            { return String_t(); }
        virtual String_t getNameByUserId(String_t /*userId*/)
            { return String_t(); }
        virtual void getNamesByUserId(afl::base::Memory<const String_t> /*userIds*/, afl::data::StringList_t& /*userNames*/)
            { }
        virtual server::Value_t* getProfileRaw(String_t /*userId*/, String_t /*key*/)
            { return 0; }
        virtual server::Value_t* getProfileRaw(String_t /*userId*/, afl::base::Memory<const String_t> /*keys*/)
            { return 0; }
        virtual void setProfile(String_t /*userId*/, afl::base::Memory<const String_t> /*config*/)
            { }
        virtual void setPassword(String_t /*userId*/, String_t /*password*/)
            { }
    };
    Tester t;
}
