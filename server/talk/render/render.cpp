/**
  *  \file server/talk/render/render.cpp
  */

#include <memory>
#include "server/talk/render/render.hpp"
#include "afl/string/format.hpp"
#include "server/talk/message.hpp"
#include "server/talk/parse/bblexer.hpp"
#include "server/talk/parse/bbparser.hpp"
#include "server/talk/render/context.hpp"
#include "server/talk/render/htmlrenderer.hpp"
#include "server/talk/render/options.hpp"
#include "server/talk/render/textrenderer.hpp"
#include "server/talk/textnode.hpp"
#include "server/talk/user.hpp"
#include "server/talk/render/mailrenderer.hpp"
#include "server/talk/render/bbrenderer.hpp"

namespace server { namespace talk { namespace render { namespace {

    void addParagraph(TextNode& root, const String_t& str)
    {
        std::auto_ptr<TextNode> par(new TextNode(TextNode::maParagraph, TextNode::miParNormal));
        std::auto_ptr<TextNode> txt(new TextNode(TextNode::maPlain, 0, str));
        par->children.pushBackNew(txt.release());
        root.children.pushBackNew(par.release());
    }

    bool isForum(const String_t& str, InlineRecognizer::Kinds_t& set, String_t::size_type& len)
    {
        const String_t::size_type LEN = 5;

        /* Check for prefix */
        if (str.size() < LEN || str.compare(0, LEN, "forum", LEN) != 0) {
            return false;
        }

        /* Check options */
        set.clear();
        String_t::size_type i = LEN;
        while (i < str.size()) {
            if (str[i] == 'S') {
                set += InlineRecognizer::Smiley;
                ++i;
            } else if (str[i] == 'L') {
                set += InlineRecognizer::Link;
                ++i;
            } else if (str[i] >= 'A' && str[i] <= 'Z') {
                // Forward compatibility
                ++i;
            } else {
                break;
            }
        }
        len = i;
        return true;
    }

    // FIXME: should we move this function into namespace server::talk::parse?
    // Doing so would require sharing the 'isForum' function somehow.
    TextNode* doParse(const String_t& str, InlineRecognizer& recog)
    {
        String_t::size_type len;
        InlineRecognizer::Kinds_t set;
        if (isForum(str, set, len) && len < str.size() && str[len] == ':') {
            // parse as BBCode
            server::talk::parse::BBLexer lex(str.substr(len+1));
            return server::talk::parse::BBParser(lex, recog, set).parse();
        } else if (str.size() >= 5 && str.compare(0, 5, "text:", 5) == 0) {
            // parse as text
            std::auto_ptr<TextNode> result(new TextNode(TextNode::maGroup, TextNode::miGroupRoot));
            String_t::size_type i = 5, n;
            while ((n = str.find_first_of("\r\n", i)) != String_t::npos) {
                if (n != i) {
                    addParagraph(*result, String_t(str, i, n-i));
                }
                i = n+1;
            }
            if (i != str.size() || result->children.empty()) {
                addParagraph(*result, String_t(str, i));
            }
            return result.release();
        } else if (str.size() >= 5 && str.compare(0, 5, "code:", 5) == 0) {
            // syntax highlighting
            String_t::size_type n = str.find(':', 5);
            String_t language;
            if (n != str.npos) {
                language.assign(str, 5, n-5);
                ++n;
            } else {
                n = 5;
            }

            std::auto_ptr<TextNode> result(new TextNode(TextNode::maGroup, TextNode::miGroupRoot));
            std::auto_ptr<TextNode> par(new TextNode(TextNode::maParagraph, TextNode::miParCode, language));
            std::auto_ptr<TextNode> txt(new TextNode(TextNode::maPlain, 0, str.substr(n)));
            par->children.pushBackNew(txt.release());
            result->children.pushBackNew(par.release());
            return result.release();
        } else {
            // error, unsupported format.
            std::auto_ptr<TextNode> result(new TextNode(TextNode::maGroup, TextNode::miGroupRoot));
            addParagraph(*result, str);
            return result.release();
        }
    }

    bool stripBreak(TextNode* n)
    {
        if (n->major == TextNode::maParagraph && n->minor == TextNode::miParBreak) {
            return true;
        } else {
            size_t i = 0, e = n->children.size();
            while (i < e && !stripBreak(n->children[i])) {
                ++i;
            }
            if (i < e) {
                // There was a break among our children: drop everything after
                ++i;
                while (i < n->children.size()) {
                    n->children.popBack();
                }
                return true;
            } else {
                // No break, keep it
                return false;
            }
        }
    }

    /****************************** 'abstract:' ******************************/

    class Abstract {
     public:
        Abstract();
        bool strip(TextNode* n);
     private:
        size_t paras;
        size_t chars;
    };

    Abstract::Abstract()
        : paras(2),
          chars(200)
    { }

    bool Abstract::strip(TextNode* n)
    {
        if (n->major == TextNode::maParagraph && n->minor == TextNode::miParBreak) {
            // [break]
            return true;
        } else if ((n->major == TextNode::maParagraph || n->major == TextNode::maGroup || n->major == TextNode::maPlain) && (paras == 0 || chars == 0)) {
            // a paragraph although no more are allowed
            return true;
        } else if (n->major == TextNode::maPlain) {
            // text
            if (n->text.size() <= chars) {
                chars -= n->text.size();
            } else {
                while (chars > 0 && std::strchr(" \t\r\n", n->text[chars-1]) == 0) {
                    --chars;
                }
                n->text.erase(chars);
                n->text += "...";
                chars = 0;
            }
            return false;
        } else {
            // check content
            std::size_t i = 0, e = n->children.size();
            while (i < e && !strip(n->children[i])) {
                ++i;
            }
            if (i < e) {
                // There was a break among our children: drop everything after
                while (i < n->children.size()) {
                    n->children.popBack();
                }
            }

            // count paragraphs
            if (n->major == TextNode::maParagraph && paras > 0) {
                --paras;
            }

            // No break, keep it
            return false;
        }
    }

} } } }

String_t
server::talk::render::renderText(const String_t& text, const Context& ctx, const Options& opts, Root& root)
{
    // ex RenderState::render
    const String_t& format = opts.getFormat();
    if (format == "raw") {
        // raw format requested
        return text;
    } else if (format == "format") {
        // format requested
        String_t::size_type n = text.find(':');
        if (n == String_t::npos) {
            n = text.size();
        }
        return text.substr(0, n);
    } else if (format.find(':') == format.npos
               && text.size() > format.size()
               && text.compare(0, format.size(), format) == 0
               && text[format.size()] == ':')
    {
        // requested same format as stored
        return text.substr(format.size()+1);
    } else {
        // transformation required
        std::auto_ptr<TextNode> tree(doParse(text, root.recognizer()));
        return renderText(tree, ctx, opts, root);
    }
}

String_t
server::talk::render::renderText(std::auto_ptr<TextNode> tree, const Context& ctx, const Options& opts, Root& root)
{
    // perform conversions
    // FIXME: port to StringParser?
    String_t fmt = opts.getFormat();
    while (1) {
        if (fmt.size() > 6 && fmt.compare(0, 6, "quote:", 6) == 0) {
            // quote: quote entire message
            fmt.erase(0, 6);
            std::auto_ptr<TextNode> tmp(new TextNode(TextNode::maGroup, TextNode::miGroupQuote));

            if (ctx.getMessageId() > 0) {
                // messageId is trusted, so no permission checks required
                Message m(root, ctx.getMessageId());
                User u(root, m.author().get());
                tmp->text = afl::string::Format("%s;%d", u.getLoginName(), ctx.getMessageId());
            } else if (!ctx.getMessageAuthor().empty()) {
                User u(root, ctx.getMessageAuthor());
                tmp->text = u.getLoginName();
            } else {
                // No context information given for quote
            }
            tmp->children.swap(tree->children);
            tree->children.pushBackNew(tmp.release());
        } else if (fmt.size() > 8 && fmt.compare(0, 8, "noquote:", 8) == 0) {
            // noquote: remove all quotes
            fmt.erase(0, 8);
            tree->stripQuotes();
        } else if (fmt.size() > 6 && fmt.compare(0, 6, "break:", 6) == 0) {
            // render up to break
            fmt.erase(0, 6);
            stripBreak(tree.get());
        } else if (fmt.size() > 6 && fmt.compare(0, 9, "abstract:", 9) == 0) {
            // render abstract
            fmt.erase(0, 9);
            tree->stripQuotes();
            Abstract().strip(tree.get());
        } else if (fmt.size() > 6 && fmt.compare(0, 6, "force:", 6) == 0) {
            // force: null operation just to force re-rendering
            fmt.erase(0, 6);
        } else {
            break;
        }
    }

    // render it
    String_t::size_type len;
    server::talk::InlineRecognizer::Kinds_t set;
    if (fmt == "html") {
        return renderHTML(*tree, ctx, opts, root);
    } else if (isForum(fmt, set, len) && len == fmt.size()) {
        return renderBB(*tree, ctx, opts, root, set);
    } else if (fmt == "mail") {
        return renderMail(tree.get(), ctx, opts, root, false);
    } else if (fmt == "news") {
        return renderMail(tree.get(), ctx, opts, root, true);
    } else if (fmt == "text") {
        return renderText(tree.get(), ctx, root);
    } else {
        // error
        return "ERROR: invalid format '" + fmt + "'";
    }
}
