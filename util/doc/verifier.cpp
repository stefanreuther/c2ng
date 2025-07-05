/**
  *  \file util/doc/verifier.cpp
  *  \brief Class util::doc::Verifier
  */

#include "util/doc/verifier.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/filemapping.hpp"
#include "afl/io/xml/defaultentityhandler.hpp"
#include "afl/io/xml/parser.hpp"
#include "afl/io/xml/reader.hpp"
#include "afl/io/xml/tagnode.hpp"
#include "afl/io/xml/visitor.hpp"
#include "afl/string/format.hpp"
#include "util/charsetfactory.hpp"
#include "util/doc/blobstore.hpp"
#include "util/doc/htmlrenderer.hpp"
#include "util/doc/renderoptions.hpp"
#include "util/string.hpp"

using afl::base::Ref;
using afl::io::ConstMemoryStream;
using afl::io::FileMapping;
using afl::io::xml::DefaultEntityHandler;
using afl::io::xml::Nodes_t;
using afl::io::xml::PINode;
using afl::io::xml::Parser;
using afl::io::xml::Reader;
using afl::io::xml::TagNode;
using afl::io::xml::TextNode;
using afl::string::Format;

namespace {
    const size_t Address_Used = 1;
    const size_t Address_Defined = 2;

    String_t redactTag(const String_t& tag)
    {
        // The "size=#", "date=#" tags are used for files; do not report each individual instance
        if (util::strStartsWith(tag, "size=")) {
            return "size=#";
        } else if (util::strStartsWith(tag, "date=")) {
            return "date=#";
        } else {
            return tag;
        }
    }
}

/*
 *  Visitor
 *
 *  Visits the parsed content XML and checks selected properties.
 */
class util::doc::Verifier::Visitor : public afl::io::xml::Visitor {
 public:
    Visitor(Verifier& parent, const Index& idx, const BlobStore& blobStore, Index::Handle_t node)
        : m_parent(parent),
          m_index(idx),
          m_blobStore(blobStore),
          m_node(node)
        { }

    virtual void visitPI(const PINode& /*node*/)
        { }

    virtual void visitTag(const TagNode& node)
        {
            // Check this node
            if (node.getName() == "a") {
                verifyLink(node.getAttributeByName("href"), true);
            }
            if (node.getName() == "img") {
                verifyLink(node.getAttributeByName("src"), false);
            }

            // Check Ids
            String_t id = node.getAttributeByName("id");
            if (!id.empty()) {
                String_t key = m_index.getNodeContentId(m_node) + "#" + id;
                AnchorInfo& info = m_parent.m_usedAnchors.insert(std::make_pair(key, AnchorInfo(m_node, ""))).first->second;

                // FIXME: cannot currently detect multiple definitions here
                // because this might be called multiple times
                info.n |= Address_Defined;
            }

            // Children
            visit(node.getChildren());
        }

    virtual void visitText(const TextNode& /*node*/)
        { }

 private:
    Verifier& m_parent;
    const Index& m_index;
    const BlobStore& m_blobStore;
    Index::Handle_t m_node;

    void verifyLink(String_t s, bool isLink);
};

void
util::doc::Verifier::Visitor::verifyLink(String_t s, bool isLink)
{
    if (strStartsWith(s, "http:") || strStartsWith(s, "https:") || strStartsWith(s, "mailto:") || strStartsWith(s, "ftp:")
        || strStartsWith(s, "news:") || strStartsWith(s, "nntp:") || strStartsWith(s, "data:"))
    {
        // Verbatim
        m_parent.addMessage(Info_ExternalLinks, m_index, m_node, s);
    } else if (const char* p = strStartsWith(s, "site:")) {
        // Site URL ("site:foo", same as "$(html_CGI_RELROOT)foo" in a template)
        m_parent.addMessage(Info_SiteLinks, m_index, m_node, p);
    } else if (const char* p = strStartsWith(s, "asset:")) {
        // Asset URL ("asset:foo")
        String_t link = p;
        String_t::size_type x = link.find_first_of("#/");
        if (x != String_t::npos) {
            link.erase(x);
        }
        if (isLink) {
            // Link used in <a href>
            m_parent.addMessage(Warn_AssetLink, m_index, m_node, link);
        } else {
            // Check existance of blob
            try {
                m_blobStore.getObject(link);
            }
            catch (std::exception& e) {
                m_parent.addMessage(Warn_InvalidAsset, m_index, m_node, link);
            }
        }
    } else {
        if (!isLink) {
            // Link used in <img src>
            m_parent.addMessage(Warn_DocumentImage, m_index, m_node, s);
        } else if (const char* id = strStartsWith(s, "#")) {
            // Fragment ("#frag")
            String_t key = m_index.getNodeContentId(m_node) + "#" + id;
            AnchorInfo& info = m_parent.m_usedAnchors.insert(std::make_pair(key, AnchorInfo(m_node, "#" + String_t(id)))).first->second;
            info.n |= Address_Used;
        } else {
            // Document link. Must preserve relative position of fragment.
            String_t frag;
            String_t::size_type p = s.find('#');
            if (p != String_t::npos) {
                frag = s.substr(p);
                s.erase(p);
            }

            // Resolve to full link
            String_t fullName;
            if (const char* p = strStartsWith(s, "/")) {
                // Global document URL (e.g. "/pcc2-current/toc")
                fullName = p;
            } else {
                // Local document URL
                fullName = m_index.getNodeAddress(m_index.getNodeContainingDocument(m_node), "")
                    + "/" + s;
            }

            Index::Handle_t linkTarget;
            String_t docId;
            if (!m_index.findNodeByAddress(fullName, linkTarget, docId)) {
                m_parent.addMessage(Warn_DeadLink, m_index, m_node, fullName);
            } else {
                // Track fragment usage
                if (!frag.empty()) {
                    String_t contentId = m_index.getNodeContentId(linkTarget);
                    if (contentId.empty()) {
                        m_parent.addMessage(Warn_BadAnchor, m_index, m_node, s + frag);
                    } else {
                        String_t key = contentId + frag;
                        AnchorInfo& info = m_parent.m_usedAnchors.insert(std::make_pair(key, AnchorInfo(m_node, s + frag))).first->second;
                        info.n |= Address_Used;
                    }
                }
            }
        }
    }
}


/*
 *  Verifier
 */

util::doc::Verifier::Verifier()
    : m_enabledMessages(allMessages()),
      m_usedAddresses(),
      m_secondaryIds(),
      m_usedAnchors()
{ }

util::doc::Verifier::~Verifier()
{ }

void
util::doc::Verifier::setEnabledMessages(Messages_t msg)
{
    m_enabledMessages = msg;
}

void
util::doc::Verifier::verify(const Index& idx, const BlobStore& blobStore)
{
    // Clear status
    m_usedAddresses.clear();
    m_secondaryIds.clear();
    m_usedAnchors.clear();

    // Verify
    verifyDocument(idx, blobStore, idx.root());

    // Report messages that need whole-world knowledge
    reportSecondaryIds(idx);
    reportBadAnchors(idx);
}

String_t
util::doc::Verifier::getNodeName(const Index& idx, Index::Handle_t node) const
{
    // Special case for root
    if (node == idx.root()) {
        return "(root)";
    }

    // Use standard address if it is valid and unique enough
    String_t nn = idx.getNodeAddress(node, String_t());
    if (!nn.empty()) {
        InfoMap_t::const_iterator it = m_usedAddresses.find(nn);
        if (it == m_usedAddresses.end()
            || ((it->second.n & Address_Defined) != 0 && (it->second.node == node)))
        {
            return nn;
        }
    }

    // Fallback
    std::vector<Index::Handle_t> parents = idx.getNodeParents(node);
    if (!parents.empty()) {
        return Format("%s>#%d", getNodeName(idx, parents.back()), idx.getNodeParentIndex(node));
    }

    return "?";
}

String_t
util::doc::Verifier::getMessage(Message msg, afl::string::Translator& tx)
{
    switch (msg) {
     case Warn_NodeHasNoId:         return tx("Warning: node has no Id");
     case Warn_NodeHasNoTitle:      return tx("Warning: node has no title");
     case Warn_NodeIsEmpty:         return tx("Warning: node is empty (no children and no content)");
     case Warn_UnresolvableContent: return tx("Warning: node content cannot be resolved (blob does not exist)");
     case Warn_UniqueSecondaryId:   return tx("Warning: node has unique secondary Id");
     case Warn_NestingError:        return tx("Warning: nesting error (child node of page is not a page)");
     case Warn_DuplicateAddress:    return tx("Warning: duplicate address");
     case Warn_ContentError:        return tx("Warning: content error (cannot be parsed)");
     case Warn_InvalidComment:      return tx("Warning: comment in renderer output (invalid tag in input)");
     case Warn_AssetLink:           return tx("Warning: 'asset:' used in <a href> link");
     case Warn_DocumentImage:       return tx("Warning: document name used in <img src> link");
     case Warn_InvalidAsset:        return tx("Warning: 'asset:' content cannot be resolved (blob does not exist)");
     case Warn_DeadLink:            return tx("Warning: dead link");
     case Warn_BadAnchor:           return tx("Warning: bad anchor ('#link')");
     case Info_UsedTags:            return tx("Info: used tags");
     case Info_UsedClasses:         return tx("Info: used CSS classes");
     case Info_ExternalLinks:       return tx("Info: external links");
     case Info_SiteLinks:           return tx("Info: site links");
    }
    return String_t();
}

util::doc::Verifier::Messages_t
util::doc::Verifier::warningMessages()
{
    return Messages_t()
        + Warn_NodeHasNoId
        + Warn_NodeHasNoTitle
        + Warn_NodeIsEmpty
        + Warn_UnresolvableContent
        + Warn_UniqueSecondaryId
        + Warn_NestingError
        + Warn_DuplicateAddress
        + Warn_ContentError
        + Warn_InvalidComment
        + Warn_AssetLink
        + Warn_DocumentImage
        + Warn_InvalidAsset
        + Warn_DeadLink
        + Warn_BadAnchor;
}

util::doc::Verifier::Messages_t
util::doc::Verifier::infoMessages()
{
    return Messages_t()
        + Info_UsedTags
        + Info_UsedClasses
        + Info_ExternalLinks
        + Info_SiteLinks;
}

util::doc::Verifier::Messages_t
util::doc::Verifier::summaryMessages()
{
    // For now, these are the same as infoMessages()
    return Messages_t()
        + Info_UsedTags
        + Info_UsedClasses
        + Info_ExternalLinks
        + Info_SiteLinks;
}

util::doc::Verifier::Messages_t
util::doc::Verifier::allMessages()
{
    return warningMessages() + infoMessages();
}

void
util::doc::Verifier::addMessage(Message msg, const Index& idx, Index::Handle_t refNode, String_t info)
{
    if (m_enabledMessages.contains(msg)) {
        reportMessage(msg, idx, refNode, info);
    }
}

void
util::doc::Verifier::verifyDocument(const Index& idx, const BlobStore& blobStore, Index::Handle_t node)
{
    // Generic tests
    verifyNode(idx, blobStore, node);

    // Record addresses
    for (size_t i = 0, n = idx.getNumNodeIds(node); i < n; ++i) {
        verifyAddress(idx, node, idx.getNodeIdByIndex(node, i));
    }

    // Verify children
    for (size_t i = 0, n = idx.getNumNodeChildren(node); i < n; ++i) {
        Index::Handle_t child = idx.getNodeChildByIndex(node, i);
        if (idx.isNodePage(child)) {
            verifyPage(idx, blobStore, node, child);
        } else {
            verifyDocument(idx, blobStore, child);
        }
    }
}

void
util::doc::Verifier::verifyPage(const Index& idx, const BlobStore& blobStore, Index::Handle_t doc, Index::Handle_t page)
{
    // Generic tests
    verifyNode(idx, blobStore, page);

    // Record addresses
    for (size_t i = 0, n = idx.getNumNodeIds(doc); i < n; ++i) {
        for (size_t j = 0, m = idx.getNumNodeIds(page); j < m; ++j) {
            verifyAddress(idx, page, Format("%s/%s", idx.getNodeIdByIndex(doc, i), idx.getNodeIdByIndex(page, j)));
        }
    }
    for (size_t j = 0, m = idx.getNumNodeIds(page); j < m; ++j) {
        verifySecondaryId(page, idx.getNodeIdByIndex(page, j), j == 0);
    }

    // Verify children
    for (size_t i = 0, n = idx.getNumNodeChildren(page); i < n; ++i) {
        Index::Handle_t child = idx.getNodeChildByIndex(page, i);
        if (idx.isNodePage(child)) {
            verifyPage(idx, blobStore, doc, child);
        } else {
            addMessage(Warn_NestingError, idx, child, String_t());
        }
    }
}

void
util::doc::Verifier::verifyNode(const Index& idx, const BlobStore& blobStore, Index::Handle_t node)
{
    // Info_UsedTags
    for (size_t i = 0, n = idx.getNumNodeTags(node); i < n; ++i) {
        addMessage(Info_UsedTags, idx, node, redactTag(idx.getNodeTagByIndex(node, i)));
    }

    // Warn_NodeHasNoId, Warn_NodeHasNoTitle
    if (node != idx.root()) {
        if (idx.getNumNodeIds(node) == 0) {
            addMessage(Warn_NodeHasNoId, idx, node, String_t());
        }
        if (idx.getNodeTitle(node).empty()) {
            addMessage(Warn_NodeHasNoTitle, idx, node, String_t());
        }
    }

    if (idx.isNodeBlob(node)) {
        // Blob; ignore
    } else if (idx.getNodeContentId(node).empty()) {
        // Warn_NodeIsEmpty
        if (idx.getNumNodeChildren(node) == 0) {
            addMessage(Warn_NodeIsEmpty, idx, node, String_t());
        }
    } else {
        // Warn_UnresolvableContent, Warn_ContentError
        try {
            Ref<FileMapping> m = blobStore.getObject(idx.getNodeContentId(node));
            try {
                verifyContent(idx, blobStore, node, m->get());
            }
            catch (std::exception&) {
                addMessage(Warn_ContentError, idx, node, String_t());
            }
        }
        catch (std::exception&) {
            addMessage(Warn_UnresolvableContent, idx, node, String_t());
        }
    }
}

void
util::doc::Verifier::verifyContent(const Index& idx, const BlobStore& blobStore, Index::Handle_t node, afl::base::ConstBytes_t mem)
{
    // Parse XML
    ConstMemoryStream ms(mem);
    CharsetFactory csFactory;
    DefaultEntityHandler eh;
    Nodes_t nodes;
    Reader rdr(ms, eh, csFactory);
    Parser(rdr).parseNodes(nodes);

    // Verify XML
    Visitor(*this, idx, blobStore, node).visit(nodes);

    // Render and verify
    RenderOptions opts;
    verifyRenderedContent(idx, node, renderHTML(nodes, opts));
}

void
util::doc::Verifier::verifyRenderedContent(const Index& idx, Index::Handle_t node, const String_t& content)
{
    // Lexer
    ConstMemoryStream ms(afl::string::toBytes(content));
    CharsetFactory csFactory;
    DefaultEntityHandler eh;
    Nodes_t nodes;
    Reader rdr(ms, eh, csFactory);

    // Info_UsedClasses, Warn_InvalidComment
    Reader::Token tok;
    while ((tok = rdr.readNext()) != Reader::Eof) {
        if (tok == Reader::TagAttribute && rdr.getName() == "class") {
            addMessage(Info_UsedClasses, idx, node, Format("%s.%s", rdr.getTag(), rdr.getValue()));
        } else if (tok == Reader::Comment) {
            addMessage(Warn_InvalidComment, idx, node, afl::string::strTrim(rdr.getValue()));
        } else {
            // ignore
        }
    }
}

void
util::doc::Verifier::verifyAddress(const Index& idx, Index::Handle_t node, String_t addr)
{
    LinkInfo& info = m_usedAddresses.insert(std::make_pair(addr, LinkInfo(node))).first->second;
    if ((info.n & Address_Defined) != 0 && info.node != node) {
        addMessage(Warn_DuplicateAddress, idx, node, getNodeName(idx, info.node));
    } else {
        info.n |= Address_Defined;
        info.node = node;
    }
}

void
util::doc::Verifier::verifySecondaryId(Index::Handle_t node, String_t id, bool isPrimary)
{
    size_t& n = m_secondaryIds.insert(std::make_pair(id, LinkInfo(node))).first->second.n;
    if (isPrimary) {
        n = 2;
    } else {
        ++n;
    }
}

void
util::doc::Verifier::reportSecondaryIds(const Index& idx)
{
    for (InfoMap_t::const_iterator it = m_secondaryIds.begin(); it != m_secondaryIds.end(); ++it) {
        if (it->second.n == 1) {
            addMessage(Warn_UniqueSecondaryId, idx, it->second.node, it->first);
        }
    }
}

void
util::doc::Verifier::reportBadAnchors(const Index& idx)
{
    for (AnchorMap_t::const_iterator it = m_usedAnchors.begin(); it != m_usedAnchors.end(); ++it) {
        if (it->second.n == Address_Used) {
            addMessage(Warn_BadAnchor, idx, it->second.node, it->second.link);
        }
    }
}
