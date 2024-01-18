/**
  *  \file test/server/interface/talkfolderclienttest.cpp
  *  \brief Test for server::interface::TalkFolderClient
  */

#include "server/interface/talkfolderclient.hpp"

#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/segment.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/test/commandhandler.hpp"
#include "afl/test/testrunner.hpp"
#include "server/types.hpp"
#include <memory>

using afl::data::Hash;
using afl::data::HashValue;
using afl::data::Segment;
using afl::data::Vector;
using afl::data::VectorValue;

/** Simple test. */
AFL_TEST("server.interface.TalkFolderClient", a)
{
    afl::test::CommandHandler mock(a);
    server::interface::TalkFolderClient testee(mock);

    // getFolders
    {
        afl::data::IntegerList_t result;
        mock.expectCall("FOLDERLS");
        mock.provideNewResult(0);
        testee.getFolders(result);
        a.checkEqual("01. size", result.size(), 0U);
    }
    {
        afl::data::IntegerList_t result;
        mock.expectCall("FOLDERLS");
        mock.provideNewResult(new VectorValue(Vector::create(Segment().pushBackInteger(1).pushBackInteger(2).pushBackInteger(100))));
        testee.getFolders(result);
        a.checkEqual("02. size", result.size(), 3U);
        a.checkEqual("03. result", result[0], 1);
        a.checkEqual("04. result", result[1], 2);
        a.checkEqual("05. result", result[2], 100);
    }

    // getInfo
    {
        mock.expectCall("FOLDERSTAT, 103");
        mock.provideNewResult(0);
        server::interface::TalkFolder::Info i = testee.getInfo(103);
        a.checkEqual("11. name",              i.name, "");
        a.checkEqual("12. description",       i.description, "");
        a.checkEqual("13. numMessages",       i.numMessages, 0);
        a.checkEqual("14. isFixedFolder",     i.isFixedFolder, false);
        a.checkEqual("15. hasUnreadMessages", i.hasUnreadMessages, false);
    }
    {
        Hash::Ref_t in = Hash::create();
        in->setNew("name", server::makeStringValue("The Name"));
        in->setNew("description", server::makeStringValue("Description..."));
        in->setNew("messages", server::makeIntegerValue(42));
        in->setNew("fixed", server::makeIntegerValue(true));
        in->setNew("unread", server::makeIntegerValue(true));
        mock.expectCall("FOLDERSTAT, 104");
        mock.provideNewResult(new HashValue(in));

        server::interface::TalkFolder::Info out = testee.getInfo(104);
        a.checkEqual("21. name",              out.name, "The Name");
        a.checkEqual("22. description",       out.description, "Description...");
        a.checkEqual("23. numMessages",       out.numMessages, 42);
        a.checkEqual("24. isFixedFolder",     out.isFixedFolder, true);
        a.checkEqual("25. hasUnreadMessages", out.hasUnreadMessages, true);
    }

    // getInfos
    {
        Hash::Ref_t in = Hash::create();
        in->setNew("name", server::makeStringValue("N"));
        in->setNew("description", server::makeStringValue("D"));
        in->setNew("messages", server::makeIntegerValue(23));
        in->setNew("fixed", server::makeIntegerValue(true));
        in->setNew("unread", server::makeIntegerValue(false));

        mock.expectCall("FOLDERMSTAT, 50, 105");
        mock.provideNewResult(new VectorValue(Vector::create(Segment().pushBackNew(0).pushBackNew(new HashValue(in)))));

        static const int32_t ufids[] = { 50, 105 };
        afl::container::PtrVector<server::interface::TalkFolder::Info> out;
        testee.getInfo(ufids, out);

        a.checkEqual("31. size", out.size(), 2U);
        a.checkNull("32. result",             out[0]);
        a.checkNonNull("33. result",          out[1]);
        a.checkEqual("34. name",              out[1]->name, "N");
        a.checkEqual("35. description",       out[1]->description, "D");
        a.checkEqual("36. numMessages",       out[1]->numMessages, 23);
        a.checkEqual("37. isFixedFolder",     out[1]->isFixedFolder, true);
        a.checkEqual("38. hasUnreadMessages", out[1]->hasUnreadMessages, false);
    }

    // create
    mock.expectCall("FOLDERNEW, N");
    mock.provideNewResult(server::makeIntegerValue(108));
    a.checkEqual("41. create", testee.create("N", afl::base::Nothing), 108);
    {
        mock.expectCall("FOLDERNEW, Nam., description, Desc.");
        mock.provideNewResult(server::makeIntegerValue(109));

        const String_t args[] = { "description", "Desc." };
        a.checkEqual("51. create", testee.create("Nam.", args), 109);
    }

    // remove
    mock.expectCall("FOLDERRM, 105");
    mock.provideNewResult(0);
    testee.remove(105);

    // configure
    mock.expectCall("FOLDERSET, 109");
    mock.provideNewResult(0);
    testee.configure(109, afl::base::Nothing);
    {
        mock.expectCall("FOLDERSET, 109, description, Desc2.");
        mock.provideNewResult(0);

        const String_t args[] = { "description", "Desc2." };
        testee.configure(109, args);
    }

    // getPMs
    {
        mock.expectCall("FOLDERLSPM, 109");
        mock.provideNewResult(server::makeIntegerValue(9));
        std::auto_ptr<afl::data::Value> p(testee.getPMs(109, server::interface::TalkFolder::ListParameters(), server::interface::TalkFolder::FilterParameters()));
        a.checkEqual("61. folderlspm", server::toInteger(p.get()), 9);
    }
    {
        mock.expectCall("FOLDERLSPM, 109, LIMIT, 5, 3, SORT, subject");
        server::interface::TalkFolder::ListParameters ps;
        ps.mode = ps.WantRange;
        ps.start = 5;
        ps.count = 3;
        ps.sortKey = "subject";
        mock.provideNewResult(server::makeIntegerValue(9));
        std::auto_ptr<afl::data::Value> p(testee.getPMs(109, ps, server::interface::TalkFolder::FilterParameters()));
        a.checkEqual("62. folderlspm", server::toInteger(p.get()), 9);
    }
    {
        mock.expectCall("FOLDERLSPM, 109, CONTAINS, 9, FLAGS, 7, 4");
        server::interface::TalkFolder::ListParameters ps;
        ps.mode = ps.WantMemberCheck;
        ps.item = 9;

        server::interface::TalkFolder::FilterParameters fs;
        fs.flagMask = 7;
        fs.flagCheck = 4;

        mock.provideNewResult(server::makeIntegerValue(1));
        std::auto_ptr<afl::data::Value> p(testee.getPMs(109, ps, fs));
        a.checkEqual("71. folderlspm", server::toInteger(p.get()), 1);
    }
}
