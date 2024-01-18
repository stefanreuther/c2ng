/**
  *  \file test/util/doc/indextest.cpp
  *  \brief Test for util::doc::Index
  */

#include "util/doc/index.hpp"

#include "afl/except/fileproblemexception.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/test/testrunner.hpp"

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
AFL_TEST("util.doc.Index:empty", a)
{
    Index testee;
    Index::Handle_t h = testee.root();
    a.checkEqual("01. isNodePage",         testee.isNodePage(h), false);
    a.checkEqual("02. getNodeTitle",       testee.getNodeTitle(h), "");
    a.checkEqual("03. getNodeContentId",   testee.getNodeContentId(h), "");
    a.checkEqual("04. getNumNodeIds",      testee.getNumNodeIds(h), 0U);
    a.checkEqual("05. getNumNodeChildren", testee.getNumNodeChildren(h), 0U);
}

/** Test saving of empty index. */
AFL_TEST("util.doc.Index:empty:save", a)
{
    InternalStream str;

    // Save
    {
        Index testee;
        testee.save(str);
    }

    // Verify content
    a.checkEqual("01. getContent", simplify(str.getContent()), "<index/>");

    // Load
    Index other;
    str.setPos(0);
    other.load(str);

    Index::Handle_t h = other.root();
    a.checkEqual("11. isNodePage", other.isNodePage(h), false);
}

/** Test building and verifying a tree. */
AFL_TEST("util.doc.Index:build", a)
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
    a.checkEqual("01. getNumNodeChildren",     testee.getNumNodeChildren(testee.root()), 1U);
    a.checkEqual("02. getNodeChildByIndex",    testee.getNodeChildByIndex(testee.root(), 0), group);
    a.check     ("03. NAV_UP",                !findTag(rootContext, Index::NAV_UP));
    a.check     ("04. NAV_NEXT_DIRECT",       !findTag(rootContext, Index::NAV_NEXT_DIRECT));
    a.check     ("05. NAV_NEXT_INDIRECT",      checkTag(rootContext, Index::NAV_NEXT_INDIRECT, group));
    a.check     ("06. NAV_PREVIOUS_DIRECT",   !findTag(rootContext, Index::NAV_PREVIOUS_DIRECT));
    a.check     ("07. NAV_PREVIOUS_INDIRECT", !findTag(rootContext, Index::NAV_PREVIOUS_INDIRECT));

    // Verify properties of group
    NodeVector_t groupContext =                   testee.getNodeNavigationContext(group);
    a.checkEqual("11. getNumNodeChildren",        testee.getNumNodeChildren(group), 2U);
    a.checkEqual("12. getNodeChildByIndex",       testee.getNodeChildByIndex(group, 0), doc1);
    a.checkEqual("13. getNodeChildByIndex",       testee.getNodeChildByIndex(group, 1), doc2);
    a.checkEqual("14. getNodeAddress",            testee.getNodeAddress(group, String_t()), "group");
    a.checkEqual("15. getNodeParentIndex",        testee.getNodeParentIndex(group), 0U);
    a.checkEqual("16. getNodeContainingDocument", testee.getNodeContainingDocument(group), group);
    a.check     ("17. NAV_UP",                    checkTag(groupContext, Index::NAV_UP, testee.root()));
    a.check     ("18. NAV_NEXT_DIRECT",          !findTag(groupContext, Index::NAV_NEXT_DIRECT));
    a.check     ("19. NAV_NEXT_INDIRECT",         checkTag(groupContext, Index::NAV_NEXT_INDIRECT, doc1));
    a.check     ("20. NAV_PREVIOUS_DIRECT",      !findTag(groupContext, Index::NAV_PREVIOUS_DIRECT));
    a.check     ("21. NAV_PREVIOUS_INDIRECT",     checkTag(groupContext, Index::NAV_PREVIOUS_INDIRECT, testee.root()));

    // Verify properties of doc1
    NodeVector_t doc1Context =                    testee.getNodeNavigationContext(doc1);
    a.checkEqual("31. getNumNodeChildren",        testee.getNumNodeChildren(doc1), 1U);
    a.checkEqual("32. getNodeChildByIndex",       testee.getNodeChildByIndex(doc1, 0), page1a);
    a.checkEqual("33. getNodeAddress",            testee.getNodeAddress(doc1, String_t()), "doc1");
    a.checkEqual("34. getNodeParentIndex",        testee.getNodeParentIndex(doc1), 0U);
    a.checkEqual("35. getNodeContainingDocument", testee.getNodeContainingDocument(doc1), doc1);
    a.check     ("36. NAV_UP",                    checkTag(doc1Context, Index::NAV_UP, group));
    a.check     ("37. NAV_NEXT_DIRECT",           checkTag(doc1Context, Index::NAV_NEXT_DIRECT, doc2));
    a.check     ("38. NAV_NEXT_INDIRECT",         checkTag(doc1Context, Index::NAV_NEXT_INDIRECT, page1a));
    a.check     ("39. NAV_PREVIOUS_DIRECT",      !findTag(doc1Context, Index::NAV_PREVIOUS_DIRECT));
    a.check     ("40. NAV_PREVIOUS_INDIRECT",     checkTag(doc1Context, Index::NAV_PREVIOUS_INDIRECT, group));

    // Verify properties of doc2
    NodeVector_t doc2Context =                    testee.getNodeNavigationContext(doc2);
    a.checkEqual("41. getNumNodeChildren",        testee.getNumNodeChildren(doc2), 2U);
    a.checkEqual("42. getNodeChildByIndex",       testee.getNodeChildByIndex(doc2, 0), page2a);
    a.checkEqual("43. getNodeChildByIndex",       testee.getNodeChildByIndex(doc2, 1), page2b);
    a.checkEqual("44. getNodeAddress",            testee.getNodeAddress(doc2, String_t()), "doc2");
    a.checkEqual("45. getNodeParentIndex",        testee.getNodeParentIndex(doc2), 1U);
    a.checkEqual("46. getNodeContainingDocument", testee.getNodeContainingDocument(doc2), doc2);
    a.check     ("47. NAV_UP",                    checkTag(doc2Context, Index::NAV_UP, group));
    a.check     ("48. NAV_NEXT_DIRECT",          !findTag(doc2Context, Index::NAV_NEXT_DIRECT));
    a.check     ("49. NAV_NEXT_INDIRECT",         checkTag(doc2Context, Index::NAV_NEXT_INDIRECT, page2a));
    a.check     ("50. NAV_PREVIOUS_DIRECT",       checkTag(doc2Context, Index::NAV_PREVIOUS_DIRECT, doc1));
    a.check     ("51. NAV_PREVIOUS_INDIRECT",     checkTag(doc2Context, Index::NAV_PREVIOUS_INDIRECT, page1a));

    // Verify properties of page1a
    NodeVector_t page1aContext =                  testee.getNodeNavigationContext(page1a);
    a.checkEqual("61. getNumNodeChildren",        testee.getNumNodeChildren(page1a), 0U);
    a.checkEqual("62. getNodeAddress",            testee.getNodeAddress(page1a, String_t()), "doc1/page1");
    a.checkEqual("63. getNodeParentIndex",        testee.getNodeParentIndex(page1a), 0U);
    a.checkEqual("64. getNodeContainingDocument", testee.getNodeContainingDocument(page1a), doc1);
    a.check     ("65. NAV_UP",                    checkTag(page1aContext, Index::NAV_UP, doc1));
    a.check     ("66. NAV_NEXT_DIRECT",          !findTag(page1aContext, Index::NAV_NEXT_DIRECT));
    a.check     ("67. NAV_NEXT_INDIRECT",         checkTag(page1aContext, Index::NAV_NEXT_INDIRECT, doc2));
    a.check     ("68. NAV_PREVIOUS_DIRECT",      !findTag(page1aContext, Index::NAV_PREVIOUS_DIRECT));
    a.check     ("69. NAV_PREVIOUS_INDIRECT",     checkTag(page1aContext, Index::NAV_PREVIOUS_INDIRECT, doc1));

    // Verify properties of page2a
    NodeVector_t page2aContext =                  testee.getNodeNavigationContext(page2a);
    a.checkEqual("71. getNumNodeChildren",        testee.getNumNodeChildren(page2a), 1U);
    a.checkEqual("72. getNodeChildByIndex",       testee.getNodeChildByIndex(page2a, 0), page2aa);
    a.checkEqual("73. getNodeAddress",            testee.getNodeAddress(page2a, String_t()), "doc2/page2a");
    a.checkEqual("74. getNodeParentIndex",        testee.getNodeParentIndex(page2a), 0U);
    a.checkEqual("75. getNodeContainingDocument", testee.getNodeContainingDocument(page2a), doc2);
    a.check     ("76. NAV_UP",                    checkTag(page2aContext, Index::NAV_UP, doc2));
    a.check     ("77. NAV_NEXT_DIRECT",           checkTag(page2aContext, Index::NAV_NEXT_DIRECT, page2b));
    a.check     ("78. NAV_NEXT_INDIRECT",         checkTag(page2aContext, Index::NAV_NEXT_INDIRECT, page2aa));
    a.check     ("79. NAV_PREVIOUS_DIRECT",      !findTag(page2aContext, Index::NAV_PREVIOUS_DIRECT));
    a.check     ("80. NAV_PREVIOUS_INDIRECT",     checkTag(page2aContext, Index::NAV_PREVIOUS_INDIRECT, doc2));

    // Verify properties of page2aa
    NodeVector_t page2aaContext =                 testee.getNodeNavigationContext(page2aa);
    a.checkEqual("81. getNumNodeChildren",        testee.getNumNodeChildren(page2aa), 0U);
    a.checkEqual("82. getNodeAddress",            testee.getNodeAddress(page2aa, String_t()), "doc2/page2aa");
    a.checkEqual("83. getNodeParentIndex",        testee.getNodeParentIndex(page2aa), 0U);
    a.checkEqual("84. getNodeContainingDocument", testee.getNodeContainingDocument(page2aa), doc2);
    a.check     ("85. NAV_UP",                    checkTag(page2aaContext, Index::NAV_UP, page2a));
    a.check     ("86. NAV_NEXT_DIRECT",          !findTag(page2aaContext, Index::NAV_NEXT_DIRECT));
    a.check     ("87. NAV_NEXT_INDIRECT",         checkTag(page2aaContext, Index::NAV_NEXT_INDIRECT, page2b));
    a.check     ("88. NAV_PREVIOUS_DIRECT",      !findTag(page2aaContext, Index::NAV_PREVIOUS_DIRECT));
    a.check     ("89. NAV_PREVIOUS_INDIRECT",     checkTag(page2aaContext, Index::NAV_PREVIOUS_INDIRECT, page2a));

    // Verify properties of page2b
    NodeVector_t page2bContext =                  testee.getNodeNavigationContext(page2b);
    a.checkEqual("91. getNumNodeChildren",        testee.getNumNodeChildren(page2b), 0U);
    a.checkEqual("92. getNodeAddress",            testee.getNodeAddress(page2b, String_t()), "doc2/page2b");
    a.checkEqual("93. getNodeParentIndex",        testee.getNodeParentIndex(page2b), 1U);
    a.checkEqual("94. getNodeContainingDocument", testee.getNodeContainingDocument(page2b), doc2);
    a.check     ("95. NAV_UP",                    checkTag(page2bContext, Index::NAV_UP, doc2));
    a.check     ("96. NAV_NEXT_DIRECT",          !findTag(page2bContext, Index::NAV_NEXT_DIRECT));
    a.check     ("97. NAV_NEXT_INDIRECT",        !findTag(page2bContext, Index::NAV_NEXT_INDIRECT));
    a.check     ("98. NAV_PREVIOUS_DIRECT",       checkTag(page2bContext, Index::NAV_PREVIOUS_DIRECT, page2a));
    a.check     ("99. NAV_PREVIOUS_INDIRECT",     checkTag(page2bContext, Index::NAV_PREVIOUS_INDIRECT, page2aa));

    // Verify getNodeParents
    std::vector<Index::Handle_t> path = testee.getNodeParents(page2aa);
    a.checkEqual("101. size", path.size(), 4U);
    a.checkEqual("102. path", path[0], testee.root());
    a.checkEqual("103. path", path[1], group);
    a.checkEqual("104. path", path[2], doc2);
    a.checkEqual("105. path", path[3], page2a);

    // Verify lookup
    Index::Handle_t out;
    String_t docOut;
    a.check     ("111. findNodeByAddress", testee.findNodeByAddress("group", out, docOut));
    a.checkEqual("112. out", out, group);
    a.checkEqual("113. docOut", docOut, "group");
    a.check     ("114. findNodeByAddress", testee.findNodeByAddress("doc1", out, docOut));
    a.checkEqual("115. out", out, doc1);
    a.checkEqual("116. docOut", docOut, "doc1");
    a.check     ("117. findNodeByAddress", testee.findNodeByAddress("doc1/page1", out, docOut));
    a.checkEqual("118. out", out, page1a);
    a.checkEqual("119. docOut", docOut, "doc1");
    a.check     ("120. findNodeByAddress", testee.findNodeByAddress("doc2/page2aa", out, docOut));
    a.checkEqual("121. out", out, page2aa);
    a.checkEqual("122. docOut", docOut, "doc2");

    a.check("131. findNodeByAddress", !testee.findNodeByAddress("", out, docOut));
    a.check("132. findNodeByAddress", !testee.findNodeByAddress("group/doc1", out, docOut));
    a.check("133. findNodeByAddress", !testee.findNodeByAddress("group/page1", out, docOut));
    a.check("134. findNodeByAddress", !testee.findNodeByAddress("doc1/", out, docOut));
    a.check("135. findNodeByAddress", !testee.findNodeByAddress("doc1/doc1", out, docOut));
    a.check("136. findNodeByAddress", !testee.findNodeByAddress("doc1/page2aa", out, docOut));

    // Verify table of content
    {
        // Root -> shows documents
        NodeVector_t groupDir = testee.getNodeChildren(testee.root(), 1000, false);
        a.checkEqual("141. size", groupDir.size(), 3U);
        a.checkEqual("142. tag",  groupDir[0].tag, 1);
        a.checkEqual("143. node", groupDir[0].node, group);
        a.checkEqual("144. tag",  groupDir[1].tag, 2);
        a.checkEqual("145. node", groupDir[1].node, doc1);
        a.checkEqual("146. tag",  groupDir[2].tag, 2);
        a.checkEqual("147. node", groupDir[2].node, doc2);
    }

    {
        // Group -> shows documents
        NodeVector_t docDir = testee.getNodeChildren(group, 2, false);
        a.checkEqual("151. size", docDir.size(), 2U);
        a.checkEqual("152. tag",  docDir[0].tag, 1);
        a.checkEqual("153. node", docDir[0].node, doc1);
        a.checkEqual("154. tag",  docDir[1].tag, 1);
        a.checkEqual("155. node", docDir[1].node, doc2);
    }

    {
        // Group -> shows children when requested
        NodeVector_t docDir = testee.getNodeChildren(group, 2, true);
        a.checkEqual("161. size", docDir.size(), 5U);
        a.checkEqual("162. tag",  docDir[0].tag, 1);
        a.checkEqual("163. node", docDir[0].node, doc1);
        a.checkEqual("164. tag",  docDir[1].tag, 2);
        a.checkEqual("165. node", docDir[1].node, page1a);
        a.checkEqual("166. tag",  docDir[2].tag, 1);
        a.checkEqual("167. node", docDir[2].node, doc2);
        a.checkEqual("168. tag",  docDir[3].tag, 2);
        a.checkEqual("169. node", docDir[3].node, page2a);
        a.checkEqual("170. tag",  docDir[4].tag, 2);
        a.checkEqual("171. node", docDir[4].node, page2b);
    }

    {
        // Document -> shows all children
        NodeVector_t docDir = testee.getNodeChildren(doc2, 2, false);
        a.checkEqual("181. size", docDir.size(), 3U);
        a.checkEqual("182. tag",  docDir[0].tag, 1);
        a.checkEqual("183. node", docDir[0].node, page2a);
        a.checkEqual("184. tag",  docDir[1].tag, 2);
        a.checkEqual("185. node", docDir[1].node, page2aa);
        a.checkEqual("186. tag",  docDir[2].tag, 1);
        a.checkEqual("187. node", docDir[2].node, page2b);
    }
}

/** Test setting, retrieving and persisting attributes. */
AFL_TEST("util.doc.Index:attributes", a)
{
    Index testee;
    Index::Handle_t doc = testee.addDocument(testee.root(), "group", "Group", "groupContent");
    Index::Handle_t page = testee.addPage(doc, "page", "Page", "pageContent");
    testee.addNodeIds(doc, "g2,g3, g4");
    testee.addNodeTags(page, "red, blue");

    a.checkEqual("01. getNumNodeIds",    testee.getNumNodeIds(doc), 4U);
    a.checkEqual("02. getNodeIdByIndex", testee.getNodeIdByIndex(doc, 0), "group");
    a.checkEqual("03. getNodeIdByIndex", testee.getNodeIdByIndex(doc, 1), "g2");
    a.checkEqual("04. getNodeIdByIndex", testee.getNodeIdByIndex(doc, 2), "g3");
    a.checkEqual("05. getNodeIdByIndex", testee.getNodeIdByIndex(doc, 3), "g4");
    a.checkEqual("06. getNumNodeTags",   testee.getNumNodeTags(doc), 0U);
    a.checkEqual("07. getNodeTitle",     testee.getNodeTitle(doc), "Group");
    a.checkEqual("08. getNodeContentId", testee.getNodeContentId(doc), "groupContent");
    a.checkEqual("09. isNodePage",       testee.isNodePage(doc), false);
    a.checkEqual("10. getNodeAddress",   testee.getNodeAddress(doc, String_t()), "group");
    a.checkEqual("11. getNodeAddress",   testee.getNodeAddress(doc, "g4"), "g4");
    a.checkEqual("12. getNodeAddress",   testee.getNodeAddress(doc, "x"), "group");

    a.checkEqual("21. getNumNodeIds",     testee.getNumNodeIds(page), 1U);
    a.checkEqual("22. getNumNodeTags",    testee.getNumNodeTags(page), 2U);
    a.checkEqual("23. getNodeTagByIndex", testee.getNodeTagByIndex(page, 0), "red");
    a.checkEqual("24. getNodeTagByIndex", testee.getNodeTagByIndex(page, 1), "blue");
    a.checkEqual("25. getNodeTitle",      testee.getNodeTitle(page), "Page");
    a.checkEqual("26. getNodeContentId",  testee.getNodeContentId(page), "pageContent");
    a.checkEqual("27. isNodePage",        testee.isNodePage(page), true);
    a.checkEqual("28. getNodeAddress",    testee.getNodeAddress(page, String_t()), "group/page");
    a.checkEqual("29. getNodeAddress",    testee.getNodeAddress(page, "g4"), "g4/page");
    a.checkEqual("30. getNodeAddress",    testee.getNodeAddress(page, "x"), "group/page");

    testee.setNodeContentId(page, "newPageContent");
    testee.setNodeTitle(page, "New&Shiny");
    a.checkEqual("31. getNodeTitle",     testee.getNodeTitle(page), "New&Shiny");
    a.checkEqual("32. getNodeContentId", testee.getNodeContentId(page), "newPageContent");

    Index::Handle_t out;
    String_t docOut;
    a.check("41. findNodeByAddress", testee.findNodeByAddress("group/page", out, docOut));
    a.checkEqual("42. out", out, page);
    a.checkEqual("43. docOut", docOut, "group");
    a.check("44. findNodeByAddress", testee.findNodeByAddress("g4/page", out, docOut));
    a.checkEqual("45. out", out, page);
    a.checkEqual("46. docOut", docOut, "g4");
    a.checkEqual("47. getNodeAddress", testee.getNodeAddress(out, String_t()), "group/page");

    // Save
    InternalStream str;
    testee.save(str);

    // Verify content
    a.checkEqual("51. save", simplify(str.getContent()),
                 "<index>"
                 "<docid=\"group,g2,g3,g4\"title=\"Group\"content=\"groupContent\">"
                 "<pageid=\"page\"tag=\"red,blue\"title=\"New&amp;Shiny\"content=\"newPageContent\"/>"
                 "</doc>"
                 "</index>");

    // Load
    Index other;
    str.setPos(0);
    other.load(str);

    a.checkEqual("61. getNumNodeChildren", other.getNumNodeChildren(other.root()), 1U);
    Index::Handle_t doc1 = other.getNodeChildByIndex(other.root(), 0);
    a.checkEqual("62. getNumNodeChildren", other.getNumNodeChildren(doc1), 1U);
    Index::Handle_t page1 = other.getNodeChildByIndex(doc1, 0);
    a.checkEqual("63. getNumNodeChildren", other.getNumNodeChildren(page1), 0U);

    a.checkEqual("71. getNumNodeIds",    testee.getNumNodeIds(doc1), 4U);
    a.checkEqual("72. getNodeIdByIndex", testee.getNodeIdByIndex(doc1, 3), "g4");

    a.checkEqual("81. getNumNodeTags",    testee.getNumNodeTags(page1), 2U);
    a.checkEqual("82. getNodeTagByIndex", testee.getNodeTagByIndex(page1, 1), "blue");
}

/** Test I/O of a structure. */
AFL_TEST("util.doc.Index:structure-io", a)
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
    a.checkEqual("01. save", simplify(str.getContent()),
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

    a.checkEqual("11. getNumNodeChildren", other.getNumNodeChildren(other.root()), 1U);
    Index::Handle_t otherGroup = testee.getNodeChildByIndex(other.root(), 0);
    a.checkEqual("12. getNumNodeChildren", other.getNumNodeChildren(otherGroup), 2U);
    Index::Handle_t otherDoc1 = testee.getNodeChildByIndex(otherGroup, 0);
    a.checkEqual("13. getNumNodeChildren", other.getNumNodeChildren(otherDoc1), 2U);
    Index::Handle_t otherDoc2 = testee.getNodeChildByIndex(otherGroup, 1);
    a.checkEqual("14. getNumNodeChildren", other.getNumNodeChildren(otherDoc2), 0U);

    a.checkEqual("21. getNodeTitle", other.getNodeTitle(other.getNodeChildByIndex(otherDoc1, 0)), "First Page");
}

/** Test syntax errors in loading. */
AFL_TEST("util.doc.Index:load:error", a)
{
    // Base case: empty
    AFL_CHECK_SUCCEEDS(a("01. empty"), testLoad(""));
    AFL_CHECK_SUCCEEDS(a("02. empty"), testLoad("<index/>"));

    // Misplaced <index>
    AFL_CHECK_THROWS(a("11. misplaced index"), testLoad("<index><index /></index>"), FileProblemException);

    // Misplaced <doc>
    AFL_CHECK_THROWS(a("21. misplaced doc"), testLoad("<doc id=\"a\"></doc>"), FileProblemException);
    AFL_CHECK_THROWS(a("22. misplaced doc"), testLoad("<index><what><doc id=\"a\"></doc></what></index>"), FileProblemException);
    AFL_CHECK_THROWS(a("23. misplaced doc"), testLoad("<index><doc id=\"a\"><page id=\"b\"><doc id=\"c\"></doc></page></doc></index>"), FileProblemException);

    // Misplaced <page>
    AFL_CHECK_THROWS(a("31. misplaced page"), testLoad("<page id=\"a\"></doc>"), FileProblemException);
    AFL_CHECK_THROWS(a("32. misplaced page"), testLoad("<index><page id=\"a\"></doc></index>"), FileProblemException);

    // Misplaced close
    AFL_CHECK_THROWS(a("41. misplaced close"), testLoad("</page>"), FileProblemException);

    // Mismatching close
    AFL_CHECK_THROWS(a("51. mismatching close"), testLoad("<index></page>"), FileProblemException);
    AFL_CHECK_THROWS(a("52. mismatching close"), testLoad("<index><doc id=\"a\"></page>"), FileProblemException);
    AFL_CHECK_THROWS(a("53. mismatching close"), testLoad("<index><doc id=\"a\"><page id=\"b\"></doc>"), FileProblemException);

    // Missing id
    AFL_CHECK_THROWS(a("61. missing id"), testLoad("<index><doc></doc></index>"), FileProblemException);
    AFL_CHECK_THROWS(a("62. missing id"), testLoad("<index><doc id=\"a\"><page></page></doc></index>"), FileProblemException);

    // Syntax error
    AFL_CHECK_THROWS(a("71. syntax error"), testLoad("<![FOOBAR["), FileProblemException);

    // Missing closing tag
    AFL_CHECK_THROWS(a("81. missing close"), testLoad("<index>"), FileProblemException);
}

AFL_TEST("util.doc.Index:getNodeRelatedVersions", a)
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
        a.checkEqual("01. size", r.size(), 3U);
        a.checkEqual("02. node",    r[0].node, p1);
        a.checkEqual("03. docNode", r[0].docNode, d1);
        a.checkEqual("04. node",    r[1].node, p2);
        a.checkEqual("05. docNode", r[1].docNode, d2);
        a.checkEqual("06. node",    r[2].node, p2a);
        a.checkEqual("07. docNode", r[2].docNode, d2a);
    }
    {
        std::vector<Index::RelatedNode> r = testee.getNodeRelatedVersions(p2);
        a.checkEqual("08. size", r.size(), 3U);
        a.checkEqual("09. node",    r[0].node, p1);
        a.checkEqual("10. docNode", r[0].docNode, d1);
        a.checkEqual("11. node",    r[1].node, p2);
        a.checkEqual("12. docNode", r[1].docNode, d2);
        a.checkEqual("13. node",    r[2].node, p2a);
        a.checkEqual("14. docNode", r[2].docNode, d2a);
    }
    {
        std::vector<Index::RelatedNode> r = testee.getNodeRelatedVersions(p2a);
        a.checkEqual("15. size", r.size(), 3U);
        a.checkEqual("16. node",    r[0].node, p1);
        a.checkEqual("17. docNode", r[0].docNode, d1);
        a.checkEqual("18. node",    r[1].node, p2);
        a.checkEqual("19. docNode", r[1].docNode, d2);
        a.checkEqual("20. node",    r[2].node, p2a);
        a.checkEqual("21. docNode", r[2].docNode, d2a);
    }

    // Alternative to p1a is only p1a itself
    {
        std::vector<Index::RelatedNode> r = testee.getNodeRelatedVersions(p1a);
        a.checkEqual("31. size", r.size(), 1U);
        a.checkEqual("32. node",    r[0].node, p1a);
        a.checkEqual("33. docNode", r[0].docNode, d1);
    }

    // No alternatives to any of the documents
    a.checkEqual("41. getNodeRelatedVersions", testee.getNodeRelatedVersions(d1).size(), 0U);
    a.checkEqual("42. getNodeRelatedVersions", testee.getNodeRelatedVersions(d2).size(), 0U);
    a.checkEqual("43. getNodeRelatedVersions", testee.getNodeRelatedVersions(d2a).size(), 0U);
    a.checkEqual("44. getNodeRelatedVersions", testee.getNodeRelatedVersions(testee.root()).size(), 0U);
}

/* Test handling of non-ASCII characters in index file */
AFL_TEST("util.doc.Index:charset", a)
{
    const String_t TEXT = "\xc3\xa4";
    Index testee;
    testee.addDocument(testee.root(), "doc-id", TEXT, Index::ObjectId_t());

    // Save
    InternalStream str;
    testee.save(str);

    // Verify content
    a.checkEqual("01. save", simplify(str.getContent()), "<index><docid=\"doc-id\"title=\""+TEXT+"\"/></index>");

    // Reload
    Index copy;
    str.setPos(0);
    copy.load(str);

    // Verify loaded content
    a.checkEqual("11. getNumNodeChildren", copy.getNumNodeChildren(copy.root()), 1U);
    Index::Handle_t copyId = copy.getNodeChildByIndex(copy.root(), 0);
    a.checkEqual("12. getNodeTitle", copy.getNodeTitle(copyId), TEXT);
}
