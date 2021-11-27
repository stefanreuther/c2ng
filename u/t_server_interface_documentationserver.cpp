/**
  *  \file u/t_server_interface_documentationserver.cpp
  *  \brief Test for server::interface::DocumentationServer
  */

#include <stdexcept>
#include "server/interface/documentationserver.hpp"

#include "t_server_interface.hpp"
#include "afl/data/access.hpp"
#include "afl/data/segment.hpp"
#include "afl/string/format.hpp"
#include "afl/test/assert.hpp"
#include "afl/test/callreceiver.hpp"
#include "server/interface/documentationclient.hpp"
#include "server/types.hpp"

using afl::data::Access;
using afl::data::Segment;
using afl::string::Format;
using afl::test::Assert;
using afl::test::CallReceiver;
using server::Value_t;
using server::interface::Documentation;
using server::interface::DocumentationClient;
using server::interface::DocumentationServer;

namespace {
    class DocumentationMock : public Documentation, public CallReceiver {
     public:
        DocumentationMock(Assert a)
            : Documentation(),
              CallReceiver(a)
            { }
        virtual String_t getBlob(String_t blobId)
            {
                checkCall(Format("getBlob(%d)", blobId));
                return consumeReturnValue<String_t>();
            }
        virtual String_t renderNode(String_t nodeId, const RenderOptions& opts)
            {
                checkCall(Format("renderNode(%s,a=%s,d=%s|%s,s=%s)")
                          << nodeId << opts.assetRoot.orElse("-")
                          << opts.docRoot.orElse("-")
                          << opts.docSuffix.orElse("-")
                          << opts.siteRoot.orElse("-"));
                return consumeReturnValue<String_t>();
            }
        virtual NodeInfo getNodeInfo(String_t nodeId)
            {
                checkCall(Format("getNodeInfo(%s)", nodeId));
                return consumeReturnValue<NodeInfo>();
            }
        virtual std::vector<NodeInfo> getNodeChildren(String_t nodeId, const ChildOptions& opts)
            {
                checkCall(Format("getNodeChildren(%s,d=%d,a=%d)", nodeId, opts.maxDepth.orElse(-1), int(opts.acrossDocuments)));
                return consumeNodeInfoVector();
            }
        virtual std::vector<NodeInfo> getNodeParents(String_t nodeId)
            {
                checkCall(Format("getNodeParents(%s)", nodeId));
                return consumeNodeInfoVector();
            }
        virtual std::vector<NodeInfo> getNodeNavigationContext(String_t nodeId)
            {
                checkCall(Format("getNodeNavigationContext(%s)", nodeId));
                return consumeNodeInfoVector();
            }
        virtual std::vector<NodeInfo> getNodeRelatedVersions(String_t nodeId)
            {
                checkCall(Format("getNodeRelatedVersions(%s)", nodeId));
                return consumeNodeInfoVector();
            }
     private:
        std::vector<NodeInfo> consumeNodeInfoVector()
            {
                std::vector<NodeInfo> result;
                int n = consumeReturnValue<int>();
                while (n > 0) {
                    result.push_back(consumeReturnValue<NodeInfo>());
                    --n;
                }
                return result;
            }
    };

    Documentation::NodeInfo makeNodeInfo(String_t id, String_t title)
    {
        Documentation::NodeInfo result;
        result.nodeId = id;
        result.title = title;
        result.tags.push_back("t1");
        result.tags.push_back("t2");
        result.tags.push_back("t3");
        result.isPage = false;
        result.hasChildren = true;
        result.infoTag = 42;
        return result;
    }
}

/** Test all the commands. */
void
TestServerInterfaceDocumentationServer::testIt()
{
    DocumentationMock mock("testIt");
    DocumentationServer testee(mock);

    // PING, HELP
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("PING")), "PONG");
    TS_ASSERT_DIFFERS(testee.callString(Segment().pushBackString("HELP")), "");

    // GET
    {
        mock.expectCall("getBlob(xyzzy)");
        mock.provideReturnValue(String_t("blob content..."));

        TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("GET").pushBackString("xyzzy")), "blob content...");
    }

    // RENDER
    {
        mock.expectCall("renderNode(n,a=-,d=-|-,s=-)");
        mock.provideReturnValue(String_t("<p>"));
        TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("RENDER").pushBackString("n")), "<p>");
    }
    {
        mock.expectCall("renderNode(n,a=/a/,d=/d/|?q,s=/s/)");
        mock.provideReturnValue(String_t("<p>"));
        TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("RENDER").pushBackString("n")
                                           .pushBackString("SITE").pushBackString("/s/")
                                           .pushBackString("DOCSUFFIX").pushBackString("?q")
                                           .pushBackString("ASSET").pushBackString("/a/")
                                           .pushBackString("DOC").pushBackString("/d/")),
                         "<p>");
    }

    // STAT
    {
        mock.expectCall("getNodeInfo(si)");
        mock.provideReturnValue(makeNodeInfo("s", "Stat"));

        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("STAT").pushBackString("si")));
        Access a(p.get());
        TS_ASSERT_EQUALS(a("id").toString(), "s");
        TS_ASSERT_EQUALS(a("title").toString(), "Stat");
        TS_ASSERT_EQUALS(a("tags").getArraySize(), 3U);
        TS_ASSERT_EQUALS(a("tags")[0].toString(), "t1");
        TS_ASSERT_EQUALS(a("tags")[1].toString(), "t2");
        TS_ASSERT_EQUALS(a("tags")[2].toString(), "t3");
        TS_ASSERT_EQUALS(a("type").toInteger(), 1);
        TS_ASSERT_EQUALS(a("children").toInteger(), 1);
        TS_ASSERT_EQUALS(a("info").toInteger(), 42);
    }

    // LS
    {
        mock.expectCall("getNodeChildren(r,d=-1,a=0)");
        mock.provideReturnValue(2);
        mock.provideReturnValue(makeNodeInfo("f", "first"));
        mock.provideReturnValue(makeNodeInfo("s", "second"));

        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("LS").pushBackString("r")));
        Access a(p.get());
        TS_ASSERT_EQUALS(a.getArraySize(), 2U);
        TS_ASSERT_EQUALS(a[0]("id").toString(), "f");
        TS_ASSERT_EQUALS(a[0]("title").toString(), "first");
        TS_ASSERT_EQUALS(a[1]("id").toString(), "s");
        TS_ASSERT_EQUALS(a[1]("title").toString(), "second");
    }
    {
        mock.expectCall("getNodeChildren(r,d=3,a=1)");
        mock.provideReturnValue(0);

        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("LS").pushBackString("r").pushBackString("ACROSS").pushBackString("DEPTH").pushBackInteger(3)));
        Access a(p.get());
        TS_ASSERT_EQUALS(a.getArraySize(), 0U);
    }

    // PATH
    {
        mock.expectCall("getNodeParents(g)");
        mock.provideReturnValue(1);
        mock.provideReturnValue(makeNodeInfo("n", "N"));

        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("PATH").pushBackString("g")));
        Access a(p.get());
        TS_ASSERT_EQUALS(a.getArraySize(), 1U);
    }

    // NAV
    {
        mock.expectCall("getNodeNavigationContext(g)");
        mock.provideReturnValue(1);
        mock.provideReturnValue(makeNodeInfo("n", "N"));

        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("NAV").pushBackString("g")));
        Access a(p.get());
        TS_ASSERT_EQUALS(a.getArraySize(), 1U);
    }

    // VER
    {
        mock.expectCall("getNodeRelatedVersions(g)");
        mock.provideReturnValue(1);
        mock.provideReturnValue(makeNodeInfo("n", "N"));

        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("VER").pushBackString("g")));
        Access a(p.get());
        TS_ASSERT_EQUALS(a.getArraySize(), 1U);
    }

    // Variants
    mock.expectCall("renderNode(n,a=/a/,d=/d/|-,s=/s/)");
    mock.provideReturnValue(String_t("<q>"));
    TS_ASSERT_EQUALS(testee.callString(Segment().pushBackString("render").pushBackString("n")
                                       .pushBackString("asset").pushBackString("/a/")
                                       .pushBackString("site").pushBackString("/s/")
                                       .pushBackString("doc").pushBackString("/d/")),
                     "<q>");

    mock.checkFinish();
}

/** Test errors. */
void
TestServerInterfaceDocumentationServer::testErrors()
{
    DocumentationMock mock("testIt");
    DocumentationServer testee(mock);

    // Missing command verb
    Segment empty;
    TS_ASSERT_THROWS(testee.callVoid(empty), std::exception);

    // Wrong command verb
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("narf")), std::exception);

    // Missing parameter
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("GET")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("RENDER").pushBackString("x").pushBackString("ASSET")), std::exception);

    // Wrong parameter
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("RENDER").pushBackString("x").pushBackString("LOLWHAT")), std::exception);
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("LS").pushBackString("x").pushBackString("LOLWHAT")), std::exception);

    // Too many parameters
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("GET").pushBackString("a").pushBackString("b")), std::exception);

    // Wrong type parameter
    TS_ASSERT_THROWS(testee.callVoid(Segment().pushBackString("LS").pushBackString("a").pushBackString("DEPTH").pushBackString("xx")), std::exception);
}

/** Test round-trip compatibility between DocumentationServer and DocumentationClient. */
void
TestServerInterfaceDocumentationServer::testRoundtrip()
{
    DocumentationMock mock("testRoundtrip");
    DocumentationServer level1(mock);
    DocumentationClient level2(level1);
    DocumentationServer level3(level2);
    DocumentationClient level4(level3);

    // getBlob
    {
        mock.expectCall("getBlob(xyzzy)");
        mock.provideReturnValue(String_t("blob content..."));
        TS_ASSERT_EQUALS(level4.getBlob("xyzzy"), "blob content...");
    }

    // renderNode
    {
        mock.expectCall("renderNode(n,a=-,d=-|-,s=-)");
        mock.provideReturnValue(String_t("<p>"));
        TS_ASSERT_EQUALS(level4.renderNode("n", Documentation::RenderOptions()), "<p>");
    }
    {
        mock.expectCall("renderNode(n,a=/a/,d=/d/|?q,s=/s/)");
        mock.provideReturnValue(String_t("<p>"));

        Documentation::RenderOptions opts;
        opts.siteRoot = "/s/";
        opts.docRoot = "/d/";
        opts.assetRoot = "/a/";
        opts.docSuffix = "?q";
        TS_ASSERT_EQUALS(level4.renderNode("n", opts), "<p>");
    }

    // getNodeInfo
    {
        mock.expectCall("getNodeInfo(si)");
        mock.provideReturnValue(makeNodeInfo("s", "Stat"));

        Documentation::NodeInfo ni = level4.getNodeInfo("si");
        TS_ASSERT_EQUALS(ni.nodeId, "s");
        TS_ASSERT_EQUALS(ni.title, "Stat");
        TS_ASSERT_EQUALS(ni.tags.size(), 3U);
        TS_ASSERT_EQUALS(ni.tags[0], "t1");
        TS_ASSERT_EQUALS(ni.tags[1], "t2");
        TS_ASSERT_EQUALS(ni.tags[2], "t3");
        TS_ASSERT_EQUALS(ni.isPage, false);
        TS_ASSERT_EQUALS(ni.hasChildren, true);
        TS_ASSERT_EQUALS(ni.infoTag, 42);
    }

    // getNodeChildren
    {
        mock.expectCall("getNodeChildren(r,d=-1,a=0)");
        mock.provideReturnValue(2);
        mock.provideReturnValue(makeNodeInfo("f", "first"));
        mock.provideReturnValue(makeNodeInfo("s", "second"));

        std::vector<Documentation::NodeInfo> nis = level4.getNodeChildren("r", Documentation::ChildOptions());
        TS_ASSERT_EQUALS(nis.size(), 2U);
        TS_ASSERT_EQUALS(nis[0].nodeId, "f");
        TS_ASSERT_EQUALS(nis[0].title, "first");
        TS_ASSERT_EQUALS(nis[1].nodeId, "s");
        TS_ASSERT_EQUALS(nis[1].title, "second");
    }
    {
        mock.expectCall("getNodeChildren(r,d=3,a=1)");
        mock.provideReturnValue(0);

        Documentation::ChildOptions opts;
        opts.maxDepth = 3;
        opts.acrossDocuments = true;
        std::vector<Documentation::NodeInfo> nis = level4.getNodeChildren("r", opts);
        TS_ASSERT_EQUALS(nis.size(), 0U);
    }

    // getNodeParents
    {
        mock.expectCall("getNodeParents(g)");
        mock.provideReturnValue(1);
        mock.provideReturnValue(makeNodeInfo("n", "N"));

        std::vector<Documentation::NodeInfo> nis = level4.getNodeParents("g");
        TS_ASSERT_EQUALS(nis.size(), 1U);
    }

    // getNodeNavigationContext
    {
        mock.expectCall("getNodeNavigationContext(g)");
        mock.provideReturnValue(1);
        mock.provideReturnValue(makeNodeInfo("n", "N"));

        std::vector<Documentation::NodeInfo> nis = level4.getNodeNavigationContext("g");
        TS_ASSERT_EQUALS(nis.size(), 1U);
    }

    // getNodeRelatedVersions
    {
        mock.expectCall("getNodeRelatedVersions(g)");
        mock.provideReturnValue(1);
        mock.provideReturnValue(makeNodeInfo("n", "N"));

        std::vector<Documentation::NodeInfo> nis = level4.getNodeRelatedVersions("g");
        TS_ASSERT_EQUALS(nis.size(), 1U);
    }

    mock.checkFinish();
}

