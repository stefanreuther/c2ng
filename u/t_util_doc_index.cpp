/**
  *  \file u/t_util_doc_index.cpp
  *  \brief Test for util::doc::Index
  */

#include "util/doc/index.hpp"

#include "t_util_doc.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/except/fileproblemexception.hpp"

using afl::except::FileProblemException;
using afl::io::ConstMemoryStream;
using afl::io::InternalStream;
using util::doc::Index;

namespace {
    String_t simplify(afl::base::ConstBytes_t b)
    {
        String_t result;
        while (const uint8_t* pc = b.eat()) {
            char ch = static_cast<char>(*pc);
            if (ch != '\r' && ch != '\n' && ch != ' ' && ch != '\t') {
                result += ch;
            }
        }
        return result;
    }

    void testLoad(const char* text)
    {
        ConstMemoryStream ms(afl::string::toBytes(text));
        Index idx;
        idx.load(ms);
    }

    const Index::TaggedNode* findTag(const std::vector<Index::TaggedNode>& vec, int tag)
    {
        for (size_t i = 0; i < vec.size(); ++i) {
            if (tag == vec[i].tag) {
                return &vec[i];
            }
        }
        return 0;
    }

    bool checkTag(const std::vector<Index::TaggedNode>& vec, int tag, Index::Handle_t hdl)
    {
        const Index::TaggedNode* n = findTag(vec, tag);
        return n != 0 && n->node == hdl;
    }
}


/** Test behaviour of empty index. */
void
TestUtilDocIndex::testEmpty()
{
    Index testee;
    Index::Handle_t h = testee.root();
    TS_ASSERT_EQUALS(testee.isNodePage(h), false);
    TS_ASSERT_EQUALS(testee.getNodeTitle(h), "");
    TS_ASSERT_EQUALS(testee.getNodeContentId(h), "");
    TS_ASSERT_EQUALS(testee.getNumNodeIds(h), 0U);
    TS_ASSERT_EQUALS(testee.getNumNodeChildren(h), 0U);
}

/** Test saving of empty index. */
void
TestUtilDocIndex::testEmptySave()
{
    InternalStream str;

    // Save
    {
        Index testee;
        testee.save(str);
    }

    // Verify content
    TS_ASSERT_EQUALS(simplify(str.getContent()), "<index/>");

    // Load
    Index other;
    str.setPos(0);
    other.load(str);

    Index::Handle_t h = other.root();
    TS_ASSERT_EQUALS(other.isNodePage(h), false);
}

/** Test building and verifying a tree. */
void
TestUtilDocIndex::testBuild()
{
    typedef std::vector<Index::TaggedNode> NodeVector_t;

    // Tree:
    //   (root)
    //     group
    //       doc1
    //         page1a
    //       doc2
    //         page2a
    //           page2aa
    //         page2b
    Index testee;
    Index::Handle_t group   = testee.addDocument(testee.root(), "group", "Group", Index::ObjectId_t());
    Index::Handle_t doc1    = testee.addDocument(group, "doc1", "First", Index::ObjectId_t());
    Index::Handle_t doc2    = testee.addDocument(group, "doc2", "Second", Index::ObjectId_t());
    Index::Handle_t page1a  = testee.addPage(doc1, "page1", "First Page", "p1");
    Index::Handle_t page2a  = testee.addPage(doc2, "page2a", "Second doc, first page", "p2a");
    Index::Handle_t page2aa = testee.addPage(page2a, "page2aa", "Second doc, sub-page", "p2aa");
    Index::Handle_t page2b  = testee.addPage(doc2, "page2b", "Second doc, second page", "p2b");

    // Verify properties of root
    NodeVector_t rootContext = testee.getNodeNavigationContext(testee.root());
    TS_ASSERT_EQUALS(testee.getNumNodeChildren(testee.root()), 1U);
    TS_ASSERT_EQUALS(testee.getNodeChildByIndex(testee.root(), 0), group);
    TS_ASSERT(!findTag(rootContext, Index::NAV_UP));
    TS_ASSERT(!findTag(rootContext, Index::NAV_NEXT_DIRECT));
    TS_ASSERT(checkTag(rootContext, Index::NAV_NEXT_INDIRECT, group));
    TS_ASSERT(!findTag(rootContext, Index::NAV_PREVIOUS_DIRECT));
    TS_ASSERT(!findTag(rootContext, Index::NAV_PREVIOUS_INDIRECT));

    // Verify properties of group
    NodeVector_t groupContext = testee.getNodeNavigationContext(group);
    TS_ASSERT_EQUALS(testee.getNumNodeChildren(group), 2U);
    TS_ASSERT_EQUALS(testee.getNodeChildByIndex(group, 0), doc1);
    TS_ASSERT_EQUALS(testee.getNodeChildByIndex(group, 1), doc2);
    TS_ASSERT_EQUALS(testee.getNodeAddress(group, String_t()), "group");
    TS_ASSERT_EQUALS(testee.getNodeParentIndex(group), 0U);
    TS_ASSERT_EQUALS(testee.getNodeContainingDocument(group), group);
    TS_ASSERT(checkTag(groupContext, Index::NAV_UP, testee.root()));
    TS_ASSERT(!findTag(groupContext, Index::NAV_NEXT_DIRECT));
    TS_ASSERT(checkTag(groupContext, Index::NAV_NEXT_INDIRECT, doc1));
    TS_ASSERT(!findTag(groupContext, Index::NAV_PREVIOUS_DIRECT));
    TS_ASSERT(checkTag(groupContext, Index::NAV_PREVIOUS_INDIRECT, testee.root()));

    // Verify properties of doc1
    NodeVector_t doc1Context = testee.getNodeNavigationContext(doc1);
    TS_ASSERT_EQUALS(testee.getNumNodeChildren(doc1), 1U);
    TS_ASSERT_EQUALS(testee.getNodeChildByIndex(doc1, 0), page1a);
    TS_ASSERT_EQUALS(testee.getNodeAddress(doc1, String_t()), "doc1");
    TS_ASSERT_EQUALS(testee.getNodeParentIndex(doc1), 0U);
    TS_ASSERT_EQUALS(testee.getNodeContainingDocument(doc1), doc1);
    TS_ASSERT(checkTag(doc1Context, Index::NAV_UP, group));
    TS_ASSERT(checkTag(doc1Context, Index::NAV_NEXT_DIRECT, doc2));
    TS_ASSERT(checkTag(doc1Context, Index::NAV_NEXT_INDIRECT, page1a));
    TS_ASSERT(!findTag(doc1Context, Index::NAV_PREVIOUS_DIRECT));
    TS_ASSERT(checkTag(doc1Context, Index::NAV_PREVIOUS_INDIRECT, group));

    // Verify properties of doc2
    NodeVector_t doc2Context = testee.getNodeNavigationContext(doc2);
    TS_ASSERT_EQUALS(testee.getNumNodeChildren(doc2), 2U);
    TS_ASSERT_EQUALS(testee.getNodeChildByIndex(doc2, 0), page2a);
    TS_ASSERT_EQUALS(testee.getNodeChildByIndex(doc2, 1), page2b);
    TS_ASSERT_EQUALS(testee.getNodeAddress(doc2, String_t()), "doc2");
    TS_ASSERT_EQUALS(testee.getNodeParentIndex(doc2), 1U);
    TS_ASSERT_EQUALS(testee.getNodeContainingDocument(doc2), doc2);
    TS_ASSERT(checkTag(doc2Context, Index::NAV_UP, group));
    TS_ASSERT(!findTag(doc2Context, Index::NAV_NEXT_DIRECT));
    TS_ASSERT(checkTag(doc2Context, Index::NAV_NEXT_INDIRECT, page2a));
    TS_ASSERT(checkTag(doc2Context, Index::NAV_PREVIOUS_DIRECT, doc1));
    TS_ASSERT(checkTag(doc2Context, Index::NAV_PREVIOUS_INDIRECT, page1a));

    // Verify properties of page1a
    NodeVector_t page1aContext = testee.getNodeNavigationContext(page1a);
    TS_ASSERT_EQUALS(testee.getNumNodeChildren(page1a), 0U);
    TS_ASSERT_EQUALS(testee.getNodeAddress(page1a, String_t()), "doc1/page1");
    TS_ASSERT_EQUALS(testee.getNodeParentIndex(page1a), 0U);
    TS_ASSERT_EQUALS(testee.getNodeContainingDocument(page1a), doc1);
    TS_ASSERT(checkTag(page1aContext, Index::NAV_UP, doc1));
    TS_ASSERT(!findTag(page1aContext, Index::NAV_NEXT_DIRECT));
    TS_ASSERT(checkTag(page1aContext, Index::NAV_NEXT_INDIRECT, doc2));
    TS_ASSERT(!findTag(page1aContext, Index::NAV_PREVIOUS_DIRECT));
    TS_ASSERT(checkTag(page1aContext, Index::NAV_PREVIOUS_INDIRECT, doc1));

    // Verify properties of page2a
    NodeVector_t page2aContext = testee.getNodeNavigationContext(page2a);
    TS_ASSERT_EQUALS(testee.getNumNodeChildren(page2a), 1U);
    TS_ASSERT_EQUALS(testee.getNodeChildByIndex(page2a, 0), page2aa);
    TS_ASSERT_EQUALS(testee.getNodeAddress(page2a, String_t()), "doc2/page2a");
    TS_ASSERT_EQUALS(testee.getNodeParentIndex(page2a), 0U);
    TS_ASSERT_EQUALS(testee.getNodeContainingDocument(page2a), doc2);
    TS_ASSERT(checkTag(page2aContext, Index::NAV_UP, doc2));
    TS_ASSERT(checkTag(page2aContext, Index::NAV_NEXT_DIRECT, page2b));
    TS_ASSERT(checkTag(page2aContext, Index::NAV_NEXT_INDIRECT, page2aa));
    TS_ASSERT(!findTag(page2aContext, Index::NAV_PREVIOUS_DIRECT));
    TS_ASSERT(checkTag(page2aContext, Index::NAV_PREVIOUS_INDIRECT, doc2));

    // Verify properties of page2aa
    NodeVector_t page2aaContext = testee.getNodeNavigationContext(page2aa);
    TS_ASSERT_EQUALS(testee.getNumNodeChildren(page2aa), 0U);
    TS_ASSERT_EQUALS(testee.getNodeAddress(page2aa, String_t()), "doc2/page2aa");
    TS_ASSERT_EQUALS(testee.getNodeParentIndex(page2aa), 0U);
    TS_ASSERT_EQUALS(testee.getNodeContainingDocument(page2aa), doc2);
    TS_ASSERT(checkTag(page2aaContext, Index::NAV_UP, page2a));
    TS_ASSERT(!findTag(page2aaContext, Index::NAV_NEXT_DIRECT));
    TS_ASSERT(checkTag(page2aaContext, Index::NAV_NEXT_INDIRECT, page2b));
    TS_ASSERT(!findTag(page2aaContext, Index::NAV_PREVIOUS_DIRECT));
    TS_ASSERT(checkTag(page2aaContext, Index::NAV_PREVIOUS_INDIRECT, page2a));

    // Verify properties of page2b
    NodeVector_t page2bContext = testee.getNodeNavigationContext(page2b);
    TS_ASSERT_EQUALS(testee.getNumNodeChildren(page2b), 0U);
    TS_ASSERT_EQUALS(testee.getNodeAddress(page2b, String_t()), "doc2/page2b");
    TS_ASSERT_EQUALS(testee.getNodeParentIndex(page2b), 1U);
    TS_ASSERT_EQUALS(testee.getNodeContainingDocument(page2b), doc2);
    TS_ASSERT(checkTag(page2bContext, Index::NAV_UP, doc2));
    TS_ASSERT(!findTag(page2bContext, Index::NAV_NEXT_DIRECT));
    TS_ASSERT(!findTag(page2bContext, Index::NAV_NEXT_INDIRECT));
    TS_ASSERT(checkTag(page2bContext, Index::NAV_PREVIOUS_DIRECT, page2a));
    TS_ASSERT(checkTag(page2bContext, Index::NAV_PREVIOUS_INDIRECT, page2aa));

    // Verify getNodeParents
    std::vector<Index::Handle_t> path = testee.getNodeParents(page2aa);
    TS_ASSERT_EQUALS(path.size(), 4U);
    TS_ASSERT_EQUALS(path[0], testee.root());
    TS_ASSERT_EQUALS(path[1], group);
    TS_ASSERT_EQUALS(path[2], doc2);
    TS_ASSERT_EQUALS(path[3], page2a);

    // Verify lookup
    Index::Handle_t out;
    String_t docOut;
    TS_ASSERT(testee.findNodeByAddress("group", out, docOut));
    TS_ASSERT_EQUALS(out, group);
    TS_ASSERT_EQUALS(docOut, "group");
    TS_ASSERT(testee.findNodeByAddress("doc1", out, docOut));
    TS_ASSERT_EQUALS(out, doc1);
    TS_ASSERT_EQUALS(docOut, "doc1");
    TS_ASSERT(testee.findNodeByAddress("doc1/page1", out, docOut));
    TS_ASSERT_EQUALS(out, page1a);
    TS_ASSERT_EQUALS(docOut, "doc1");
    TS_ASSERT(testee.findNodeByAddress("doc2/page2aa", out, docOut));
    TS_ASSERT_EQUALS(out, page2aa);
    TS_ASSERT_EQUALS(docOut, "doc2");

    TS_ASSERT(!testee.findNodeByAddress("", out, docOut));
    TS_ASSERT(!testee.findNodeByAddress("group/doc1", out, docOut));
    TS_ASSERT(!testee.findNodeByAddress("group/page1", out, docOut));
    TS_ASSERT(!testee.findNodeByAddress("doc1/", out, docOut));
    TS_ASSERT(!testee.findNodeByAddress("doc1/doc1", out, docOut));
    TS_ASSERT(!testee.findNodeByAddress("doc1/page2aa", out, docOut));

    // Verify table of content
    {
        // Root -> shows documents
        NodeVector_t groupDir = testee.getNodeChildren(testee.root(), 1000, false);
        TS_ASSERT_EQUALS(groupDir.size(), 3U);
        TS_ASSERT_EQUALS(groupDir[0].tag, 1);
        TS_ASSERT_EQUALS(groupDir[0].node, group);
        TS_ASSERT_EQUALS(groupDir[1].tag, 2);
        TS_ASSERT_EQUALS(groupDir[1].node, doc1);
        TS_ASSERT_EQUALS(groupDir[2].tag, 2);
        TS_ASSERT_EQUALS(groupDir[2].node, doc2);
    }

    {
        // Group -> shows documents
        NodeVector_t docDir = testee.getNodeChildren(group, 2, false);
        TS_ASSERT_EQUALS(docDir.size(), 2U);
        TS_ASSERT_EQUALS(docDir[0].tag, 1);
        TS_ASSERT_EQUALS(docDir[0].node, doc1);
        TS_ASSERT_EQUALS(docDir[1].tag, 1);
        TS_ASSERT_EQUALS(docDir[1].node, doc2);
    }

    {
        // Group -> shows children when requested
        NodeVector_t docDir = testee.getNodeChildren(group, 2, true);
        TS_ASSERT_EQUALS(docDir.size(), 5U);
        TS_ASSERT_EQUALS(docDir[0].tag, 1);
        TS_ASSERT_EQUALS(docDir[0].node, doc1);
        TS_ASSERT_EQUALS(docDir[1].tag, 2);
        TS_ASSERT_EQUALS(docDir[1].node, page1a);
        TS_ASSERT_EQUALS(docDir[2].tag, 1);
        TS_ASSERT_EQUALS(docDir[2].node, doc2);
        TS_ASSERT_EQUALS(docDir[3].tag, 2);
        TS_ASSERT_EQUALS(docDir[3].node, page2a);
        TS_ASSERT_EQUALS(docDir[4].tag, 2);
        TS_ASSERT_EQUALS(docDir[4].node, page2b);
    }

    {
        // Document -> shows all children
        NodeVector_t docDir = testee.getNodeChildren(doc2, 2, false);
        TS_ASSERT_EQUALS(docDir.size(), 3U);
        TS_ASSERT_EQUALS(docDir[0].tag, 1);
        TS_ASSERT_EQUALS(docDir[0].node, page2a);
        TS_ASSERT_EQUALS(docDir[1].tag, 2);
        TS_ASSERT_EQUALS(docDir[1].node, page2aa);
        TS_ASSERT_EQUALS(docDir[2].tag, 1);
        TS_ASSERT_EQUALS(docDir[2].node, page2b);
    }
}

/** Test setting, retrieving and persisting attributes. */
void
TestUtilDocIndex::testAttributes()
{
    Index testee;
    Index::Handle_t doc = testee.addDocument(testee.root(), "group", "Group", "groupContent");
    Index::Handle_t page = testee.addPage(doc, "page", "Page", "pageContent");
    testee.addNodeIds(doc, "g2,g3, g4");
    testee.addNodeTags(page, "red, blue");

    TS_ASSERT_EQUALS(testee.getNumNodeIds(doc), 4U);
    TS_ASSERT_EQUALS(testee.getNodeIdByIndex(doc, 0), "group");
    TS_ASSERT_EQUALS(testee.getNodeIdByIndex(doc, 1), "g2");
    TS_ASSERT_EQUALS(testee.getNodeIdByIndex(doc, 2), "g3");
    TS_ASSERT_EQUALS(testee.getNodeIdByIndex(doc, 3), "g4");
    TS_ASSERT_EQUALS(testee.getNumNodeTags(doc), 0U);
    TS_ASSERT_EQUALS(testee.getNodeTitle(doc), "Group");
    TS_ASSERT_EQUALS(testee.getNodeContentId(doc), "groupContent");
    TS_ASSERT_EQUALS(testee.isNodePage(doc), false);
    TS_ASSERT_EQUALS(testee.getNodeAddress(doc, String_t()), "group");
    TS_ASSERT_EQUALS(testee.getNodeAddress(doc, "g4"), "g4");
    TS_ASSERT_EQUALS(testee.getNodeAddress(doc, "x"), "group");

    TS_ASSERT_EQUALS(testee.getNumNodeIds(page), 1U);
    TS_ASSERT_EQUALS(testee.getNumNodeTags(page), 2U);
    TS_ASSERT_EQUALS(testee.getNodeTagByIndex(page, 0), "red");
    TS_ASSERT_EQUALS(testee.getNodeTagByIndex(page, 1), "blue");
    TS_ASSERT_EQUALS(testee.getNodeTitle(page), "Page");
    TS_ASSERT_EQUALS(testee.getNodeContentId(page), "pageContent");
    TS_ASSERT_EQUALS(testee.isNodePage(page), true);
    TS_ASSERT_EQUALS(testee.getNodeAddress(page, String_t()), "group/page");
    TS_ASSERT_EQUALS(testee.getNodeAddress(page, "g4"), "g4/page");
    TS_ASSERT_EQUALS(testee.getNodeAddress(page, "x"), "group/page");

    testee.setNodeContentId(page, "newPageContent");
    testee.setNodeTitle(page, "New&Shiny");
    TS_ASSERT_EQUALS(testee.getNodeTitle(page), "New&Shiny");
    TS_ASSERT_EQUALS(testee.getNodeContentId(page), "newPageContent");

    Index::Handle_t out;
    String_t docOut;
    TS_ASSERT(testee.findNodeByAddress("group/page", out, docOut));
    TS_ASSERT_EQUALS(out, page);
    TS_ASSERT_EQUALS(docOut, "group");
    TS_ASSERT(testee.findNodeByAddress("g4/page", out, docOut));
    TS_ASSERT_EQUALS(out, page);
    TS_ASSERT_EQUALS(docOut, "g4");
    TS_ASSERT_EQUALS(testee.getNodeAddress(out, String_t()), "group/page");

    // Save
    InternalStream str;
    testee.save(str);

    // Verify content
    TS_ASSERT_EQUALS(simplify(str.getContent()),
                     "<index>"
                     "<docid=\"group,g2,g3,g4\"title=\"Group\"content=\"groupContent\">"
                     "<pageid=\"page\"tag=\"red,blue\"title=\"New&amp;Shiny\"content=\"newPageContent\"/>"
                     "</doc>"
                     "</index>");

    // Load
    Index other;
    str.setPos(0);
    other.load(str);

    TS_ASSERT_EQUALS(other.getNumNodeChildren(other.root()), 1U);
    Index::Handle_t doc1 = other.getNodeChildByIndex(other.root(), 0);
    TS_ASSERT_EQUALS(other.getNumNodeChildren(doc1), 1U);
    Index::Handle_t page1 = other.getNodeChildByIndex(doc1, 0);
    TS_ASSERT_EQUALS(other.getNumNodeChildren(page1), 0U);

    TS_ASSERT_EQUALS(testee.getNumNodeIds(doc1), 4U);
    TS_ASSERT_EQUALS(testee.getNodeIdByIndex(doc1, 3), "g4");

    TS_ASSERT_EQUALS(testee.getNumNodeTags(page1), 2U);
    TS_ASSERT_EQUALS(testee.getNodeTagByIndex(page1, 1), "blue");
}

/** Test I/O of a structure. */
void
TestUtilDocIndex::testStructureIO()
{
    Index testee;
    Index::Handle_t group     = testee.addDocument(testee.root(), "group", "Group", Index::ObjectId_t());
    Index::Handle_t doc1      = testee.addDocument(group, "doc1", "First", Index::ObjectId_t());
    /*Index::Handle_t doc2 = */ testee.addDocument(group, "doc2", "Second", Index::ObjectId_t());
    /*Index::Handle_t page1a=*/ testee.addPage(doc1, "page1a", "First Page", "p1a");
    /*Index::Handle_t page1b=*/ testee.addPage(doc1, "page1b", "Second page", "p1b");

    // Save
    InternalStream str;
    testee.save(str);

    // Verify content
    TS_ASSERT_EQUALS(simplify(str.getContent()),
                     "<index>"
                     "<docid=\"group\"title=\"Group\">"
                     "<docid=\"doc1\"title=\"First\">"
                     "<pageid=\"page1a\"title=\"FirstPage\"content=\"p1a\"/>"
                     "<pageid=\"page1b\"title=\"Secondpage\"content=\"p1b\"/>"
                     "</doc>"
                     "<docid=\"doc2\"title=\"Second\"/>"
                     "</doc>"
                     "</index>");

    // Load
    Index other;
    str.setPos(0);
    other.load(str);

    TS_ASSERT_EQUALS(other.getNumNodeChildren(other.root()), 1U);
    Index::Handle_t otherGroup = testee.getNodeChildByIndex(other.root(), 0);
    TS_ASSERT_EQUALS(other.getNumNodeChildren(otherGroup), 2U);
    Index::Handle_t otherDoc1 = testee.getNodeChildByIndex(otherGroup, 0);
    TS_ASSERT_EQUALS(other.getNumNodeChildren(otherDoc1), 2U);
    Index::Handle_t otherDoc2 = testee.getNodeChildByIndex(otherGroup, 1);
    TS_ASSERT_EQUALS(other.getNumNodeChildren(otherDoc2), 0U);

    TS_ASSERT_EQUALS(other.getNodeTitle(other.getNodeChildByIndex(otherDoc1, 0)), "First Page");
}

/** Test syntax errors in loading. */
void
TestUtilDocIndex::testErrors()
{
    // Base case: empty
    TS_ASSERT_THROWS_NOTHING(testLoad(""));
    TS_ASSERT_THROWS_NOTHING(testLoad("<index/>"));

    // Misplaced <index>
    TS_ASSERT_THROWS(testLoad("<index><index /></index>"), FileProblemException);

    // Misplaced <doc>
    TS_ASSERT_THROWS(testLoad("<doc id=\"a\"></doc>"), FileProblemException);
    TS_ASSERT_THROWS(testLoad("<index><what><doc id=\"a\"></doc></what></index>"), FileProblemException);
    TS_ASSERT_THROWS(testLoad("<index><doc id=\"a\"><page id=\"b\"><doc id=\"c\"></doc></page></doc></index>"), FileProblemException);

    // Misplaced <page>
    TS_ASSERT_THROWS(testLoad("<page id=\"a\"></doc>"), FileProblemException);
    TS_ASSERT_THROWS(testLoad("<index><page id=\"a\"></doc></index>"), FileProblemException);

    // Misplaced close
    TS_ASSERT_THROWS(testLoad("</page>"), FileProblemException);

    // Mismatching close
    TS_ASSERT_THROWS(testLoad("<index></page>"), FileProblemException);
    TS_ASSERT_THROWS(testLoad("<index><doc id=\"a\"></page>"), FileProblemException);
    TS_ASSERT_THROWS(testLoad("<index><doc id=\"a\"><page id=\"b\"></doc>"), FileProblemException);

    // Missing id
    TS_ASSERT_THROWS(testLoad("<index><doc></doc></index>"), FileProblemException);
    TS_ASSERT_THROWS(testLoad("<index><doc id=\"a\"><page></page></doc></index>"), FileProblemException);

    // Syntax error
    TS_ASSERT_THROWS(testLoad("<![FOOBAR["), FileProblemException);

    // Missing closing tag
    TS_ASSERT_THROWS(testLoad("<index>"), FileProblemException);
}

void
TestUtilDocIndex::testRelated()
{
    Index testee;
    Index::Handle_t d1 = testee.addDocument(testee.root(), "d1", "One", Index::ObjectId_t());
    Index::Handle_t p1 = testee.addPage(d1, "pg", "Page", Index::ObjectId_t());
    Index::Handle_t p1a = testee.addPage(d1, "pg2", "Other", Index::ObjectId_t());

    Index::Handle_t d2 = testee.addDocument(testee.root(), "d2", "Two", Index::ObjectId_t());
    Index::Handle_t p2 = testee.addPage(d2, "pg", "Page", Index::ObjectId_t());

    Index::Handle_t d2a = testee.addDocument(d2, "d2a", "Two again", Index::ObjectId_t());
    Index::Handle_t p2a = testee.addPage(d2a, "pg", "Page", Index::ObjectId_t());

    // Alteratives to p1,p2,p2a are p1,p2,p2a
    {
        std::vector<Index::RelatedNode> r = testee.getNodeRelatedVersions(p1);
        TS_ASSERT_EQUALS(r.size(), 3U);
        TS_ASSERT_EQUALS(r[0].node, p1);
        TS_ASSERT_EQUALS(r[0].docNode, d1);
        TS_ASSERT_EQUALS(r[1].node, p2);
        TS_ASSERT_EQUALS(r[1].docNode, d2);
        TS_ASSERT_EQUALS(r[2].node, p2a);
        TS_ASSERT_EQUALS(r[2].docNode, d2a);
    }
    {
        std::vector<Index::RelatedNode> r = testee.getNodeRelatedVersions(p2);
        TS_ASSERT_EQUALS(r.size(), 3U);
        TS_ASSERT_EQUALS(r[0].node, p1);
        TS_ASSERT_EQUALS(r[0].docNode, d1);
        TS_ASSERT_EQUALS(r[1].node, p2);
        TS_ASSERT_EQUALS(r[1].docNode, d2);
        TS_ASSERT_EQUALS(r[2].node, p2a);
        TS_ASSERT_EQUALS(r[2].docNode, d2a);
    }
    {
        std::vector<Index::RelatedNode> r = testee.getNodeRelatedVersions(p2a);
        TS_ASSERT_EQUALS(r.size(), 3U);
        TS_ASSERT_EQUALS(r[0].node, p1);
        TS_ASSERT_EQUALS(r[0].docNode, d1);
        TS_ASSERT_EQUALS(r[1].node, p2);
        TS_ASSERT_EQUALS(r[1].docNode, d2);
        TS_ASSERT_EQUALS(r[2].node, p2a);
        TS_ASSERT_EQUALS(r[2].docNode, d2a);
    }

    // Alternative to p1a is only p1a itself
    {
        std::vector<Index::RelatedNode> r = testee.getNodeRelatedVersions(p1a);
        TS_ASSERT_EQUALS(r.size(), 1U);
        TS_ASSERT_EQUALS(r[0].node, p1a);
        TS_ASSERT_EQUALS(r[0].docNode, d1);
    }

    // No alternatives to any of the documents
    TS_ASSERT_EQUALS(testee.getNodeRelatedVersions(d1).size(), 0U);
    TS_ASSERT_EQUALS(testee.getNodeRelatedVersions(d2).size(), 0U);
    TS_ASSERT_EQUALS(testee.getNodeRelatedVersions(d2a).size(), 0U);
    TS_ASSERT_EQUALS(testee.getNodeRelatedVersions(testee.root()).size(), 0U);
}

/* Test handling of non-ASCII characters in index file */
void
TestUtilDocIndex::testCharset()
{
    const String_t TEXT = "\xc3\xa4";
    Index testee;
    testee.addDocument(testee.root(), "doc-id", TEXT, Index::ObjectId_t());

    // Save
    InternalStream str;
    testee.save(str);

    // Verify content
    TS_ASSERT_EQUALS(simplify(str.getContent()), "<index><docid=\"doc-id\"title=\""+TEXT+"\"/></index>");

    // Reload
    Index copy;
    str.setPos(0);
    copy.load(str);

    // Verify loaded content
    TS_ASSERT_EQUALS(copy.getNumNodeChildren(copy.root()), 1U);
    Index::Handle_t copyId = copy.getNodeChildByIndex(copy.root(), 0);
    TS_ASSERT_EQUALS(copy.getNodeTitle(copyId), TEXT);
}

