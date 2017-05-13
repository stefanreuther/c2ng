/**
  *  \file u/t_server_interface_talkforum.cpp
  *  \brief Test for server::interface::TalkForum
  */

#include "server/interface/talkforum.hpp"

#include "t_server_interface.hpp"
#include "server/types.hpp"

/** Interface test. */
void
TestServerInterfaceTalkForum::testIt()
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
    };
    Tester t;
}

/** Interface test. */
void
TestServerInterfaceTalkForum::testGetValue()
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
    };

    {
        class IntTester : public Tester {
         public:
            virtual afl::data::Value* getValue(int32_t fid, String_t keyName)
                {
                    TS_ASSERT_EQUALS(fid, 12);
                    TS_ASSERT_EQUALS(keyName, "key");
                    return server::makeIntegerValue(99);
                }
        };
        TS_ASSERT_EQUALS(IntTester().getIntegerValue(12, "key"), 99);
    }

    {
        class StringTester : public Tester {
         public:
            virtual afl::data::Value* getValue(int32_t fid, String_t keyName)
                {
                    TS_ASSERT_EQUALS(fid, 15);
                    TS_ASSERT_EQUALS(keyName, "otherKey");
                    return server::makeStringValue("result");
                }
        };
        TS_ASSERT_EQUALS(StringTester().getStringValue(15, "otherKey"), "result");
    }
}

