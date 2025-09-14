/**
  *  \file server/talk/render/textrenderer.cpp
  *  \brief Text renderer
  */

#include "server/talk/render/textrenderer.hpp"
#include "afl/charset/utf8.hpp"
#include "afl/string/parse.hpp"
#include "server/talk/forum.hpp"
#include "server/talk/message.hpp"
#include "server/talk/topic.hpp"

namespace {
    using server::talk::LinkParser;
    using server::talk::Root;

    String_t hackSubject(const String_t s)
    {
        if (s.empty()) {
            return "(no subject)";
        } else if (s.size() > 30) {
            size_t i = 28;
            while (i > 0 && (s[i-1] == ' ' || afl::charset::Utf8::isContinuationByte(s[i]))) {
                // Yes, this is intended to check i-1 for spaces, and i for continuation:
                // - if [i-1] is a space, remove it
                // - if [i] is a continuation, we must at least remove [i-1] to not leave a partial rune
                --i;
            }
            return String_t(s, 0, i) + "...";
        } else {
            return s;
        }
    }

    String_t makeGameName(const String_t& text, const LinkParser& lp)
    {
        afl::base::Optional<LinkParser::Result_t> r = lp.parseGameLink(text);
        if (const LinkParser::Result_t* p = r.get()) {
            return p->second;
        } else {
            return text;
        }
    }

    String_t makeForumName(const String_t& text, const LinkParser& lp)
    {
        afl::base::Optional<LinkParser::Result_t> r = lp.parseForumLink(text);
        if (const LinkParser::Result_t* p = r.get()) {
            return p->second;
        } else {
            return text;
        }
    }

    String_t makePostName(const String_t& text, const LinkParser& lp)
    {
        afl::base::Optional<LinkParser::Result_t> r = lp.parseMessageLink(text);
        if (const LinkParser::Result_t* p = r.get()) {
            return hackSubject(p->second);
        } else {
            return text;
        }
    }

    String_t makeThreadName(const String_t& text, const LinkParser& lp)
    {
        afl::base::Optional<LinkParser::Result_t> r = lp.parseTopicLink(text);
        if (const LinkParser::Result_t* p = r.get()) {
            return hackSubject(p->second);
        } else {
            return text;
        }
    }

}

String_t
server::talk::render::renderPlainText(const TextNode& node, const LinkParser& lp)
{
    // This is for generating abstracts or similar; reduce maximum resource usage by applying a (very soft) limit to output size.
    const size_t LIMIT = 10000;

    if (node.major == TextNode::maPlain) {
        return node.text;
    } else {
        String_t result;
        for (size_t i = 0, n = node.children.size(); i < n && result.size() < LIMIT; ++i) {
            String_t next = renderPlainText(*node.children[i], lp);
            if (!next.empty() && node.children[i]->major == TextNode::maParagraph && !result.empty() && result[result.size()] != ' ') {
                result += ' ';
            }
            result += next;
        }
        if (result.empty() && node.major == TextNode::maLink) {
            switch (TextNode::LinkFormat(node.minor)) {
             case TextNode::miLinkUrl:
             case TextNode::miLinkEmail:
             case TextNode::miLinkUser:
                // Fallback is using the text
                result = node.text;
                break;

             case TextNode::miLinkThread:
                result = makeThreadName(node.text, lp);
                break;

             case TextNode::miLinkPost:
                result = makePostName(node.text, lp);
                break;

             case TextNode::miLinkGame:
                result = makeGameName(node.text, lp);
                break;

             case TextNode::miLinkForum:
                result = makeForumName(node.text, lp);
                break;
            }
        }
        return result;
    }
}
