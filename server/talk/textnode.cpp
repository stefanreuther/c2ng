/**
  *  \file server/talk/textnode.cpp
  */

#include "server/talk/textnode.hpp"

bool
server::talk::TextNode::isSimpleList() const
{
    // ex TextNode::isSimpleList
    // Is this a list?
    if (major != maGroup || minor != miGroupList) {
        return false;
    }

    // Check list items
    for (size_t i = 0, n = children.size(); i != n; ++i) {
        if (children[i]->major != maGroup || children[i]->minor != miGroupListItem) {
            return false;
        }
        if (children[i]->children.size() != 1) {
            return false;
        }

        TextNode* c = children[i]->children[0];
        if (c->major != maParagraph || c->minor != miParNormal) {
            return false;
        }
    }

    // No violation found
    return true;
}

void
server::talk::TextNode::stripQuotes()
{
    // ex TextNode::stripQuotes
    // Sort all quotes to the end
    size_t in = 0, out = 0, n = children.size();
    while (in < n) {
        if (children[in]->major == maGroup && children[in]->minor == miGroupQuote) {
            // it's a quote, drop it
        } else {
            // not a quote
            children.swapElements(in, out);
            ++out;
        }
        ++in;
    }

    // Now, all quotes are after position /out/. Delete them.
    while (children.size() > out) {
        children.popBack();
    }
}

// FIXME: document that/why this limits to 10000
String_t
server::talk::TextNode::getTextContent() const
{
    // ex TextNode::rawTextContent
    if (major == maPlain) {
        return text;
    } else {
        String_t result;
        for (size_t i = 0, n = children.size(); i < n && result.size() < 10000; ++i) {
            result += children[i]->getTextContent();
        }
        return result;
    }
}
