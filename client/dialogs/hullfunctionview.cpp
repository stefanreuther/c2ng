/**
  *  \file client/dialogs/hullfunctionview.cpp
  *  \brief Hull function detail view dialog
  */

#include "client/dialogs/hullfunctionview.hpp"
#include "afl/string/format.hpp"
#include "client/widgets/helpwidget.hpp"
#include "gfx/context.hpp"
#include "ui/draw.hpp"
#include "ui/group.hpp"
#include "ui/icons/image.hpp"
#include "ui/icons/stylableicon.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/rich/documentview.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/abstractlistbox.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/keydispatcher.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/scrollbarcontainer.hpp"
#include "ui/window.hpp"

namespace {
    /*
     *  List Widget
     */

    class HullFunctionList : public ui::widgets::AbstractListbox {
     public:
        HullFunctionList(ui::Root& root, const game::spec::info::AbilityDetails_t& content)
            : m_root(root),
              m_content(content)
            { }

        // AbstractListbox:
        virtual size_t getNumItems()
            { return m_content.size(); }
        virtual bool isItemAccessible(size_t /*n*/)
            { return true; }
        virtual int getItemHeight(size_t /*n*/)
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
                // ex WHullFunctionList::drawPart, CHullCapabilityList.DrawPart
                // List item context
                afl::base::Deleter del;
                gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
                prepareColorListItem(ctx, area, state, m_root.colorScheme(), del);

                // Font
                afl::base::Ref<gfx::Font> font     = getFont();
                afl::base::Ref<gfx::Font> boldFont = m_root.provider().getFont("b");

                // Dimensions
                const int w1 = font->getTextWidth(" [U] ") + 3;
                const int w2 = font->getTextWidth("+ ") + 3;

                /* Each line consists of three segments:
                   - Assignment indicator icon
                       [U]   Universal ability (all ships, all races)
                       [R]   Racial ability (all ships, some races)
                       [G]   Global class function (this class, all races)
                       [C]   Class function (this class, some races)
                       [S]   Ship function (this ship)
                   - Availability marker
                       >     You have it
                       +     Need higher level
                       -     Level too high
                       blank Other races only
                   - Function name */
                String_t s1, s2, s3;
                bool damaged = false, available = false;
                util::SkinColor::Color color = util::SkinColor::Static;
                if (item < m_content.size()) {
                    const game::spec::info::AbilityDetail& hf = m_content[item];

                    // Assignment indicator icon
                    switch (hf.kind) {
                     case game::spec::info::UniversalAbility:   s1 = " [U] "; break;
                     case game::spec::info::RacialAbility:      s1 = " [R] "; break;
                     case game::spec::info::GlobalClassAbility: s1 = " [G] "; break;
                     case game::spec::info::ClassAbility:       s1 = " [C] "; break;
                     case game::spec::info::ShipAbility:        s1 = " [S] "; break;
                    }

                    // Availability marker
                    if (hf.flags.contains(game::spec::info::ForeignAbility)) {
                        color = util::SkinColor::Faded;
                    } else if (hf.flags.contains(game::spec::info::ReachableAbility)) {
                        s2 = "+";
                        color = util::SkinColor::Blue;
                    } else if (hf.flags.contains(game::spec::info::OutgrownAbility)) {
                        s2 = "-";
                        color = util::SkinColor::Blue;
                    } else {
                        s2 = "\xE2\x96\xB6";
                        available = true;
                    }

                    // Name
                    s3 = hf.description;

                    // Damage status
                    damaged = hf.flags.contains(game::spec::info::DamagedAbility);
                }

                // Draw it
                ctx.setColor(color);
                ctx.useFont(*font);
                outTextF(ctx, area.splitX(w1), s1);

                if (damaged && available) {
                    ctx.setColor(util::SkinColor::Red);
                }
                outTextF(ctx, area.splitX(w2), s2);

                if (available) {
                    ctx.useFont(*boldFont);
                }
                ctx.setColor(color);
                outTextF(ctx, area, s3);
            }

        // Widget:
        virtual void handlePositionChange(gfx::Rectangle& oldPosition)
            { defaultHandlePositionChange(oldPosition); }
        virtual ui::layout::Info getLayoutInfo() const
            {
                gfx::Point sz = getFont()->getCellSize().scaledBy(30, 10);
                return ui::layout::Info(sz, sz, ui::layout::Info::GrowBoth);
            }
        virtual bool handleKey(util::Key_t key, int prefix)
            { return defaultHandleKey(key, prefix); }

        const game::spec::info::AbilityDetail* getCurrentFunction() const
            {
                // ex WHullFunctionList::getSelectedFunction
                size_t n = getCurrentItem();
                if (n < m_content.size()) {
                    return &m_content[n];
                } else {
                    return 0;
                }
            }

     private:
        ui::Root& m_root;
        const game::spec::info::AbilityDetails_t& m_content;

        afl::base::Ref<gfx::Font> getFont() const
            { return m_root.provider().getFont(gfx::FontRequest()); }
    };

    /*
     *  Dialog
     */

    class Dialog {
     public:
        Dialog(const game::spec::info::AbilityDetails_t& content, ui::Root& root, afl::string::Translator& tx);

        void run(util::RequestSender<game::Session> gameSender);
        void onScroll();

     private:
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        HullFunctionList m_listWidget;
        ui::rich::DocumentView m_infoWidget;
        afl::base::SignalConnection conn_imageChange;
    };
}

Dialog::Dialog(const game::spec::info::AbilityDetails_t& content, ui::Root& root, afl::string::Translator& tx)
    : m_root(root),
      m_translator(tx),
      m_listWidget(root, content),
      m_infoWidget(root.provider().getFont(gfx::FontRequest())->getCellSize().scaledBy(30, 11), 0, m_root.provider()),
      conn_imageChange(root.provider().sig_imageChange.add(this, &Dialog::onScroll))
{
    m_listWidget.sig_change.add(this, &Dialog::onScroll);
}

void
Dialog::run(util::RequestSender<game::Session> gameSender)
{
    // ex WHullFunctionView::init
    // VBox
    //   Frame > Scrollbar > List
    //   Info
    //   HBox
    //     Help | UISpacer | OK
    afl::base::Deleter del;
    ui::Window& win = del.addNew(new ui::Window(m_translator("Ship Functions"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));

    // Content
    win.add(ui::widgets::FrameGroup::wrapWidget(del, m_root.colorScheme(), ui::LoweredFrame, del.addNew(new ui::widgets::ScrollbarContainer(m_listWidget, m_root))));
    win.add(m_infoWidget);

    // Buttons
    ui::widgets::Button& btnHelp  = del.addNew(new ui::widgets::Button(m_translator("Help"), 'h',               m_root));
    ui::widgets::Button& btnClose = del.addNew(new ui::widgets::Button(m_translator("Close"), util::Key_Escape, m_root));
    ui::Group& g = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    g.add(btnHelp);
    g.add(del.addNew(new ui::Spacer()));
    g.add(btnClose);
    win.add(g);

    // Connect events
    ui::Widget& help = del.addNew(new client::widgets::HelpWidget(m_root, m_translator, gameSender, "pcc2:specsheet"));
    win.add(help);
    btnHelp.dispatchKeyTo(help);

    ui::EventLoop loop(m_root);
    btnClose.sig_fire.addNewClosure(loop.makeStop(0));

    ui::widgets::KeyDispatcher& disp = del.addNew(new ui::widgets::KeyDispatcher());
    disp.addNewClosure(' ', loop.makeStop(0));
    disp.addNewClosure(util::Key_Return, loop.makeStop(0));
    win.add(disp);

    win.add(del.addNew(new ui::widgets::Quit(m_root, loop)));

    // Show
    win.pack();
    m_root.centerWidget(win);
    onScroll();                    // render after layout

    m_root.add(win);
    loop.run();
}

void
Dialog::onScroll()
{
    // ex WHullFunctionInfo::setFunction, CHullCapabilityView.DrawInterior
    ui::rich::Document& doc = m_infoWidget.getDocument();
    doc.clear();
    if (const game::spec::info::AbilityDetail* d = m_listWidget.getCurrentFunction()) {
        // Image as float-right object
        if (!d->pictureName.empty()) {
            afl::base::Ptr<gfx::Canvas> pic = m_root.provider().getImage(d->pictureName);
            if (pic.get() != 0) {
                ui::icons::Icon& image = doc.deleter().addNew(new ui::icons::Image(*pic));
                ui::icons::StylableIcon& obj = doc.deleter().addNew(new ui::icons::StylableIcon(image, m_root.colorScheme()));
                obj.setFrameWidth(1);
                obj.setFrameType(ui::LoweredFrame);
                obj.setBackgroundColor(m_root.colorScheme().getColor(ui::Color_Black));
                doc.addFloatObject(obj, false);
            }
        }

        // Function title
        doc.add(util::rich::Text(d->description).withStyle(util::rich::StyleAttribute::Bold));
        doc.addParagraph();

        // Availability information
        if (!d->playerLimit.empty()) {
            doc.add(afl::string::Format(m_translator("Available to %s"), d->playerLimit));
            doc.addNewline();
        }
        if (!d->levelLimit.empty()) {
            doc.add(afl::string::Format(m_translator("Available at %s"), d->levelLimit));
            if (d->minimumExperience > 0) {
                doc.add(" ");
                doc.add(afl::string::Format(m_translator("(%d EP)"), d->minimumExperience));
            }
            doc.addNewline();
        }
        switch (d->kind) {
         case game::spec::info::UniversalAbility:
            doc.add(m_translator("Universal ability"));
            break;
         case game::spec::info::RacialAbility:
            doc.add(m_translator("Racial ability"));
            break;
         case game::spec::info::GlobalClassAbility:
         case game::spec::info::ClassAbility:
            doc.add(m_translator("Assigned to ship class"));
            break;
         case game::spec::info::ShipAbility:
            doc.add(m_translator("Assigned to ship"));
            break;
        }

        int damageLimit;
        if (d->damageLimit.get(damageLimit)) {
            doc.addNewline();
            doc.add(afl::string::Format(m_translator("Fails at %d%% damage"), damageLimit));
            if (d->flags.contains(game::spec::info::DamagedAbility)) {
                doc.add(" ");
                doc.add(util::rich::Text(util::SkinColor::Red, m_translator("(currently broken)")));
            }
        }
        doc.addParagraph();

        // Description
        doc.add(d->explanation);
    }
    doc.finish();
    m_infoWidget.handleDocumentUpdate();
}


/*
 *  Entry Point
 */

void
client::dialogs::showHullFunctions(const game::spec::info::AbilityDetails_t& content, ui::Root& root, util::RequestSender<game::Session> gameSender, afl::string::Translator& tx)
{
    // ex showHullFunctions, shipspec.pas:ShowHullCapabilityList
    if (!content.empty()) {
        Dialog(content, root, tx).run(gameSender);
    }
}
