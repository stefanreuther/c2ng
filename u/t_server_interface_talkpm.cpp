/**
  *  \file u/t_server_interface_talkpm.cpp
  *  \brief Test for server::interface::TalkPM
  */

#include "server/interface/talkpm.hpp"

#include "t_server_interface.hpp"

/** Interface test. */
void
TestServerInterfaceTalkPM::testInterface()
{
    class Tester : public server::interface::TalkPM {
     public:
        virtual int32_t create(String_t /*receivers*/, String_t /*subject*/, String_t /*text*/, afl::base::Optional<int32_t> /*parent*/)
            { return 0; }
        virtual Info getInfo(int32_t /*folder*/, int32_t /*pmid*/)
            { return Info(); }
        virtual void getInfo(int32_t /*folder*/, afl::base::Memory<const int32_t> /*pmids*/, afl::container::PtrVector<Info>& /*results*/)
            { }
        virtual int32_t copy(int32_t /*sourceFolder*/, int32_t /*destFolder*/, afl::base::Memory<const int32_t> /*pmids*/)
            { return 0; }
        virtual int32_t move(int32_t /*sourceFolder*/, int32_t /*destFolder*/, afl::base::Memory<const int32_t> /*pmids*/)
            { return 0; }
        virtual int32_t remove(int32_t /*folder*/, afl::base::Memory<const int32_t> /*pmids*/)
            { return 0; }
        virtual String_t render(int32_t /*folder*/, int32_t /*pmid*/, const Options& /*options*/)
            { return String_t(); }
        virtual void render(int32_t /*folder*/, afl::base::Memory<const int32_t> /*pmids*/, afl::container::PtrVector<String_t>& /*result*/)
            { }
        virtual int32_t changeFlags(int32_t /*folder*/, int32_t /*flagsToClear*/, int32_t /*flagsToSet*/, afl::base::Memory<const int32_t> /*pmids*/)
            { return 0; }
    };
    Tester t;
}

