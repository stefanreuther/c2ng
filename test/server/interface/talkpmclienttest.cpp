/**
  *  \file test/server/interface/talkpmclienttest.cpp
  *  \brief Test for server::interface::TalkPMClient
  */

#include "server/interface/talkpmclient.hpp"

#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/test/commandhandler.hpp"
#include "afl/test/testrunner.hpp"
#include "server/types.hpp"

using afl::data::Hash;
using afl::data::HashValue;
using afl::data::Vector;
using afl::data::VectorValue;

/** Simple test. */
AFL_TEST("server.interface.TalkPMClient", a)
{
    afl::test::CommandHandler mock(a);
    server::interface::TalkPMClient testee(mock);

    // a PMID list we use...
    static const int32_t pmids[] = { 145, 146 };

    // create
    mock.expectCall("PMNEW, u:foo, title, body");
    mock.provideNewResult(server::makeIntegerValue(143));
    a.checkEqual("01. create", testee.create("u:foo", "title", "body", afl::base::Nothing), 143);

    mock.expectCall("PMNEW, u:foo, title, body, PARENT, 110");
    mock.provideNewResult(server::makeIntegerValue(144));
    a.checkEqual("11. create", testee.create("u:foo", "title", "body", 110), 144);

    // getInfo
    {
        Hash::Ref_t in = Hash::create();
        in->setNew("subject", server::makeStringValue("subj"));
        in->setNew("to", server::makeStringValue("user"));
        in->setNew("author", server::makeStringValue("aa"));
        in->setNew("time", server::makeIntegerValue(987654));
        in->setNew("parent", server::makeIntegerValue(12));
        in->setNew("parentFolder", server::makeIntegerValue(5));
        in->setNew("parentFolderName", server::makeStringValue("five"));
        in->setNew("parentSubject", server::makeStringValue("old-subj"));
        in->setNew("suggestedFolder", server::makeIntegerValue(9));
        in->setNew("suggestedFolderName", server::makeStringValue("sug"));
        in->setNew("flags", server::makeIntegerValue(3));
        mock.expectCall("PMSTAT, 105, 145");
        mock.provideNewResult(new HashValue(in));

        server::interface::TalkPM::Info out = testee.getInfo(105, 145);
        a.checkEqual("21. subject",             out.subject, "subj");
        a.checkEqual("22. author",              out.author, "aa");
        a.checkEqual("23. receivers",           out.receivers, "user");
        a.checkEqual("24. time",                out.time, 987654);
        a.checkNonNull("25. parent",            out.parent.get());
        a.checkEqual("26. parent",             *out.parent.get(), 12);
        a.checkEqual("27. flags",               out.flags, 3);
        a.checkEqual("28. parentFolder",        out.parentFolder.orElse(-1), 5);
        a.checkEqual("29. parentFolderName",    out.parentFolderName.orElse(""), "five");
        a.checkEqual("30. parentSubject",       out.parentSubject.orElse(""), "old-subj");
        a.checkEqual("31. suggestedFolder",     out.suggestedFolder.orElse(-1), 9);
        a.checkEqual("32. suggestedFolderName", out.suggestedFolderName.orElse(""), "sug");
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

        mock.expectCall("PMMSTAT, 105, 145, 146");
        mock.provideNewResult(new VectorValue(vec));

        afl::container::PtrVector<server::interface::TalkPM::Info> out;
        testee.getInfo(105, pmids, out);

        a.checkEqual("41. size", out.size(), 2U);
        a.checkNull("42. result", out[0]);
        a.checkNonNull("43. result", out[1]);

        a.checkEqual  ("51. subject",             out[1]->subject, "subj");
        a.checkEqual  ("52. author",              out[1]->author, "aa");
        a.checkEqual  ("53. receivers",           out[1]->receivers, "user");
        a.checkEqual  ("54. time",                out[1]->time, 987654);
        a.checkNonNull("55. parent",              out[1]->parent.get());
        a.checkEqual  ("56. parent",             *out[1]->parent.get(), 12);
        a.checkEqual  ("57. flags",               out[1]->flags, 3);
        a.checkEqual  ("58. parentFolder",        out[1]->parentFolder.isValid(), false);
        a.checkEqual  ("59. parentFolderName",    out[1]->parentFolderName.isValid(), false);
        a.checkEqual  ("60. parentSubject",       out[1]->parentSubject.isValid(), false);
        a.checkEqual  ("61. suggestedFolder",     out[1]->suggestedFolder.isValid(), false);
        a.checkEqual  ("62. suggestedFolderName", out[1]->suggestedFolderName.isValid(), false);
    }

    // copy
    mock.expectCall("PMCP, 104, 105");
    mock.provideNewResult(server::makeIntegerValue(0));
    a.checkEqual("71. copy", testee.copy(104, 105, afl::base::Nothing), 0);

    mock.expectCall("PMCP, 104, 105, 145, 146");
    mock.provideNewResult(server::makeIntegerValue(2));
    a.checkEqual("81. copy", testee.copy(104, 105, pmids), 2);

    // move
    mock.expectCall("PMMV, 107, 103");
    mock.provideNewResult(server::makeIntegerValue(0));
    a.checkEqual("91. move", testee.move(107, 103, afl::base::Nothing), 0);

    mock.expectCall("PMMV, 103, 104, 145, 146");
    mock.provideNewResult(server::makeIntegerValue(2));
    a.checkEqual("101. move", testee.move(103, 104, pmids), 2);

    // remove
    mock.expectCall("PMRM, 102");
    mock.provideNewResult(server::makeIntegerValue(0));
    a.checkEqual("111. remove", testee.remove(102, afl::base::Nothing), 0);

    mock.expectCall("PMRM, 103, 145, 146");
    mock.provideNewResult(server::makeIntegerValue(1));
    a.checkEqual("121. remove", testee.remove(103, pmids), 1);

    // render
    mock.expectCall("PMRENDER, 101, 155");
    mock.provideNewResult(server::makeStringValue("formatted text"));
    a.checkEqual("131. render", testee.render(101, 155, server::interface::TalkPM::Options()), "formatted text");

    {
        server::interface::TalkPM::Options opts;
        opts.baseUrl = "/base";
        opts.format = "html";

        mock.expectCall("PMRENDER, 101, 185, BASEURL, /base, FORMAT, html");
        mock.provideNewResult(server::makeStringValue("<html>formatted text"));
        a.checkEqual("141. render", testee.render(101, 185, opts), "<html>formatted text");
    }

    // render multiple
    {
        Vector::Ref_t vec = Vector::create();
        vec->pushBackNew(server::makeStringValue("m1"));
        vec->pushBackNew(0);
        vec->pushBackNew(server::makeStringValue("m3"));
        mock.expectCall("PMMRENDER, 101, 642, 643, 648");
        mock.provideNewResult(new VectorValue(vec));

        static const int32_t pmids[] = { 642, 643, 648 };
        afl::container::PtrVector<String_t> out;
        testee.render(101, pmids, out);

        a.checkEqual  ("151. size", out.size(), 3U);
        a.checkNonNull("152. result", out[0]);
        a.checkNull   ("153. result", out[1]);
        a.checkNonNull("154. result", out[2]);
        a.checkEqual  ("155. result", *out[0], "m1");
        a.checkEqual  ("156. result", *out[2], "m3");
    }

    // flags
    mock.expectCall("PMFLAG, 102, 4, 3");
    mock.provideNewResult(server::makeIntegerValue(0));
    a.checkEqual("161. changeFlags", testee.changeFlags(102, 4, 3, afl::base::Nothing), 0);

    mock.expectCall("PMFLAG, 102, 4, 3, 145, 146");
    mock.provideNewResult(server::makeIntegerValue(2));
    a.checkEqual("171. changeFlags", testee.changeFlags(102, 4, 3, pmids), 2);
}
