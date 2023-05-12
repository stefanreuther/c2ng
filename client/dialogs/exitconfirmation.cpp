/**
  *  \file client/dialogs/exitconfirmation.cpp
  *  \brief Exit Confirmation Dialog
  */

#include <algorithm>
#include "client/dialogs/exitconfirmation.hpp"
#include "gfx/complex.hpp"
#include "gfx/context.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/draw.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/abstractlistbox.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/keydispatcher.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/window.hpp"
#include "util/skincolor.hpp"

namespace {
    /*
     *  Two-Line list box.
     *
     *  For now, a single-use widget, only used in this one dialog.
     */
    class TwoLineListbox : public ui::widgets::AbstractListbox {
     public:
        struct Item {
            int id;
            util::SkinColor::Color bottomColor;
            String_t top;
            String_t bottom;
            Item(int id, util::SkinColor::Color bottomColor, String_t top, String_t bottom)
                : id(id), bottomColor(bottomColor), top(top), bottom(bottom)
                { }
        };

        TwoLineListbox(const std::vector<Item>& content, ui::Root& root);

        virtual size_t getNumItems() const
            { return m_content.size(); }
        virtual bool isItemAccessible(size_t /*n*/) const
            { return true; }
        virtual int getItemHeight(size_t /*n*/) const
            { return m_itemHeight; }
        virtual int getHeaderHeight() const
            { return 0; }
        virtual int getFooterHeight() const
            { return 0; }
        virtual void drawHeader(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
            { }
        virtual void drawFooter(gfx::Canvas& /*can*/, gfx::Rectangle /*area*/)
            { }
        virtual void drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state);
        virtual void handlePositionChange()
            { return defaultHandlePositionChange(); }
        virtual ui::layout::Info getLayoutInfo() const
            { return gfx::Point(m_width, m_itemHeight * int(m_content.size())); }
        virtual bool handleKey(util::Key_t key, int prefix)
            { return defaultHandleKey(key, prefix); }

        int getCurrentId() const;

     private:
        std::vector<Item> m_content;
        ui::Root& m_root;
        int m_width;
        int m_itemHeight;
    };

    int computeWidth(const std::vector<TwoLineListbox::Item>& content, gfx::ResourceProvider& provider)
    {
        afl::base::Ref<gfx::Font> titleFont = provider.getFont("+");
        afl::base::Ref<gfx::Font> normalFont = provider.getFont(gfx::FontRequest());
        int max = 0;
        for (size_t i = 0; i < content.size(); ++i) {
            max = std::max(max, titleFont->getTextWidth(content[i].top));
            max = std::max(max, normalFont->getTextWidth(content[i].bottom));
        }
        return max + 30;
    }

    int computeItemHeight(gfx::ResourceProvider& provider)
    {
        return provider.getFont("+")->getLineHeight()
            + provider.getFont(gfx::FontRequest())->getLineHeight();
    }
}

TwoLineListbox::TwoLineListbox(const std::vector<Item>& content, ui::Root& root)
    : m_content(content),
      m_root(root),
      m_width(computeWidth(content, root.provider())),
      m_itemHeight(computeItemHeight(root.provider()))
{
    // TwoLineListbox::TwoLineListbox
}

void
TwoLineListbox::drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state)
{
    // TwoLineListbox::drawPart(GfxCanvas& can, int from, int to)
    const int EXTRA = 20;

    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    afl::base::Deleter del;
    ui::prepareColorListItem(ctx, area, state, m_root.colorScheme(), del);

    if (item < m_content.size()) {
        ctx.setColor(util::SkinColor::Static);
        ctx.useFont(*m_root.provider().getFont("+"));

        gfx::Rectangle left = area.splitX(EXTRA);
        outTextF(ctx, area.splitY(ctx.getFont()->getLineHeight()), m_content[item].top);

        if (state == ActiveItem || state == FocusedItem) {
            int center = left.getTopY() + ctx.getFont()->getLineHeight()/2;
            for (int i = 0; i < 5; ++i) {
                gfx::drawVLine(ctx, left.getLeftX() + 7 + i, center - 4 + i, center + 3 - i);
            }
        }

        ctx.setColor(m_content[item].bottomColor);
        ctx.useFont(*m_root.provider().getFont(gfx::FontRequest()));
        outTextF(ctx, area, m_content[item].bottom);
    }
}

int
TwoLineListbox::getCurrentId() const
{
    size_t index = getCurrentItem();
    if (index < m_content.size()) {
        return m_content[index].id;
    } else {
        return 0;
    }
}

/*
 *  Public Entry Point
 */

int
client::dialogs::askExitConfirmation(ui::Root& root, afl::string::Translator& tx)
{
    // ex askExitConfirmation()
    std::vector<TwoLineListbox::Item> content;
    content.push_back(TwoLineListbox::Item(ExitDialog_Save + ExitDialog_Exit, util::SkinColor::Faded, tx("Save & Exit"),         tx("Return to race selection")));
    content.push_back(TwoLineListbox::Item(ExitDialog_Save,                   util::SkinColor::Faded, tx("Save only"),           tx("Save and keep playing")));
    content.push_back(TwoLineListbox::Item(ExitDialog_Exit,                   util::SkinColor::Red,   tx("Exit without Saving"), tx("Discard all changes")));

    afl::base::Deleter del;
    ui::Window& win = del.addNew(new ui::Window(tx("Exit Game"), root.provider(), root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));
    win.add(del.addNew(new ui::widgets::StaticText(tx("Choose an action:"), util::SkinColor::Static, gfx::FontRequest(), root.provider())));

    TwoLineListbox& box = del.addNew(new TwoLineListbox(content, root));
    win.add(box);

    ui::widgets::Button& btnOK = del.addNew(new ui::widgets::Button(tx("OK"), util::Key_Return, root));
    ui::widgets::Button& btnCancel = del.addNew(new ui::widgets::Button(tx("Cancel"), util::Key_Escape, root));
    ui::Group& g = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    g.add(del.addNew(new ui::Spacer()));
    g.add(btnOK);
    g.add(btnCancel);
    g.add(del.addNew(new ui::Spacer()));
    win.add(g);

    ui::EventLoop loop(root);
    btnOK.sig_fire.addNewClosure(loop.makeStop(1));
    btnCancel.sig_fire.addNewClosure(loop.makeStop(0));

    ui::widgets::KeyDispatcher& disp = del.addNew(new ui::widgets::KeyDispatcher());
    disp.addNewClosure(' ', loop.makeStop(1));
    win.add(disp);
    win.pack();

    root.centerWidget(win);
    root.add(win);
    if (loop.run() == 0) {
        return 0; /* ex ExitCancelled */
    }

    int result = box.getCurrentId();
    if (result == ExitDialog_Exit /* ex ExitWithoutSaving */) {
        if (!ui::dialogs::MessageBox(tx("Do you really want to exit without saving?"), tx("Exit Game"), root).doYesNoDialog(tx)) {
            return 0;
        }
    }
    return result;
}
