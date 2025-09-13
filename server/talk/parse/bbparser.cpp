/**
  *  \file server/talk/parse/bbparser.cpp
  *  \brief Class server::talk::parse::BBParser
  */

#include <cstring>
#include <memory>
#include "server/talk/parse/bbparser.hpp"
#include "afl/base/countof.hpp"
#include "afl/string/string.hpp"
#include "afl/string/parse.hpp"
#include "afl/string/format.hpp"

using server::talk::TextNode;

namespace {
    /*
     *  Color Table
     */

    struct BBColor {
        const char* name;
        const char* rgb;

        static const BBColor* findColorByName(const char* name);
        // static const BBColor* findColorByValue(const char* rgb);
    };

    const BBColor colors[] = {
        // Some usual colors
        { "black",                     "#000000" },
        { "red",                       "#ff0000" },
        { "green",                     "#00ff00" },
        { "blue",                      "#0000ff" },
        { "white",                     "#ffffff" },
        { "yellow",                    "#ffff00" },
        { "magenta",                   "#ff00ff" },
        { "cyan",                      "#00ffff" },

        // PCC colors
        { "player1",                   "#808096" },
        { "player2",                   "#ffffff" },
        { "player3",                   "#ffff00" },
        { "player4",                   "#61f261" },
        { "player5",                   "#6161c2" },
        { "player6",                   "#ff0000" },
        { "player7",                   "#ff55ff" },
        { "player8",                   "#c26100" },
        { "player9",                   "#ffc200" },
        { "player10",                  "#55ffff" },
        { "player11",                  "#00aa00" },
    };

    const BBColor* BBColor::findColorByName(const char* name)
    {
        for (size_t i = 0; i < countof(colors); ++i) {
            if (std::strcmp(colors[i].name, name) == 0) {
                return &colors[i];
            }
        }
        return 0;
    }

    // const BBColor* BBColor::findColorByValue(const char* rgb)
    // {
    //     for (size_t i = 0; i < countof(colors); ++i) {
    //         if (std::strcmp(colors[i].rgb, rgb) == 0) {
    //             return &colors[i];
    //         }
    //     }
    //     return 0;
    // }


    /*
     *  Tag Table
     */
    struct Tag {
        const char* name;
        TextNode::MajorKind major : 8;
        uint8_t minor;
    };

    /** Tags. All tags that are handled in a pretty regular fashion.
        Notable exclusions:
        - [noparse] which has no tree equivalent and is handled ad-hoc
        - [code] which is handled ad-hoc
        - [*] which has no close tag */
    static const Tag tags[] = {
        { "b",      TextNode::maInline,     TextNode::miInBold },
        { "i",      TextNode::maInline,     TextNode::miInItalic },
        { "s",      TextNode::maInline,     TextNode::miInStrikeThrough },
        { "strike", TextNode::maInline,     TextNode::miInStrikeThrough },
        { "u",      TextNode::maInline,     TextNode::miInUnderline },
        { "tt",     TextNode::maInline,     TextNode::miInMonospace },
        { "color",  TextNode::maInlineAttr, TextNode::miIAColor },
        { "size",   TextNode::maInlineAttr, TextNode::miIASize },
        { "font",   TextNode::maInlineAttr, TextNode::miIAFont },
        { "url",    TextNode::maLink,       TextNode::miLinkUrl },
        { "email",  TextNode::maLink,       TextNode::miLinkEmail },
        { "thread", TextNode::maLink,       TextNode::miLinkThread },
        { "post",   TextNode::maLink,       TextNode::miLinkPost },
        { "game",   TextNode::maLink,       TextNode::miLinkGame },
        { "forum",  TextNode::maLink,       TextNode::miLinkForum },
        { "user",   TextNode::maLink,       TextNode::miLinkUser },
        { "img",    TextNode::maSpecial,    TextNode::miSpecialImage },
        { "center", TextNode::maParagraph,  TextNode::miParCentered },
        { "quote",  TextNode::maGroup,      TextNode::miGroupQuote },
        { "list",   TextNode::maGroup,      TextNode::miGroupList },
    };

    const Tag* identify(const String_t& tag)
    {
        for (size_t i = 0; i != countof(tags); ++i) {
            if (tag == tags[i].name) {
                return &tags[i];
            }
        }
        return 0;
    }

    const Tag* findTagName(const TextNode& n)
    {
        for (size_t i = 0; i != countof(tags); ++i) {
            if (tags[i].major == n.major && tags[i].minor == n.minor) {
                return &tags[i];
            }
        }
        return 0;
    }

    /** Convert a maInlineAttr attribute to canonical format. */
    bool canonicalizeAttribute(uint8_t kind, const String_t& in, String_t& out)
    {
        // An empty attribute is never valid
        String_t t(afl::string::strTrim(in));
        if (t.empty()) {
            return false;
        }

        // Type-dependant processing
        int delta;
        static const char digits[] = "0123456789abcdef";
        switch (kind) {
         case TextNode::miIAColor:
            // We accept a number of formats:
            // 1. "#rrggbb"
            // 2. "#rgb"
            // 3. "rrggbb"
            // 4. "rgb"
            // 5. named
            // Output always is "#rrggbb".
            t = afl::string::strLCase(t);
            if (t[0] == '#' && t.size() == 7 && t.find_first_not_of(digits, 1) == t.npos) {
                out = t;
                return true;
            } else if (t[0] == '#' && t.size() == 4 && t.find_first_not_of(digits, 1) == t.npos) {
                out = "#";
                out += t[1]; out += t[1];
                out += t[2]; out += t[2];
                out += t[3]; out += t[3];
                return true;
            } else if (t.size() == 6 && t.find_first_not_of(digits) == t.npos) {
                out = "#";
                out += t;
                return true;
            } else if (t.size() == 3 && t.find_first_not_of(digits) == t.npos) {
                out = "#";
                out += t[0]; out += t[0];
                out += t[1]; out += t[1];
                out += t[2]; out += t[2];
                return true;
            } else if (const BBColor* c = BBColor::findColorByName(t.c_str())) {
                out = c->rgb;
                return true;
            } else {
                return false;
            }

         case TextNode::miIASize:
            // We accept numeric values. Constants are translated into relative sizes, assuming that
            // the default size is 5. Whatever unit that might be.
            if (afl::string::strToInteger(in, delta)) {
                if (in[0] != '-' && in[0] != '+') {
                    delta -= 5;
                }
                if (delta >= -8 && delta <= +8) {
                    out = afl::string::Format("%+d", delta);
                    return true;
                } else {
                    return false;
                }
            } else {
                return false;
            }

         case TextNode::miIAFont:
            // Fonts are taken verbatim, but we refuse characters that we'll most likely
            // not be able to quote
            if (t.find_first_of("\"';\\\n/") != in.npos) {
                return false;
            }
            out = t;
            return true;

         default:
            return false;
        }
    }

    /** Complete a link node.
        If the user didn't specify the link as the attribute, derive it from the content. */
    void completeLink(TextNode& node)
    {
        if (node.text.empty()) {
            // There is no target attribute. Get it from the content.
            if (node.children.size() == 1 && node.children[0]->major == TextNode::maPlain) {
                // It contains just plain text, as in "[user]fruno[/user]".
                // This will give it an empty content, so the renderer will
                // generate it anew.
                node.text = afl::string::strTrim(node.children[0]->text);
                node.children.popBack();
            } else {
                // The content has formatting, as in "[user][b]f[/b]runo[/user]".
                // Assume the user wants to keep that, and just use the raw text.
                node.text = afl::string::strTrim(node.getTextContent());
            }
        }
    }

    /** Check for own text.
        \param node Root node (containing further groups) */
    bool hasOwnText(const TextNode& node)
    {
        for (size_t i = 0; i < node.children.size(); ++i) {
            TextNode* ch = node.children[i];
            if (ch->major != TextNode::maGroup || ch->minor != TextNode::miGroupQuote) {
                return true;
            }
        }
        return false;
    }
}


/*
 *  Invariants: The stack always consists of
 *  - 0 or more groups (maGroup)
 *  - one paragraph (maParagraph)
 *  - 0 or more inline markups (maInline, maInlineAttr, maLink)
 *
 *  There never is a plain-text node on the stack.
 */


server::talk::parse::BBParser::BBParser(BBLexer& lex, InlineRecognizer& recog, InlineRecognizer::Kinds_t options, const LinkParser& lp)
    : lex(lex),
      m_recognizer(recog),
      m_linkParser(lp),
      options(options),
      current(),
      stack()
{
    // ex BBParser::BBParser
    open(TextNode::maGroup, TextNode::miGroupRoot);
    open(TextNode::maParagraph, TextNode::miParNormal);
}


server::talk::TextNode*
server::talk::parse::BBParser::parse()
{
    // ex BBParser::parse
    next();
    while (current != BBLexer::Eof) {
        switch (current) {
         case BBLexer::TagStart:  handleStart();     next(); break;
         case BBLexer::TagEnd:    handleEnd();       next(); break;
         case BBLexer::Smiley:    handleSmiley();    next(); break;
         case BBLexer::Paragraph: handleParagraph(); next(); break;
         case BBLexer::AtLink:    handleAtLink();    next(); break;
         case BBLexer::SuspiciousText:
         case BBLexer::Text:      handleText();              break;
         case BBLexer::Eof:                                  break;
        }
    }
    while (stack.size() > 1) {
        closeAndWarn();
    }
    if (!hasOwnText(*stack.back())) {
        addWarning(NoOwnText, "");
    }
    return stack.extractLast();
}

bool
server::talk::parse::BBParser::isKnownTag(const String_t& tag)
{
    // ex BBParser::isKnownTag
    // xref handleStart()
    return tag == "noparse"
        || tag == "code"
        || tag == "*"
        || tag == "break"
        || tag == "nl"
        || identify(tag) != 0;
}

void
server::talk::parse::BBParser::handleStart()
{
    // ex BBParser::handleStart
    // xref isKnownTag()
    if (lex.getTag() == "noparse") {
        handleNoparse();
    } else if (lex.getTag() == "code") {
        handleCode();
    } else if (lex.getTag() == "*") {
        handleListItem();
    } else if (lex.getTag() == "break") {
        // Close inline and paragraph, insert a break, open a paragraph
        closeInline();
        close();
        open(TextNode::maParagraph, TextNode::miParBreak);
        close();
        open(TextNode::maParagraph, TextNode::miParNormal);
        lex.skipBlanks();
    } else if (lex.getTag() == "nl") {
        // Line break
        open(TextNode::maSpecial, TextNode::miSpecialBreak);
        close();
    } else if (const Tag* t = identify(lex.getTag())) {
        switch (TextNode::MajorKind(t->major)) {
         case TextNode::maInline:
            // Just open it.
            // FIXME: refuse nonempty attributes?
            open(t->major, t->minor);
            break;

         case TextNode::maInlineAttr:
            // Just open it if attribute allows.
            {
                String_t attr;
                if (canonicalizeAttribute(t->minor, lex.getAttribute(), attr)) {
                    open(t->major, t->minor);
                    stack.back()->text = attr;
                } else {
                    appendText(lex.getTokenString());
                }
            }
            break;

         case TextNode::maLink:
            // Open it, but make sure we don't nest it.
            closeLinks();
            open(t->major, t->minor);
            stack.back()->text = lex.getAttribute();
            break;

         case TextNode::maParagraph:
            // Close inline, open paragraph
            closeInline();
            close();
            open(t->major, t->minor);
            lex.skipBlanks();
            break;

         case TextNode::maGroup:
            if (t->minor == TextNode::miGroupList) {
                // Close inline, open the list
                closeInline();
                close();
                open(TextNode::maGroup, TextNode::miGroupList);
                stack.back()->text = lex.getAttribute();
                open(TextNode::maGroup, TextNode::miGroupListItem);
                open(TextNode::maParagraph, TextNode::miParNormal);
                lex.skipBlanks();
            } else if (t->minor == TextNode::miGroupQuote) {
                // Close inline, open the quote
                closeInline();
                close();
                open(TextNode::maGroup, TextNode::miGroupQuote);
                stack.back()->text = lex.getAttribute();
                open(TextNode::maParagraph, TextNode::miParNormal);
                lex.skipBlanks();
            } else {
                // Can't happen
                appendText(lex.getTokenString());
            }
            break;

         case TextNode::maSpecial:
            // Usually an image. Just open it, but don't nest.
            closeUntil(t->major, t->minor);
            open(t->major, t->minor);
            stack.back()->text = lex.getAttribute();
            break;

         case TextNode::maPlain:
            // Can't happen
            appendText(lex.getTokenString());
            break;
        }
    } else {
        // Unknown
        addWarning(SuspiciousText, "");
        appendText(lex.getTokenString());
    }
}

void
server::talk::parse::BBParser::handleEnd()
{
    // ex BBParser::handleEnd
    const Tag* t = identify(lex.getTag());
    if (t == 0) {
        addWarning(SuspiciousText, "");
        appendText(lex.getTokenString());
    } else {
        switch (TextNode::MajorKind(t->major)) {
         case TextNode::maInline:
         case TextNode::maInlineAttr:
         case TextNode::maLink:
         case TextNode::maSpecial:
            // Inline formatting.
            {
                depth_t i = stack.size()-1;
                while (i > 0
                       && (stack[i]->major != t->major || stack[i]->minor != t->minor)
                       && (stack[i]->major == TextNode::maInline
                           || stack[i]->major == TextNode::maInlineAttr
                           || stack[i]->major == TextNode::maLink
                           || (stack[i]->major == TextNode::maSpecial && stack[i]->minor == TextNode::miSpecialImage)))
                {
                    --i;
                }
                if (i == 0) {
                    // Cannot close it (can't happen)
                    appendText(lex.getTokenString());
                } else if (stack[i]->major == t->major && stack[i]->minor == t->minor) {
                    // We can close it
                    close(i);

                    // FIXME:
                    // // Verify link
                    // // We do this only when explicitly closing a link to avoid that errors pile up
                    // if (!stack.empty() && !stack.back()->children.empty()) {
                    //     checkLink(*stack.back()->children.back());
                    // }
                } else if (stack[i]->major == TextNode::maParagraph && t->major == TextNode::maInline) {
                    // We found a paragraph, and this is the end of parameterless inline markup.
                    // Auto-open it, pretending it has always been open.
                    openAt(i+1, t->major, t->minor);
                    stack[i+1]->children.swap(stack[i]->children);
                    close(i+1);
                    addWarning(TagNotOpen, "");
                } else {
                    // Cannot auto-open
                    appendText(lex.getTokenString());
                    addWarning(SuspiciousText, "");
                }
            }
            break;

         case TextNode::maParagraph:
            // Close it if it is open.
            if (closeUntil(t->major, t->minor)) {
                open(TextNode::maParagraph, TextNode::miParNormal);
                lex.skipBlanks();
            } else {
                appendText(lex.getTokenString());
            }
            break;

         case TextNode::maGroup:
            if (t->minor == TextNode::miGroupList) {
                // Closing a list: only close it when there actually is one open
                // FIXME: shouldn't pass Quote
                if (closeUntil(TextNode::maGroup, TextNode::miGroupList)) {
                    open(TextNode::maParagraph, TextNode::miParNormal);
                    lex.skipBlanks();
                } else {
                    // No open list.
                    appendText(lex.getTokenString());
                }
            } else if (t->minor == TextNode::miGroupQuote) {
                // Closing a quote: always close
                if (closeUntil(t->major, t->minor)) {
                    open(TextNode::maParagraph, TextNode::miParNormal);
                    lex.skipBlanks();
                } else {
                    appendText(lex.getTokenString());
                }
            } else {
                appendText(lex.getTokenString());
            }
            break;

         default:
            appendText(lex.getTokenString());
            break;
        }
    }
}

void
server::talk::parse::BBParser::handleSmiley()
{
    // ex BBParser::handleSmiley
    // FIXME: should we refuse invalid smileys at this point?
    open(TextNode::maSpecial, TextNode::miSpecialSmiley);
    stack.back()->text = lex.getTag();
    close();
}

void
server::talk::parse::BBParser::handleParagraph()
{
    // ex BBParser::handleParagraph
    closeInline();
    close();
    open(TextNode::maParagraph, TextNode::miParNormal);
}

void
server::talk::parse::BBParser::handleAtLink()
{
    // ex BBParser::handleAtLink
    closeLinks();
    open(TextNode::maLink, TextNode::miLinkUser);
    stack.back()->text = lex.getAttribute();
    checkLink(*stack.back());
    close();
}

void
server::talk::parse::BBParser::handleText()
{
    // ex BBParser::handleText
    /* This is used for the inside of tags, where text is treated normally
       (i.e. not [code] or [noparse]).

       BBLexer is free to split text anywhere it seems fit, and now also splits at "/".
       This means we need to re-combine to detect URLs. */
    String_t str = lex.getTokenString();
    if (current == BBLexer::SuspiciousText) {
        addWarning(SuspiciousText, "");
    }
    while (1) {
        next();
        if (current == BBLexer::Text || current == BBLexer::SuspiciousText) {
            str += lex.getTokenString();
            if (current == BBLexer::SuspiciousText) {
                addWarning(SuspiciousText, "");
            }
        } else {
            break;
        }
    }

    String_t::size_type pos = 0;
    if (!options.empty()) {
        InlineRecognizer::Info info;
        while (m_recognizer.find(str, pos, options, info)) {
            appendText(String_t(str, pos, info.start - pos));
            switch (info.kind) {
             case InlineRecognizer::Smiley:
                open(TextNode::maSpecial, TextNode::miSpecialSmiley);
                stack.back()->text = info.text;
                close();
                break;

             case InlineRecognizer::Link:
                // No link detection when we are inside a link!
                if (inLink()) {
                    appendText(String_t(str, info.start, info.length));
                } else {
                    open(TextNode::maLink, TextNode::miLinkUrl);
                    stack.back()->text = info.text;
                    close();
                }
                break;
            }
            pos = info.start + info.length;
        }
    }
    appendText(str.substr(pos));
}


void
server::talk::parse::BBParser::handleNoparse()
{
    // ex BBParser::handleNoparse
    next();                     // [noparse]
    while (current != BBLexer::Eof && (current != BBLexer::TagEnd || lex.getTag() != "noparse")) {
        appendText(lex.getTokenString());
        next();
    }
}

void
server::talk::parse::BBParser::handleCode()
{
    // ex BBParser::handleCode
    closeInline();
    close();                    // closes a paragraph
    open(TextNode::maParagraph, TextNode::miParCode);
    stack.back()->text = lex.getAttribute();
    next();                     // [code]
    while (current != BBLexer::Eof && (current != BBLexer::TagEnd || lex.getTag() != "code")) {
        appendText(lex.getTokenString());
        next();
    }
    close();
    open(TextNode::maParagraph, TextNode::miParNormal);
}

void
server::talk::parse::BBParser::handleListItem()
{
    // ex BBParser::handleListItem
    depth_t i = stack.size()-1;
    while (i > 0 && (stack[i]->major != TextNode::maGroup || (stack[i]->minor != TextNode::miGroupListItem && stack[i]->minor != TextNode::miGroupQuote))) {
        --i;
    }
    if (stack[i]->major == TextNode::maGroup && stack[i]->minor == TextNode::miGroupListItem) {
        // Regular case. Close list item and open new one.
        close(i);
        open(TextNode::maGroup, TextNode::miGroupListItem);
        open(TextNode::maParagraph, TextNode::miParNormal);
        lex.skipBlanks();
    } else if (stack[i]->major == TextNode::maGroup && stack[i]->minor == TextNode::miGroupQuote) {
        // Special case. We are inside a quote and have no list open.
        // Open one; probably the user broke the list in the middle when quoting.
        openAt(i+1, TextNode::maGroup, TextNode::miGroupList);
        openAt(i+2, TextNode::maGroup, TextNode::miGroupListItem);
        close(i+2);
        open(TextNode::maGroup, TextNode::miGroupListItem);
        open(TextNode::maParagraph, TextNode::miParNormal);
        lex.skipBlanks();
    } else {
        // No open list item
        appendText(lex.getTokenString());
    }
}

void
server::talk::parse::BBParser::next()
{
    // ex BBParser::next
    current = lex.read();
}

void
server::talk::parse::BBParser::open(TextNode::MajorKind major, uint8_t minor)
{
    // ex BBParser::open
    TextNode* node = new TextNode(major, minor);
    stack.pushBackNew(node);
}

void
server::talk::parse::BBParser::openAt(depth_t n, TextNode::MajorKind major, uint8_t minor)
{
    // ex BBParser::openAt
    TextNode* node = new TextNode(major, minor);
    stack.insertNew(stack.begin()+n, node);
}

void
server::talk::parse::BBParser::appendText(String_t what)
{
    // ex BBParser::appendText
    TextNode* last = stack.back();
    if (last->major == TextNode::maParagraph && last->children.empty() && what.find_first_not_of(" \t\r\n") == String_t::npos) {
        // blank string starting a paragraph, ignore
    } else if (!last->children.empty() && last->children.back()->major == TextNode::maPlain) {
        last->children.back()->text += what;
    } else {
        open(TextNode::maPlain, 0);
        stack.back()->text = what;
        close();
    }
}

void
server::talk::parse::BBParser::close()
{
    // ex BBParser::close
    std::auto_ptr<TextNode> node(stack.extractLast());
    bool keep = true;
    switch (TextNode::MajorKind(node->major)) {
     case TextNode::maPlain:
        keep = !node->text.empty();
        break;
     case TextNode::maInline:
        keep = !node->children.empty();
        break;
     case TextNode::maInlineAttr:
        keep = !node->children.empty();
        break;
     case TextNode::maLink:
        completeLink(*node);
        checkLink(*node);
        keep = true;
        break;
     case TextNode::maParagraph:
        keep = (node->minor == TextNode::miParBreak || !node->children.empty());
        break;
     case TextNode::maGroup:
        if (node->minor == TextNode::miGroupListItem || node->minor == TextNode::miGroupList) {
            keep = !node->children.empty();
        } else {
            keep = true;
        }
        break;
     case TextNode::maSpecial:
        if (node->minor == TextNode::miSpecialImage) {
            completeLink(*node);
        }
        keep = true;
        break;
    }

    if (keep) {
        stack.back()->children.pushBackNew(node.release());
    }
}

void
server::talk::parse::BBParser::close(depth_t n)
{
    // ex BBParser::close
    while (stack.size() > n) {
        close();
    }
}

/** Close all links.
    If the stack contains a link, closes everything up to and including it. */
void
server::talk::parse::BBParser::closeLinks()
{
    // ex BBParser::closeLinks
    depth_t i = stack.size()-1;
    while (i > 0 && stack[i]->major != TextNode::maParagraph && stack[i]->major != TextNode::maGroup && stack[i]->major != TextNode::maLink) {
        --i;
    }
    if (i > 0 && stack[i]->major == TextNode::maLink) {
        while (stack.size() > i) {
            closeAndWarn();
        }
    }
}

/** Close all inline tags.
    Upon return, the stack contains a paragraph. */
void
server::talk::parse::BBParser::closeInline()
{
    // ex BBParser::closeInline
    while (!stack.empty() && stack.back()->major != TextNode::maParagraph) {
        closeAndWarn();
    }
}

/** Close all tags up to a specific one.
    \param major,minor Tag to close
    \retval true tag found and closed
    \retval false tag not found, stack unchanged */
bool
server::talk::parse::BBParser::closeUntil(uint8_t major, uint8_t minor)
{
    // ex BBParser::closeUntil
    depth_t i = stack.size()-1;
    while (i > 0 && (stack[i]->major != major || stack[i]->minor != minor)) {
        --i;
    }
    if (i > 0) {
        close(i);
        return true;
    } else {
        // Tag is not open
        return false;
    }
}

void
server::talk::parse::BBParser::closeAndWarn()
{
    if (const Tag* t = findTagName(*stack.back())) {
        addWarning(MissingClose, t->name);
    }
    close();
}

/** Check for link on stack. */
bool
server::talk::parse::BBParser::inLink() const
{
    // ex BBParser::inLink
    depth_t i = stack.size()-1;
    while (i > 0 && stack[i]->major != TextNode::maParagraph && stack[i]->major != TextNode::maGroup && stack[i]->major != TextNode::maLink && stack[i]->major != TextNode::maSpecial) {
        --i;
    }
    return (i > 0 && (stack[i]->major == TextNode::maLink || stack[i]->major == TextNode::maSpecial));
}

void
server::talk::parse::BBParser::checkLink(TextNode& node)
{
    bool ok = true;
    switch (node.minor) {
     case TextNode::miLinkThread: ok = m_linkParser.parseTopicLink(node.text).isValid();   break;
     case TextNode::miLinkPost:   ok = m_linkParser.parseMessageLink(node.text).isValid(); break;
     case TextNode::miLinkGame:   ok = m_linkParser.parseGameLink(node.text).isValid();    break;
     case TextNode::miLinkForum:  ok = m_linkParser.parseForumLink(node.text).isValid();   break;
     case TextNode::miLinkUser:   ok = m_linkParser.parseUserLink(node.text).isValid();    break;
    }
    if (!ok) {
        addWarning(BadLink, node.text);
    }
}

void
server::talk::parse::BBParser::addWarning(WarningType type, const String_t& extra)
{
    // If multiple warnings are detected at the same place, only add the first one.
    // This reduces the number of warnings if closing tags are missing.
    // NoOwnText is an exception because it is created only once, and seems pretty important.
    const size_t pos = lex.getTokenStart();
    if (type == NoOwnText
        || m_warnings.empty()
        || pos != m_warnings.back().pos)
    {
        m_warnings.push_back(Warning(type, lex.getTokenString(), extra, pos));
    }
}
