/**
  *  \file server/talk/render/htmlrenderer.cpp
  *  \brief HTML Rendering
  */

#include <cstring>
#include "server/talk/render/htmlrenderer.hpp"
#include "afl/base/countof.hpp"
#include "afl/base/deleter.hpp"
#include "afl/charset/utf8.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "server/talk/forum.hpp"
#include "server/talk/inlinerecognizer.hpp"
#include "server/talk/message.hpp"
#include "server/talk/render/context.hpp"
#include "server/talk/render/options.hpp"
#include "server/talk/topic.hpp"
#include "server/talk/user.hpp"
#include "util/string.hpp"
#include "util/syntax/factory.hpp"
#include "util/syntax/format.hpp"
#include "util/syntax/highlighter.hpp"
#include "util/syntax/segment.hpp"

using server::talk::LinkParser;
using server::talk::TextNode;

namespace {
    class HtmlRenderer {
     public:
        HtmlRenderer(const server::talk::render::Context& ctx,
                     const server::talk::render::Options& opts,
                     server::talk::Root& root, String_t& result)
            : m_context(ctx),
              m_options(opts),
              m_root(root),
              result(result)
            { }
        void renderText(const String_t& text);
        void renderAttribution(const String_t& text);
        bool renderLink(const TextNode& n, const char* pre, const char* mid, const char* post);
        bool renderGameLink(const TextNode& n);
        bool renderForumLink(const TextNode& n);
        bool renderUserLink(const TextNode& n);
        bool renderPostLink(const TextNode& n);
        bool renderThreadLink(const TextNode& n);
        bool renderImage(const TextNode& n);
        void renderCode(const TextNode& n);
        void renderChildren(const TextNode& n);
        void renderFailedLink(const TextNode& n, const char* pfx);
        void render(const TextNode& n);

     private:
        const server::talk::render::Context& m_context;
        const server::talk::render::Options& m_options;
        server::talk::Root& m_root;
        String_t& result;
    };

    /** Check for valid URLs.
        The main reason for this function is to avoid users executing Javascript
        in other users' context. */
    bool isValidUrl(String_t s)
    {
        const char*const permittedPrefixes[] = {
            "/",
            "http://",
            "https://",
            "mailto:",
            "ftp://",
            "news:",
            "nntp:",
            "data:image/",
            "data:text/plain",
            "data:text/html"
        };
        for (size_t i = 0; i < countof(permittedPrefixes); ++i) {
            size_t n = std::strlen(permittedPrefixes[i]);
            if (s.size() >= n && s.compare(0, n, permittedPrefixes[i], n) == 0) {
                return true;
            }
        }
        return false;
    }

    String_t abbreviate(String_t s)
    {
        // FIXME: duplicate!
        if (s.empty()) {
            return "(no subject)";
        } else {
            if (s.size() > 30) {
                std::size_t i = 28;
                while (i > 0 && (s[i-1] == ' ' || afl::charset::Utf8::isContinuationByte(s[i]))) {
                    // If s[i-1] is a space, we want to cut before it.
                    // If s[i] is a continuation byte, s[i-1] is part of the same UTF-8 rune, so go before it.
                    --i;
                }
                s.erase(i);
                s += "...";
            }
            return s;
        }
    }

    const char* getInlineTagName(TextNode::InlineFormat minor)
    {
        switch (minor) {
         case TextNode::miInBold:          return "b";
         case TextNode::miInItalic:        return "em";
         case TextNode::miInStrikeThrough: return "s";
         case TextNode::miInUnderline:     return "u";
         case TextNode::miInMonospace:     return "tt";
        }
        return 0;
    }

    const char* getSyntaxClassName(util::syntax::Format fmt)
    {
        switch (fmt) {
         case util::syntax::DefaultFormat:  return 0;
         case util::syntax::KeywordFormat:  return "syn-kw";
         case util::syntax::NameFormat:     return "syn-name";
         case util::syntax::StringFormat:   return "syn-str";
         case util::syntax::CommentFormat:  return "syn-com";
         case util::syntax::Comment2Format: return "syn-com2";
         case util::syntax::SectionFormat:  return "syn-sec";
         case util::syntax::QuoteFormat:    return "syn-quote";
         case util::syntax::ErrorFormat:    return "syn-err";
        }
        return 0;
    }
}




void
HtmlRenderer::renderText(const String_t& text)
{
    result += util::encodeHtml(text, true);
}

void
HtmlRenderer::renderAttribution(const String_t& text)
{
    String_t::size_type i = text.rfind(';');
    if (i != text.npos && i != text.size()-1) {
        // It is the form "user;posting"
        TextNode n(TextNode::maLink, TextNode::miLinkUser);
        n.text.assign(text, 0, i);
        if (!renderUserLink(n)) {
            renderText(n.text);
        }
        renderText(" in ");
        n.minor = TextNode::miLinkPost;
        n.text.assign(text, i+1, String_t::npos);
        if (!renderPostLink(n)) {
            renderText(n.text);
        }
    } else {
        // Just a user name
        TextNode n(TextNode::maLink, TextNode::miLinkUser, text);
        if (!renderUserLink(n)) {
            renderText(n.text);
        }
    }
    renderText(":");
}

bool
HtmlRenderer::renderLink(const TextNode& n, const char* pre, const char* mid, const char* post)
{
    result += pre;
    renderText(n.text);
    result += mid;
    if (n.children.empty()) {
        renderText(n.text);
    } else {
        renderChildren(n);
    }
    result += post;
    return true;
}

bool
HtmlRenderer::renderGameLink(const TextNode& n)
{
    // Parse game Id
    afl::base::Optional<LinkParser::Result_t> r = m_context.parseGameLink(n.text);
    if (const LinkParser::Result_t* p = r.get()) {
        result += "<a href=\"";
        renderText(m_options.getBaseUrl());
        renderText(m_root.linkFormatter().makeGameUrl(p->first, p->second));
        result += "\">";
        if (n.children.empty()) {
            renderText(p->second);
        } else {
            renderChildren(n);
        }
        result += "</a>";
        return true;
    } else {
        return false;
    }
}

bool
HtmlRenderer::renderForumLink(const TextNode& n)
{
    // Parse forum Id
    afl::base::Optional<LinkParser::Result_t> r = m_context.parseForumLink(n.text);
    if (const LinkParser::Result_t* p = r.get()) {
        result += "<a href=\"";
        renderText(m_options.getBaseUrl());
        renderText(m_root.linkFormatter().makeForumUrl(p->first, p->second));
        result += "\">";
        if (n.children.empty()) {
            renderText(p->second);
        } else {
            renderChildren(n);
        }
        result += "</a>";
        return true;
    } else {
        return false;
    }
}

bool
HtmlRenderer::renderUserLink(const TextNode& n)
{
    afl::base::Optional<String_t> r = m_context.parseUserLink(n.text);
    if (const String_t* p = r.get()) {
        server::talk::User u(m_root, *p);

        // Build it
        result += "<a class=\"userlink";
        if (*p == m_context.getUser()) {
            result += " userlink-me";
        }
        result += "\" href=\"";
        renderText(m_options.getBaseUrl());
        renderText(m_root.linkFormatter().makeUserUrl(u.getLoginName()));
        result += "\">";
        if (n.children.empty()) {
            renderText(u.getScreenName());
        } else {
            renderChildren(n);
        }
        result += "</a>";
        return true;
    } else {
        return false;
    }
}

bool
HtmlRenderer::renderPostLink(const TextNode& n)
{
    afl::base::Optional<LinkParser::Result_t> r = m_context.parseMessageLink(n.text);
    if (const LinkParser::Result_t* p = r.get()) {
        // Message
        server::talk::Message m(m_root, p->first);

        // Topic
        const int32_t topicId = m.topicId().get();
        server::talk::Topic t(m_root, topicId);

        // Render
        const String_t topicName = t.subject().get();
        result += "<a href=\"";
        renderText(m_options.getBaseUrl());
        renderText(m_root.linkFormatter().makePostUrl(topicId, topicName, p->first));
        result += "\">";
        if (n.children.empty()) {
            renderText(abbreviate(p->second));
        } else {
            renderChildren(n);
        }
        result += "</a>";
        return true;
    } else {
        return false;
    }
}

bool
HtmlRenderer::renderThreadLink(const TextNode& n)
{
    // Validate
    afl::base::Optional<LinkParser::Result_t> r = m_context.parseTopicLink(n.text);
    if (const LinkParser::Result_t* p = r.get()) {
        result += "<a href=\"";
        renderText(m_options.getBaseUrl());
        renderText(m_root.linkFormatter().makeTopicUrl(p->first, p->second));
        result += "\">";
        if (n.children.empty()) {
            renderText(abbreviate(p->second));
        } else {
            renderChildren(n);
        }
        result += "</a>";
        return true;
    } else {
        return false;
    }
}

bool
HtmlRenderer::renderImage(const TextNode& n)
{
    // Validate
    if (!isValidUrl(n.text)) {
        return false;
    }

    // Render
    result += "<img src=\"";
    renderText(n.text);
    if (!n.children.empty()) {
        result += "\" alt=\"";
        renderText(n.getTextContent());
    }
    result += "\" />";
    return true;
}

void
HtmlRenderer::renderCode(const TextNode& n)
{
    afl::base::Deleter del;
    util::syntax::Highlighter& syntax = util::syntax::Factory(m_root.keywordTable()).create(n.text, del);
    result += "<pre>";
    for (size_t i = 0, e = n.children.size(); i != e; ++i) {
        const TextNode& ch = *n.children[i];
        if (ch.major == TextNode::maPlain) {
            // It's text. Render it using the highlighter.
            util::syntax::Segment seg;
            syntax.init(afl::string::toMemory(ch.text));
            while (syntax.scan(seg)) {
                const char* cls = getSyntaxClassName(seg.getFormat());
                if (!seg.getLink().empty()) {
                    // It's a link.
                    const String_t& link = seg.getLink();
                    result += "<a href=\"";
                    String_t::size_type n = link.find_first_of(":/");
                    if (n == String_t::npos || link[n] == '/') {
                        renderText(m_options.getBaseUrl());
                    }
                    renderText(link);
                    if (!seg.getInfo().empty()) {
                        result += "\" title=\"";
                        renderText(seg.getInfo());
                    }
                    if (cls) {
                        result += "\" class=\"";
                        result += cls;
                    }
                    result += "\">";
                    renderText(afl::string::fromMemory(seg.getText()));
                    result += "</a>";
                } else if (cls || !seg.getInfo().empty()) {
                    // Not a link.
                    result += "<span";
                    if (cls) {
                        result += " class=\"";
                        result += cls;
                        result += "\"";
                    }
                    if (!seg.getInfo().empty()) {
                        result += " title=\"";
                        renderText(seg.getInfo());
                        result += "\"";
                    }
                    result += ">";
                    renderText(afl::string::fromMemory(seg.getText()));
                    result += "</span>";
                } else {
                    // No class, no info.
                    renderText(afl::string::fromMemory(seg.getText()));
                }
            }
        } else {
            // Not text. Render directly.
            render(ch);
        }
    }
    result += "</pre>\n";
}

void
HtmlRenderer::renderChildren(const TextNode& n)
{
    for (size_t i = 0, e = n.children.size(); i != e; ++i) {
        render(*n.children[i]);
    }
}

void
HtmlRenderer::renderFailedLink(const TextNode& n, const char* pfx)
{
    result += "<span class=\"tfailedlink\">";
    if (n.children.empty()) {
        renderText(pfx);
        renderText(n.text);
    } else {
        renderChildren(n);
    }
    result += "</span>";
}

void
HtmlRenderer::render(const TextNode& n)
{
    bool ok;
    const char* pfx;
    int i;
    switch (TextNode::MajorKind(n.major)) {
     case TextNode::maPlain:
        renderText(n.text);
        break;

     case TextNode::maInline:
        if (const char* tag = getInlineTagName(TextNode::InlineFormat(n.minor))) {
            result += "<";
            result += tag;
            result += ">";
            renderChildren(n);
            result += "</";
            result += tag;
            result += ">";
        } else {
            renderChildren(n);
        }
        break;

     case TextNode::maInlineAttr:
        switch (TextNode::InlineAttrFormat(n.minor)) {
         case TextNode::miIAColor:
            result += "<font color=\"";
            renderText(n.text);
            result += "\">";
            renderChildren(n);
            result += "</font>";
            break;

         case TextNode::miIAFont:
            result += "<span style=\"font-family: ";
            renderText(n.text);
            result += ";\">";
            renderChildren(n);
            result += "</span>";
            break;

         case TextNode::miIASize:
            if (afl::string::strToInteger(n.text, i) && i >= -8 && i <= +8 && i != 0) {
                int32_t size = 100;
                while (i > 0) {
                    size *= 100;
                    size /= 80;
                    --i;
                }
                while (i < 0) {
                    size *= 80;
                    size /= 100;
                    ++i;
                }
                result += afl::string::Format("<span style=\"font-size: %d%%;\">", size);
                renderChildren(n);
                result += "</span>";
            } else {
                renderChildren(n);
            }
            break;

         default:
            renderChildren(n);
        }
        break;

     case TextNode::maLink:
        switch (TextNode::LinkFormat(n.minor)) {
         case TextNode::miLinkUrl:
            ok = isValidUrl(n.text) && renderLink(n, "<a href=\"", "\" rel=\"nofollow\">", "</a>");
            pfx = "link ";
            break;
         case TextNode::miLinkEmail:
            ok = renderLink(n, "<a href=\"mailto:", "\">", "</a>");
            pfx = "mail ";
            break;
         case TextNode::miLinkThread:
            ok = renderThreadLink(n);
            pfx = "thread ";
            break;
         case TextNode::miLinkPost:
            ok = renderPostLink(n);
            pfx = "post ";
            break;
         case TextNode::miLinkGame:
            ok = renderGameLink(n);
            pfx = "game ";
            break;
         case TextNode::miLinkUser:
            ok = renderUserLink(n);
            pfx = "user ";
            break;
         case TextNode::miLinkForum:
            ok = renderForumLink(n);
            pfx = "forum ";
            break;
         default:
            pfx = "";
            ok = false;
            break;
        }
        if (!ok) {
            renderFailedLink(n, pfx);
        }
        break;

     case TextNode::maParagraph:
        switch (TextNode::ParagraphFormat(n.minor)) {
         case TextNode::miParNormal:
            result += "<p>";
            renderChildren(n);
            result += "</p>\n";
            break;
         case TextNode::miParCode:
            renderCode(n);
            break;
         case TextNode::miParCentered:
            result += "<center>";
            renderChildren(n);
            result += "</center>\n";
            break;
         case TextNode::miParBreak:
            break;
         case TextNode::miParFragment:
         default:
            renderChildren(n);
            break;
        }
        break;

     case TextNode::maGroup:
        switch (TextNode::GroupFormat(n.minor)) {
         case TextNode::miGroupQuote:
            if (!n.text.empty()) {
                result += "<div class=\"attribution\">";
                renderAttribution(n.text);
                result += "</div>\n";
            }
            result += "<blockquote>";
            renderChildren(n);
            result += "</blockquote>\n";
            break;
         case TextNode::miGroupList:
            {
                const char* close;
                if (n.text.empty()) {
                    result += "<ul>";
                    close = "</ul>";
                } else if (n.text == "1") {
                    result += "<ol>";
                    close = "</ol>";
                } else {
                    result += "<ol type=\"";
                    renderText(n.text);
                    result += "\">";
                    close = "</ol>";
                }
                if (n.isSimpleList()) {
                    for (size_t i = 0, e = n.children.size(); i != e; ++i) {
                        result += "<li>";
                        renderChildren(*n.children[i]->children[0]);
                        result += "</li>\n";
                    }
                } else {
                    renderChildren(n);
                }
                result += close;
            }
            break;
         case TextNode::miGroupListItem:
            result += "<li>";
            renderChildren(n);
            result += "</li>\n";
            break;
         case TextNode::miGroupRoot:
         default:
            renderChildren(n);
            break;
        }
        break;

     case TextNode::maSpecial:
        switch (TextNode::SpecialFormat(n.minor)) {
         case TextNode::miSpecialBreak:
            result += "<br />";
            break;

         case TextNode::miSpecialImage:
            if (!renderImage(n)) {
                renderFailedLink(n, "image ");
            }
            break;

         case TextNode::miSpecialSmiley:
            if (const server::talk::InlineRecognizer::SmileyDefinition* s = m_root.recognizer().getSmileyDefinitionByName(n.text)) {
                result += "<img src=\"";
                renderText(m_options.getBaseUrl());
                renderText(s->image);
                result += afl::string::Format("\" width=\"%d\" height=\"%d\" alt=\":%s:\" />", s->width, s->height, s->name);
            } else {
                result += ":";
                renderText(n.text);
                result += ":";
            }
            break;
        }
    }
}

String_t
server::talk::render::renderHTML(const TextNode& node, const Context& ctx, const Options& opts, Root& root)
{
    // ex planetscentral/talk/htmlout.cc:renderHTML
    String_t result;
    HtmlRenderer(ctx, opts, root, result).render(node);
    return result;
}
