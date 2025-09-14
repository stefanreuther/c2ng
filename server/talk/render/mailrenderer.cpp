/**
  *  \file server/talk/render/mailrenderer.cpp
  *  \brief Mail renderer
  */

#include "server/talk/render/mailrenderer.hpp"
#include "afl/string/parse.hpp"
#include "server/talk/forum.hpp"
#include "server/talk/message.hpp"
#include "server/talk/render/context.hpp"
#include "server/talk/render/options.hpp"
#include "server/talk/root.hpp"
#include "server/talk/topic.hpp"
#include "server/talk/user.hpp"

using server::talk::LinkParser;
using server::talk::TextNode;

namespace {
    class MailRenderer {
     public:
        MailRenderer(const LinkParser& lp, const server::talk::render::Options& opts,
                     server::talk::Root& root, bool forNNTP, String_t& result);

        void renderUserName(const String_t& name);
        String_t formatMessageId(const String_t& name);
        bool renderThreadId(const String_t& name);
        bool renderGameLink(const String_t& text);
        bool renderUserLink(const String_t& text);
        bool renderForumLink(const String_t& text);
        void renderPlaintext(const String_t& text);
        void renderCode(const String_t& text);
        void renderAttribution(const String_t& text);
        void renderPG(const TextNode& n);
        void renderChildrenPG(const TextNode& n);

        void renderInline(const TextNode& n);
        void renderChildrenInline(const TextNode& n);

        MailRenderer& withPrefix(String_t pfx, String_t second);

        void renderWord(String_t part);
        void renderSpace();
        void flushLine();

     private:
        void emitLine(const String_t& theLine);

        String_t& result;
        const LinkParser& m_linkParser;
        const server::talk::render::Options& m_options;
        server::talk::Root& m_root;
        const bool forNNTP;

        String_t line;
        String_t word;
        String_t prefix;
        String_t second_prefix;
    };

    void discardTrailingSpace(String_t& out)
    {
        size_t i = out.size();
        while (i > 0 && out[i-1] == ' ') {
            --i;
        }
        if (i < out.size()) {
            out.erase(i, out.size());
        }
    }
}

MailRenderer::MailRenderer(const LinkParser& lp, const server::talk::render::Options& opts,
                           server::talk::Root& root, bool forNNTP, String_t& result)
    : result(result),
      m_linkParser(lp),
      m_options(opts),
      m_root(root),
      forNNTP(forNNTP),
      line(),
      word(),
      prefix(),
      second_prefix()
{
    // ex MailRenderer::MailRenderer
}

/** Render a user name.
    Renders the user's real name or screen name.
    Used for rendering attributions. */
void
MailRenderer::renderUserName(const String_t& name)
{
    // Map user name to user Id
    String_t userId = m_root.getUserIdFromLogin(name);
    if (userId.empty()) {
        renderWord(name);
    } else {
        server::talk::User u(m_root, userId);
        String_t realName = u.getRealName();
        if (realName.empty()) {
            renderWord(u.getScreenName());
        } else {
            renderWord(realName);
        }
    }
}

/** Render a message Id.
    For NNTP, render as the RFC message Id.
    Otherwise, render a <post:XXX> pseudo-link. */
String_t
MailRenderer::formatMessageId(const String_t& name)
{
    int32_t mid;
    if (!forNNTP || !afl::string::strToInteger(name, mid)) {
        return "<post:" + name + ">";
    } else {
        server::talk::Message m(m_root, mid);
        if (!m.exists()) {
            return "<post:" + name + ">";
        } else {
            return "<" +m.getRfcMessageId(m_root) + ">";
        }
    }
}

/** Render a thread Id.
    For NNTP, render it as the RFC message Id of the first posting.
    If it has no message Id (or we're not rendering for NNTP),
    returns false, causing the caller to produce a <thread:XXX> pseudo-link. */
bool
MailRenderer::renderThreadId(const String_t& name)
{
    int32_t tid;
    if (!forNNTP || !afl::string::strToInteger(name, tid)) {
        return false;
    }

    server::talk::Topic t(m_root, tid);
    if (!t.exists()) {
        return false;
    }

    server::talk::Message m(m_root, t.firstPostingId().get());
    if (!m.exists()) {
        return false;
    }

    renderWord("<" + m.getRfcMessageId(m_root) + ">");
    return true;
}

/** Render a game link.
    Always renders the complete link. */
bool
MailRenderer::renderGameLink(const String_t& text)
{
    afl::base::Optional<LinkParser::Result_t> r = m_linkParser.parseGameLink(text);
    if (const LinkParser::Result_t* p = r.get()) {
        renderWord("<" + m_options.getBaseUrl() + m_root.linkFormatter().makeGameUrl(p->first, p->second) + ">");
        return true;
    } else {
        return false;
    }
}

bool
MailRenderer::renderUserLink(const String_t& text)
{
    afl::base::Optional<String_t> r = m_linkParser.parseUserLink(text);
    if (const String_t* p = r.get()) {
        server::talk::User u(m_root, *p);
        renderWord("<" + m_options.getBaseUrl() + m_root.linkFormatter().makeUserUrl(u.getLoginName()) + ">");
        return true;
    } else {
        return false;
    }
}

bool
MailRenderer::renderForumLink(const String_t& text)
{
    afl::base::Optional<LinkParser::Result_t> r = m_linkParser.parseForumLink(text);
    if (const LinkParser::Result_t* p = r.get()) {
        // Can we render as newsgroup name?
        if (forNNTP) {
            String_t ngName = server::talk::Forum(m_root, p->first).getNewsgroup();
            if (!ngName.empty()) {
                renderWord("<news:" + ngName + ">");
                return true;
            }
        }

        // Render as link
        renderWord("<" + m_options.getBaseUrl() + m_root.linkFormatter().makeForumUrl(p->first, p->second) + ">");
        return true;
    } else {
        return false;
    }
}

void
MailRenderer::renderPlaintext(const String_t& text)
{
    static const char WHITESPACE[] = " \t\r\n";
    String_t::size_type i = text.find_first_not_of(WHITESPACE);
    if (i != 0 && i != String_t::npos) {
        renderSpace();
    }
    while (i != String_t::npos) {
        String_t::size_type j = text.find_first_of(WHITESPACE, i);
        if (j == String_t::npos) {
            renderWord(String_t(text, i));
            break;
        } else {
            renderWord(String_t(text, i, j-i));
            renderSpace();
            i = text.find_first_not_of(WHITESPACE, j);
        }
    }
}

/** Render preformatted text. */
void
MailRenderer::renderCode(const String_t& text)
{
    size_t i = 0;
    size_t j;
    while ((j = text.find_first_of("\r\n", i)) != text.npos) {
        renderWord("  " + String_t(text, i, j-i));
        flushLine();
        i = j+1;
        if (text[j] == '\r' && i < text.size() && text[i] == '\n') {
            ++i;
        }
    }
    if (i != text.size()) {
        renderWord("  " + String_t(text, i, text.size()-i));
        flushLine();
    }
}

/** Render attribution of a message. */
void
MailRenderer::renderAttribution(const String_t& text)
{
    if (!text.empty()) {
        String_t::size_type i = text.rfind(';');
        if (i != text.npos && i != text.size()-1) {
            // It is the form "user;posting"
            renderWord("* ");
            renderUserName(String_t(text, 0, i));
            renderWord(" in ");
            renderWord(formatMessageId(String_t(text, i+1)));
            renderWord(":");
            flushLine();
        } else {
            // Just a user name
            renderWord("* ");
            renderUserName(text);
            renderWord(":");
            flushLine();
        }
    }
}

/** Render a paragraph/group. These always produce complete lines. */
void
MailRenderer::renderPG(const TextNode& n)
{
    String_t::size_type x = result.size();
    switch (TextNode::MajorKind(n.major)) {
     case TextNode::maGroup:
        switch (TextNode::GroupFormat(n.minor)) {
         case TextNode::miGroupQuote:
            renderAttribution(n.text);
            if (prefix.empty()) {
                MailRenderer(*this).withPrefix("> ", "> ").renderChildrenPG(n);
            } else {
                MailRenderer(*this).withPrefix(">" + prefix, ">" + second_prefix).renderChildrenPG(n);
            }
            if (x != result.size()) {
                prefix = second_prefix;
            }
            break;
         case TextNode::miGroupList:
            renderChildrenPG(n);
            break;
         case TextNode::miGroupListItem:
            MailRenderer(*this).withPrefix(prefix + "* ", second_prefix + "  ").renderChildrenPG(n);
            if (x != result.size()) {
                prefix = second_prefix;
            }
            break;
         case TextNode::miGroupRoot:
            renderChildrenPG(n);
            break;
        }
        break;

     case TextNode::maParagraph:
        switch (TextNode::ParagraphFormat(n.minor)) {
         case TextNode::miParCode:
            renderCode(n.getTextContent());
            break;
         case TextNode::miParCentered:    // centering is not rendered in mails
         case TextNode::miParNormal:      // regular paragraphs
         case TextNode::miParBreak:
            renderChildrenInline(n);
            flushLine();
            break;
         case TextNode::miParFragment:
            renderChildrenInline(n);
            flushLine();
            if (!result.empty() && result[result.size()-1] == '\n') {
                result.erase(result.size()-1);
            }
            break;
        }
     default:
        break;
    }
}

/** Render a paragraph/group container. */
void
MailRenderer::renderChildrenPG(const TextNode& n)
{
    if (n.major == TextNode::maParagraph && n.minor == TextNode::miParFragment) {
        // miParFragment means we do not want to end the output with a newline; renderPG() does that.
        renderPG(n);
    } else {
        for (size_t i = 0, e = n.children.size(); i != e; ++i) {
            if (i != 0) {
                result += prefix;
                discardTrailingSpace(result);
                result += "\n";
            }
            renderPG(*n.children[i]);
        }
    }
}

/** Render inline markup. Inline markup produced by accumulating and
    word-wrapping it in \c line. */
void
MailRenderer::renderInline(const TextNode& n)
{
    switch (TextNode::MajorKind(n.major)) {
     case TextNode::maPlain:
        renderPlaintext(n.text);
        break;
     case TextNode::maInline:
        renderChildrenInline(n);
        break;
     case TextNode::maInlineAttr:
        renderChildrenInline(n);
        break;
     case TextNode::maLink:
        renderChildrenInline(n);
        renderSpace();
        switch (TextNode::LinkFormat(n.minor)) {
         case TextNode::miLinkUrl:
            renderWord("<" + n.text + ">");
            break;
         case TextNode::miLinkEmail:
            renderWord("<mailto:" + n.text + ">");
            break;
         case TextNode::miLinkPost:
            renderWord(formatMessageId(n.text));
            break;
         case TextNode::miLinkThread:
            if (!renderThreadId(n.text)) {
                renderWord("<thread:" + n.text + ">");
            }
            break;
         case TextNode::miLinkGame:
            if (!renderGameLink(n.text)) {
                renderWord("<game:" + n.text + ">");
            }
            break;
         case TextNode::miLinkUser:
            if (!renderUserLink(n.text)) {
                renderWord("<user:" + n.text + ">");
            }
            break;
         case TextNode::miLinkForum:
            if (!renderForumLink(n.text)) {
                renderWord("<forum:" + n.text + ">");
            }
            break;
        }
        break;

     case TextNode::maSpecial:
        switch (TextNode::SpecialFormat(n.minor)) {
         case TextNode::miSpecialBreak:
            flushLine();
            break;

         case TextNode::miSpecialImage:
            renderChildrenInline(n);
            renderSpace();
            renderWord("<" + n.text + ">");
            break;

         case TextNode::miSpecialSmiley:
            renderWord(":" + n.text + ":");
            break;
        }
        break;

     case TextNode::maParagraph:
     case TextNode::maGroup:
        break;
    }
}

/** Render a inline markup container. */
void
MailRenderer::renderChildrenInline(const TextNode& n)
{
    for (size_t i = 0, e = n.children.size(); i != e; ++i) {
        renderInline(*n.children[i]);
    }
}

MailRenderer&
MailRenderer::withPrefix(String_t pfx, String_t second)
{
    prefix = pfx;
    second_prefix = second;
    return *this;
}

void
MailRenderer::renderWord(String_t part)
{
    word += part;
}

void
MailRenderer::renderSpace()
{
    if (!word.empty()) {
        // Get permitted line size
        String_t::size_type lineLength = 72;
        if (prefix.size() > 30) {
            lineLength = prefix.size() + 42;
        }

        // Format it
        if (!line.empty() && prefix.size() + line.size() + word.size() >= lineLength) {
            emitLine(line);
            line.clear();
        }
        if (!line.empty()) {
            line += " ";
        }
        line += word;
        word.clear();
    }
}

void
MailRenderer::flushLine()
{
    renderSpace();
    if (!line.empty()) {
        emitLine(line);
        line.clear();
    }
}

void
MailRenderer::emitLine(const String_t& theLine)
{
    result += prefix;
    result += theLine;
    discardTrailingSpace(result);
    result += "\n";
    prefix = second_prefix;
}

String_t
server::talk::render::renderMail(const TextNode& node, const LinkParser& lp, const Options& opts, Root& root, bool forNNTP)
{
    // ex planetscentral/talk/mailout.h:renderMail
    String_t result;
    MailRenderer(lp, opts, root, forNNTP, result).renderChildrenPG(node);
    return result;
}
