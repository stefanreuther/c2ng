/**
  *  \file test/server/interface/documentationservertest.cpp
  *  \brief Test for server::interface::DocumentationServer
  */

#include "server/interface/documentationserver.hpp"

#include "afl/data/access.hpp"
#include "afl/data/segment.hpp"
#include "afl/string/format.hpp"
#include "afl/test/assert.hpp"
#include "afl/test/callreceiver.hpp"
#include "afl/test/testrunner.hpp"
#include "server/interface/documentationclient.hpp"
#include "server/types.hpp"
#include <stdexcept>

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
AFL_TEST("server.interface.DocumentationServer:commands", a)
{
    DocumentationMock mock(a);
    DocumentationServer testee(mock);

    // PING, HELP
    a.checkEqual("01. ping", testee.callString(Segment().pushBackString("PING")), "PONG");
    a.checkDifferent("02. help", testee.callString(Segment().pushBackString("HELP")), "");

    // GET
    {
        mock.expectCall("getBlob(xyzzy)");
        mock.provideReturnValue(String_t("blob content..."));

        a.checkEqual("11. get", testee.callString(Segment().pushBackString("GET").pushBackString("xyzzy")), "blob content...");
    }

    // RENDER
    {
        mock.expectCall("renderNode(n,a=-,d=-|-,s=-)");
        mock.provideReturnValue(String_t("<p>"));
        a.checkEqual("21. render", testee.callString(Segment().pushBackString("RENDER").pushBackString("n")), "<p>");
    }
    {
        mock.expectCall("renderNode(n,a=/a/,d=/d/|?q,s=/s/)");
        mock.provideReturnValue(String_t("<p>"));
        a.checkEqual("22. render", testee.callString(Segment().pushBackString("RENDER").pushBackString("n")
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
        Access ap(p.get());
        a.checkEqual("31. id",       ap("id").toString(), "s");
        a.checkEqual("32. title",    ap("title").toString(), "Stat");
        a.checkEqual("33. tags",     ap("tags").getArraySize(), 3U);
        a.checkEqual("34. tags",     ap("tags")[0].toString(), "t1");
        a.checkEqual("35. tags",     ap("tags")[1].toString(), "t2");
        a.checkEqual("36. tags",     ap("tags")[2].toString(), "t3");
        a.checkEqual("37. type",     ap("type").toInteger(), 1);
        a.checkEqual("38. children", ap("children").toInteger(), 1);
        a.checkEqual("39. info",     ap("info").toInteger(), 42);
    }

    // LS
    {
        mock.expectCall("getNodeChildren(r,d=-1,a=0)");
        mock.provideReturnValue(2);
        mock.provideReturnValue(makeNodeInfo("f", "first"));
        mock.provideReturnValue(makeNodeInfo("s", "second"));

        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("LS").pushBackString("r")));
        Access ap(p.get());
        a.checkEqual("41. getArraySize", ap.getArraySize(), 2U);
        a.checkEqual("42. id",    ap[0]("id").toString(), "f");
        a.checkEqual("43. title", ap[0]("title").toString(), "first");
        a.checkEqual("44. id",    ap[1]("id").toString(), "s");
        a.checkEqual("45. title", ap[1]("title").toString(), "second");
    }
    {
        mock.expectCall("getNodeChildren(r,d=3,a=1)");
        mock.provideReturnValue(0);

        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("LS").pushBackString("r").pushBackString("ACROSS").pushBackString("DEPTH").pushBackInteger(3)));
        Access ap(p.get());
        a.checkEqual("51. getArraySize", ap.getArraySize(), 0U);
    }

    // PATH
    {
        mock.expectCall("getNodeParents(g)");
        mock.provideReturnValue(1);
        mock.provideReturnValue(makeNodeInfo("n", "N"));

        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("PATH").pushBackString("g")));
        Access ap(p.get());
        a.checkEqual("61. getArraySize", ap.getArraySize(), 1U);
    }

    // NAV
    {
        mock.expectCall("getNodeNavigationContext(g)");
        mock.provideReturnValue(1);
        mock.provideReturnValue(makeNodeInfo("n", "N"));

        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("NAV").pushBackString("g")));
        Access ap(p.get());
        a.checkEqual("71. getArraySize", ap.getArraySize(), 1U);
    }

    // VER
    {
        mock.expectCall("getNodeRelatedVersions(g)");
        mock.provideReturnValue(1);
        mock.provideReturnValue(makeNodeInfo("n", "N"));

        std::auto_ptr<Value_t> p(testee.call(Segment().pushBackString("VER").pushBackString("g")));
        Access ap(p.get());
        a.checkEqual("81. getArraySize", ap.getArraySize(), 1U);
    }

    // Variants
    mock.expectCall("renderNode(n,a=/a/,d=/d/|-,s=/s/)");
    mock.provideReturnValue(String_t("<q>"));
    a.checkEqual("91. render", testee.callString(Segment().pushBackString("render").pushBackString("n")
                                                 .pushBackString("asset").pushBackString("/a/")
                                                 .pushBackString("site").pushBackString("/s/")
                                                 .pushBackString("doc").pushBackString("/d/")),
                 "<q>");

    mock.checkFinish();
}

/** Test errors. */
AFL_TEST("server.interface.DocumentationServer:errors", a)
{
    DocumentationMock mock(a);
    DocumentationServer testee(mock);

    // Missing command verb
    Segment empty;
    AFL_CHECK_THROWS(a("01. empty"), testee.callVoid(empty), std::exception);

    // Wrong command verb
    AFL_CHECK_THROWS(a("11. bad verb"), testee.callVoid(Segment().pushBackString("narf")), std::exception);

    // Missing parameter
    AFL_CHECK_THROWS(a("21. missing parameter"), testee.callVoid(Segment().pushBackString("GET")), std::exception);
    AFL_CHECK_THROWS(a("22. missing parameter"), testee.callVoid(Segment().pushBackString("RENDER").pushBackString("x").pushBackString("ASSET")), std::exception);

    // Wrong parameter
    AFL_CHECK_THROWS(a("31. bad parameter"), testee.callVoid(Segment().pushBackString("RENDER").pushBackString("x").pushBackString("LOLWHAT")), std::exception);
    AFL_CHECK_THROWS(a("32. bad parameter"), testee.callVoid(Segment().pushBackString("LS").pushBackString("x").pushBackString("LOLWHAT")), std::exception);

    // Too many parameters
    AFL_CHECK_THROWS(a("41. too many parameters"), testee.callVoid(Segment().pushBackString("GET").pushBackString("a").pushBackString("b")), std::exception);

    // Wrong type parameter
    AFL_CHECK_THROWS(a("51. bad type"), testee.callVoid(Segment().pushBackString("LS").pushBackString("a").pushBackString("DEPTH").pushBackString("xx")), std::exception);
}

/** Test round-trip compatibility between DocumentationServer and DocumentationClient. */
AFL_TEST("server.interface.DocumentationServer:roundtrip", a)
{
    DocumentationMock mock(a);
    DocumentationServer level1(mock);
    DocumentationClient level2(level1);
    DocumentationServer level3(level2);
    DocumentationClient level4(level3);

    // getBlob
    {
        mock.expectCall("getBlob(xyzzy)");
        mock.provideReturnValue(String_t("blob content..."));
        a.checkEqual("01. getBlob", level4.getBlob("xyzzy"), "blob content...");
    }

    // renderNode
    {
        mock.expectCall("renderNode(n,a=-,d=-|-,s=-)");
        mock.provideReturnValue(String_t("<p>"));
        a.checkEqual("11. renderNode", level4.renderNode("n", Documentation::RenderOptions()), "<p>");
    }
    {
        mock.expectCall("renderNode(n,a=/a/,d=/d/|?q,s=/s/)");
        mock.provideReturnValue(String_t("<p>"));

        Documentation::RenderOptions opts;
        opts.siteRoot = "/s/";
        opts.docRoot = "/d/";
        opts.assetRoot = "/a/";
        opts.docSuffix = "?q";
        a.checkEqual("21. renderNode", level4.renderNode("n", opts), "<p>");
    }

    // getNodeInfo
    {
        mock.expectCall("getNodeInfo(si)");
        mock.provideReturnValue(makeNodeInfo("s", "Stat"));

        Documentation::NodeInfo ni = level4.getNodeInfo("si");
        a.checkEqual("31. nodeId",      ni.nodeId, "s");
        a.checkEqual("32. title",       ni.title, "Stat");
        a.checkEqual("33. tags",        ni.tags.size(), 3U);
        a.checkEqual("34. tags",        ni.tags[0], "t1");
        a.checkEqual("35. tags",        ni.tags[1], "t2");
        a.checkEqual("36. tags",        ni.tags[2], "t3");
        a.checkEqual("37. isPage",      ni.isPage, false);
        a.checkEqual("38. hasChildren", ni.hasChildren, true);
        a.checkEqual("39. infoTag",     ni.infoTag, 42);
    }

    // getNodeChildren
    {
        mock.expectCall("getNodeChildren(r,d=-1,a=0)");
        mock.provideReturnValue(2);
        mock.provideReturnValue(makeNodeInfo("f", "first"));
        mock.provideReturnValue(makeNodeInfo("s", "second"));

        std::vector<Documentation::NodeInfo> nis = level4.getNodeChildren("r", Documentation::ChildOptions());
        a.checkEqual("41. size",   nis.size(), 2U);
        a.checkEqual("42. nodeId", nis[0].nodeId, "f");
        a.checkEqual("43. title",  nis[0].title, "first");
        a.checkEqual("44. nodeId", nis[1].nodeId, "s");
        a.checkEqual("45. title",  nis[1].title, "second");
    }
    {
        mock.expectCall("getNodeChildren(r,d=3,a=1)");
        mock.provideReturnValue(0);

        Documentation::ChildOptions opts;
        opts.maxDepth = 3;
        opts.acrossDocuments = true;
        std::vector<Documentation::NodeInfo> nis = level4.getNodeChildren("r", opts);
        a.checkEqual("51. size", nis.size(), 0U);
    }

    // getNodeParents
    {
        mock.expectCall("getNodeParents(g)");
        mock.provideReturnValue(1);
        mock.provideReturnValue(makeNodeInfo("n", "N"));

        std::vector<Documentation::NodeInfo> nis = level4.getNodeParents("g");
        a.checkEqual("61. size", nis.size(), 1U);
    }

    // getNodeNavigationContext
    {
        mock.expectCall("getNodeNavigationContext(g)");
        mock.provideReturnValue(1);
        mock.provideReturnValue(makeNodeInfo("n", "N"));

        std::vector<Documentation::NodeInfo> nis = level4.getNodeNavigationContext("g");
        a.checkEqual("71. size", nis.size(), 1U);
    }

    // getNodeRelatedVersions
    {
        mock.expectCall("getNodeRelatedVersions(g)");
        mock.provideReturnValue(1);
        mock.provideReturnValue(makeNodeInfo("n", "N"));

        std::vector<Documentation::NodeInfo> nis = level4.getNodeRelatedVersions("g");
        a.checkEqual("81. size", nis.size(), 1U);
    }

    mock.checkFinish();
}
