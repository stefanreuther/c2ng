/**
  *  \file test/server/interface/talkforumtest.cpp
  *  \brief Test for server::interface::TalkForum
  */

#include "server/interface/talkforum.hpp"

#include "afl/test/testrunner.hpp"
#include "server/types.hpp"

/** Interface test. */
AFL_TEST_NOARG("server.interface.TalkForum:interface")
{
    class Tester : public server::interface::TalkForum {
     public:
        virtual int32_t add(afl::base::Memory<const String_t> /*config*/)
            { return 0; }
        virtual void configure(int32_t /*fid*/, afl::base::Memory<const String_t> /*config*/)
            { }
        virtual afl::data::Value* getValue(int32_t /*fid*/, String_t /*keyName*/)
            { return 0; }
        virtual Info getInfo(int32_t /*fid*/)
            { return Info(); }
        virtual void getInfo(afl::base::Memory<const int32_t> /*fids*/, afl::container::PtrVector<Info>& /*result*/)
            { }
        virtual int32_t getPermissions(int32_t /*fid*/, afl::base::Memory<const String_t> /*permissionList*/)
            { return 0; }
        virtual Size getSize(int32_t /*fid*/)
            { return Size(); }
        virtual afl::data::Value* getThreads(int32_t /*fid*/, const ListParameters& /*params*/)
            { return 0; }
        virtual afl::data::Value* getStickyThreads(int32_t /*fid*/, const ListParameters& /*params*/)
            { return 0; }
        virtual afl::data::Value* getPosts(int32_t /*fid*/, const ListParameters& /*params*/)
            { return 0; }
        virtual int32_t findForum(String_t /*key*/)
            { return 0; }
    };
    Tester t;
}

AFL_TEST("server.interface.TalkForum:getValue", a)
{
    class Tester : public server::interface::TalkForum {
     public:
        virtual int32_t add(afl::base::Memory<const String_t> /*config*/)
            { return 0; }
        virtual void configure(int32_t /*fid*/, afl::base::Memory<const String_t> /*config*/)
            { }
        virtual Info getInfo(int32_t /*fid*/)
            { return Info(); }
        virtual void getInfo(afl::base::Memory<const int32_t> /*fids*/, afl::container::PtrVector<Info>& /*result*/)
            { }
        virtual int32_t getPermissions(int32_t /*fid*/, afl::base::Memory<const String_t> /*permissionList*/)
            { return 0; }
        virtual Size getSize(int32_t /*fid*/)
            { return Size(); }
        virtual afl::data::Value* getThreads(int32_t /*fid*/, const ListParameters& /*params*/)
            { return 0; }
        virtual afl::data::Value* getStickyThreads(int32_t /*fid*/, const ListParameters& /*params*/)
            { return 0; }
        virtual afl::data::Value* getPosts(int32_t /*fid*/, const ListParameters& /*params*/)
            { return 0; }
        virtual int32_t findForum(String_t /*key*/)
            { return 0; }
    };

    {
        class IntTester : public Tester {
         public:
            IntTester(afl::test::Assert a)
                : m_assert(a)
                { }
            virtual afl::data::Value* getValue(int32_t fid, String_t keyName)
                {
                    m_assert.checkEqual("getValue > fid", fid, 12);
                    m_assert.checkEqual("getValue > keyName", keyName, "key");
                    return server::makeIntegerValue(99);
                }
            afl::test::Assert m_assert;
        };
        a.checkEqual("01. int", IntTester(a).getIntegerValue(12, "key"), 99);
    }

    {
        class StringTester : public Tester {
         public:
            StringTester(afl::test::Assert a)
                : m_assert(a)
                { }
            virtual afl::data::Value* getValue(int32_t fid, String_t keyName)
                {
                    m_assert.checkEqual("getValue > fid", fid, 15);
                    m_assert.checkEqual("getValue > keyName", keyName, "otherKey");
                    return server::makeStringValue("result");
                }
            afl::test::Assert m_assert;
        };
        a.checkEqual("02. str", StringTester(a).getStringValue(15, "otherKey"), "result");
    }
}
