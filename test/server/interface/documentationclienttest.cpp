/**
  *  \file test/server/interface/documentationclienttest.cpp
  *  \brief Test for server::interface::DocumentationClient
  */

#include "server/interface/documentationclient.hpp"

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
using server::Value_t;
using server::interface::Documentation;
using server::makeIntegerValue;
using server::makeStringValue;

namespace {
    Value_t* makeNodeInfo(String_t id, String_t title)
    {
        Hash::Ref_t h = Hash::create();
        h->setNew("id", makeStringValue(id));
        h->setNew("title", makeStringValue(title));

        Vector::Ref_t v = Vector::create();
        v->pushBackNew(makeStringValue("t"));
        v->pushBackNew(makeStringValue("s"));
        h->setNew("tags", new VectorValue(v));
        h->setNew("blob", makeStringValue("ppqqrrss"));

        h->setNew("type", makeIntegerValue(0));
        h->setNew("children", makeIntegerValue(1));
        h->setNew("info", makeIntegerValue(7));
        return new HashValue(h);
    }
}

AFL_TEST("server.interface.DocumentationClient", a)
{
    afl::test::CommandHandler mock(a);
    server::interface::DocumentationClient testee(mock);

    // getBlob
    {
        mock.expectCall("GET, 1234567");
        mock.provideNewResult(makeStringValue("content"));

        a.checkEqual("01", testee.getBlob("1234567"), "content");
    }

    // renderNode
    {
        mock.expectCall("RENDER, a/b");
        mock.provideNewResult(makeStringValue("text..."));

        a.checkEqual("11", testee.renderNode("a/b", Documentation::RenderOptions()), "text...");
    }
    {
        mock.expectCall("RENDER, a/b, ASSET, a/, SITE, s/, DOC, d/, DOCSUFFIX, ?dd");
        mock.provideNewResult(makeStringValue("more text"));

        Documentation::RenderOptions opts;
        opts.assetRoot = "a/";
        opts.siteRoot = "s/";
        opts.docRoot = "d/";
        opts.docSuffix = "?dd";
        a.checkEqual("21", testee.renderNode("a/b", opts), "more text");
    }

    // getNodeInfo
    {
        mock.expectCall("STAT, x");
        mock.provideNewResult(makeNodeInfo("x/y/z", "Title"));

        Documentation::NodeInfo ni = testee.getNodeInfo("x");

        a.checkEqual("31. nodeId",      ni.nodeId, "x/y/z");
        a.checkEqual("32. title",       ni.title, "Title");
        a.checkEqual("33. tags",        ni.tags.size(), 2U);
        a.checkEqual("34. tags",        ni.tags[0], "t");
        a.checkEqual("35. tags",        ni.tags[1], "s");
        a.checkEqual("36. isPage",      ni.isPage, true);
        a.checkEqual("37. hasChildren", ni.hasChildren, true);
        a.checkEqual("38. infoTag",     ni.infoTag, 7);
        a.checkEqual("39. blob",        ni.blobId, "ppqqrrss");
    }

    // getNodeChildren
    {
        mock.expectCall("LS, n");

        Vector::Ref_t v = Vector::create();
        v->pushBackNew(makeNodeInfo("n1", "First"));
        v->pushBackNew(makeNodeInfo("n2", "Second"));
        v->pushBackNew(makeNodeInfo("n3", "Third"));
        mock.provideNewResult(new VectorValue(v));

        std::vector<Documentation::NodeInfo> nis = testee.getNodeChildren("n", Documentation::ChildOptions());

        a.checkEqual("41. size",   nis.size(), 3U);
        a.checkEqual("42. nodeId", nis[0].nodeId, "n1");
        a.checkEqual("43. title",  nis[0].title, "First");
        a.checkEqual("44. nodeId", nis[1].nodeId, "n2");
        a.checkEqual("45. title",  nis[1].title, "Second");
        a.checkEqual("46. nodeId", nis[2].nodeId, "n3");
        a.checkEqual("47. title",  nis[2].title, "Third");
    }
    {
        mock.expectCall("LS, n, DEPTH, 7, ACROSS");
        mock.provideNewResult(new VectorValue(Vector::create()));

        Documentation::ChildOptions opts;
        opts.maxDepth = 7;
        opts.acrossDocuments = true;

        std::vector<Documentation::NodeInfo> nis = testee.getNodeChildren("n", opts);

        a.checkEqual("51. size", nis.size(), 0U);
    }

    // getNodeParents
    {
        mock.expectCall("PATH, pp");
        mock.provideNewResult(new VectorValue(Vector::create()));
        std::vector<Documentation::NodeInfo> nis = testee.getNodeParents("pp");
        a.checkEqual("61. size", nis.size(), 0U);
    }

    // getNodeNavigationContext
    {
        mock.expectCall("NAV, pp");
        mock.provideNewResult(new VectorValue(Vector::create()));
        std::vector<Documentation::NodeInfo> nis = testee.getNodeNavigationContext("pp");
        a.checkEqual("71. size", nis.size(), 0U);
    }

    // getNodeRelatedVersions
    {
        mock.expectCall("VER, pp");
        mock.provideNewResult(new VectorValue(Vector::create()));
        std::vector<Documentation::NodeInfo> nis = testee.getNodeRelatedVersions("pp");
        a.checkEqual("81. size", nis.size(), 0U);
    }

    mock.checkFinish();
}
