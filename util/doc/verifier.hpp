/**
  *  \file util/doc/verifier.hpp
  *  \brief Class util::doc::Verifier
  */
#ifndef C2NG_UTIL_DOC_VERIFIER_HPP
#define C2NG_UTIL_DOC_VERIFIER_HPP

#include <map>
#include "util/doc/index.hpp"
#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"
#include "afl/bits/smallset.hpp"

namespace util { namespace doc {

    class BlobStore;

    /** Documentation verifier.
        Processes an Index + BlobStore and generates a set of warnings and informational messages.

        To use,
        - derive a class that processes the messages
        - configure using setEnabledMessages()
        - call verify() */
    class Verifier {
     public:
        /** Type of message. */
        enum Message {
            Warn_NodeHasNoId,            // (verifyNode) node has no Id
            Warn_NodeHasNoTitle,         // (verifyNode) node has no title
            Warn_NodeIsEmpty,            // (verifyNode) node has no content and no children
            Warn_UnresolvableContent,    // (verifyNode) content Id cannot be resolved
            Warn_UniqueSecondaryId,      // (verifySecondaryId, reportSecondaryIds) unique secondary Id
            Warn_NestingError,           // (verifyPage) child document is not a page
            Warn_DuplicateAddress,       // (verifyAddress) duplicate address
            Warn_ContentError,           // (verifyNode) content is not parseable or otherwise erroneous
            Warn_InvalidComment,         // (verifyRenderedContent) invalid comment in code (=bad tag)
            Warn_AssetLink,              // (verifyLink) asset: used in link
            Warn_DocumentImage,          // (verifyLink) document used as image reference
            Warn_InvalidAsset,           // (verifyLink) unknown asset in link
            Warn_DeadLink,               // (verifyLink) dead link
            Warn_BadAnchor,              // (verifyLink) anchor does not exist
            Info_UsedTags,               // (verifyNode) set of all tags
            Info_UsedClasses,            // (verifyRenderedContent) set of all tag/class names
            Info_ExternalLinks,          // (verifyLink) external link (http etc.)
            Info_SiteLinks               // (verifyLink) site link (site: etc.)
        };
        static const size_t MAX_MESSAGE = static_cast<size_t>(Info_SiteLinks) + 1;
        typedef afl::bits::SmallSet<Message> Messages_t;

        /** Constructor. */
        Verifier();

        /** Destructor. */
        virtual ~Verifier();

        /** Set enabled messages.
            Only messages contained in @c msg will be forwarded to reportMessage().
            @param msg Messages */
        void setEnabledMessages(Messages_t msg);

        /** Verify an installation.
            Will repeatedly call addMessage() for each issue/information found.
            @param idx Index
            @param blobStore Blob store */
        void verify(const Index& idx, const BlobStore& blobStore);

        /** Get node name.
            Like Index::getNodeName(), but tries to deal with erroneous nodes in a sensible way:
            tries to uniquely name nameless or ambiguous nodes.

            @param idx Index
            @param node Node
            @return node name */
        String_t getNodeName(const Index& idx, Index::Handle_t node) const;

        /** Get message type as string.
            @param msg Message type
            @param tx  Translator
            @return human-readable message */
        static String_t getMessage(Message msg, afl::string::Translator& tx);

        /** Get set of all warning messages.
            @return set */
        static Messages_t warningMessages();

        /** Get set of all info messages.
            @return set */
        static Messages_t infoMessages();

        /** Get set of all summary messages.
            For those, the originating page is not important.
            @return set */
        static Messages_t summaryMessages();

        /** Get set of all messages.
            @return set */
        static Messages_t allMessages();

     protected:
        /** Report a message.
            @param msg      Message type
            @param idx      Index
            @param refNode  Reference node (typically, node that contains the problem)
            @param info     Additional information (e.g. for a bad link, the link) */
        virtual void reportMessage(Message msg, const Index& idx, Index::Handle_t refNode, String_t info) = 0;

     private:
        /** Add a message.
            See addMessage().
            @param msg      Message type
            @param idx      Index
            @param refNode  Reference node (typically, node that contains the problem)
            @param info     Additional information (e.g. for a bad link, the link) */
        void addMessage(Message msg, const Index& idx, Index::Handle_t refNode, String_t info);

        /** Verify document.
            Checks the document and all its children.
            @param idx       Index
            @param blobStore BlobStore
            @param node      Document node to check */
        void verifyDocument(const Index& idx, const BlobStore& blobStore, Index::Handle_t node);

        /** Verify page.
            Checks the page and all its children.
            @param idx       Index
            @param blobStore BlobStore
            @param doc       Containing document
            @param page      Page node to check */
        void verifyPage(const Index& idx, const BlobStore& blobStore, Index::Handle_t doc, Index::Handle_t page);

        /** Verify node.
            Common checks for all nodes.
            @param idx       Index
            @param blobStore BlobStore
            @param node      Node to check */
        void verifyNode(const Index& idx, const BlobStore& blobStore, Index::Handle_t node);

        /** Verify content of a node.
            @param idx       Index
            @param blobStore BlobStore
            @param node      Node to check
            @param mem       Node content */
        void verifyContent(const Index& idx, const BlobStore& blobStore, Index::Handle_t node, afl::base::ConstBytes_t mem);

        /** Verify content rendered of a node.
            @param idx       Index
            @param node      Node to check
            @param content   Rendered content (HTML) */
        void verifyRenderedContent(const Index& idx, Index::Handle_t node, const String_t& content);

        /** Verify an address.
            Used to detect duplicate addresses.
            @param idx       Index
            @param node      Node
            @param addr      One possible address of the node */
        void verifyAddress(const Index& idx, Index::Handle_t node, String_t addr);

        /** Verify secondary Ids.
            @param node      Node
            @param id        Node Id
            @param isPrimary true for primary Id, false for secondary */
        void verifySecondaryId(Index::Handle_t node, String_t id, bool isPrimary);

        /** Report secondary Ids.
            Generates messages for all unique secondary Ids.
            @param idx Index */
        void reportSecondaryIds(const Index& idx);

        /** Report bad anchors.
            Generates messages for referenced but not defined anchors.
            @param idx Index */
        void reportBadAnchors(const Index& idx);

        class Visitor;

        /** Enabled messages. */
        Messages_t m_enabledMessages;

        /*
         *  Link tracking
         *
         *  Register every link (cross product of document Id x page Id).
         *  It is an error if a link is generated twice
         *
         *  It would be possible to do bad link checking through this data structure as well,
         *  but so far that's not possible as we resolve the links regularily through the index.
         */
        struct LinkInfo {
            Index::Handle_t node;
            size_t n;
            LinkInfo(Index::Handle_t node)
                : node(node), n(0)
                { }
        };
        typedef std::map<String_t, LinkInfo> InfoMap_t;
        InfoMap_t m_usedAddresses;

        /*
         *  Unique Secondary Ids:
         *
         *  A secondary Id (<page id="primary,secondary">) is used to build invisible "same-page-as" relations,
         *  e.g. to match a PCC1 help page to a PCC2 help page for the "Versions" feature.
         *  Using a unique secondary Id defeats that purpose, so this can be used to detect typos.
         *
         *  Count occurrences: multiple uses as a secondary Id are ok and establish a "same-page-as" relation.
         *  Use as a primary Id is ok; pretend by making two uses as secondary Id.
         *  It is a warning only if there is a single use as secondary Id.
         *
         *  A unique secondary Id might be useful to keep external links stable, so this is not entirely useless.
         *  Should this become necessary, consider adding a white-list.
         */
        InfoMap_t m_secondaryIds;

        /*
         *  Anchor tracking ("p/p/p#anchor"):
         *
         *  For a used anchor (link target), record
         *       content hash => (using node, link, Address_Used flag)
         *  For a defined anchor (id attribute), record
         *       content hash => (<whatever>, <whatever>, Address_Defined flag)
         *
         *  It is a warning if an anchor remains in Address_Used state.
         */
        struct AnchorInfo {
            Index::Handle_t node;    // referring page
            String_t link;           // full link, specimen
            size_t n;
            AnchorInfo(Index::Handle_t node, String_t link)
                : node(node), link(link), n(0)
                { }
        };
        typedef std::map<String_t, AnchorInfo> AnchorMap_t;
        AnchorMap_t m_usedAnchors;
    };

} }

#endif
