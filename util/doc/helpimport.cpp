/**
  *  \file util/doc/helpimport.cpp
  *  \brief Import PCC2 Help Files
  */

#include <memory>
#include <map>
#include "util/doc/helpimport.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/io/internalsink.hpp"
#include "afl/io/xml/defaultentityhandler.hpp"
#include "afl/io/xml/reader.hpp"
#include "afl/io/xml/tagnode.hpp"
#include "afl/io/xml/textnode.hpp"
#include "afl/io/xml/writer.hpp"
#include "afl/string/format.hpp"
#include "util/charsetfactory.hpp"

using afl::container::PtrVector;
using afl::io::InternalSink;
using afl::io::Stream;
using afl::io::xml::DefaultEntityHandler;
using afl::io::xml::Node;
using afl::io::xml::Nodes_t;
using afl::io::xml::PINode;
using afl::io::xml::Reader;
using afl::io::xml::TagNode;
using afl::io::xml::TextNode;
using afl::io::xml::Writer;
using afl::string::Format;
using afl::string::Translator;
using afl::sys::LogListener;
using util::doc::BlobStore;
using util::doc::Index;

namespace {
    // Log channel name
    const char*const LOG_NAME = "util.doc.import";

    // State for a single page
    struct State {
        // Currently-open tags, innermost last. If the last tag is closed, it is appended as a child to the previous one.
        // If the last tag is closed, it is appended to the result.
        PtrVector<TagNode> pendingTags;

        // Result node list.
        Nodes_t result;

        // Page handle to output the result to
        Index::Handle_t page;

        State(Index::Handle_t page)
            : pendingTags(), result(), page(page)
            { }
    };

    const char* startsWith(const String_t& str, const char* pfx)
    {
        size_t len = std::strlen(pfx);
        if (str.compare(0, len, pfx, len) == 0) {
            return str.data() + len;
        } else {
            return 0;
        }
    }

    // Check for absolute link.
    // Those are not transformed.
    bool isAbsoluteLink(const String_t& s)
    {
        return (startsWith(s, "http:") || startsWith(s, "https:") || startsWith(s, "mailto:") || startsWith(s, "ftp:")
                || startsWith(s, "news:") || startsWith(s, "nntp:") || startsWith(s, "data:")
                || startsWith(s, "site:")
                || startsWith(s, "asset:")
                || startsWith(s, "/"));
    }

    // Transform page name.
    // When we support links pointing outside our space, this would have to detect those.
    // For now, just replace ":" -> "/".
    String_t transformPageName(String_t name)
    {
        for (size_t i = 0; i < name.size(); ++i) {
            if (name[i] == ':') {
                name[i] = '/';
            }
        }
        return name;
    }

    // Transform text. This is an optional step to improve the quality of output (=make it possible to de-duplicate more).
    // It is not required for main functionality.
    // The idea is to normalize text, to make output identical even if input is re-indented, maybe.
    String_t transformText(const String_t& text, bool hasSpace, bool isBlockContext)
    {
        String_t result;
        for (size_t i = 0; i < text.size(); ++i) {
            const char ch = text[i];
            if (ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t') {
                if (!hasSpace) {
                    result += ' ';
                }
                hasSpace = true;
            } else {
                result += ch;
                hasSpace = false;
            }
        }
        if (isBlockContext && !result.empty() && result[result.size()-1] == ' ') {
            result.erase(result.size()-1);
        }
        return result;
    }

    // Get final text node tag.
    // For example, when looking at
    //       <p>......<b>text</b></p>
    // returns text.
    TextNode* getFinalTextNode(const TagNode& n)
    {
        const TagNode* p = &n;
        while (p != 0) {
            if (p->getChildren().empty()) {
                return 0;
            } else {
                Node* last = p->getChildren().back();
                if (const TagNode* asTag = dynamic_cast<TagNode*>(last)) {
                    p = asTag;
                } else {
                    return dynamic_cast<TextNode*>(last);
                }
            }
        }
        return 0;
    }

    // Check for flow-text markup tag.
    bool isFlowTextMarkup(const String_t& tagName)
    {
        return tagName == "a"
            || tagName == "align"
            || tagName == "b"
            || tagName == "big"
            || tagName == "cfg"
            || tagName == "em"
            || tagName == "tt"
            || tagName == "kbd"
            || tagName == "font"
            || tagName == "small"
            || tagName == "cfg";
    }

    // Check for ignorable tag: tag, that only affects grouping
    bool isIgnorableTag(const String_t& tagName)
    {
        return tagName == "help"
            || tagName == "group";
    }

    // If tag ends with whitespace, remove that.
    void trimWhitespace(TagNode& t)
    {
        if (!isFlowTextMarkup(t.getName())) {
            if (TextNode* txt = getFinalTextNode(t)) {
                txt->set(afl::string::strRTrim(txt->get()));
            }
        }
    }

    // Check presence of <pre> tag in a tag stack.
    bool hasPreformattedTag(const PtrVector<TagNode>& ns)
    {
        for (size_t i = 0; i < ns.size(); ++i) {
            if (ns[i]->getName() == "pre") {
                return true;
            }
        }
        return false;
    }

    // Check whether tag stack ends with a space (or an equivalent tag).
    bool hasSpaceOrBreak(const PtrVector<TagNode>& ns)
    {
        for (size_t i = ns.size(); i > 0; --i) {
            const TagNode& n = *ns[i-1];
            if (n.getChildren().empty()) {
                // Last tag has no child: it starts a new context (=ends with space)
                // unless it's flow-text markup.
                if (!isFlowTextMarkup(n.getName())) {
                    return true;
                }
            } else if (const TextNode* tn = getFinalTextNode(n)) {
                // It ends with a text node: check that
                const String_t& text = tn->get();
                return !text.empty() && text[text.size()-1] == ' ';
            } else {
                // Entirely empty: check predecessor
            }
        }
        return false;
    }

    // Check for block context. In block context, we don't expect text, so we can liberally strip spaces.
    bool isBlockContext(const TagNode& n)
    {
        const String_t& nn = n.getName();
        return nn == "dl"
            || nn == "kl"
            || nn == "ol"
            || nn == "ul"
            || nn == "table"
            || nn == "tr";
    }

    // Check for source note: <p><font color="dim"><small>(from FILE:LINE)</small></font></p>
    bool isSourceNote(const Node* n)
    {
        const TagNode* t1 = dynamic_cast<const TagNode*>(n);
        if (t1 == 0 || t1->getName() != "p" || t1->getChildren().size() != 1) {
            return false;
        }

        const TagNode* t2 = dynamic_cast<const TagNode*>(t1->getChildren()[0]);
        if (t2 == 0 || t2->getName() != "font" || t2->getChildren().size() != 1 || t2->getAttributeByName("color") != "dim") {
            return false;
        }

        const TagNode* t3 = dynamic_cast<const TagNode*>(t2->getChildren()[0]);
        if (t3 == 0 || t3->getName() != "small" || t3->getChildren().size() != 1) {
            return false;
        }

        const TextNode* t = dynamic_cast<const TextNode*>(t3->getChildren()[0]);
        if (t == 0 || t->get().substr(0, 6) != "(from ") {
            return false;
        }
        return true;
    }

    // Finish a page: save its content.
    void finishPage(Index& idx, BlobStore& blobStore, State& st, int flags)
    {
        // Remove source note
        if ((flags & util::doc::ImportHelp_RemoveSource) != 0) {
            for (size_t i = 0; i < st.result.size();) {
                if (isSourceNote(st.result[i])) {
                    st.result.erase(st.result.begin() + i);
                } else {
                    ++i;
                }
            }
        }

        // Write
        InternalSink sink;
        Writer(sink).visit(st.result);
        if (!sink.getContent().empty()) {
            idx.setNodeContentId(st.page, blobStore.addObject(sink.getContent()));
        }
    }
}


void
util::doc::importHelp(Index& idx, Index::Handle_t root, BlobStore& blobStore, afl::io::Stream& file, int flags, afl::sys::LogListener& log, afl::string::Translator& tx)
{
    // XML reader
    CharsetFactory csFactory;
    DefaultEntityHandler entityHandler;
    Reader rdr(file, entityHandler, csFactory);
    rdr.setWhitespaceMode(Reader::AllWS);

    // State
    PtrVector<State> state;
    state.pushBackNew(new State(root));

    // Main loop
    Reader::Token tok;
    while ((tok = rdr.readNext()) != Reader::Eof && !state.empty()) {
        State& me = *state.back();
        switch (tok) {
         case Reader::TagStart:
            if (isIgnorableTag(rdr.getTag())) {
                // Ignore
            } else if (rdr.getTag() == "page") {
                // It's a new page: create as template. All attributes added later on.
                state.pushBackNew(new State(idx.addPage(me.page, "", "", "")));
            } else {
                // New tag in page
                me.pendingTags.pushBackNew(new TagNode(rdr.getTag()));
            }
            break;

         case Reader::TagAttribute:
            if (me.pendingTags.empty()) {
                // This is an attribute of the page
                if (rdr.getName() == "id") {
                    if (isAbsoluteLink(rdr.getValue())) {
                        log.write(LogListener::Warn, LOG_NAME, Format(tx("%s:%d: absolute link '%s' used as <page id>"), file.getName(), rdr.getPos(), rdr.getValue()));
                    } else {
                        idx.addNodeIds(me.page, transformPageName(rdr.getValue()));
                    }
                }
            } else {
                // This is an attribute of some document element
                TagNode& tag = *me.pendingTags.back();
                if (tag.getName() == "a" && rdr.getName() == "href") {
                    // Transform link
                    String_t target = isAbsoluteLink(rdr.getValue()) ? rdr.getValue() : transformPageName(rdr.getValue());
                    tag.setAttribute(rdr.getName(), target);
                } else if (tag.getName() == "img" && rdr.getName() == "src") {
                    // <img src> would mean we need to import the image as an asset somehow.
                    if (isAbsoluteLink(rdr.getValue())) {
                        tag.setAttribute(rdr.getName(), rdr.getValue());
                    } else {
                        log.write(LogListener::Warn, LOG_NAME, Format(tx("%s:%d: found <img src>; image import not implemented yet"), file.getName(), rdr.getPos()));
                    }
                } else {
                    // Normal attribute, add it
                    tag.setAttribute(rdr.getName(), rdr.getValue());
                }
            }
            break;

         case Reader::TagEnd:
            if (isIgnorableTag(rdr.getTag())) {
                // Ignore <help>; it should only contain <page>s and no content
            } else if (me.pendingTags.empty()) {
                // Closing a page
                if (rdr.getTag() != "page") {
                    if (state.size() > 1) {
                        log.write(LogListener::Warn, LOG_NAME, Format(tx("%s:%d: mismatching tag names, expect \"</%s>\", found \"</%s>\""), file.getName(), rdr.getPos(), "page", rdr.getTag()));
                    } else {
                        log.write(LogListener::Warn, LOG_NAME, Format(tx("%s:%d: unexpected closing tag \"</%s>\""), file.getName(), rdr.getPos(), rdr.getTag()));
                    }
                }

                finishPage(idx, blobStore, me, flags);
                state.popBack();
            } else {
                // Validate
                std::auto_ptr<TagNode> n(me.pendingTags.extractLast());
                if (n->getName() != rdr.getTag()) {
                    log.write(LogListener::Warn, LOG_NAME, Format(tx("%s:%d: mismatching tag names, expect \"</%s>\", found \"</%s>\""), file.getName(), rdr.getPos(), n->getName(), rdr.getTag()));
                }

                // Process
                trimWhitespace(*n);
                if (me.pendingTags.empty()) {
                    if (n->getName() == "h1") {
                        // Top-level <h1> turns into page name
                        idx.setNodeTitle(me.page, n->getTextContent());
                    } else {
                        // Remember it
                        me.result.pushBackNew(n.release());
                    }
                } else {
                    me.pendingTags.back()->addNewChild(n.release());
                }
            }
            break;

         case Reader::Text:
            // Text
            if (me.pendingTags.empty()) {
                // Raw text on page [irregular case]
                const String_t text = transformText(rdr.getValue(), true, true);
                if (!text.empty()) {
                    me.result.pushBackNew(new TextNode());
                }
            } else {
                // Text within tag
                TagNode& tag = *me.pendingTags.back();
                if (!hasPreformattedTag(me.pendingTags)) {
                    // Normal (flow) text. Normalize whitespace.
                    const String_t text = transformText(rdr.getValue(), hasSpaceOrBreak(me.pendingTags), isBlockContext(tag));
                    if (!text.empty()) {
                        tag.addNewChild(new TextNode(text));
                    }
                } else {
                    // Text in a <pre>.
                    // Do not normalize. However, it commonly starts with a new line; remove that.
                    const String_t text = rdr.getValue();
                    size_t pos = (tag.getName() == "pre" && tag.getChildren().empty()) ? text.find_first_not_of("\r\n") : 0;
                    if (pos != String_t::npos) {
                        tag.addNewChild(new TextNode(text.substr(pos)));
                    }
                }
            }
            break;

         case Reader::Eof:
         case Reader::PIStart:
         case Reader::PIAttribute:
         case Reader::Comment:
         case Reader::Null:
         case Reader::Error:
            // Ignore
            break;
        }
    }

    // Finish remainder
    while (!state.empty()) {
        finishPage(idx, blobStore, *state.back(), flags);
        state.popBack();
    }
}
