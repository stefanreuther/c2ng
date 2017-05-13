/**
  *  \file u/t_server_interface_talkpmclient.cpp
  *  \brief Test for server::interface::TalkPMClient
  */

#include "server/interface/talkpmclient.hpp"

#include "t_server_interface.hpp"
#include "u/helper/commandhandlermock.hpp"
#include "server/types.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"

using afl::data::Hash;
using afl::data::HashValue;
using afl::data::Vector;
using afl::data::VectorValue;

/** Simple test. */
void
TestServerInterfaceTalkPMClient::testIt()
{
    CommandHandlerMock mock;
    server::interface::TalkPMClient testee(mock);

    // a PMID list we use...
    static const int32_t pmids[] = { 145, 146 };

    // create
    mock.expectCall("PMNEW|u:foo|title|body");
    mock.provideReturnValue(server::makeIntegerValue(143));
    TS_ASSERT_EQUALS(testee.create("u:foo", "title", "body", afl::base::Nothing), 143);

    mock.expectCall("PMNEW|u:foo|title|body|PARENT|110");
    mock.provideReturnValue(server::makeIntegerValue(144));
    TS_ASSERT_EQUALS(testee.create("u:foo", "title", "body", 110), 144);

    // getInfo
    {
        Hash::Ref_t in = Hash::create();
        in->setNew("subject", server::makeStringValue("subj"));
        in->setNew("to", server::makeStringValue("user"));
        in->setNew("author", server::makeStringValue("aa"));
        in->setNew("time", server::makeIntegerValue(987654));
        in->setNew("parent", server::makeIntegerValue(12));
        in->setNew("flags", server::makeIntegerValue(3));
        mock.expectCall("PMSTAT|105|145");
        mock.provideReturnValue(new HashValue(in));

        server::interface::TalkPM::Info out = testee.getInfo(105, 145);
        TS_ASSERT_EQUALS(out.subject, "subj");
        TS_ASSERT_EQUALS(out.author, "aa");
        TS_ASSERT_EQUALS(out.receivers, "user");
        TS_ASSERT_EQUALS(out.time, 987654);
        TS_ASSERT(out.parent.get() != 0);
        TS_ASSERT_EQUALS(*out.parent.get(), 12);
        TS_ASSERT_EQUALS(out.flags, 3);
    }

    // getInfos
    {
        Hash::Ref_t in = Hash::create();
        in->setNew("subject", server::makeStringValue("subj"));
        in->setNew("to", server::makeStringValue("user"));
        in->setNew("author", server::makeStringValue("aa"));
        in->setNew("time", server::makeIntegerValue(987654));
        in->setNew("parent", server::makeIntegerValue(12));
        in->setNew("flags", server::makeIntegerValue(3));

        Vector::Ref_t vec = Vector::create();
        vec->pushBackNew(0);
        vec->pushBackNew(new HashValue(in));

        mock.expectCall("PMMSTAT|105|145|146");
        mock.provideReturnValue(new VectorValue(vec));

        afl::container::PtrVector<server::interface::TalkPM::Info> out;
        testee.getInfo(105, pmids, out);

        TS_ASSERT_EQUALS(out.size(), 2U);
        TS_ASSERT(out[0] == 0);
        TS_ASSERT(out[1] != 0);

        TS_ASSERT_EQUALS(out[1]->subject, "subj");
        TS_ASSERT_EQUALS(out[1]->author, "aa");
        TS_ASSERT_EQUALS(out[1]->receivers, "user");
        TS_ASSERT_EQUALS(out[1]->time, 987654);
        TS_ASSERT(out[1]->parent.get() != 0);
        TS_ASSERT_EQUALS(*out[1]->parent.get(), 12);
        TS_ASSERT_EQUALS(out[1]->flags, 3);
    }

    // copy
    mock.expectCall("PMCP|104|105");
    mock.provideReturnValue(server::makeIntegerValue(0));
    TS_ASSERT_EQUALS(testee.copy(104, 105, afl::base::Nothing), 0);

    mock.expectCall("PMCP|104|105|145|146");
    mock.provideReturnValue(server::makeIntegerValue(2));
    TS_ASSERT_EQUALS(testee.copy(104, 105, pmids), 2);

    // move
    mock.expectCall("PMMV|107|103");
    mock.provideReturnValue(server::makeIntegerValue(0));
    TS_ASSERT_EQUALS(testee.move(107, 103, afl::base::Nothing), 0);

    mock.expectCall("PMMV|103|104|145|146");
    mock.provideReturnValue(server::makeIntegerValue(2));
    TS_ASSERT_EQUALS(testee.move(103, 104, pmids), 2);

    // remove
    mock.expectCall("PMRM|102");
    mock.provideReturnValue(server::makeIntegerValue(0));
    TS_ASSERT_EQUALS(testee.remove(102, afl::base::Nothing), 0);

    mock.expectCall("PMRM|103|145|146");
    mock.provideReturnValue(server::makeIntegerValue(1));
    TS_ASSERT_EQUALS(testee.remove(103, pmids), 1);

    // render
    mock.expectCall("PMRENDER|101|155");
    mock.provideReturnValue(server::makeStringValue("formatted text"));
    TS_ASSERT_EQUALS(testee.render(101, 155, server::interface::TalkPM::Options()), "formatted text");

    {
        server::interface::TalkPM::Options opts;
        opts.baseUrl = "/base";
        opts.format = "html";

        mock.expectCall("PMRENDER|101|185|BASEURL|/base|FORMAT|html");
        mock.provideReturnValue(server::makeStringValue("<html>formatted text"));
        TS_ASSERT_EQUALS(testee.render(101, 185, opts), "<html>formatted text");
    }

    // render multiple
    {
        Vector::Ref_t vec = Vector::create();
        vec->pushBackNew(server::makeStringValue("m1"));
        vec->pushBackNew(0);
        vec->pushBackNew(server::makeStringValue("m3"));
        mock.expectCall("PMMRENDER|101|642|643|648");
        mock.provideReturnValue(new VectorValue(vec));

        static const int32_t pmids[] = { 642, 643, 648 };
        afl::container::PtrVector<String_t> out;
        testee.render(101, pmids, out);

        TS_ASSERT_EQUALS(out.size(), 3U);
        TS_ASSERT(out[0] != 0);
        TS_ASSERT(out[1] == 0);
        TS_ASSERT(out[2] != 0);
        TS_ASSERT_EQUALS(*out[0], "m1");
        TS_ASSERT_EQUALS(*out[2], "m3");
    }

    // flags
    mock.expectCall("PMFLAG|102|4|3");
    mock.provideReturnValue(server::makeIntegerValue(0));
    TS_ASSERT_EQUALS(testee.changeFlags(102, 4, 3, afl::base::Nothing), 0);

    mock.expectCall("PMFLAG|102|4|3|145|146");
    mock.provideReturnValue(server::makeIntegerValue(2));
    TS_ASSERT_EQUALS(testee.changeFlags(102, 4, 3, pmids), 2);
}
