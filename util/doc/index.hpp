/**
  *  \file util/doc/index.hpp
  *  \brief Class util::doc::Index
  */
#ifndef C2NG_UTIL_DOC_INDEX_HPP
#define C2NG_UTIL_DOC_INDEX_HPP

#include <memory>
#include "afl/io/stream.hpp"
#include "afl/io/textfile.hpp"
#include "util/doc/blobstore.hpp"              // ObjectId_t

namespace util { namespace doc {

    /** Document index.
        Represents the overall structure of a documentation set, which is a tree of nested:
        - one or more documents (e.g. "PCC2 > PCC 2.0.10")
        - one or more pages (e.g. "User Interface > Control Screens > Ship Screen")

        Each node has
        - one or more Ids, where each Id shall be a valid file system/URL path (i.e. alphanumeric, with slashes)
        - zero or more tags, where each tag is a string (e.g. "lang=en", "beta")
        - a title
        - an optional content document, identified by a content Id

        A node's type affects generation of content lists and related documents.

        At each level, we will provide for the user to see:
        - navigation to previous and next items
        - navigation to parent items
        - one or more levels of child items
        - the content document, if any

        Operations are provided to build this index from scratch, add to it, and save/load it to a file. */
    class Index {
     public:
        struct Node;

        /** Shortcut for an Object Id. */
        typedef BlobStore::ObjectId_t ObjectId_t;

        /** Handle to a node.
            Valid operations:
            - pass this handle into functions of the object that created it
            - compare to other handles derived from that same object
            @see root() */
        typedef Node* Handle_t;

        /** Tagged node.
            Represents a node with additional meta-information, the meaning of which depends on the context. */
        struct TaggedNode {
            Handle_t node;       ///< Node handle.
            int tag;             ///< Meta information.
            TaggedNode(Handle_t node, int tag)
                : node(node), tag(tag)
                { }
        };

        /** Related node. */
        struct RelatedNode {
            Handle_t node;       ///< Node handle.
            Handle_t docNode;    ///< Document containing the node.
            RelatedNode(Handle_t node, Handle_t docNode)
                : node(node), docNode(docNode)
                { }
        };

        /** getNodeNavigationContext tag: indirect predecessor.
            Refers to the previous node in reading order, e.g. last section of previous chapter. */
        static const int NAV_PREVIOUS_INDIRECT = -2;

        /** getNodeNavigationContext tag: direct predecessor.
            Refers to the previous node on the same level, e.g. previous chapter. */
        static const int NAV_PREVIOUS_DIRECT = -1;

        /** getNodeNavigationContext tag: parent. */
        static const int NAV_UP = 0;

        /** getNodeNavigationContext tag: direct successor.
            Refers to the next node on the same level, e.g. next chapter. */
        static const int NAV_NEXT_DIRECT = +1;

        /** getNodeNavigationContext tag: indirect successor.
            Refers to the next node in reading order, e.g. first child or next node. */
        static const int NAV_NEXT_INDIRECT = +2;


        /** Constructor.
            Make an empty index. */
        Index();

        /** Destructor. */
        ~Index();

        /** Load from file.
            @param in File */
        void load(afl::io::Stream& in);

        /** Save to file.
            @param out File */
        void save(afl::io::Stream& out) const;

        /** Get handle to root node.
            @return handle */
        Handle_t root() const;

        /** Add a new document.
            @param parent     Handle to root or another document
            @param ids        Comma-separated list of Ids (usually a single Id)
            @param title      Title of document, should not be empty
            @param contentId  Id of content document, can be empty
            @return handle to new node */
        Handle_t addDocument(Handle_t parent, String_t ids, String_t title, ObjectId_t contentId);

        /** Add a new page.
            @param parent     Handle to document or parent page
            @param ids        Comma-separated list of Ids (usually a single Id)
            @param title      Title of document, should not be empty
            @param contentId  Id of content document, can be empty
            @return handle to new node */
        Handle_t addPage(Handle_t parent, String_t ids, String_t title, ObjectId_t contentId);

        /** Add node Ids.
            @param node Node
            @param ids  New Ids */
        void addNodeIds(Handle_t node, String_t ids);

        /** Add node tags.
            @param node Node
            @param tags New tags */
        void addNodeTags(Handle_t node, String_t tags);

        /** Check node type.
            @param node Node
            @return true if node is a page, false if node is root or document */
        bool isNodePage(Handle_t node) const;

        /** Set node title.
            @param node Node
            @param title Title */
        void setNodeTitle(Handle_t node, const String_t& title);

        /** Get node title.
            @param node Node
            @return title */
        String_t getNodeTitle(Handle_t node) const;

        /** Set node content Id.
            @param node Node
            @param contentId Id of content document, can be empty */
        void setNodeContentId(Handle_t node, ObjectId_t contentId);

        /** Get node content Id.
            @param node Node
            @return content Id, empty if node has no content */
        ObjectId_t getNodeContentId(Handle_t node) const;

        /** Get number of Ids this node has.
            @param node Node */
        size_t getNumNodeIds(Handle_t node) const;

        /** Get node Id.
            @param node Node
            @param index 0-based index, [0,getNumNodeIds())
            @return Node Id */
        String_t getNodeIdByIndex(Handle_t node, size_t index) const;

        /** Get number of tags this node has.
            @param node Node */
        size_t getNumNodeTags(Handle_t node) const;

        /** Get node tag.
            @param node Node
            @param index 0-based index, [0,getNumNodeTags())
            @return Tag */
        String_t getNodeTagByIndex(Handle_t node, size_t index) const;

        /** Get number of children this node has.
            @param node Node */
        size_t getNumNodeChildren(Handle_t node) const;

        /** Get node child.
            @param node Node
            @param index 0-based index, [0,getNumNodeChildren())
            @return handle to child node */
        Handle_t getNodeChildByIndex(Handle_t node, size_t index) const;

        /** Get index of a node in its parent.
            To obtain the parent, use getNodeParents().
            @param node Node (not root())
            @return Index in parent such that getNodeChildByIndex(parent, return) == node */
        size_t getNodeParentIndex(Handle_t node) const;

        /** Get parents of a node.
            @param node Node.
            @return List of parents, starting with root(), ending with the node's immediate parent.
                    Empty if node has no parent (=is root()). */
        std::vector<Handle_t> getNodeParents(Handle_t node) const;

        /** Get navigation context for a node.
            Navigation context includes sensible nodes to read next or before.
            All nodes in the returned list will have a NAV_XXX constant as their tag.
            If a particular navigation does not exist (e.g. node has no direct successor),
            the corresponding element is missing.
            @param node Node
            @return Context nodes */
        std::vector<TaggedNode> getNodeNavigationContext(Handle_t node) const;

        /** Get children of a node.
            Recursively collects all the children.
            The nodes in the returned list will be tagged by their depth,
            i.e. 1=direct child, 2=grandchild, etc.
            @param node             Node
            @param maxDepth         Maximum depth to return (inclusive). 0 will return an empty list.
            @param acrossDocuments  If true, also return nodes across a type change (e.g. pages within a document),
                                    if false, only nodes of the same type are returned.
            @return Children */
        std::vector<TaggedNode> getNodeChildren(Handle_t node, int maxDepth, bool acrossDocuments) const;

        /** Get related versions of a node.
            Related versions are documents that have the same Id as the given node.
            This function lists all appearances of that node Id, including the node itself.
            @param node Node
            @return Related nodes */
        std::vector<RelatedNode> getNodeRelatedVersions(Handle_t node) const;

        /** Get node address.
            A preferred document name can be specified:
            if the node is part (=child) of the named document, uses that name instead of the document's canonical (=first) name.
            This can be used to keep document-relative links in the correct document,
            for example if we have a document that responds to both "-current" and "-1.2.3".

            @param node Node to get the address for
            @param docId Preferred document Id; may be empty
            @return address (concatenation of Ids, separted with "/"); empty on error */
        String_t getNodeAddress(Handle_t node, String_t docId) const;

        /** Get containing document for a node.
            If the node refers to a document, returns that node; otherwise, the closest parent.
            @param node Node to get the containing document for
            @return document (node for which isNodePage() returns false) */
        Handle_t getNodeContainingDocument(Handle_t node) const;

        /** Find node by address.
            @param [in]  address   Address to look up
            @param [out] result    Resulting node if found
            @param [out] docId   Resulting document name (same as address, or a prefix)
            @return true if node found; false on error */
        bool findNodeByAddress(const String_t& address, Handle_t& result, String_t& docId) const;

     private:
        std::auto_ptr<Node> m_root;

        static void saveNode(afl::io::TextFile& out, const Node& node, size_t level);
        static Node* findDocumentByAddress(Node& node, afl::string::ConstStringMemory_t address, String_t& docId);
        static Node* findPageByAddress(Node& node, afl::string::ConstStringMemory_t address);
        static void listNodeChildren(std::vector<TaggedNode>& out, Node& node, int thisDepth, int maxDepth, bool acrossDocuments);

        static void listNodeRelatedDocuments(std::vector<RelatedNode>& out, Node& node, const std::vector<String_t>& ids);
        static bool listNodeRelatedPages(std::vector<RelatedNode>& out, Node& node, const std::vector<String_t>& ids, Node& thisDocument);
    };

} }

#endif
