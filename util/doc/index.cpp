/**
  *  \file util/doc/index.cpp
  *  \brief Class util::doc::Index
  */

#include <vector>
#include "util/doc/index.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/except/fileformatexception.hpp"
#include "afl/io/xml/defaultentityhandler.hpp"
#include "afl/io/xml/reader.hpp"
#include "afl/io/xml/writer.hpp"
#include "afl/string/format.hpp"
#include "afl/test/assert.hpp"
#include "util/charsetfactory.hpp"
#include "util/stringparser.hpp"

using afl::container::PtrVector;
using afl::except::FileFormatException;
using afl::io::Stream;
using afl::io::xml::DefaultEntityHandler;
using afl::io::xml::Reader;
using afl::io::xml::Writer;
using afl::string::Format;
using afl::test::Assert;
using util::StringParser;
using util::doc::Index;

struct util::doc::Index::Node {
    enum Type {
        Document,
        Page
    };
    Type type;
    std::vector<String_t> ids;
    std::vector<String_t> tags;
    String_t title;
    ObjectId_t contentId;
    PtrVector<Node> children;
    Node* parent;
    size_t indexInParent;

    explicit Node(Type type)
        : type(type), ids(), tags(), title(), contentId(), children(), parent(), indexInParent()
        { }
};

namespace {
    void reportError(Stream& in, Reader& rdr, String_t message)
    {
        throw FileFormatException(in, Format("%s (file position: %d)", message, rdr.getPos()));
    }

    // Split text at commas and add parts to out vector.
    void addCommaSeparated(std::vector<String_t>& out, const String_t& text)
    {
        StringParser p(text);
        while (!p.parseEnd()) {
            String_t tmp;
            p.parseDelim(",", tmp);
            p.parseCharacter(',');

            String_t id = afl::string::strTrim(tmp);
            if (!id.empty()) {
                out.push_back(id);
            }
        }
    }

    // Output a vector as attribute
    void addAttribute(String_t& out, const std::vector<String_t>& vec, const char* name)
    {
        if (!vec.empty()) {
            out += " ";
            out += name;
            out += "=\"";
            out += Writer::escape(vec[0]);
            for (size_t i = 1; i < vec.size(); ++i) {
                out += ",";
                out += Writer::escape(vec[i]);
            }
            out += "\"";
        }
    }

    // Output a value as attribute
    void addAttribute(String_t& out, const String_t& att, const char* name)
    {
        if (!att.empty()) {
            out += " ";
            out += name;
            out += "=\"";
            out += Writer::escape(att);
            out += "\"";
        }
    }

    // Create a child node. Use this to properly set the backlinks.
    Index::Node& addNode(Index::Node& node, Index::Node::Type type)
    {
        Index::Node& p = *node.children.pushBackNew(new Index::Node(type));
        p.parent = &node;
        p.indexInParent = node.children.size()-1;
        return p;
    }

    // Dereference a handle into a Node&. Throws assertion failure on error.
    Index::Node& getNode(const Assert& a, Index::Handle_t hdl)
    {
        a.check("null handle", hdl != 0);
        return *hdl;
    }

    // Get next sibling of a node; null if none.
    Index::Node* nextSibling(Index::Node& node)
    {
        if (node.parent != 0 && node.indexInParent+1 < node.parent->children.size()) {
            return node.parent->children[node.indexInParent+1];
        } else {
            return 0;
        }
    }

    // Pick Id for a node. Use prefId if that would be a valid choice.
    const String_t& pickId(const std::vector<String_t>& ids, const String_t& prefId)
    {
        if (!prefId.empty()) {
            for (size_t i = 0, n = ids.size(); i < n; ++i) {
                if (ids[i] == prefId) {
                    return prefId;
                }
            }
        }
        return ids[0];
    }

    // Check whether any Id in a matches any in b.
    bool matchIds(const std::vector<String_t>& a, const std::vector<String_t>& b)
    {
        for (size_t i = 0, n = a.size(); i < n; ++i) {
            for (size_t j = 0, m = b.size(); j < m; ++j) {
                if (a[i] == b[j]) {
                    return true;
                }
            }
        }
        return false;
    }
}

/*
 *  Index
 */

util::doc::Index::Index()
    : m_root(new Node(Node::Document))
{ }

util::doc::Index::~Index()
{ }

void
util::doc::Index::load(afl::io::Stream& in)
{
    // XML stuff
    DefaultEntityHandler eh;
    CharsetFactory csFactory;
    Reader rdr(in, eh, csFactory);

    // Stack
    std::vector<Node*> stack;
    Reader::Token tok;
    while ((tok = rdr.readNext()) != Reader::Eof) {
        switch (tok) {
         case Reader::TagStart:
            // Opening tag: push on stack
            if (rdr.getTag() == "index") {
                // <index> is only valid on top-level
                if (!stack.empty()) {
                    reportError(in, rdr, "<index> only valid on top-level");
                }
                stack.push_back(root());
            } else if (rdr.getTag() == "doc") {
                // <doc> is only valid on not-top-level, below <doc>
                if (stack.empty() || stack.back() == 0 || stack.back()->type == Node::Page) {
                    reportError(in, rdr, "<doc> not valid here");
                }
                stack.push_back(&addNode(*stack.back(), Node::Document));
            } else if (rdr.getTag() == "page") {
                // <page> requires at least two parents (root, document)
                if (stack.size() < 2 || stack.back() == 0) {
                    reportError(in, rdr, "<page> not valid here");
                }
                stack.push_back(&addNode(*stack.back(), Node::Page));
            } else {
                // Unknown; ignore
                stack.push_back(0);
            }
            break;

         case Reader::TagAttribute:
            // Attribute
            if (stack.size() > 1 && stack.back() != 0) {
                if (rdr.getName() == "id") {
                    addNodeIds(stack.back(), rdr.getValue());
                } else if (rdr.getName() == "tag") {
                    addNodeTags(stack.back(), rdr.getValue());
                } else if (rdr.getName() == "title") {
                    stack.back()->title = rdr.getValue();
                } else if (rdr.getName() == "content") {
                    stack.back()->contentId = rdr.getValue();
                }
            }
            break;

         case Reader::TagEnd:
            // Closing tag
            if (stack.empty()) {
                reportError(in, rdr, "Misplaced closing tag");
            }
            if (stack.back() == 0) {
                // Unknown: always accept
            } else {
                // Match tag
                const char* expectedTag = (stack.back()->type == Node::Page ? "page" : stack.size() == 1 ? "index" : "doc");
                if (rdr.getTag() != expectedTag) {
                    reportError(in, rdr, Format("Mismatching closing tag, expecting </%s>, found </%s>", expectedTag, rdr.getTag()));
                }

                // Everything but the root requires an Id
                if (stack.size() > 1 && stack.back()->ids.empty()) {
                    reportError(in, rdr, "Missing Id attribute");
                }
            }
            stack.pop_back();
            break;

         case Reader::Eof:
         case Reader::PIStart:
         case Reader::PIAttribute:
         case Reader::Comment:
         case Reader::Text:
         case Reader::Null:
            // Ignore
            break;
         case Reader::Error:
            reportError(in, rdr, "XML parser reports error");
            break;
        }
    }

    // Post-verify
    if (!stack.empty()) {
        reportError(in, rdr, Format("Missing %d closing tags", stack.size()));
    }
}

void
util::doc::Index::save(afl::io::Stream& out) const
{
    afl::io::TextFile textOut(out);
    saveNode(textOut, *m_root, 0);
    textOut.flush();
}

util::doc::Index::Handle_t
util::doc::Index::root() const
{
    return &*m_root;
}

util::doc::Index::Handle_t
util::doc::Index::addDocument(Handle_t parent, String_t ids, String_t title, ObjectId_t contentId)
{
    Assert a("<addDocument>");
    Node& p = getNode(a, parent);
    a.check("type", p.type == Node::Document);

    Node& result = addNode(p, Node::Document);
    addNodeIds(&result, ids);
    result.title = title;
    result.contentId = contentId;
    return &result;
}

util::doc::Index::Handle_t
util::doc::Index::addPage(Handle_t parent, String_t ids, String_t title, ObjectId_t contentId)
{
    Assert a("<addPage>");
    Node& p = getNode(a, parent);
    a.check("root", parent != root());

    Node& result = addNode(p, Node::Page);
    addNodeIds(&result, ids);
    result.title = title;
    result.contentId = contentId;
    return &result;
}

void
util::doc::Index::addNodeIds(Handle_t node, String_t ids)
{
    addCommaSeparated(getNode("<addNodeIds>", node).ids, ids);
}

void
util::doc::Index::addNodeTags(Handle_t node, String_t tags)
{
    addCommaSeparated(getNode("<addNodeTags>", node).tags, tags);
}

bool
util::doc::Index::isNodePage(Handle_t node) const
{
    return getNode("<isNodePage>", node).type == Node::Page;
}

void
util::doc::Index::setNodeTitle(Handle_t node, const String_t& title)
{
    getNode("<setNodeTitle>", node).title = title;
}

String_t
util::doc::Index::getNodeTitle(Handle_t node) const
{
    return getNode("<getNodeTitle>", node).title;
}

void
util::doc::Index::setNodeContentId(Handle_t node, ObjectId_t contentId)
{
    getNode("<setNodeContentId>", node).contentId = contentId;
}

util::doc::Index::ObjectId_t
util::doc::Index::getNodeContentId(Handle_t node) const
{
    return getNode("<getNodeContentId>", node).contentId;
}

size_t
util::doc::Index::getNumNodeIds(Handle_t node) const
{
    return getNode("<getNumNodeIds>", node).ids.size();
}

String_t
util::doc::Index::getNodeIdByIndex(Handle_t node, size_t index) const
{
    Assert a("<getNodeIdByIndex>");
    const Node& n = getNode(a, node);
    a.check("index", index < n.ids.size());
    return n.ids[index];
}

size_t
util::doc::Index::getNumNodeTags(Handle_t node) const
{
    return getNode("<getNumNodeTags>", node).tags.size();
}

String_t
util::doc::Index::getNodeTagByIndex(Handle_t node, size_t index) const
{
    Assert a("<getNodeTagByIndex>");
    const Node& n = getNode(a, node);
    a.check("index", index < n.tags.size());
    return n.tags[index];
}

size_t
util::doc::Index::getNumNodeChildren(Handle_t node) const
{
    return getNode("<getNumNodeChildren>", node).children.size();
}

util::doc::Index::Handle_t
util::doc::Index::getNodeChildByIndex(Handle_t node, size_t index) const
{
    Assert a("<getNodeChildByIndex>");
    const Node& n = getNode(a, node);
    a.check("index", index < n.children.size());
    return n.children[index];
}

size_t
util::doc::Index::getNodeParentIndex(Handle_t node) const
{
    return getNode("<getNodeParentIndex>", node).indexInParent;
}

std::vector<util::doc::Index::Handle_t>
util::doc::Index::getNodeParents(Handle_t node) const
{
    const Node& n = getNode("<getNodeParents>", node);
    if (n.parent != 0) {
        std::vector<Handle_t> result = getNodeParents(n.parent);
        result.push_back(n.parent);
        return result;
    } else {
        return std::vector<Handle_t>();
    }
}

std::vector<util::doc::Index::TaggedNode>
util::doc::Index::getNodeNavigationContext(Handle_t node) const
{
    Node& n = getNode("<getNodeNavigationContext>", node);
    std::vector<TaggedNode> result;

    // Up
    if (n.parent != 0) {
        result.push_back(TaggedNode(n.parent, NAV_UP));
    }

    // Previous direct and indirect
    if (n.parent != 0) {
        if (n.indexInParent != 0) {
            Node* p = n.parent->children[n.indexInParent-1];
            result.push_back(TaggedNode(p, NAV_PREVIOUS_DIRECT));

            // Indirect predecessor is last child of direct predecessor
            while (!p->children.empty()) {
                p = p->children.back();
            }
            result.push_back(TaggedNode(p, NAV_PREVIOUS_INDIRECT));
        } else {
            result.push_back(TaggedNode(n.parent, NAV_PREVIOUS_INDIRECT));
        }
    }

    // Next direct
    if (Node* next = nextSibling(n)) {
        result.push_back(TaggedNode(next, NAV_NEXT_DIRECT));
    }

    // Next indirect
    if (!n.children.empty()) {
        result.push_back(TaggedNode(n.children[0], NAV_NEXT_INDIRECT));
    } else {
        Node* p = &n;
        while (p != 0) {
            if (Node* next = nextSibling(*p)) {
                result.push_back(TaggedNode(next, NAV_NEXT_INDIRECT));
                break;
            }
            p = p->parent;
        }
    }

    return result;
}

std::vector<util::doc::Index::TaggedNode>
util::doc::Index::getNodeChildren(Handle_t node, int maxDepth, bool acrossDocuments) const
{
    std::vector<TaggedNode> result;
    listNodeChildren(result, getNode("<getNodeChildren>", node), 1, maxDepth, acrossDocuments);
    return result;
}

std::vector<util::doc::Index::RelatedNode>
util::doc::Index::getNodeRelatedVersions(Handle_t node) const
{
    std::vector<RelatedNode> result;
    Node& n = getNode("<getNodeRelatedVersions>", node);
    if (n.type == Node::Page) {
        listNodeRelatedDocuments(result, *root(), n.ids);
    }
    return result;
}

String_t
util::doc::Index::getNodeAddress(Handle_t node, String_t docId) const
{
    Assert a("<getNodeAddress>");
    const Node& n = getNode(a, node);
    switch (n.type) {
     case Node::Document:
        // Document: use first Id if any
        if (!n.ids.empty()) {
            return pickId(n.ids, docId);
        }
        break;

     case Node::Page:
        // Page: use first Id, plus first Id of containing document
        if (!n.ids.empty()) {
            const Node* p = getNodeContainingDocument(node);
            if (p != 0 && !p->ids.empty()) {
                return pickId(p->ids, docId) + "/" + n.ids.front();
            }
        }
        break;
    }

    // Error case
    return String_t();
}

util::doc::Index::Handle_t
util::doc::Index::getNodeContainingDocument(Handle_t node) const
{
    Node* n = node;
    while (n != 0 && n->type != Node::Document) {
        n = n->parent;
    }
    return n ? n : root();
}

bool
util::doc::Index::findNodeByAddress(const String_t& address, Handle_t& result, String_t& docId) const
{
    if (Node* p = findDocumentByAddress(*m_root, afl::string::toMemory(address), docId)) {
        result = p;
        return true;
    } else {
        return false;
    }
}

// Create output for a node (and its children).
// @param out Output stream
// @param node Node to output
// @param level Indentation level
void
util::doc::Index::saveNode(afl::io::TextFile& out, const Node& node, size_t level)
{
    const char* tagName = (level == 0 ? "index" : node.type == Node::Page ? "page" : "doc");
    String_t line(level, ' ');
    line += '<';
    line += tagName;

    // Attributes
    addAttribute(line, node.ids, "id");
    addAttribute(line, node.tags, "tag");
    addAttribute(line, node.title, "title");
    addAttribute(line, node.contentId, "content");

    // Children
    if (node.children.empty()) {
        line += " />";
        out.writeLine(line);
    } else {
        line += ">";
        out.writeLine(line);
        for (size_t i = 0; i < node.children.size(); ++i) {
            saveNode(out, *node.children[i], level+1);
        }

        String_t close(level, ' ');
        close += "</";
        close += tagName;
        close += ">";
        out.writeLine(close);
    }
}

// Find node, assuming we are in a document node.
// This means we accept exact matches (=return document), or prefix matches (=search child page).
// @param [in]  node     Node to consider
// @param [in]  address  Address to look for
// @param [out] docId    Id of found document
util::doc::Index::Node*
util::doc::Index::findDocumentByAddress(Node& node, afl::string::ConstStringMemory_t address, String_t& docId)
{
    // Correct type?
    if (node.type != Node::Document) {
        return 0;
    }

    // Match name?
    for (size_t i = 0, n = node.ids.size(); i < n; ++i) {
        afl::string::ConstStringMemory_t id(afl::string::toMemory(node.ids[i]));
        if (id.size() == address.size() && address.equalContent(id)) {
            // Exact match?
            docId = node.ids[i];
            return &node;
        }
        if (id.size() < address.size() && address.subrange(0, id.size()).equalContent(id)) {
            const char* ch = address.at(id.size());
            if (ch != 0 && *ch == '/') {
                // Prefix match; check for page
                for (size_t j = 0, m = node.children.size(); j < m; ++j) {
                    if (Node* p = findPageByAddress(*node.children[j], address.subrange(id.size()+1))) {
                        docId = node.ids[i];
                        return p;
                    }
                }
            }
        }
    }


    // No match so far; match sub-documents
    for (size_t i = 0, n = node.children.size(); i < n; ++i) {
        if (Node* p = findDocumentByAddress(*node.children[i], address, docId)) {
            return p;
        }
    }
    return 0;
}

// Find node, assuming we are in a page node.
// This means we only accept exact matches.
// @param node     Node to consider
// @param address  Partial address to look for (=page name suffix)
util::doc::Index::Node*
util::doc::Index::findPageByAddress(Node& node, afl::string::ConstStringMemory_t address)
{
    // Correct type?
    if (node.type != Node::Page) {
        return 0;
    }

    // Match name?
    for (size_t i = 0, n = node.ids.size(); i < n; ++i) {
        afl::string::ConstStringMemory_t id(afl::string::toMemory(node.ids[i]));
        if (address.equalContent(id)) {
            // Exact match?
            return &node;
        }
    }

    // No match so far; match sub-pages
    for (size_t i = 0, n = node.children.size(); i < n; ++i) {
        if (Node* p = findPageByAddress(*node.children[i], address)) {
            return p;
        }
    }
    return 0;
}

// List children of node.
// @param [out] out              Result produced here
// @param [in]  node             Node to consider
// @param [in]  thisDepth        Depth of this node (1=top)
// @param [in]  maxDepth         Maximum depth limit
// @param [in]  acrossDocuments  true to recurse beyond a document>page boundary
void
util::doc::Index::listNodeChildren(std::vector<TaggedNode>& out, Node& node, int thisDepth, int maxDepth, bool acrossDocuments)
{
    if (thisDepth <= maxDepth) {
        for (size_t i = 0, n = node.children.size(); i < n; ++i) {
            Node& ch = *node.children[i];

            // Check node type
            const bool match = (acrossDocuments || node.type == ch.type);

            // Accept at level 1 regardless of type
            if (thisDepth == 1 || match) {
                out.push_back(TaggedNode(&ch, thisDepth));
                listNodeChildren(out, ch, thisDepth+1, maxDepth, acrossDocuments);
            }
        }
    }
}

// List related documents for a node, starting at a document.
// @param [out] out              Result produced here
// @param [in]  node             Starting node (=document)
// @param [in]  ids              List of Ids of pages to accept (=Ids of document to query)
void
util::doc::Index::listNodeRelatedDocuments(std::vector<RelatedNode>& out, Node& node, const std::vector<String_t>& ids)
{
    for (size_t i = 0, n = node.children.size(); i < n; ++i) {
        Node& ch = *node.children[i];
        switch (ch.type) {
         case Node::Document:
            listNodeRelatedDocuments(out, ch, ids);
            break;

         case Node::Page:
            listNodeRelatedPages(out, ch, ids, node);
            break;
        }
    }
}

// List related documents for a node, starting at a page.
// Checks this document and its children.
// @param [out] out           Result produced here
// @param [in]  node          Starting node (=document)
// @param [in]  ids           List of Ids of pages to accept (=Ids of document to query)
// @param [in]  thisDocument  Containing document
bool
util::doc::Index::listNodeRelatedPages(std::vector<RelatedNode>& out, Node& node, const std::vector<String_t>& ids, Node& thisDocument)
{
    if (matchIds(ids, node.ids)) {
        out.push_back(RelatedNode(&node, &thisDocument));
        return true;
    }

    for (size_t i = 0, n = node.children.size(); i < n; ++i) {
        if (listNodeRelatedPages(out, *node.children[i], ids, thisDocument)) {
            return true;
        }
    }
    return false;
}
