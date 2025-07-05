/**
  *  \file test/server/doc/documentationimpltest.cpp
  *  \brief Test for server::doc::DocumentationImpl
  */

#include "server/doc/documentationimpl.hpp"

#include "afl/test/testrunner.hpp"
#include "server/doc/root.hpp"
#include "util/doc/index.hpp"
#include "util/doc/internalblobstore.hpp"
#include <stdexcept>

using server::doc::Root;
using server::doc::DocumentationImpl;
using server::interface::Documentation;
using util::doc::BlobStore;
using util::doc::InternalBlobStore;
using util::doc::Index;

/** Test getBlob(). */
AFL_TEST("server.doc.DocumentationImpl:getBlob", a)
{
    // Environment
    InternalBlobStore blobs;
    Root r(blobs);
    String_t id = blobs.addObject(afl::string::toBytes("hi"));

    // Test
    DocumentationImpl testee(r);

    a.checkEqual("01. success", testee.getBlob(id), "hi");
    AFL_CHECK_THROWS(a("02. error"), testee.getBlob(""), std::exception);
}

/** Test node access. */
AFL_TEST("server.doc.DocumentationImpl:node-access", a)
{
    // Environment
    InternalBlobStore blobs;
    Root r(blobs);

    // Create some documents
    String_t p1 = "<p>First page, see <a href=\"p2\">second</a></p>";
    String_t p21 = "<p>Second page</p>";
    String_t p22 = "<p>Second page, updated</p>";
    String_t blob = "whatever";

    Index& idx = r.index();
    Index::Handle_t g = idx.addDocument(idx.root(), "g", "Group", "");
    Index::Handle_t v1 = idx.addDocument(g, "v1", "Version 1", "");
    Index::Handle_t v2 = idx.addDocument(g, "v2", "Version 2", "");
    BlobStore::ObjectId_t o1 = blobs.addObject(afl::string::toBytes(p1));
    idx.addPage(v1, "p1", "Page 1", o1);
    idx.addPage(v2, "p1", "Page 1", blobs.addObject(afl::string::toBytes(p1)));
    idx.addPage(v1, "p2", "Page 2", blobs.addObject(afl::string::toBytes(p21)));
    idx.addPage(v2, "p2", "Page 2", blobs.addObject(afl::string::toBytes(p22)));
    idx.addNodeTags(v1, "old");
    idx.addNodeTags(v2, "new");

    Index::Handle_t blobId = idx.addPage(v2, "bb", "Blob", blobs.addObject(afl::string::toBytes(blob)));
    idx.addNodeTags(blobId, "blob");

    // Test
    DocumentationImpl testee(r);

    // renderNode()
    {
        Documentation::RenderOptions opts;
        opts.docRoot = "/doc/";
        opts.docSuffix = "?m";
        a.checkEqual("01. renderNode", testee.renderNode("v1/p1", opts), "<p>First page, see <a href=\"/doc/v1/p2?m\">second</a></p>");
        a.checkEqual("02. renderNode", testee.renderNode("", opts), "");
        AFL_CHECK_THROWS(a("03. renderNode"), testee.renderNode("x/y", opts), std::exception);
    }

    // renderNode(), blob - precise format is unspecified, but must contain link to assetRoot
    {
        Documentation::RenderOptions opts;
        opts.assetRoot = "/asset/";

        String_t result = testee.renderNode("v2/bb", opts);
        a.checkContains("06. renderNode", result, "/asset/");
    }

    // getNodeInfo
    {
        Documentation::NodeInfo i1 = testee.getNodeInfo("v1/p1");
        a.checkEqual("11. nodeId",      i1.nodeId, "v1/p1");
        a.checkEqual("12. title",       i1.title, "Page 1");
        a.checkEqual("13. tags",        i1.tags.size(), 0U);
        a.checkEqual("14. isPage",      i1.isPage, true);
        a.checkEqual("15. hasChildren", i1.hasChildren, false);
        a.checkEqual("16. blob",        i1.blobId, o1);

        Documentation::NodeInfo i2 = testee.getNodeInfo("v1");
        a.checkEqual("21. nodeId",      i2.nodeId, "v1");
        a.checkEqual("22. title",       i2.title, "Version 1");
        a.checkEqual("23. tags",        i2.tags.size(), 1U);
        a.checkEqual("24. tags",        i2.tags[0], "old");
        a.checkEqual("25. isPage",      i2.isPage, false);
        a.checkEqual("26. hasChildren", i2.hasChildren, true);

        Documentation::NodeInfo i3 = testee.getNodeInfo("");
        a.checkEqual("31. nodeId",      i3.nodeId, "");
        a.checkEqual("32. title",       i3.title, "");
        a.checkEqual("33. tags",        i3.tags.size(), 0U);
        a.checkEqual("34. isPage",      i3.isPage, false);
        a.checkEqual("35. hasChildren", i3.hasChildren, true);

        AFL_CHECK_THROWS(a("41. getNodeInfo"), testee.getNodeInfo("asdklja"), std::exception);
    }

    // getNodeChildren
    {
        Documentation::ChildOptions opts;
        std::vector<Documentation::NodeInfo> v1 = testee.getNodeChildren("v1/p1", opts);
        a.checkEqual("51. size", v1.size(), 0U);

        std::vector<Documentation::NodeInfo> v2 = testee.getNodeChildren("v1", opts);
        a.checkEqual("61. size", v2.size(), 2U);
        a.checkEqual("62. nodeId", v2[0].nodeId, "v1/p1");
        a.checkEqual("63. title",  v2[0].title, "Page 1");
        a.checkEqual("64. nodeId", v2[1].nodeId, "v1/p2");
        a.checkEqual("65. title",  v2[1].title, "Page 2");

        std::vector<Documentation::NodeInfo> v3 = testee.getNodeChildren("", opts);
        a.checkEqual("71. size", v3.size(), 3U);
        a.checkEqual("72. nodeId",  v3[0].nodeId, "g");
        a.checkEqual("73. title",   v3[0].title, "Group");
        a.checkEqual("74. infoTag", v3[0].infoTag, 1);
        a.checkEqual("75. nodeId",  v3[1].nodeId, "v1");
        a.checkEqual("76. title",   v3[1].title, "Version 1");
        a.checkEqual("77. infoTag", v3[1].infoTag, 2);
        a.checkEqual("78. nodeId",  v3[2].nodeId, "v2");
        a.checkEqual("79. title",   v3[2].title, "Version 2");
        a.checkEqual("80. infoTag", v3[2].infoTag, 2);

        Documentation::ChildOptions allOpts;
        allOpts.acrossDocuments = true;
        allOpts.maxDepth = 10;
        std::vector<Documentation::NodeInfo> v4 = testee.getNodeChildren("", allOpts);
        a.checkEqual("81. size", v4.size(), 8U);

        AFL_CHECK_THROWS(a("91. getNodeChildren"), testee.getNodeChildren("asljk", opts), std::exception);
    }

    // getNodeParents
    {
        std::vector<Documentation::NodeInfo> p1 = testee.getNodeParents("v1/p1");
        a.checkEqual("101. size", p1.size(), 2U);
        a.checkEqual("102. nodeId", p1[0].nodeId, "g");
        a.checkEqual("103. title",  p1[0].title, "Group");
        a.checkEqual("104. nodeId", p1[1].nodeId, "v1");
        a.checkEqual("105. title",  p1[1].title, "Version 1");

        std::vector<Documentation::NodeInfo> p2 = testee.getNodeParents("g");
        a.checkEqual("111. size", p2.size(), 0U);

        std::vector<Documentation::NodeInfo> p3 = testee.getNodeParents("");
        a.checkEqual("121. size", p3.size(), 0U);

        AFL_CHECK_THROWS(a("131. getNodeParents"), testee.getNodeParents("v1/p7"), std::exception);
    }

    // getNodeNavigationContext
    {
        std::vector<Documentation::NodeInfo> p1 = testee.getNodeNavigationContext("v1/p1");
        bool hasUp = false, hasPrev = false, hasNext = false;
        for (size_t i = 0; i < p1.size(); ++i) {
            // These tags are part of the wire protocol so we can hardcode them
            switch (p1[i].infoTag) {
             case -2: a.check("141. hasPrev", !hasPrev); hasPrev = true; a.checkEqual("141. nodeId", p1[i].nodeId, "v1"); break;
             case  0: a.check("142. hasUp",   !hasUp);   hasUp   = true; a.checkEqual("142. nodeId", p1[i].nodeId, "v1"); break;
             case +2: a.check("143. hasNext", !hasNext); hasNext = true; a.checkEqual("143. nodeId", p1[i].nodeId, "v1/p2"); break;
            }
        }
        a.check("144. hasPrev", hasPrev);
        a.check("145. hasUp",   hasUp);
        a.check("146. hasNext", hasNext);

        AFL_CHECK_THROWS(a("151. getNodeNavigationContext"), testee.getNodeNavigationContext("v1/p7"), std::exception);
    }

    // getNodeRelatedVersions
    {
        std::vector<Documentation::NodeInfo> r1 = testee.getNodeRelatedVersions("v1/p1");
        a.checkEqual("161. size", r1.size(), 2U);
        a.checkEqual("162. nodeId",  r1[0].nodeId, "v1/p1");
        a.checkEqual("163. title",   r1[0].title, "Version 1");
        a.checkEqual("164. tags",    r1[0].tags.size(), 1U);
        a.checkEqual("165. tags",    r1[0].tags[0], "old");
        a.checkEqual("166. infoTag", r1[0].infoTag, 1);
        a.checkEqual("167. nodeId",  r1[1].nodeId, "v2/p1");
        a.checkEqual("168. title",   r1[1].title, "Version 2");
        a.checkEqual("169. tags",    r1[1].tags.size(), 1U);
        a.checkEqual("170. tags",    r1[1].tags[0], "new");
        a.checkEqual("171. infoTag", r1[1].infoTag, 1);

        std::vector<Documentation::NodeInfo> r2 = testee.getNodeRelatedVersions("v1/p2");
        a.checkEqual("181. size", r2.size(), 2U);
        a.checkEqual("182. nodeId",  r2[0].nodeId, "v1/p2");
        a.checkEqual("183. title",   r2[0].title, "Version 1");
        a.checkEqual("184. tags",    r2[0].tags.size(), 1U);
        a.checkEqual("185. tags",    r2[0].tags[0], "old");
        a.checkEqual("186. infoTag", r2[0].infoTag, 1);            // same (=we come from here)
        a.checkEqual("187. nodeId",  r2[1].nodeId, "v2/p2");
        a.checkEqual("188. title",   r2[1].title, "Version 2");
        a.checkEqual("189. tags",    r2[1].tags.size(), 1U);
        a.checkEqual("190. tags",    r2[1].tags[0], "new");
        a.checkEqual("191. infoTag", r2[1].infoTag, 0);            // not same
    }
}
