/**
  *  \file test/server/talk/textnodetest.cpp
  *  \brief Test for server::talk::TextNode
  */

#include "server/talk/textnode.hpp"
#include "afl/test/testrunner.hpp"

using server::talk::TextNode;

/*
 *  Test stripQuotes.
 */

// Removing a quote, normal case.
// Notice how nested quotes (which are not normally possible) remain.
AFL_TEST("server.talk.TextNode:stripQuotes:normal", a)
{
    TextNode testee(TextNode::maGroup, TextNode::miGroupRoot);
    testee.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal));
    testee.children.pushBackNew(new TextNode(TextNode::maGroup, TextNode::miGroupQuote));
    testee.children.pushBackNew(new TextNode(TextNode::maGroup, TextNode::miGroupList));
    testee.children[2]->children.pushBackNew(new TextNode(TextNode::maGroup, TextNode::miGroupQuote));

    testee.stripQuotes();

    a.checkEqual("01", testee.children.size(), 2U);
    a.checkEqual("02", testee.children[1]->major, TextNode::maGroup);
    a.checkEqual("03", testee.children[1]->minor, TextNode::miGroupList);
    a.checkEqual("04", testee.children[1]->children.size(), 1U);
    a.checkEqual("05", testee.children[1]->children[0]->major, TextNode::maGroup);
    a.checkEqual("06", testee.children[1]->children[0]->minor, TextNode::miGroupQuote);
}

// Only quotes, nothing remains
AFL_TEST("server.talk.TextNode:stripQuotes:empty", a)
{
    TextNode testee(TextNode::maGroup, TextNode::miGroupRoot);
    testee.children.pushBackNew(new TextNode(TextNode::maGroup, TextNode::miGroupQuote));
    testee.children.pushBackNew(new TextNode(TextNode::maGroup, TextNode::miGroupQuote));
    testee.children.pushBackNew(new TextNode(TextNode::maGroup, TextNode::miGroupQuote));

    testee.stripQuotes();

    a.checkEqual("", testee.children.size(), 0U);
}

/** Test basic behaviour. */
AFL_TEST("server.talk.TextNode:basic", a)
{
    // Exercise both constructors
    TextNode t1(TextNode::maLink, TextNode::miLinkUrl, "http://x.y.z/");
    TextNode t2(TextNode::maGroup, TextNode::miGroupQuote);

    a.checkEqual("01. major", t1.major, TextNode::maLink);
    a.checkEqual("02. minor", t1.minor, TextNode::miLinkUrl);
    a.checkEqual("03. text",  t1.text, "http://x.y.z/");

    a.checkEqual("11. major", t2.major, TextNode::maGroup);
    a.checkEqual("12. minor", t2.minor, TextNode::miGroupQuote);
    a.checkEqual("13. text",  t2.text, "");
}

/** Test isSimpleList. */
AFL_TEST("server.talk.TextNode:isSimpleList", a)
{
    // Empty list is a valid simple list
    TextNode t(TextNode::maGroup, TextNode::miGroupList);
    a.check("01. isSimpleList", t.isSimpleList());

    // Add some list items.
    // This is NOT a simple list because the children have no content.
    // Parsers should not produce this.
    t.children.pushBackNew(new TextNode(TextNode::maGroup, TextNode::miGroupListItem));
    t.children.pushBackNew(new TextNode(TextNode::maGroup, TextNode::miGroupListItem));
    a.check("11. isSimpleList", !t.isSimpleList());

    // Add paragraphs to the children.
    // This is a simple list.
    t.children[0]->children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal));
    t.children[1]->children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal));
    a.check("21. isSimpleList", t.isSimpleList());

    // Add more paragraphs to one child, making this not a simple list anymore.
    t.children[0]->children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal));
    a.check("31. isSimpleList", !t.isSimpleList());
}

/** Test isSimpleList, second part. */
AFL_TEST("server.talk.TextNode:isSimpleList:2", a)
{
    // Container is not a list
    a.check("01", !TextNode(TextNode::maGroup, TextNode::miGroupListItem).isSimpleList());
    a.check("02", !TextNode(TextNode::maGroup, TextNode::miGroupQuote).isSimpleList());
    a.check("03", !TextNode(TextNode::maParagraph, TextNode::miParNormal).isSimpleList());
    a.check("04", !TextNode(TextNode::maPlain, 0).isSimpleList());

    // First-level child is not a list item
    {
        TextNode t(TextNode::maGroup, TextNode::miGroupList);
        t.children.pushBackNew(new TextNode(TextNode::maGroup, TextNode::miGroupList));
        a.check("11. isSimpleList", !t.isSimpleList());
    }
    {
        TextNode t(TextNode::maGroup, TextNode::miGroupList);
        t.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal));
        a.check("12. isSimpleList", !t.isSimpleList());
    }

    // Second-level child is not a paragraph
    {
        TextNode t(TextNode::maGroup, TextNode::miGroupList);
        t.children.pushBackNew(new TextNode(TextNode::maGroup, TextNode::miGroupListItem));
        t.children[0]->children.pushBackNew(new TextNode(TextNode::maGroup, TextNode::miGroupList));
        a.check("21. isSimpleList", !t.isSimpleList());
    }
    {
        TextNode t(TextNode::maGroup, TextNode::miGroupList);
        t.children.pushBackNew(new TextNode(TextNode::maGroup, TextNode::miGroupListItem));
        t.children[0]->children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParCode));
        a.check("22. isSimpleList", !t.isSimpleList());
    }
}

/*
 *  Test getTextContent.
 */

// Standard case
AFL_TEST("server.talk.TextNode:getTextContent", a)
{
    TextNode t(TextNode::maParagraph, TextNode::miParNormal);
    t.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "This is "));
    t.children.pushBackNew(new TextNode(TextNode::maInline, TextNode::miInBold));
    t.children[1]->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "bold"));
    t.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " text."));
    a.checkEqual("", t.getTextContent(), "This is bold text.");
}

// Overflow case
AFL_TEST("server.talk.TextNode:getTextContent:overflow", a)
{
    TextNode t(TextNode::maParagraph, TextNode::miParNormal);
    for (size_t i = 0; i < 2000; ++i) {
        t.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "12345678910"));
    }

    // getTextContent limits to (roughly) 10000.
    a.checkLessThan("", t.getTextContent().size(), 12000U);
}
