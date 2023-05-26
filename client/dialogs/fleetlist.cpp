/**
  *  \file client/dialogs/fleetlist.cpp
  *  \brief Fleet list standard dialog
  */

#include "client/dialogs/fleetlist.hpp"
#include "afl/base/deleter.hpp"
#include "client/widgets/helpwidget.hpp"
#include "gfx/context.hpp"
#include "ui/draw.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/widgets/abstractlistbox.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/scrollbarcontainer.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/window.hpp"

using game::ref::UserList;
using game::ref::FleetList;

namespace {
    /*
     *  FleetListbox widget
     *  (ex WFleetListWidget)
     */
    class FleetListbox : public ui::widgets::AbstractListbox {
     public:
        FleetListbox(ui::Root& root, afl::string::Translator& tx)
            : m_root(root),
              m_translator(tx),
              m_content()
            { }

        void setContent(const FleetList& list)
            {
                m_content = list;
                handleModelChange();
            }

        virtual size_t getNumItems() const
            { return m_content.size(); }

        virtual bool isItemAccessible(size_t n) const
            {
                bool ok = false;
                if (const FleetList::Item* p = m_content.get(n)) {
                    switch (p->type) {
                     case UserList::OtherItem:
                     case UserList::ReferenceItem:
                        ok = true;
                        break;

                     case UserList::DividerItem:
                     case UserList::SubdividerItem:
                        ok = false;
                        break;
                    }
                }
                return ok;
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
                // WFleetListWidget::drawPart(GfxCanvas& can, int from, int to)
                String_t hereMark = m_translator("(here)");
                afl::base::Ref<gfx::Font> font = getFont();
                int hereWidth = font->getTextWidth(hereMark) + 30;
                int indentWidth = 5;

                afl::base::Deleter del;
                gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
                prepareColorListItem(ctx, area, state, m_root.colorScheme(), del);
                if (const FleetList::Item* p = m_content.get(item)) {
                    switch (p->type) {
                     case UserList::OtherItem:
                     case UserList::ReferenceItem:
                        ctx.useFont(*font);
                        area.consumeX(indentWidth);
                        if (p->isAtReferenceLocation) {
                            outTextF(ctx, area.splitRightX(hereWidth), hereMark);
                        }
                        outTextF(ctx, area, p->name);
                        break;

                     case UserList::DividerItem:
                     case UserList::SubdividerItem:
                        ctx.useFont(*m_root.provider().getFont(gfx::FontRequest().addWeight(1)));
                        ctx.setColor(util::SkinColor::Faded);
                        ui::drawDivider(ctx, area, p->name, p->type == UserList::DividerItem);
                        break;
                    }
                }
            }

        virtual void handlePositionChange()
            { defaultHandlePositionChange(); }

        virtual ui::layout::Info getLayoutInfo() const
            {
                int numLines = int(std::min(size_t(15), std::max(size_t(5), m_content.size())));
                gfx::Point size(getFont()->getCellSize().scaledBy(30, numLines));
                return ui::layout::Info(size, ui::layout::Info::GrowBoth);
            }

        virtual bool handleKey(util::Key_t key, int prefix)
            { return defaultHandleKey(key, prefix); }

     private:
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        FleetList m_content;

        afl::base::Ref<gfx::Font> getFont() const
            { return m_root.provider().getFont(gfx::FontRequest()); }
    };
}


/*
 *  Main Entry Point
 */

game::Reference
client::dialogs::doFleetList(ui::Root& root, String_t okLabel, String_t title, const game::ref::FleetList& list,
                             util::RequestSender<game::Session> gameSender, afl::string::Translator& tx)
{
    // ex doFleetList
    // For now, this just takes a static list and does not receive updates from the game.
    // Therefore, it can be pretty simple.
    // In particular, there is no need for a specific proxy.
    afl::base::Deleter del;
    ui::Window& win = del.addNew(new ui::Window(title, root.provider(), root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));
    ui::EventLoop loop(root);

    // List
    FleetListbox& listbox = del.addNew(new FleetListbox(root, tx));
    listbox.setContent(list);
    listbox.setCurrentItem(list.findInitialSelection());
    win.add(ui::widgets::FrameGroup::wrapWidget(del, root.colorScheme(), ui::LoweredFrame, del.addNew(new ui::widgets::ScrollbarContainer(listbox, root))));

    // Buttons
    ui::widgets::StandardDialogButtons& btn = del.addNew(new ui::widgets::StandardDialogButtons(root, tx));
    btn.ok().setText(okLabel);
    btn.addStop(loop);
    win.add(btn);

    // Admin
    ui::Widget& help = del.addNew(new client::widgets::HelpWidget(root, tx, gameSender, "pcc2:fleetscreen"));
    btn.addHelp(help);
    win.add(help);
    win.add(del.addNew(new ui::widgets::Quit(root, loop)));

    // Run
    win.pack();
    root.centerWidget(win);
    root.add(win);
    bool ok = (loop.run() != 0);

    // Check result
    game::Reference result;
    if (ok) {
        if (const FleetList::Item* p = list.get(listbox.getCurrentItem())) {
            if (p->type == UserList::ReferenceItem) {
                result = p->reference;
            }
        }
    }
    return result;
}
