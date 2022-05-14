/**
  *  \file ui/widgets/treelistbox.cpp
  *  \brief Class ui::widgets::TreeListbox
  */

#include "ui/widgets/treelistbox.hpp"
#include "afl/base/deleter.hpp"
#include "gfx/context.hpp"

struct ui::widgets::TreeListbox::Node {
    int32_t id;
    int32_t level;
    bool open;
    bool hasChildren;
    String_t label;
    ui::icons::Icon* pIcon;
    Node(int32_t id, int32_t level, bool open, String_t label)
        : id(id), level(level), open(open), hasChildren(false), label(label), pIcon(0)
        { }
};


ui::widgets::TreeListbox::TreeListbox(Root& root, int lines, int width)
    : m_root(root),
      m_numLines(lines),
      m_preferredWidth(width),
      m_nodes(),
      m_itemToNode()
{
    sig_itemClickAt.add(this, &TreeListbox::onItemClickAt);
}

ui::widgets::TreeListbox::~TreeListbox()
{ }

ui::widgets::TreeListbox::Node*
ui::widgets::TreeListbox::addNode(int32_t id, int32_t level, String_t label, bool open)
{
    // ex UITreeWidget::addItem
    // Mark parent as having children
    if (!m_nodes.empty() && m_nodes.back()->level < level) {
        m_nodes.back()->hasChildren = true;
    }

    // Add new node
    size_t newNodeIndex = m_nodes.size();
    Node* result = m_nodes.pushBackNew(new Node(id, level, open, label));

    // Is this node visible by default?
    bool isVisible;
    if (m_itemToNode.empty()) {
        // First node is always visible
        isVisible = true;
    } else if (level <= m_nodes[m_itemToNode.back()]->level) {
        // Previous node is a nephew of the last visible one, and must therefore be visible because they have a common open parent
        isVisible = true;
    } else {
        // Either I'm the first child of an open parent, so I'm visible.
        //   (if I'm second child, the previous condition would have been used)
        // Or I'm a direct or indirect child of a closed parent, so I'm invisible.
        isVisible = m_nodes[m_itemToNode.back()]->open;
    }

    if (isVisible) {
        m_itemToNode.push_back(newNodeIndex);
        handleModelChange();
    }

    return result;
}

void
ui::widgets::TreeListbox::updateAfterModification()
{
    // ex UITreeWidget::updateAfterModification
    // Remember which node is active
    size_t activeNode = (getCurrentItem() < m_itemToNode.size()) ? m_itemToNode[getCurrentItem()] : 0;

    // Rebuild m_itemToNode
    m_itemToNode.clear();
    size_t i = 0;
    while (i < m_nodes.size()) {
        int32_t level = m_nodes[i]->level;
        m_itemToNode.push_back(i);
        if (m_nodes[i]->open) {
            // It's open, so copy the children as well
            ++i;
        } else {
            // It's closed, so skip all children
            ++i;
            while (i < m_nodes.size() && m_nodes[i]->level > level) {
                ++i;
            }
        }
    }

    /* Now, find out new place to put cursor to. This implicitly handles the
       pathological case of an empty m_itemToNode, because getLineOf returns
       0 in that case. */
    size_t line = getLineOf(activeNode);
    while (line != 0 && m_itemToNode[line] != activeNode) {
        activeNode = getParentOf(activeNode);
        line = getLineOf(activeNode);
    }

    // Configure list box
    setCurrentItem(line);
    handleModelChange();   // implies requestRedraw, sig_change
}

ui::widgets::TreeListbox::Node*
ui::widgets::TreeListbox::getCurrentNode() const
{
    // ex UITreeWidget::getCurrentItemIndex
    return getNodeFromItem(getCurrentItem());
}

ui::widgets::TreeListbox::Node*
ui::widgets::TreeListbox::getNodeFromItem(size_t line) const
{
    // ex UITreeWidget::getItemIndex
    if (line < m_itemToNode.size()) {
        return m_nodes[m_itemToNode[line]];
    } else {
        return 0;
    }
}

size_t
ui::widgets::TreeListbox::getNumNodes() const
{
    return m_nodes.size();
}

ui::widgets::TreeListbox::Node*
ui::widgets::TreeListbox::getNodeByIndex(size_t index) const
{
    if (index < m_nodes.size()) {
        return m_nodes[index];
    } else {
        return 0;
    }
}

ui::widgets::TreeListbox::Node*
ui::widgets::TreeListbox::findNodeById(int32_t id) const
{
    for (size_t i = 0, n = m_nodes.size(); i < n; ++i) {
        Node* p = m_nodes[i];
        if (p->id == id) {
            return p;
        }
    }
    return 0;
}

int32_t
ui::widgets::TreeListbox::getIdFromNode(const Node* node) const
{
    // ex UITreeWidget::getItemId
    if (node != 0) {
        return node->id;
    } else {
        return 0;
    }
}

bool
ui::widgets::TreeListbox::hasChildren(const Node* node) const
{
    // ex UITreeWidget::hasChildren
    return node != 0
        && node->hasChildren;
}

void
ui::widgets::TreeListbox::toggleNode(Node* node)
{
    // ex UITreeWidget::toggleItem
    if (node != 0 && hasChildren(node)) {
        node->open = !node->open;
        updateAfterModification();
    }
}

void
ui::widgets::TreeListbox::setIcon(Node* node, ui::icons::Icon* pIcon)
{
    if (node != 0 && pIcon != node->pIcon) {
        // Update
        const int lineHeight = getFont()->getLineHeight();
        const int oldIconHeight = std::max(lineHeight, getIconHeight(*node));
        node->pIcon = pIcon;
        const int newIconHeight = std::max(lineHeight, getIconHeight(*node));

        // If icon size changed, reconsider entire widget. Otherwise, just redraw.
        if (newIconHeight != oldIconHeight) {
            handleModelChange();
        } else {
            requestRedraw();
        }
    }
}

// AbstractListbox virtuals:
size_t
ui::widgets::TreeListbox::getNumItems()
{
    return m_itemToNode.size();
}

bool
ui::widgets::TreeListbox::isItemAccessible(size_t /*n*/)
{
    return true;
}

int
ui::widgets::TreeListbox::getItemHeight(size_t n)
{
    int height = getFont()->getLineHeight();
    if (n < m_itemToNode.size()) {
        if (ui::icons::Icon* pIcon = m_nodes[m_itemToNode[n]]->pIcon) {
            height = std::max(pIcon->getSize().getY(), height);
        }
    }
    return height;
}

int
ui::widgets::TreeListbox::getHeaderHeight() const
{
    return 0;
}

int
ui::widgets::TreeListbox::getFooterHeight() const
{
    return 0;
}

void
ui::widgets::TreeListbox::drawHeader(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
{ }

void
ui::widgets::TreeListbox::drawFooter(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
{ }

void
ui::widgets::TreeListbox::drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state)
{
    // ex UITreeWidget::drawPart
    afl::base::Ref<gfx::Font> font(getFont());
    afl::base::Deleter del;
    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    prepareColorListItem(ctx, area, state, m_root.colorScheme(), del);

    if (item < m_itemToNode.size()) {
        /* Find indentation */
        const size_t slot = m_itemToNode[item];
        const Node& it = *m_nodes[slot];
        const int wi = font->getEmWidth();
        const int indent = wi * it.level + 5;

        area.consumeX(indent);

        /* +/- */
        if (hasChildren(&it)) {
            ctx.setTextAlign(gfx::LeftAlign, gfx::MiddleAlign);
            ctx.useFont(*font);
            ctx.setColor(util::SkinColor::Faded);
            outTextF(ctx, area.splitX(wi), it.open ? "-" : "+");
        } else {
            area.consumeX(wi);
        }

        /* Icon */
        if (it.pIcon != 0) {
            it.pIcon->draw(ctx, area.splitRightX(it.pIcon->getSize().getX()), ButtonFlags_t());
        }

        /* Text */
        ctx.setTextAlign(gfx::LeftAlign, gfx::MiddleAlign);
        ctx.useFont(*font);
        ctx.setColor(util::SkinColor::Static);
        outTextF(ctx, area, it.label);
    }
}

// Widget virtuals:
void
ui::widgets::TreeListbox::handlePositionChange(gfx::Rectangle& oldPosition)
{
    defaultHandlePositionChange(oldPosition);
}

ui::layout::Info
ui::widgets::TreeListbox::getLayoutInfo() const
{
    int preferredHeight = m_numLines * m_root.provider().getFont(gfx::FontRequest())->getLineHeight();
    gfx::Point preferredSize(m_preferredWidth, preferredHeight);
    return ui::layout::Info(preferredSize, preferredSize, ui::layout::Info::GrowBoth);
}

bool
ui::widgets::TreeListbox::handleKey(util::Key_t key, int prefix)
{
    // ex UITreeWidget::handleEvent
    /* Handle key events when the widget state allows and the cursor is in a valid position */
    if (hasState(FocusedState) && !hasState(DisabledState) && getCurrentItem() < m_itemToNode.size()) {
        const size_t slot = m_itemToNode[getCurrentItem()];
        Node& node = *m_nodes[slot];
        switch (key) {
         case util::Key_Left:
            // Left: close if it is open, otherwise go to parent
            if (node.open && hasChildren(&node)) {
                node.open = false;
                updateAfterModification();
            } else if (slot > 0) {
                setCurrentItem(getLineOf(getParentOf(slot)));
            }
            return true;
         case util::Key_Right:
            // Right: open if it is closed, otherwise go to child
            if (hasChildren(&node)) {
                if (!node.open) {
                    node.open = true;
                    updateAfterModification();
                } else {
                    setCurrentItem(getCurrentItem()+1);
                }
            }
            return true;
         case ' ':
            // Space: open/close node if it has children, click icon (a checkbox, typically) if possible,
            // otherwise ignore the keypress so a possible other widget can deal with it
            if (hasChildren(&node)) {
                node.open = !node.open;
                updateAfterModification();
                return true;
            } else if (node.pIcon != 0) {
                sig_iconClick.raise(node.id);
                return true;
            } else {
                break;
            }
         case '*':
            // Toggle current, and set all its children AND LATER SIBLINGS to same state
            if (hasChildren(&node)) {
                node.open = !node.open;
                for (size_t i = slot+1; i < m_nodes.size() && m_nodes[i]->level >= node.level; ++i) {
                    m_nodes[i]->open = node.open;
                }
                updateAfterModification();
            }
            return true;
        }
    }
    return defaultHandleKey(key, prefix);
}

/** Get node index of a node's parent.
    \param nodeIndex node whose parent to query (index into m_nodes) */
size_t
ui::widgets::TreeListbox::getParentOf(size_t nodeIndex)
{
    // ex UITreeWidget::getParentOf
    int32_t level = m_nodes[nodeIndex]->level;
    while (nodeIndex > 0 && m_nodes[nodeIndex]->level >= level) {
        --nodeIndex;
    }
    return nodeIndex;
}

/** Get line of a node. When the node is not visible, returns the line
    of the closest preceding visible node.
    \param nodeIndex item whose line to query (index into m_nodes) */
size_t
ui::widgets::TreeListbox::getLineOf(size_t nodeIndex)
{
    // ex UITreeWidget::getLineOf
    /* We could do binary search, but this way it's simpler */
    size_t i = 0;
    while (i+1 < m_itemToNode.size() && m_itemToNode[i+1] <= nodeIndex) {
        ++i;
    }
    return i;
}

int
ui::widgets::TreeListbox::getIconHeight(Node& node)
{
    if (node.pIcon != 0) {
        return node.pIcon->getSize().getY();
    } else {
        return 0;
    }
}

void
ui::widgets::TreeListbox::onItemClickAt(size_t itemIndex, gfx::Point pos)
{
    // ex UITreeWidget::onItemClick
    if (itemIndex < m_itemToNode.size()) {
        /* Check for click on a +/- */
        const size_t slot = m_itemToNode[itemIndex];
        Node& node = *m_nodes[slot];
        const int wi = getFont()->getEmWidth();
        const int indent = wi * node.level;
        if (pos.getX() >= indent && pos.getX() < indent + wi + 5 && hasChildren(&node)) {
            node.open = !node.open;
            updateAfterModification();
        } else if (node.pIcon != 0 && pos.getX() >= getExtent().getWidth() - node.pIcon->getSize().getX()) {
            sig_iconClick.raise(node.id);
        } else {
            // Just clicked the name
        }
    }
}

afl::base::Ref<gfx::Font>
ui::widgets::TreeListbox::getFont()
{
    return m_root.provider().getFont(gfx::FontRequest());
}
