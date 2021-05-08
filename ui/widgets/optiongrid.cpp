/**
  *  \file ui/widgets/optiongrid.cpp
  *  \brief Class ui::widgets::OptionGrid
  *
  *  FIXME: allow more styling:
  *  - pixmaps on the right side
  *  - indentation to the labels
  *  - multiple buttons per line
  *  - buttons on the right
  *  - right column left-aligned
  *  FIXME: more interactions: click a line (outside the button) to toggle (touch friendly?)
  *  FIXME: leftWidth can be computed internally
  */

#include "ui/widgets/optiongrid.hpp"
#include "gfx/fontrequest.hpp"
#include "ui/widgets/button.hpp"

class ui::widgets::OptionGrid::Item {
 public:
    Item(OptionGrid& parent, int id, util::Key_t key, String_t label)
        : parent(parent),
          button(util::formatKey(key), key, parent.m_root),
          id(id),
          label(label),
          value(),
          font()
        { }

    OptionGrid& parent;
    Button button;
    int id;
    String_t label;
    String_t value;
    gfx::FontRequest font;
};

// Change item value's font.
ui::widgets::OptionGrid::Ref&
ui::widgets::OptionGrid::Ref::setFont(gfx::FontRequest font)
{
    // ex WOptionGrid::Ref::setFont
    if (m_pItem != 0 && m_pItem->font != font) {
        m_pItem->font = font;
        m_pItem->parent.requestRedraw();
    }
    return *this;
}

// Change item's value.
ui::widgets::OptionGrid::Ref&
ui::widgets::OptionGrid::Ref::setValue(String_t str)
{
    // ex WOptionGrid::Ref::setValue
    if (m_pItem != 0 && m_pItem->value != str) {
        m_pItem->value = str;
        m_pItem->parent.requestRedraw();
    }
    return *this;
}

// Change item's value.
ui::widgets::OptionGrid::Ref&
ui::widgets::OptionGrid::Ref::setValue(const char* str)
{
    // ex WOptionGrid::Ref::setValue
    return setValue(String_t(str));
}

// Change item's label.
ui::widgets::OptionGrid::Ref&
ui::widgets::OptionGrid::Ref::setLabel(String_t str)
{
    // ex WOptionGrid::Ref::setLabel
    if (m_pItem && m_pItem->label != str) {
        m_pItem->label = str;
        m_pItem->parent.requestRedraw();
    }
    return *this;
}

// Change item's enabled status.
ui::widgets::OptionGrid::Ref&
ui::widgets::OptionGrid::Ref::setEnabled(bool flag)
{
    // ex WOptionGrid::Ref::enable
    if (m_pItem && m_pItem->button.hasState(DisabledState) == flag) {
        m_pItem->button.setState(DisabledState, !flag);
        m_pItem->parent.requestRedraw();
    }
    return *this;
}

ui::widgets::OptionGrid::Ref&
ui::widgets::OptionGrid::Ref::addPossibleValue(String_t str)
{
    if (m_pItem != 0) {
        OptionGrid& parent = m_pItem->parent;
        int itemWidth = parent.m_root.provider().getFont(m_pItem->font)->getTextWidth(str);
        if (itemWidth > parent.m_rightWidth) {
            parent.m_rightWidth = itemWidth;
        }
    }
    return *this;
}

ui::widgets::OptionGrid::Ref&
ui::widgets::OptionGrid::Ref::addPossibleValues(const afl::functional::StringTable_t& values)
{
    if (m_pItem != 0) {
        int32_t i;
        for (bool v = values.getFirstKey(i); v; v = values.getNextKey(i)) {
            addPossibleValue(values(i));
        }
    }
    return *this;
}

/******************************* OptionGrid ******************************/

// Constructor.
ui::widgets::OptionGrid::OptionGrid(int leftWidth, int rightWidth, Root& root)
    : m_leftWidth(leftWidth),
      m_rightWidth(rightWidth),
      m_items(),
      m_root(root)
{
    // ex WOptionGrid::WOptionGrid
}

ui::widgets::OptionGrid::~OptionGrid()
{ }

// Add an item.
ui::widgets::OptionGrid::Ref
ui::widgets::OptionGrid::addItem(int id, util::Key_t key, String_t label)
{
    class Handler : public afl::base::Closure<void(int)> {
     public:
        Handler(OptionGrid& parent, int id)
            : m_parent(parent), m_id(id)
            { }
        void call(int)
            {
                if (Item* i = m_parent.findItem(m_id).m_pItem) {
                    if (!i->button.hasState(DisabledState)) {
                        m_parent.sig_click.raise(m_id);
                    }
                }
            }
     private:
        OptionGrid& m_parent;
        int m_id;
    };

    // Update size
    int labelWidth = m_root.provider().getFont(gfx::FontRequest())->getTextWidth(label);
    if (m_leftWidth < labelWidth) {
        m_leftWidth = labelWidth;
    }

    // ex WOptionGrid::addItem
    Item* pItem = m_items.pushBackNew(new Item(*this, id, key, label));
    pItem->button.sig_fire.addNewClosure(new Handler(*this, id));
    addChild(pItem->button, getLastChild());
    doLayout(m_items.size()-1, m_items.size());
    return pItem;
}

// Find an item, given an Id.
ui::widgets::OptionGrid::Ref
ui::widgets::OptionGrid::findItem(int id)
{
    // ex WOptionGrid::find
    for (size_t i = 0, n = m_items.size(); i < n; ++i) {
        if (m_items[i]->id == id) {
            return m_items[i];
        }
    }
    return Ref();
}

gfx::Point
ui::widgets::OptionGrid::getAnchorPointForItem(int id)
{
    Ref r = findItem(id);
    if (r.m_pItem != 0) {
        return r.m_pItem->button.getExtent().getBottomLeft();
    } else {
        return getExtent().getCenter();
    }
}

void
ui::widgets::OptionGrid::draw(gfx::Canvas& can)
{
    // ex WOptionGrid::drawContent

    afl::base::Ref<gfx::Font> largeFont = m_root.provider().getFont(gfx::FontRequest().addSize(1));
    const int lineHeight = largeFont->getTextHeight("Tp");

    const gfx::Rectangle ext = getExtent();

    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    for (size_t i = 0, n = m_items.size(); i < n; ++i) {
        // Fetch item
        const Item& it = *m_items[i];
        const int y = ext.getTopY() + (lineHeight * (2*int(i)+1)) / 2;

        // Left side
        ctx.useFont(*m_root.provider().getFont(gfx::FontRequest()));
        ctx.setTextAlign(gfx::LeftAlign, gfx::MiddleAlign);
        ctx.setColor(it.button.hasState(DisabledState)
                     ? util::SkinColor::Faded
                     : util::SkinColor::Static);
        ctx.setTransparentBackground();
        outText(ctx, gfx::Point(ext.getLeftX() + lineHeight + 5, y), it.label);

        // Right side
        ctx.useFont(*m_root.provider().getFont(it.font));
        ctx.setTextAlign(gfx::RightAlign, gfx::MiddleAlign);
        ctx.setColor(it.button.hasState(DisabledState)
                     ? util::SkinColor::Faded
                     : util::SkinColor::Green);
        ctx.setSolidBackground();
        outTextF(ctx, gfx::Point(ext.getRightX(), y), m_rightWidth, it.value);
    }

    defaultDrawChildren(can);
}

void
ui::widgets::OptionGrid::handleStateChange(State /*st*/, bool /*enable*/)
{
    // Probably need not do anything
}

void
ui::widgets::OptionGrid::requestChildRedraw(Widget& /*child*/, const gfx::Rectangle& area)
{
    requestRedraw(area);
}

void
ui::widgets::OptionGrid::handleChildAdded(Widget& /*child*/)
{
    // All widgets added/removed under our control.
}

void
ui::widgets::OptionGrid::handleChildRemove(Widget& /*child*/)
{
    // All widgets added/removed under our control.
}

void
ui::widgets::OptionGrid::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{
    doLayout(0, m_items.size());
}

void
ui::widgets::OptionGrid::handleChildPositionChange(Widget& /*child*/, gfx::Rectangle& /*oldPosition*/)
{
    // All widgets added/removed under our control.
}

ui::layout::Info
ui::widgets::OptionGrid::getLayoutInfo() const
{
    afl::base::Ref<gfx::Font> largeFont = m_root.provider().getFont(gfx::FontRequest().addSize(1));
    const int lineHeight = largeFont->getTextHeight("Tp");
    const int width = lineHeight  /* buttons */
        + 5                       /* gap */
        + m_leftWidth             /* left text */
        + 20                      /* gap */
        + m_rightWidth;
    const int height = lineHeight * int(m_items.size());

    return ui::layout::Info(gfx::Point(width, height),
                            gfx::Point(width, height),
                            ui::layout::Info::GrowHorizontal);
}

bool
ui::widgets::OptionGrid::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

bool
ui::widgets::OptionGrid::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    return defaultHandleMouse(pt, pressedButtons);
}

void
ui::widgets::OptionGrid::doLayout(size_t from, size_t to)
{
    afl::base::Ref<gfx::Font> largeFont = m_root.provider().getFont(gfx::FontRequest().addSize(1));
    const int lineHeight = largeFont->getTextHeight("Tp");

    for (size_t i = from; i < to; ++i) {
        if (i < m_items.size() && m_items[i] != 0) {
            m_items[i]->button.setExtent(gfx::Rectangle(getExtent().getLeftX(),
                                                        getExtent().getTopY() + int(i) * lineHeight + 1,
                                                        lineHeight-2,
                                                        lineHeight-2));
        }
    }
}
