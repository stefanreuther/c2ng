/**
  *  \file server/talk/render/bbrenderer.cpp
  */

#include "server/talk/render/bbrenderer.hpp"
#include "server/talk/parse/bblexer.hpp"
#include "server/talk/parse/bbparser.hpp"

using server::talk::TextNode;

namespace {
    const char* getInlineTagName(TextNode::InlineFormat minor)
    {
        switch (minor) {
         case TextNode::miInBold:             return "b";
         case TextNode::miInItalic:           return "i";
         case TextNode::miInStrikeThrough:    return "s";
         case TextNode::miInUnderline:        return "u";
         case TextNode::miInMonospace:        return "tt";
        }
        return 0;
    }

    const char* getInlineAttrTagName(TextNode::InlineAttrFormat minor)
    {
        switch (minor) {
         case TextNode::miIAColor:   return "color";
         case TextNode::miIASize:    return "size";
         case TextNode::miIAFont:    return "font";
        }
        return 0;
    }

    const char* getLinkTagName(TextNode::LinkFormat minor)
    {
        switch (minor) {
         case TextNode::miLinkUrl:       return "url";
         case TextNode::miLinkEmail:     return "email";
         case TextNode::miLinkThread:    return "thread";
         case TextNode::miLinkPost:      return "post";
         case TextNode::miLinkGame:      return "game";
         case TextNode::miLinkUser:      return "user";
         case TextNode::miLinkForum:     return "forum";
        }
        return 0;
    }

    class BBRenderer {
     public:
        BBRenderer(const server::talk::render::Context& ctx,
                   const server::talk::render::Options& opts,
                   server::talk::Root& root,
                   server::talk::InlineRecognizer::Kinds_t kinds,
                   String_t& result)
            : result(result),
              m_context(ctx),
              m_options(opts),
              m_root(root),
              m_kinds(kinds)
            { }

        void renderChildrenPG(const TextNode& n);
        void renderChildrenInline(const TextNode& n, bool in_code);
        void renderAttr(const TextNode& n);
        void renderLink(const TextNode& n, const char* tag);
        void renderPlaintext(const String_t& str);
        void renderInline(const TextNode& n, bool in_code);

        void renderPG(const TextNode& n);

     private:
        String_t& result;
        const server::talk::render::Context& m_context;
        const server::talk::render::Options& m_options;
        server::talk::Root& m_root;
        server::talk::InlineRecognizer::Kinds_t m_kinds;
    };
}

void
BBRenderer::renderChildrenPG(const TextNode& n)
{
    for (size_t i = 0, e = n.children.size(); i != e; ++i) {
        if (i != 0) {
            result += "\n\n";
        }
        renderPG(*n.children[i]);
    }
}

void
BBRenderer::renderChildrenInline(const TextNode& n, bool in_code)
{
    for (size_t i = 0, e = n.children.size(); i != e; ++i) {
        renderInline(*n.children[i], in_code);
    }
}

void
BBRenderer::renderAttr(const TextNode& n)
{
    if (!n.text.empty()) {
        result += "=";
        if (n.text.find_first_of("[]") != String_t::npos) {
            result += "\"";
            result += n.text;
            result += "\"";
        } else {
            result += n.text;
        }
    }
}

void
BBRenderer::renderLink(const TextNode& n, const char* tag)
{
    if (n.children.empty()
        && n.text.find_first_of("[]") == String_t::npos
        && (n.text.empty() || n.text[0] != '@')
        && (n.text.find('@') == String_t::npos || n.text.find_first_of(" \t\n\r") == String_t::npos))
    {
        /* The above condition is true if we have no children, and we are reasonably
           sure that the content of the attribute needs no escaping, so we can use
           the abbreviated format:
           - no []
           - does not start with @
           - does not contain both @ and whitespace (this would mean we risk
           generating an at-link) */
        result += "[";
        result += tag;
        result += "]";
        result += n.text;
        result += "[/";
        result += tag;
        result += "]";
    } else {
        result += "[";
        result += tag;
        renderAttr(n);
        result += "]";
        renderChildrenInline(n, false);
        result += "[/";
        result += tag;
        result += "]";
    }
}

void
BBRenderer::renderPlaintext(const String_t& str)
{
    // Look for tags
    using server::talk::parse::BBLexer;
    BBLexer lex(str);
    BBLexer::Token t;
    String_t::size_type quote_end = String_t::npos;
    while ((t = lex.read()) != BBLexer::Eof) {
        if (t == BBLexer::Paragraph) {
            // Parsing this would break a paragraph, so replace by space
            result += " ";
        } else if (t == BBLexer::Text
                   || ((t == BBLexer::TagStart || t == BBLexer::TagEnd)
                       && !server::talk::parse::BBParser::isKnownTag(lex.getTag())))
        {
            // Parsing this would NOT create a tag, so render it. But we may have to quote smileys.
            String_t::size_type pos = 0;
            String_t token = lex.getTokenString();
            server::talk::InlineRecognizer::Info info;
            while (m_root.recognizer().find(token, pos, m_kinds, info)) {
                // Something that needs protection
                result.append(token, pos, info.start - pos);
                if (quote_end == String_t::npos) {
                    result += "[noparse]";
                }
                result.append(token, info.start, info.length);
                quote_end = result.size();

                pos = info.start + info.length;
            }
            result.append(token, pos, token.size() - pos);
        } else if (t == BBLexer::TagStart || t == BBLexer::TagEnd || t == BBLexer::AtLink || t == BBLexer::Smiley) {
            // Must be quoted
            if (quote_end == String_t::npos) {
                // We have not started protecting yet, so do it
                result += "[noparse]";
            }
            if (t == BBLexer::TagEnd && lex.getTag() == "noparse") {
                // This would end protection, so wrap a real ender inside the tag
                result += "[/[/noparse]noparse]";
                quote_end = String_t::npos;
            } else {
                // Regular case
                result += lex.getTokenString();
                quote_end = result.size();
            }
        } else {
            // Just append (can't happen)
            result += lex.getTokenString();
        }
    }
    if (quote_end != String_t::npos) {
        result.insert(quote_end, "[/noparse]");
    }
}

void
BBRenderer::renderInline(const TextNode& n, bool in_code)
{
    switch (TextNode::MajorKind(n.major)) {
     case TextNode::maPlain:
        // If the text was not generated by parsing from BBCode, it might contain BBCode tags.
        // We want to keep these text, so protect them from the parser.
        // The easiest way to locate tags is to ask the parser to find them.
        // We then wrap a [noparse] around them. We try to use as few [noparse] as possible,
        // by opening one when needed, and closing it as late as possible.
        // Unfortunately, quoting within [code] is impossible, so the best we can do is to keep the code as is.
        if (in_code) {
            // We cannot quote inside this anyway.
            result += n.text;
        } else {
            renderPlaintext(n.text);
        }
        break;
     case TextNode::maInline:
        if (const char* tag = getInlineTagName(TextNode::InlineFormat(n.minor))) {
            result += "[";
            result += tag;
            result += "]";
            renderChildrenInline(n, in_code);
            result += "[/";
            result += tag;
            result += "]";
        } else {
            renderChildrenInline(n, in_code);
        }
        break;
     case TextNode::maInlineAttr:
        if (const char* tag = getInlineAttrTagName(TextNode::InlineAttrFormat(n.minor))) {
            result += "[";
            result += tag;
            renderAttr(n);
            result += "]";
            renderChildrenInline(n, in_code);
            result += "[/";
            result += tag;
            result += "]";
        } else {
            renderChildrenInline(n, in_code);
        }
        break;
     case TextNode::maLink:
        if (const char* tag = getLinkTagName(TextNode::LinkFormat(n.minor))) {
            renderLink(n, tag);
        } else {
            renderChildrenInline(n, in_code);
        }
        break;

     case TextNode::maSpecial:
        switch (TextNode::SpecialFormat(n.minor)) {
         case TextNode::miSpecialImage:
            renderLink(n, "img");
            break;

         case TextNode::miSpecialBreak:
            result += "[nl]";
            break;

         case TextNode::miSpecialSmiley:
            result += m_kinds.contains(server::talk::InlineRecognizer::Smiley) ? ":" : "[:";
            result += n.text;
            result += m_kinds.contains(server::talk::InlineRecognizer::Smiley) ? ":" : ":]";
        }
        break;

     case TextNode::maParagraph:
     case TextNode::maGroup:
        // Should not happen
        break;
    }
}

void
BBRenderer::renderPG(const TextNode& n)
{
    switch (TextNode::MajorKind(n.major)) {
     case TextNode::maGroup:
        switch (TextNode::GroupFormat(n.minor)) {
         case TextNode::miGroupQuote:
            result += "[quote";
            renderAttr(n);
            result += "]\n";
            renderChildrenPG(n);
            result += "[/quote]";
            break;
         case TextNode::miGroupList:
            result += "[list";
            renderAttr(n);
            result += "]\n";
            renderChildrenPG(n);
            result += "[/list]";
            break;
         case TextNode::miGroupListItem:
            result += "[*] ";
            renderChildrenPG(n);
            break;
         case TextNode::miGroupRoot:
         default:
            renderChildrenPG(n);
            break;
        }
        break;
        
     case TextNode::maParagraph:
        switch (TextNode::ParagraphFormat(n.minor)) {
         case TextNode::miParCode:
            result += "[code";
            renderAttr(n);
            result += "]\n";
            renderChildrenInline(n, true);
            result += "[/code]";
            break;
         case TextNode::miParCentered:
            result += "[center";
            renderAttr(n);
            result += "]\n";
            renderChildrenInline(n, false);
            result += "[/center]";
            break;
         default:
         case TextNode::miParNormal:
            renderChildrenInline(n, false);
            break;
        }
     default:
        break;
    }
}

String_t
server::talk::render::renderBB(const TextNode& node, const Context& ctx, const Options& opts, Root& root, InlineRecognizer::Kinds_t kinds)
{
    String_t result;
    BBRenderer(ctx, opts, root, kinds, result).renderChildrenPG(node);
    return result;
}
