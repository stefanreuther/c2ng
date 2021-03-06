/**
  *  \file ui/widgets/treelistbox.hpp
  *  \brief Class ui::widgets::TreeListbox
  */
#ifndef C2NG_UI_WIDGETS_TREELISTBOX_HPP
#define C2NG_UI_WIDGETS_TREELISTBOX_HPP

#include "ui/widgets/abstractlistbox.hpp"
#include "afl/container/ptrvector.hpp"
#include "ui/icons/icon.hpp"
#include "ui/root.hpp"
#include "afl/base/signal.hpp"

namespace ui { namespace widgets {

    /** Tree list box.
        Contains a tree with collapsible nodes, implemented as a list-box.

        Terminology:
        - a NODE is an element of the tree. A node may or may not be visible.
        - an ITEM is an element of the list. Each item represents a node.

        The respective current situation of the tree is flattened into a list box.
        If a node is folded or opened, the list is rebuilt.

        Each node can have a Id, label, and an optional icon.
        Functions that access or manipulate existing nodes take a Node* pointer.
        They handle receiving null gracefully, so you can do setIcon(findNodeById(...)) etc.

        Changes to PCC2 version:
        - icons are a feature of the base class (added in derived classes in PCC2)
        - PCC2 used "item" to refer to "nodes", which conflicts with our list-box terminology
        - addNode() does not need an updateAfterModification() call.
        - nodes are accessed by Node* pointers, not indexes */
    class TreeListbox : public AbstractListbox {
     public:
        /** Opaque reference to a node.
            Can be null. */
        struct Node;

        /** Constructor.
            \param root    Root
            \param lines   Number of lines (defines getLayoutInfo())
            \param width   Preferred width in pixels (defines getLayoutInfo()) */
        TreeListbox(Root& root, int lines, int width);
        ~TreeListbox();

        /** Add node.
            \param id    Identificator for this node.
            \param level Nesting level. The root level is 0. Children have their parent's level plus one.
                         A node is followed by its children which have higher level than itself,
                         and it is preceded (directly or indirectly) by its parent, grandparent, etc.
                         with lower levels.
            \param label Item label (user-visible text)
            \param open  For inner nodes, whether it is initially open or not (default: false). For leaf nodes, ignored.
            \return node reference */
        Node* addNode(int32_t id, int32_t level, String_t label, bool open);

        /** Update after modification.
            Rebuilds the flattened list.
            Users shouldn't have to call this. */
        void updateAfterModification();

        /** Get reference to currently-selected node.
            \return node */
        Node* getCurrentNode() const;

        /** Get node corresponding to a given list item (line).
            \param line  Line number (=item index)
            \return Node, null if not found */
        Node* getNodeFromItem(size_t line) const;

        /** Find node, given an Id.
            \param id Id to look for
            \return Node, null if not found */
        Node* findNodeById(int32_t id) const;

        /** Get Id from node.
            \param node Node reference
            \return Id */
        int32_t getIdFromNode(const Node* node) const;

        /** Check whether a node has children.
            \param node Node index
            \return true if node has children, false if it is a leaf node */
        bool hasChildren(const Node* node) const;

        /** Toggle state of a given node.
            \param node Node */
        void toggleNode(Node* node);

        /** Set associated icon for a node.
            \param node Node
            \param pIcon Icon (null to remove) */
        void setIcon(Node* node, ui::icons::Icon* pIcon);

        // AbstractListbox virtuals:
        virtual size_t getNumItems();
        virtual bool isItemAccessible(size_t n);
        virtual int getItemHeight(size_t n);
        virtual int getHeaderHeight() const;
        virtual int getFooterHeight() const;
        virtual void drawHeader(gfx::Canvas& can, gfx::Rectangle area);
        virtual void drawFooter(gfx::Canvas& can, gfx::Rectangle area);
        virtual void drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state);

        // Widget virtuals:
        virtual void handlePositionChange(gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);

        /** Handle icon click.
            Raised whenever the icon is clicked on a node that has one.
            \param id  Node Id */
        afl::base::Signal<void(int32_t)> sig_iconClick;

     private:
        Root& m_root;
        int m_numLines;
        int m_preferredWidth;

        afl::container::PtrVector<Node> m_nodes;
        std::vector<size_t> m_itemToNode;

        size_t getParentOf(size_t nodeIndex);
        size_t getLineOf(size_t nodeIndex);
        static int getIconHeight(Node& node);

        void onItemClickAt(size_t itemIndex, gfx::Point pos);

        afl::base::Ref<gfx::Font> getFont();
    };

} }

#endif
