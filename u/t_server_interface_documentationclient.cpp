/**
  *  \file u/t_server_interface_documentationclient.cpp
  *  \brief Test for server::interface::DocumentationClient
  */

#include "server/interface/documentationclient.hpp"

#include "t_server_interface.hpp"
#include "afl/test/commandhandler.hpp"
#include "server/types.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"

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

        h->setNew("type", makeIntegerValue(0));
        h->setNew("children", makeIntegerValue(1));
        h->setNew("info", makeIntegerValue(7));
        return new HashValue(h);
    }
}

void
TestServerInterfaceDocumentationClient::testIt()
{
    afl::test::CommandHandler mock("testIt");
    server::interface::DocumentationClient testee(mock);

    // getBlob
    {
        mock.expectCall("GET, 1234567");
        mock.provideNewResult(makeStringValue("content"));

        TS_ASSERT_EQUALS(testee.getBlob("1234567"), "content");
    }

    // renderNode
    {
        mock.expectCall("RENDER, a/b");
        mock.provideNewResult(makeStringValue("text..."));

        TS_ASSERT_EQUALS(testee.renderNode("a/b", Documentation::RenderOptions()), "text...");
    }
    {
        mock.expectCall("RENDER, a/b, ASSET, a/, SITE, s/, DOC, d/, DOCSUFFIX, ?dd");
        mock.provideNewResult(makeStringValue("more text"));

        Documentation::RenderOptions opts;
        opts.assetRoot = "a/";
        opts.siteRoot = "s/";
        opts.docRoot = "d/";
        opts.docSuffix = "?dd";
        TS_ASSERT_EQUALS(testee.renderNode("a/b", opts), "more text");
    }

    // getNodeInfo
    {
        mock.expectCall("STAT, x");
        mock.provideNewResult(makeNodeInfo("x/y/z", "Title"));

        Documentation::NodeInfo ni = testee.getNodeInfo("x");

        TS_ASSERT_EQUALS(ni.nodeId, "x/y/z");
        TS_ASSERT_EQUALS(ni.title, "Title");
        TS_ASSERT_EQUALS(ni.tags.size(), 2U);
        TS_ASSERT_EQUALS(ni.tags[0], "t");
        TS_ASSERT_EQUALS(ni.tags[1], "s");
        TS_ASSERT_EQUALS(ni.isPage, true);
        TS_ASSERT_EQUALS(ni.hasChildren, true);
        TS_ASSERT_EQUALS(ni.infoTag, 7);
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

        TS_ASSERT_EQUALS(nis.size(), 3U);
        TS_ASSERT_EQUALS(nis[0].nodeId, "n1");
        TS_ASSERT_EQUALS(nis[0].title, "First");
        TS_ASSERT_EQUALS(nis[1].nodeId, "n2");
        TS_ASSERT_EQUALS(nis[1].title, "Second");
        TS_ASSERT_EQUALS(nis[2].nodeId, "n3");
        TS_ASSERT_EQUALS(nis[2].title, "Third");
    }
    {
        mock.expectCall("LS, n, DEPTH, 7, ACROSS");
        mock.provideNewResult(new VectorValue(Vector::create()));

        Documentation::ChildOptions opts;
        opts.maxDepth = 7;
        opts.acrossDocuments = true;

        std::vector<Documentation::NodeInfo> nis = testee.getNodeChildren("n", opts);

        TS_ASSERT_EQUALS(nis.size(), 0U);
    }

    // getNodeParents
    {
        mock.expectCall("PATH, pp");
        mock.provideNewResult(new VectorValue(Vector::create()));
        std::vector<Documentation::NodeInfo> nis = testee.getNodeParents("pp");
        TS_ASSERT_EQUALS(nis.size(), 0U);
    }

    // getNodeNavigationContext
    {
        mock.expectCall("NAV, pp");
        mock.provideNewResult(new VectorValue(Vector::create()));
        std::vector<Documentation::NodeInfo> nis = testee.getNodeNavigationContext("pp");
        TS_ASSERT_EQUALS(nis.size(), 0U);
    }

    // getNodeRelatedVersions
    {
        mock.expectCall("VER, pp");
        mock.provideNewResult(new VectorValue(Vector::create()));
        std::vector<Documentation::NodeInfo> nis = testee.getNodeRelatedVersions("pp");
        TS_ASSERT_EQUALS(nis.size(), 0U);
    }

    mock.checkFinish();
}

