/**
  *  \file util/doc/helpimport.cpp
  *  \brief Import PCC2 Help Files
  */

#include <memory>
#include <map>
#include "util/doc/helpimport.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/internalsink.hpp"
#include "afl/io/xml/defaultentityhandler.hpp"
#include "afl/io/xml/reader.hpp"
#include "afl/io/xml/tagnode.hpp"
#include "afl/io/xml/textnode.hpp"
#include "afl/io/xml/writer.hpp"
#include "afl/string/format.hpp"
#include "afl/string/posixfilenames.hpp"
#include "util/charsetfactory.hpp"
#include "util/string.hpp"

using afl::container::PtrVector;
using afl::io::FileMapping;
using afl::io::InternalSink;
using afl::io::Stream;
using afl::io::Directory;
using afl::io::xml::DefaultEntityHandler;
using afl::io::xml::Node;
using afl::io::xml::Nodes_t;
using afl::io::xml::PINode;
using afl::io::xml::Reader;
using afl::io::xml::TagNode;
using afl::io::xml::TextNode;
using afl::io::xml::Writer;
using afl::string::Format;
using afl::string::PosixFileNames;
using afl::string::Translator;
using afl::sys::LogListener;
using util::doc::BlobStore;
using util::doc::Index;
using util::strStartsWith;

namespace {
    // Log channel name
    const char*const LOG_NAME = "util.doc.import";

    // State for a single page (or directory)
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

    // State for a file
    struct FileState {
        // File name in its directory
        String_t name;

        // File source (input file)
        String_t source;

        // Title
        String_t title;

        // Tags (e.g. "lang=en")
        std::vector<String_t> tags;

        // Currently-open XML tags, innermost last. Only for error handling.
        std::vector<String_t> pendingTagNames;
    };

    // Check for absolute link.
    // Those are not transformed.
    bool isAbsoluteLink(const String_t& s)
    {
        return (strStartsWith(s, "http:") || strStartsWith(s, "https:") || strStartsWith(s, "mailto:") || strStartsWith(s, "ftp:")
                || strStartsWith(s, "news:") || strStartsWith(s, "nntp:") || strStartsWith(s, "data:")
                || strStartsWith(s, "site:")
                || strStartsWith(s, "asset:")
                || strStartsWith(s, "/"));
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
            || tagName == "fileset"
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

    // Import a picture
    String_t importImage(BlobStore& blobStore, String_t fileName, Directory& imagePath)
    {
        // Open file
        afl::base::Ptr<Stream> file = imagePath.openFileNT(fileName, afl::io::FileSystem::OpenRead);
        if (file.get() == 0) {
            return String_t();
        }

        // Import file
        BlobStore::ObjectId_t objId = blobStore.addObject(file->createVirtualMapping()->get());

        // Build result file name
        const String_t::size_type p = fileName.find_last_of("\\/:");
        const String_t userName = ((fileName.empty() || p == fileName.size()-1) ? "image"
                                   : p >= fileName.size()
                                   ? fileName
                                   : fileName.substr(p));
        return Format("asset:%s/%s", objId, userName);
    }

    // Make directory name. Each <dir> specifies a name relative to its parent, so we need to merge them.
    String_t makeDirectoryName(Index::Handle_t page, Index& idx, String_t name)
    {
        String_t prefix;
        std::vector<Index::Handle_t> parents = idx.getNodeParents(page);
        if (!parents.empty() && idx.getNumNodeIds(parents.back()) != 0) {
            prefix = idx.getNodeIdByIndex(parents.back(), 0);
        }
        return PosixFileNames().makePathName(prefix, name);
    }

    // Finish a directory
    void finishDirectory(Index& idx, BlobStore& blobStore, State& st)
    {
        // Finish page (=output content)
        finishPage(idx, blobStore, st, 0);

        // Give it a default title
        if (idx.getNodeTitle(st.page).empty() && idx.getNumNodeIds(st.page) != 0) {
            idx.setNodeTitle(st.page, PosixFileNames().getFileName(idx.getNodeIdByIndex(st.page, 0)));
        }
    }

    // Finish a file
    void finishFile(Index& idx, BlobStore& blobStore, State& st, FileState& file, Directory& filePath, LogListener& log, Translator& tx)
    {
        try {
            // Open/import file
            afl::base::Ref<Stream> in = filePath.openFile(file.source, afl::io::FileSystem::OpenRead);
            afl::base::Ref<FileMapping> map = in->createVirtualMapping();
            BlobStore::ObjectId_t objId = blobStore.addObject(map->get());

            // Find Id
            String_t fileId = file.name;
            if (fileId.empty()) {
                fileId = PosixFileNames().getFileName(file.source);
            }

            // Create
            Index::Handle_t hdl = idx.addPage(st.page, fileId, file.title.empty() ? fileId : file.title, objId);
            idx.addNodeTags(hdl, "blob");
            idx.addNodeTags(hdl, Format("size=%d", map->get().size()));
            for (size_t i = 0; i < file.tags.size(); ++i) {
                idx.addNodeTags(hdl, file.tags[i]);
            }
        }
        catch (afl::except::FileProblemException& e) {
            log.write(LogListener::Error, LOG_NAME, tx("Cannot import file"), e);
        }
    }

    // Check for matching tag; warn on mismathc
    void checkMatchingTag(Reader& rdr, afl::io::Stream& file, String_t expect, LogListener& log, Translator& tx)
    {
        if (rdr.getTag() != expect) {
            log.write(LogListener::Warn, LOG_NAME, Format(tx("%s:%d: mismatching tag names, expect \"</%s>\", found \"</%s>\""), file.getName(), rdr.getPos(), expect, rdr.getTag()));
        }
    }

    // Common handling of tag attribute in a document
    void handleTagAttribute(TagNode& tag, Reader& rdr, BlobStore& blobStore, Directory& imagePath, afl::io::Stream& file, LogListener& log, Translator& tx)
    {
        if (tag.getName() == "a" && rdr.getName() == "href") {
            // Transform link
            String_t target = isAbsoluteLink(rdr.getValue()) ? rdr.getValue() : transformPageName(rdr.getValue());
            tag.setAttribute(rdr.getName(), target);
        } else if (tag.getName() == "img" && rdr.getName() == "src") {
            // If link is not absolute, import target as a blob
            if (isAbsoluteLink(rdr.getValue())) {
                tag.setAttribute(rdr.getName(), rdr.getValue());
            } else {
                String_t importedFile = importImage(blobStore, rdr.getValue(), imagePath);
                if (importedFile.empty()) {
                    log.write(LogListener::Warn, LOG_NAME, Format(tx("%s:%d: referenced image not found"), file.getName(), rdr.getPos()));
                } else {
                    tag.setAttribute(rdr.getName(), importedFile);
                }
            }
        } else {
            // Normal attribute, add it
            tag.setAttribute(rdr.getName(), rdr.getValue());
        }
    }

    // Common handling of text in a document (<page>, <dir>)
    void handlePageText(State& me, Reader& rdr, afl::io::Stream& file, LogListener& log, Translator& tx)
    {
        if (me.pendingTags.empty()) {
            // Raw text on page [irregular case]
            const String_t text = transformText(rdr.getValue(), true, true);
            if (!text.empty()) {
                log.write(LogListener::Warn, LOG_NAME, Format(tx("%s:%d: raw text on page"), file.getName(), rdr.getPos()));
                me.result.pushBackNew(new TextNode(text));
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
    }
}

void
util::doc::importHelp(Index& idx, Index::Handle_t root, BlobStore& blobStore, afl::io::Stream& file, afl::io::Directory& imagePath, int flags, afl::sys::LogListener& log, afl::string::Translator& tx)
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
                handleTagAttribute(tag, rdr, blobStore, imagePath, file, log, tx);
            }
            break;

         case Reader::TagEnd:
            if (isIgnorableTag(rdr.getTag())) {
                // Ignore <help>; it should only contain <page>s and no content
            } else if (me.pendingTags.empty()) {
                // Closing a page
                if (rdr.getTag() != "page") {
                    if (state.size() > 1) {
                        checkMatchingTag(rdr, file, "page", log, tx);
                    } else {
                        log.write(LogListener::Warn, LOG_NAME, Format(tx("%s:%d: unexpected closing tag \"</%s>\""), file.getName(), rdr.getPos(), rdr.getTag()));
                    }
                }

                finishPage(idx, blobStore, me, flags);
                state.popBack();
            } else {
                // Validate
                std::auto_ptr<TagNode> n(me.pendingTags.extractLast());
                checkMatchingTag(rdr, file, n->getName(), log, tx);

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
            handlePageText(me, rdr, file, log, tx);
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

void
util::doc::importDownloads(Index& idx, Index::Handle_t root, BlobStore& blobStore, afl::io::Stream& file,
                           afl::io::Directory& imagePath,
                           afl::io::Directory& filePath, afl::sys::LogListener& log, afl::string::Translator& tx)
{
    // XML reader
    CharsetFactory csFactory;
    DefaultEntityHandler entityHandler;
    Reader rdr(file, entityHandler, csFactory);
    rdr.setWhitespaceMode(Reader::AllWS);

    // State
    PtrVector<State> state;
    std::auto_ptr<FileState> fileState;
    state.pushBackNew(new State(root));

    // Main loop
    Reader::Token tok;
    while ((tok = rdr.readNext()) != Reader::Eof && !state.empty()) {
        State& me = *state.back();
        switch (tok) {
         case Reader::TagStart:
            if (isIgnorableTag(rdr.getTag())) {
                // Ignore
            } else if (fileState.get() != 0) {
                // Tag inside <file>
                if (fileState->pendingTagNames.empty()) {
                    log.write(LogListener::Warn, LOG_NAME, Format(tx("%s:%d: tag <%s> inside <file> unexpected"), file.getName(), rdr.getPos(), rdr.getTag()));
                }
                fileState->pendingTagNames.push_back(rdr.getTag());
            } else if (rdr.getTag() == "file") {
                // New file
                fileState.reset(new FileState());
            } else if (rdr.getTag() == "dir") {
                // New directory: create as template. All attributes added later on.
                state.pushBackNew(new State(idx.addDocument(me.page, "", "", "")));
            } else {
                // New tag in page
                me.pendingTags.pushBackNew(new TagNode(rdr.getTag()));
            }
            break;

         case Reader::TagAttribute:
            if (fileState.get() != 0) {
                if (fileState->pendingTagNames.empty()) {
                    // This is an attribute of the file
                    if (rdr.getName() == "src") {
                        fileState->source = rdr.getValue();
                    } else if (rdr.getName() == "name") {
                        fileState->name = rdr.getValue();
                    } else if (rdr.getName() == "title") {
                        fileState->title = rdr.getValue();
                    } else if (rdr.getName() == "tag") {
                        fileState->tags.push_back(rdr.getValue());
                    } else if (rdr.getName() == "date") {
                        fileState->tags.push_back(Format("date=%s", rdr.getValue()));
                    } else {
                        // Ignore extra attribute
                    }
                }
            } else if (me.pendingTags.empty()) {
                // This is an attribute of the directory
                if (rdr.getName() == "id") {
                    idx.addNodeIds(me.page, makeDirectoryName(me.page, idx, rdr.getValue()));
                } else if (rdr.getName() == "title") {
                    idx.setNodeTitle(me.page, rdr.getValue());
                } else if (rdr.getName() == "tag") {
                    idx.addNodeTags(me.page, rdr.getValue());
                } else if (rdr.getName() == "date") {
                    idx.addNodeTags(me.page, Format("date=%s", rdr.getValue()));
                } else {
                    // ignore
                }
            } else {
                // This is an attribute of some document (directory) element
                TagNode& tag = *me.pendingTags.back();
                handleTagAttribute(tag, rdr, blobStore, imagePath, file, log, tx);
            }
            break;

         case Reader::TagEnd:
            if (isIgnorableTag(rdr.getTag())) {
                // Ignore <help>; it should only contain <page>s and no content
            } else if (fileState.get() != 0) {
                // Tag inside <file>
                if (!fileState->pendingTagNames.empty()) {
                    checkMatchingTag(rdr, file, fileState->pendingTagNames.back(), log, tx);
                    fileState->pendingTagNames.pop_back();
                } else {
                    checkMatchingTag(rdr, file, "file", log, tx);
                    finishFile(idx, blobStore, me, *fileState, filePath, log, tx);
                    fileState.reset();
                }
            } else if (me.pendingTags.empty()) {
                // Closing a page
                if (rdr.getTag() != "dir") {
                    if (state.size() > 1) {
                        checkMatchingTag(rdr, file, "dir", log, tx);
                    } else {
                        log.write(LogListener::Warn, LOG_NAME, Format(tx("%s:%d: unexpected closing tag \"</%s>\""), file.getName(), rdr.getPos(), rdr.getTag()));
                    }
                }

                finishDirectory(idx, blobStore, me);
                state.popBack();
            } else {
                // Validate
                std::auto_ptr<TagNode> n(me.pendingTags.extractLast());
                checkMatchingTag(rdr, file, n->getName(), log, tx);

                // Process
                trimWhitespace(*n);
                if (me.pendingTags.empty()) {
                    me.result.pushBackNew(n.release());
                } else {
                    me.pendingTags.back()->addNewChild(n.release());
                }
            }
            break;

         case Reader::Text:
            // Text
            if (fileState.get() != 0) {
                // Ignore text in file
            } else {
                handlePageText(me, rdr, file, log, tx);
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
        finishDirectory(idx, blobStore, *state.back());
        state.popBack();
    }
}
