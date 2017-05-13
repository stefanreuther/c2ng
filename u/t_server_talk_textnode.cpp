/**
  *  \file u/t_server_talk_textnode.cpp
  *  \brief Test for server::talk::TextNode
  */

#include "server/talk/textnode.hpp"

#include "t_server_talk.hpp"

/** Test stripQuotes. */
void
TestServerTalkTextNode::testQuote()
{
    using server::talk::TextNode;

    // Removing a quote, normal case.
    // Notice how nested quotes (which are not normally possible) remain.
    {
        TextNode testee(TextNode::maGroup, TextNode::miGroupRoot);
        testee.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal));
        testee.children.pushBackNew(new TextNode(TextNode::maGroup, TextNode::miGroupQuote));
        testee.children.pushBackNew(new TextNode(TextNode::maGroup, TextNode::miGroupList));
        testee.children[2]->children.pushBackNew(new TextNode(TextNode::maGroup, TextNode::miGroupQuote));

        testee.stripQuotes();

        TS_ASSERT_EQUALS(testee.children.size(), 2U);
        TS_ASSERT_EQUALS(testee.children[1]->major, TextNode::maGroup);
        TS_ASSERT_EQUALS(testee.children[1]->minor, TextNode::miGroupList);
        TS_ASSERT_EQUALS(testee.children[1]->children.size(), 1U);
        TS_ASSERT_EQUALS(testee.children[1]->children[0]->major, TextNode::maGroup);
        TS_ASSERT_EQUALS(testee.children[1]->children[0]->minor, TextNode::miGroupQuote);
    }

    // Only quotes, nothing remains
    {
        TextNode testee(TextNode::maGroup, TextNode::miGroupRoot);
        testee.children.pushBackNew(new TextNode(TextNode::maGroup, TextNode::miGroupQuote));
        testee.children.pushBackNew(new TextNode(TextNode::maGroup, TextNode::miGroupQuote));
        testee.children.pushBackNew(new TextNode(TextNode::maGroup, TextNode::miGroupQuote));

        testee.stripQuotes();

        TS_ASSERT_EQUALS(testee.children.size(), 0U);
    }
}

/** Test basic behaviour. */
void
TestServerTalkTextNode::testBasic()
{
    // Exercise both constructors
    server::talk::TextNode t1(server::talk::TextNode::maLink, server::talk::TextNode::miLinkUrl, "http://x.y.z/");
    server::talk::TextNode t2(server::talk::TextNode::maGroup, server::talk::TextNode::miGroupQuote);

    TS_ASSERT_EQUALS(t1.major, server::talk::TextNode::maLink);
    TS_ASSERT_EQUALS(t1.minor, server::talk::TextNode::miLinkUrl);
    TS_ASSERT_EQUALS(t1.text, "http://x.y.z/");

    TS_ASSERT_EQUALS(t2.major, server::talk::TextNode::maGroup);
    TS_ASSERT_EQUALS(t2.minor, server::talk::TextNode::miGroupQuote);
    TS_ASSERT_EQUALS(t2.text, "");
}

/** Test isSimpleList. */
void
TestServerTalkTextNode::testSimpleList()
{
    using server::talk::TextNode;

    // Empty list is a valid simple list
    TextNode t(TextNode::maGroup, TextNode::miGroupList);
    TS_ASSERT(t.isSimpleList());

    // Add some list items.
    // This is NOT a simple list because the children have no content.
    // Parsers should not produce this.
    t.children.pushBackNew(new TextNode(TextNode::maGroup, TextNode::miGroupListItem));
    t.children.pushBackNew(new TextNode(TextNode::maGroup, TextNode::miGroupListItem));
    TS_ASSERT(!t.isSimpleList());

    // Add paragraphs to the children.
    // This is a simple list.
    t.children[0]->children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal));
    t.children[1]->children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal));
    TS_ASSERT(t.isSimpleList());

    // Add more paragraphs to one child, making this not a simple list anymore.
    t.children[0]->children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal));
    TS_ASSERT(!t.isSimpleList());
}

/** Test isSimpleList, second part. */
void
TestServerTalkTextNode::testSimpleList2()
{
    using server::talk::TextNode;

    // Container is not a list
    TS_ASSERT(!TextNode(TextNode::maGroup, TextNode::miGroupListItem).isSimpleList());
    TS_ASSERT(!TextNode(TextNode::maGroup, TextNode::miGroupQuote).isSimpleList());
    TS_ASSERT(!TextNode(TextNode::maParagraph, TextNode::miParNormal).isSimpleList());
    TS_ASSERT(!TextNode(TextNode::maPlain, 0).isSimpleList());

    // First-level child is not a list item
    {
        TextNode t(TextNode::maGroup, TextNode::miGroupList);
        t.children.pushBackNew(new TextNode(TextNode::maGroup, TextNode::miGroupList));
        TS_ASSERT(!t.isSimpleList());
    }
    {
        TextNode t(TextNode::maGroup, TextNode::miGroupList);
        t.children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParNormal));
        TS_ASSERT(!t.isSimpleList());
    }

    // Second-level child is not a paragraph
    {
        TextNode t(TextNode::maGroup, TextNode::miGroupList);
        t.children.pushBackNew(new TextNode(TextNode::maGroup, TextNode::miGroupListItem));
        t.children[0]->children.pushBackNew(new TextNode(TextNode::maGroup, TextNode::miGroupList));
        TS_ASSERT(!t.isSimpleList());
    }
    {
        TextNode t(TextNode::maGroup, TextNode::miGroupList);
        t.children.pushBackNew(new TextNode(TextNode::maGroup, TextNode::miGroupListItem));
        t.children[0]->children.pushBackNew(new TextNode(TextNode::maParagraph, TextNode::miParCode));
        TS_ASSERT(!t.isSimpleList());
    }
}

/** Test getTextContent. */
void
TestServerTalkTextNode::testTextContent()
{
    using server::talk::TextNode;

    // Standard case
    {
        TextNode t(TextNode::maParagraph, TextNode::miParNormal);
        t.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "This is "));
        t.children.pushBackNew(new TextNode(TextNode::maInline, TextNode::miInBold));
        t.children[1]->children.pushBackNew(new TextNode(TextNode::maPlain, 0, "bold"));
        t.children.pushBackNew(new TextNode(TextNode::maPlain, 0, " text."));
        TS_ASSERT_EQUALS(t.getTextContent(), "This is bold text.");
    }

    // Overflow case
    {
        TextNode t(TextNode::maParagraph, TextNode::miParNormal);
        for (size_t i = 0; i < 2000; ++i) {
            t.children.pushBackNew(new TextNode(TextNode::maPlain, 0, "12345678910"));
        }

        // getTextContent limits to (roughly) 10000.
        TS_ASSERT_LESS_THAN(t.getTextContent().size(), 12000U);
    }
}

