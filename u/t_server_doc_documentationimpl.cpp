/**
  *  \file u/t_server_doc_documentationimpl.cpp
  *  \brief Test for server::doc::DocumentationImpl
  */

#include <stdexcept>
#include "server/doc/documentationimpl.hpp"

#include "t_server_doc.hpp"
#include "util/doc/internalblobstore.hpp"
#include "server/doc/root.hpp"
#include "util/doc/index.hpp"

using server::doc::Root;
using server::doc::DocumentationImpl;
using server::interface::Documentation;
using util::doc::InternalBlobStore;
using util::doc::Index;

/** Test getBlob(). */
void
TestServerDocDocumentationImpl::testGetBlob()
{
    // Environment
    InternalBlobStore blobs;
    Root r(blobs);
    String_t id = blobs.addObject(afl::string::toBytes("hi"));

    // Test
    DocumentationImpl testee(r);

    TS_ASSERT_EQUALS(testee.getBlob(id), "hi");
    TS_ASSERT_THROWS(testee.getBlob(""), std::exception);
}

/** Test node access. */
void
TestServerDocDocumentationImpl::testNodeAccess()
{
    // Environment
    InternalBlobStore blobs;
    Root r(blobs);

    // Create some documents
    String_t p1 = "<p>First page, see <a href=\"p2\">second</a></p>";
    String_t p21 = "<p>Second page</p>";
    String_t p22 = "<p>Second page, updated</p>";

    Index& idx = r.index();
    Index::Handle_t g = idx.addDocument(idx.root(), "g", "Group", "");
    Index::Handle_t v1 = idx.addDocument(g, "v1", "Version 1", "");
    Index::Handle_t v2 = idx.addDocument(g, "v2", "Version 2", "");
    idx.addPage(v1, "p1", "Page 1", blobs.addObject(afl::string::toBytes(p1)));
    idx.addPage(v2, "p1", "Page 1", blobs.addObject(afl::string::toBytes(p1)));
    idx.addPage(v1, "p2", "Page 2", blobs.addObject(afl::string::toBytes(p21)));
    idx.addPage(v2, "p2", "Page 2", blobs.addObject(afl::string::toBytes(p22)));
    idx.addNodeTags(v1, "old");
    idx.addNodeTags(v2, "new");

    // Test
    DocumentationImpl testee(r);

    // renderNode()
    {
        Documentation::RenderOptions opts;
        opts.docRoot = "/doc/";
        opts.docSuffix = "?m";
        TS_ASSERT_EQUALS(testee.renderNode("v1/p1", opts), "<p>First page, see <a href=\"/doc/v1/p2?m\">second</a></p>");
        TS_ASSERT_EQUALS(testee.renderNode("", opts), "");
        TS_ASSERT_THROWS(testee.renderNode("x/y", opts), std::exception);
    }

    // getNodeInfo
    {
        Documentation::NodeInfo i1 = testee.getNodeInfo("v1/p1");
        TS_ASSERT_EQUALS(i1.nodeId, "v1/p1");
        TS_ASSERT_EQUALS(i1.title, "Page 1");
        TS_ASSERT_EQUALS(i1.tags.size(), 0U);
        TS_ASSERT_EQUALS(i1.isPage, true);
        TS_ASSERT_EQUALS(i1.hasChildren, false);

        Documentation::NodeInfo i2 = testee.getNodeInfo("v1");
        TS_ASSERT_EQUALS(i2.nodeId, "v1");
        TS_ASSERT_EQUALS(i2.title, "Version 1");
        TS_ASSERT_EQUALS(i2.tags.size(), 1U);
        TS_ASSERT_EQUALS(i2.tags[0], "old");
        TS_ASSERT_EQUALS(i2.isPage, false);
        TS_ASSERT_EQUALS(i2.hasChildren, true);

        Documentation::NodeInfo i3 = testee.getNodeInfo("");
        TS_ASSERT_EQUALS(i3.nodeId, "");
        TS_ASSERT_EQUALS(i3.title, "");
        TS_ASSERT_EQUALS(i3.tags.size(), 0U);
        TS_ASSERT_EQUALS(i3.isPage, false);
        TS_ASSERT_EQUALS(i3.hasChildren, true);

        TS_ASSERT_THROWS(testee.getNodeInfo("asdklja"), std::exception);
    }

    // getNodeChildren
    {
        Documentation::ChildOptions opts;
        std::vector<Documentation::NodeInfo> v1 = testee.getNodeChildren("v1/p1", opts);
        TS_ASSERT_EQUALS(v1.size(), 0U);

        std::vector<Documentation::NodeInfo> v2 = testee.getNodeChildren("v1", opts);
        TS_ASSERT_EQUALS(v2.size(), 2U);
        TS_ASSERT_EQUALS(v2[0].nodeId, "v1/p1");
        TS_ASSERT_EQUALS(v2[0].title, "Page 1");
        TS_ASSERT_EQUALS(v2[1].nodeId, "v1/p2");
        TS_ASSERT_EQUALS(v2[1].title, "Page 2");

        std::vector<Documentation::NodeInfo> v3 = testee.getNodeChildren("", opts);
        TS_ASSERT_EQUALS(v3.size(), 3U);
        TS_ASSERT_EQUALS(v3[0].nodeId, "g");
        TS_ASSERT_EQUALS(v3[0].title, "Group");
        TS_ASSERT_EQUALS(v3[0].infoTag, 1);
        TS_ASSERT_EQUALS(v3[1].nodeId, "v1");
        TS_ASSERT_EQUALS(v3[1].title, "Version 1");
        TS_ASSERT_EQUALS(v3[1].infoTag, 2);
        TS_ASSERT_EQUALS(v3[2].nodeId, "v2");
        TS_ASSERT_EQUALS(v3[2].title, "Version 2");
        TS_ASSERT_EQUALS(v3[2].infoTag, 2);

        Documentation::ChildOptions allOpts;
        allOpts.acrossDocuments = true;
        allOpts.maxDepth = 10;
        std::vector<Documentation::NodeInfo> v4 = testee.getNodeChildren("", allOpts);
        TS_ASSERT_EQUALS(v4.size(), 7U);

        TS_ASSERT_THROWS(testee.getNodeChildren("asljk", opts), std::exception);
    }

    // getNodeParents
    {
        std::vector<Documentation::NodeInfo> p1 = testee.getNodeParents("v1/p1");
        TS_ASSERT_EQUALS(p1.size(), 2U);
        TS_ASSERT_EQUALS(p1[0].nodeId, "g");
        TS_ASSERT_EQUALS(p1[0].title, "Group");
        TS_ASSERT_EQUALS(p1[1].nodeId, "v1");
        TS_ASSERT_EQUALS(p1[1].title, "Version 1");

        std::vector<Documentation::NodeInfo> p2 = testee.getNodeParents("g");
        TS_ASSERT_EQUALS(p2.size(), 0U);

        std::vector<Documentation::NodeInfo> p3 = testee.getNodeParents("");
        TS_ASSERT_EQUALS(p3.size(), 0U);

        TS_ASSERT_THROWS(testee.getNodeParents("v1/p7"), std::exception);
    }

    // getNodeNavigationContext
    {
        std::vector<Documentation::NodeInfo> p1 = testee.getNodeNavigationContext("v1/p1");
        bool hasUp = false, hasPrev = false, hasNext = false;
        for (size_t i = 0; i < p1.size(); ++i) {
            // These tags are part of the wire protocol so we can hardcode them
            switch (p1[i].infoTag) {
             case -2: TS_ASSERT(!hasPrev); hasPrev = true; TS_ASSERT_EQUALS(p1[i].nodeId, "v1"); break;
             case  0: TS_ASSERT(!hasUp);   hasUp   = true; TS_ASSERT_EQUALS(p1[i].nodeId, "v1"); break;
             case +2: TS_ASSERT(!hasNext); hasNext = true; TS_ASSERT_EQUALS(p1[i].nodeId, "v1/p2"); break;
            }
        }
        TS_ASSERT(hasPrev);
        TS_ASSERT(hasUp);
        TS_ASSERT(hasNext);

        TS_ASSERT_THROWS(testee.getNodeNavigationContext("v1/p7"), std::exception);
    }

    // getNodeRelatedVersions
    {
        std::vector<Documentation::NodeInfo> r1 = testee.getNodeRelatedVersions("v1/p1");
        TS_ASSERT_EQUALS(r1.size(), 2U);
        TS_ASSERT_EQUALS(r1[0].nodeId, "v1/p1");
        TS_ASSERT_EQUALS(r1[0].title, "Version 1");
        TS_ASSERT_EQUALS(r1[0].tags.size(), 1U);
        TS_ASSERT_EQUALS(r1[0].tags[0], "old");
        TS_ASSERT_EQUALS(r1[0].infoTag, 1);
        TS_ASSERT_EQUALS(r1[1].nodeId, "v2/p1");
        TS_ASSERT_EQUALS(r1[1].title, "Version 2");
        TS_ASSERT_EQUALS(r1[1].tags.size(), 1U);
        TS_ASSERT_EQUALS(r1[1].tags[0], "new");
        TS_ASSERT_EQUALS(r1[1].infoTag, 1);

        std::vector<Documentation::NodeInfo> r2 = testee.getNodeRelatedVersions("v1/p2");
        TS_ASSERT_EQUALS(r2.size(), 2U);
        TS_ASSERT_EQUALS(r2[0].nodeId, "v1/p2");
        TS_ASSERT_EQUALS(r2[0].title, "Version 1");
        TS_ASSERT_EQUALS(r2[0].tags.size(), 1U);
        TS_ASSERT_EQUALS(r2[0].tags[0], "old");
        TS_ASSERT_EQUALS(r2[0].infoTag, 1);            // same (=we come from here)
        TS_ASSERT_EQUALS(r2[1].nodeId, "v2/p2");
        TS_ASSERT_EQUALS(r2[1].title, "Version 2");
        TS_ASSERT_EQUALS(r2[1].tags.size(), 1U);
        TS_ASSERT_EQUALS(r2[1].tags[0], "new");
        TS_ASSERT_EQUALS(r2[1].infoTag, 0);            // not same
    }
}

