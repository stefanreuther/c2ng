/**
  *  \file client/dialogs/historyship.cpp
  */

#include "client/dialogs/historyship.hpp"
#include "afl/base/staticassert.hpp"
#include "client/downlink.hpp"
#include "client/widgets/helpwidget.hpp"
#include "client/widgets/historyshiplistbox.hpp"
#include "game/config/integeroption.hpp"
#include "game/config/integervalueparser.hpp"
#include "game/proxy/configurationproxy.hpp"
#include "game/proxy/historyshiplistproxy.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/menuframe.hpp"
#include "ui/widgets/optiongrid.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/scrollbarcontainer.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/widgets/stringlistbox.hpp"
#include "ui/window.hpp"

using game::ref::HistoryShipSelection;
using game::proxy::HistoryShipListProxy;

namespace {
    /*
     *  Configuration
     */

    // Verify external representation
    static_assert(HistoryShipSelection::ById    == 0, "ById");
    static_assert(HistoryShipSelection::ByOwner == 1, "ByOwner");
    static_assert(HistoryShipSelection::ByHull  == 2, "ByHull");
    static_assert(HistoryShipSelection::ByAge   == 3, "ByAge");
    static_assert(HistoryShipSelection::ByName  == 4, "ByName");

    void loadConfiguration(game::proxy::WaitIndicator& ind, game::proxy::ConfigurationProxy& proxy, HistoryShipSelection& sel)
    {
        int32_t value = proxy.getOption(ind, game::config::UserConfiguration::Sort_History);
        if (value >= 0 && value <= int32_t(HistoryShipSelection::SortMax)) {
            sel.setSortOrder(HistoryShipSelection::SortOrder(value));
        }
    }

    void saveConfiguration(game::proxy::ConfigurationProxy& proxy, HistoryShipSelection::SortOrder sortOrder)
    {
        proxy.setOption(game::config::UserConfiguration::Sort_History, int(sortOrder));
    }


    /*
     *  HistoryShipDialog
     */

    const int IdMode = 1;

    class HistoryShipDialog {
     public:
        HistoryShipDialog(ui::Root& root, afl::string::Translator& tx, util::RequestSender<game::Session> gameSender);

        void setConfiguration(const HistoryShipSelection& sel, HistoryShipSelection::Modes_t modes);
        bool run();
        int getCurrentShipId() const;
        HistoryShipSelection::SortOrder getCurrentSortOrder() const;

     private:
        // Links
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        util::RequestSender<game::Session> m_gameSender;
        game::proxy::HistoryShipListProxy m_proxy;

        // Status
        HistoryShipSelection m_selection;
        HistoryShipSelection::Modes_t m_modes;

        // Widgets
        client::widgets::HistoryShipListbox m_list;
        ui::widgets::OptionGrid m_options;

        void onListChange(const game::ref::HistoryShipList& list);
        void onMenuRequest(gfx::Point anchor);
        void onOptionClick(int id);
        void updateConfiguration();
    };
}

HistoryShipDialog::HistoryShipDialog(ui::Root& root, afl::string::Translator& tx, util::RequestSender<game::Session> gameSender)
    : m_root(root),
      m_translator(tx),
      m_gameSender(gameSender),
      m_proxy(gameSender, root.engine().dispatcher()),
      m_selection(),
      m_modes(),
      m_list(root, tx),
      m_options(0, 0, root)
{
    m_options.addItem(IdMode, 'd', tx("Display"));
    m_proxy.sig_listChange.add(this, &HistoryShipDialog::onListChange);
    m_options.sig_click.add(this, &HistoryShipDialog::onOptionClick);
    m_list.sig_menuRequest.add(this, &HistoryShipDialog::onMenuRequest);
    m_list.setFlag(ui::widgets::AbstractListbox::KeyboardMenu, true);
}

void
HistoryShipDialog::setConfiguration(const HistoryShipSelection& sel, HistoryShipSelection::Modes_t modes)
{
    m_modes = modes;
    m_selection = sel;
    updateConfiguration();
}

bool
HistoryShipDialog::run()
{
    // ex chooseHistoryShip (part)
    afl::base::Deleter del;

    ui::Window& win = del.addNew(new ui::Window(m_translator("Select Ship"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));
    win.add(ui::widgets::FrameGroup::wrapWidget(del, m_root.colorScheme(), ui::LoweredFrame,
                                                del.addNew(new ui::widgets::ScrollbarContainer(m_list, m_root))));

    win.add(m_options);

    ui::widgets::StandardDialogButtons& buttons = del.addNew(new ui::widgets::StandardDialogButtons(m_root, m_translator));
    ui::Widget& help = del.addNew(new client::widgets::HelpWidget(m_root, m_translator, m_gameSender, "pcc2:historyselect"));
    ui::EventLoop loop(m_root);
    buttons.addStop(loop);
    buttons.addHelp(help);
    win.add(buttons);
    win.add(help);
    win.add(del.addNew(new ui::widgets::Quit(m_root, loop)));

    win.pack();
    m_root.centerWidget(win);
    m_root.add(win);
    return loop.run() != 0;
}

int
HistoryShipDialog::getCurrentShipId() const
{
    return m_list.getCurrentReference().getId();
}

HistoryShipSelection::SortOrder
HistoryShipDialog::getCurrentSortOrder() const
{
    return m_selection.getSortOrder();
}

void
HistoryShipDialog::onListChange(const game::ref::HistoryShipList& list)
{
    m_list.setContent(list);
}

void
HistoryShipDialog::onMenuRequest(gfx::Point anchor)
{
    // ex WHistoryShipControl::onClick
    ui::widgets::StringListbox box(m_root.provider(), m_root.colorScheme());
    const int32_t SORT_BASE = 1000;

    // Modes
    for (size_t i = 0; i <= HistoryShipSelection::ModeMax; ++i) {
        HistoryShipSelection::Mode mode = HistoryShipSelection::Mode(i);
        if (m_modes.contains(mode)) {
            box.addItem(int32_t(i), m_selection.getModeName(mode, m_translator));
        }
    }

    // Sorts
    for (size_t i = 0; i <= HistoryShipSelection::SortMax; ++i) {
        box.addItem(SORT_BASE + int32_t(i), m_selection.getSortOrderName(HistoryShipSelection::SortOrder(i), m_translator));
    }

    // Show menu popup
    ui::EventLoop loop(m_root);
    bool ok = ui::widgets::MenuFrame(ui::layout::VBox::instance0, m_root, loop)
        .doMenu(box, anchor);

    // Evaluate result
    int32_t found;
    if (ok && box.getCurrentKey(found)) {
        if (found >= SORT_BASE && found <= SORT_BASE + int32_t(HistoryShipSelection::SortMax)) {
            m_selection.setSortOrder(HistoryShipSelection::SortOrder(found - SORT_BASE));
            updateConfiguration();
        }
        if (found >= 0 && found <= int32_t(HistoryShipSelection::ModeMax)) {
            m_selection.setMode(HistoryShipSelection::Mode(found));
            updateConfiguration();
        }
    }
}

void
HistoryShipDialog::onOptionClick(int id)
{
    if (id == IdMode) {
        onMenuRequest(m_options.getAnchorPointForItem(IdMode));
    }
}

void
HistoryShipDialog::updateConfiguration()
{
    // Submit to proxy
    m_proxy.setSelection(m_selection);

    // Update UI
    // FIXME: as of 20210410, the addPossibleValue() is needed, otherwise OptionGrid doesn't allocate ANY space to the value.
    String_t value = m_selection.getModeName(m_translator);
    value += " / ";
    value += m_selection.getSortOrderName(m_translator);

    m_options.findItem(IdMode).addPossibleValue(value).setValue(value);
}


/*
 *  Entry Point
 */

int
client::dialogs::chooseHistoryShip(game::ref::HistoryShipSelection sel,
                                   game::ref::HistoryShipSelection::Modes_t modes,
                                   ui::Root& root,
                                   afl::string::Translator& tx,
                                   util::RequestSender<game::Session> gameSender)
{
    // Configuration access: update sort order from config
    Downlink link(root, tx);
    game::proxy::ConfigurationProxy configProxy(gameSender);
    loadConfiguration(link, configProxy, sel);

    // Run the dialog
    HistoryShipDialog dlg(root, tx, gameSender);
    dlg.setConfiguration(sel, modes);
    bool ok = dlg.run();

    // Save configuration
    saveConfiguration(configProxy, dlg.getCurrentSortOrder());

    // Result
    if (ok) {
        return dlg.getCurrentShipId();
    } else {
        return 0;
    }
}
