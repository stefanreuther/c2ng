/**
  *  \file client/dialogs/subjectlist.cpp
  *  \brief Message Subject List dialog.
  */

#include "client/dialogs/subjectlist.hpp"
#include "client/downlink.hpp"
#include "game/proxy/configurationproxy.hpp"
#include "gfx/context.hpp"
#include "ui/draw.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/widgets/abstractlistbox.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/optiongrid.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/scrollbarcontainer.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/window.hpp"

namespace {
    String_t formatBool(bool flag, afl::string::Translator& tx)
    {
        return flag ? tx("yes (skipped by default)") : tx("no (shown by default)");
    }


    /*
     *  SubjectList widget
     */

    class SubjectList : public ui::widgets::AbstractListbox {
     public:
        SubjectList(const game::msg::Browser::Summary_t& summary, ui::Root& root, util::NumberFormatter fmt)
            : AbstractListbox(),
              m_summary(summary),
              m_root(root),
              m_formatter(fmt)
            {
                // ex WSubjectList::WSubjectList
            }
        virtual size_t getNumItems() const
            { return m_summary.size(); }
        virtual bool isItemAccessible(size_t /*n*/) const
            { return true; }
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
                // ex WSubjectList::drawPart
                afl::base::Deleter del;
                gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
                prepareColorListItem(ctx, area, state, m_root.colorScheme(), del);
                ctx.useFont(*getFont());

                if (item < m_summary.size()) {
                    const int em = ctx.getFont()->getEmWidth();
                    const game::msg::Browser::SummaryEntry& e = m_summary[item];
                    ctx.setColor(e.isFiltered ? util::SkinColor::Faded : util::SkinColor::Static);
                    ctx.setTextAlign(gfx::RightAlign, gfx::MiddleAlign);
                    outTextF(ctx, area.splitX(3*em), m_formatter.formatNumber(static_cast<int32_t>(e.count)));
                    ctx.setTextAlign(gfx::LeftAlign, gfx::MiddleAlign);
                    area.consumeX(em/2);
                    outTextF(ctx, area, e.heading);
                }
            }
        virtual void handlePositionChange()
            { defaultHandlePositionChange(); }
        virtual ui::layout::Info getLayoutInfo() const
            {
                gfx::Point size = getFont()->getCellSize().scaledBy(25, 20);
                return ui::layout::Info(size, size, ui::layout::Info::GrowBoth);
            }
        virtual bool handleKey(util::Key_t key, int prefix)
            { return defaultHandleKey(key, prefix); }

        const game::msg::Browser::SummaryEntry* getCurrentEntry() const
            {
                size_t item = getCurrentItem();
                return (item < m_summary.size() ? &m_summary[item] : 0);
            }

        void setContent(const game::msg::Browser::Summary_t& summary)
            {
                m_summary = summary;
                handleModelChange();
            }

     private:
        game::msg::Browser::Summary_t m_summary;
        ui::Root& m_root;
        util::NumberFormatter m_formatter;

        afl::base::Ref<gfx::Font> getFont() const
            { return m_root.provider().getFont(gfx::FontRequest()); }
    };

    class SubjectListDialog {
     public:
        // Ids for m_options
        enum {
            IdFilter
        };

        SubjectListDialog(game::proxy::MailboxProxy& proxy, ui::Root& root, const game::msg::Browser::Summary_t& summary, util::NumberFormatter fmt, afl::string::Translator& tx);
        void run(size_t index);

     private:
        // Links
        game::proxy::MailboxProxy& m_proxy;
        ui::Root& m_root;
        afl::string::Translator& m_translator;

        // Widgets
        SubjectList m_list;
        ui::widgets::OptionGrid m_options;

        afl::base::SignalConnection conn_summaryChange;

        void onScroll();
        void onOptionClick(int id);
        void onSummaryChange(const game::msg::Browser::Summary_t& content);
    };
}


/*
 *  SubjectListDialog
 */

SubjectListDialog::SubjectListDialog(game::proxy::MailboxProxy& proxy,
                  ui::Root& root,
                  const game::msg::Browser::Summary_t& summary,
                  util::NumberFormatter fmt,
                  afl::string::Translator& tx)
    : m_proxy(proxy),
      m_root(root),
      m_translator(tx),
      m_list(summary, root, fmt),
      m_options(0, 0, root),
      conn_summaryChange(proxy.sig_summaryChanged.add(this, &SubjectListDialog::onSummaryChange))
{
    m_options.addItem(IdFilter, 'k', tx("Filtered"))
        .addPossibleValue(formatBool(true, tx))
        .addPossibleValue(formatBool(false, tx));
    m_list.sig_change.add(this, &SubjectListDialog::onScroll);
    m_options.sig_click.add(this, &SubjectListDialog::onOptionClick);
}

void
SubjectListDialog::run(size_t index)
{
    // ex WSubjectList::doStandardDialog
    m_list.setCurrentItem(index);

    // Window [VBox]
    //   ScrollbarContainer [SubjectList]
    //   OptionGrid
    //   StandardDialogButtons
    afl::base::Deleter del;
    ui::Window& win = del.addNew(new ui::Window(m_translator("Message Summary"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));
    win.add(ui::widgets::FrameGroup::wrapWidget(del, m_root.colorScheme(), ui::LoweredFrame,
                                                del.addNew(new ui::widgets::ScrollbarContainer(m_list, m_root))));
    win.add(m_options);

    ui::widgets::StandardDialogButtons& buttons = del.addNew(new ui::widgets::StandardDialogButtons(m_root, m_translator));
    win.add(buttons);

    // FIXME: help
    ui::EventLoop loop(m_root);
    buttons.addStop(loop);

    win.add(del.addNew(new ui::widgets::Quit(m_root, loop)));
    win.pack();

    m_root.centerWidget(win);
    m_root.add(win);

    if (loop.run() != 0) {
        if (const game::msg::Browser::SummaryEntry* e = m_list.getCurrentEntry()) {
            m_proxy.setCurrentMessage(e->index);
        }
    }
}

void
SubjectListDialog::onScroll()
{
    const game::msg::Browser::SummaryEntry* e = m_list.getCurrentEntry();
    bool isFiltered = (e != 0 && e->isFiltered);
    m_options.findItem(IdFilter).setValue(formatBool(isFiltered, m_translator));
}

void
SubjectListDialog::onOptionClick(int id)
{
    if (id == IdFilter) {
        if (const game::msg::Browser::SummaryEntry* e = m_list.getCurrentEntry()) {
            m_proxy.toggleHeadingFiltered(e->heading);
        }
    }
}

void
SubjectListDialog::onSummaryChange(const game::msg::Browser::Summary_t& content)
{
    m_list.setContent(content);
}


/*
 *  Entry Point
 */

void
client::dialogs::doSubjectListDialog(game::proxy::MailboxProxy& proxy, ui::Root& root, util::RequestSender<game::Session> gameSender, afl::string::Translator& tx)
{
    // Initialize data
    Downlink link(root, tx);
    game::msg::Browser::Summary_t sum;
    size_t index = 0;
    proxy.getSummary(link, sum, index);
    if (sum.empty()) {
        return;
    }

    util::NumberFormatter fmt = game::proxy::ConfigurationProxy(gameSender).getNumberFormatter(link);

    // Build dialog
    SubjectListDialog dlg(proxy, root, sum, fmt, tx);
    dlg.run(index);
}
