/**
  *  \file client/widgets/expressionlist.cpp
  *  \brief Expression List Popup
  */

#include "client/widgets/expressionlist.hpp"
#include "client/downlink.hpp"
#include "gfx/complex.hpp"
#include "gfx/context.hpp"
#include "ui/draw.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/widgets/abstractlistbox.hpp"
#include "ui/widgets/menuframe.hpp"

using game::config::ExpressionLists;

namespace {
    /*
     *  ExpressionList widget: display an ExpressionList::Items_t object
     */

    class ExpressionList : public ui::widgets::AbstractListbox {
     public:
        ExpressionList(ui::Root& root, const ExpressionLists::Items_t& items)
            : m_root(root), m_items(items)
            {
                // ex WLRUListBox::WLRUListBox
            }

        // AbstractListbox:
        virtual size_t getNumItems() const
            { return m_items.size(); }
        virtual bool isItemAccessible(size_t n) const
            {
                // ex WLRUListBox::isAccessible
                return n < m_items.size() && !m_items[n].isHeading;
            }
        virtual int getItemHeight(size_t /*n*/) const
            { return getFont()->getLineHeight(); }
        virtual int getHeaderHeight() const
            { return 0; }
        virtual int getFooterHeight() const
            { return 0; }
        virtual void drawHeader(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
            { }
        virtual void drawFooter(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
            { }
        virtual void drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state)
            {
                afl::base::Ref<gfx::Font> font(getFont());
                afl::base::Deleter del;
                gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
                ui::prepareColorListItem(ctx, area, state, m_root.colorScheme(), del);
                ctx.useFont(*font);

                if (item < m_items.size()) {
                    const ExpressionLists::Item& node = m_items[item];
                    if (node.isHeading) {
                        int y = area.getTopY() + font->getLineHeight()/2 - 1;
                        ctx.setColor(util::SkinColor::Faded);
                        drawHLine(ctx, area.getLeftX() + 2, y, area.getLeftX() + 28);
                        drawHLine(ctx, area.getLeftX() + 32 + font->getTextWidth(node.name), y, area.getRightX() - 2);
                        area.consumeX(30);
                        outTextF(ctx, area, node.name);
                    } else {
                        ctx.setColor(util::SkinColor::Static);
                        area.consumeX(10);
                        outTextF(ctx, area, node.name);
                    }
                }
            }

        // Widget:
        virtual void handlePositionChange()
            { defaultHandlePositionChange(); }
        virtual ui::layout::Info getLayoutInfo() const
            {
                int numLines = int(std::min(size_t(20), m_items.size()));
                gfx::Point size = getFont()->getCellSize().scaledBy(30, numLines);
                return ui::layout::Info(size, ui::layout::Info::GrowBoth);
            }
        virtual bool handleKey(util::Key_t key, int prefix)
            { return defaultHandleKey(key, prefix); }

     private:
        afl::base::Ref<gfx::Font> getFont() const
            { return m_root.provider().getFont(gfx::FontRequest()); }

        ui::Root& m_root;

        // As long as this widget is only used locally, keep a reference, not a copy.
        const ExpressionLists::Items_t& m_items;
    };
}


bool
client::widgets::doExpressionListPopup(ui::Root& root,
                                       game::proxy::WaitIndicator& ind,
                                       game::proxy::ExpressionListProxy& proxy,
                                       gfx::Point anchor,
                                       String_t& value,
                                       String_t& flags)
{
    // ex WLRUPopup::doPopup
    // Get list of items
    ExpressionLists::Items_t items;
    proxy.getList(ind, items);
    if (items.empty()) {
        return false;
    }

    // List widget
    ExpressionList listWidget(root, items);
    size_t pos = 0;
    for (size_t i = 0, n = items.size(); i < n; ++i) {
        if (!items[i].isHeading && items[i].value == value) {
            pos = i;
            break;
        }
    }
    listWidget.setCurrentItem(pos);

    // Operate it
    ui::EventLoop loop(root);
    ui::widgets::MenuFrame frame(ui::layout::HBox::instance0, root, loop);
    if (frame.doMenu(listWidget, anchor)) {
        size_t pos = listWidget.getCurrentItem();
        if (pos < items.size() && !items[pos].isHeading) {
            // item selected
            value = items[pos].value;
            flags = items[pos].flags;
            return true;
        } else {
            // shouldn't happen
            return false;
        }
    } else {
        // Canceled
        return false;
    }
}
