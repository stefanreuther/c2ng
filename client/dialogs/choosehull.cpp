/**
  *  \file client/dialogs/choosehull.cpp
  *  \brief Hull selection dialog
  */

#include "client/dialogs/choosehull.hpp"
#include "client/downlink.hpp"
#include "client/picturenamer.hpp"
#include "game/playerarray.hpp"
#include "game/proxy/playerproxy.hpp"
#include "game/proxy/specbrowserproxy.hpp"
#include "game/spec/info/types.hpp"
#include "ui/eventloop.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/menuframe.hpp"
#include "ui/widgets/optiongrid.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/scrollbarcontainer.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/widgets/stringlistbox.hpp"
#include "ui/window.hpp"
#include "util/string.hpp"

using game::proxy::SpecBrowserProxy;
using game::PlayerArray;
using game::proxy::PlayerProxy;

namespace gsi = game::spec::info;

namespace {
    /* Item Id for button in OptionGrid */
    const int ITEM_ID = 1;

    /* Dialog state */
    class ChooseHullDialog {
     public:
        ChooseHullDialog(ui::Root& root, afl::string::Translator& tx, int current, SpecBrowserProxy& proxy, const PlayerArray<String_t> playerNames, bool withCustom);
        bool run(const String_t& title);
        int32_t getCurrent();

     private:
        void onListChange(const gsi::ListContent& content, size_t index, game::spec::info::Page page);
        void onFilterChange(const gsi::FilterInfos_t& existing, const gsi::FilterInfos_t& available);
        void onSortChange(gsi::FilterAttribute active, gsi::FilterAttributes_t available);
        void onItemDoubleClick();
        void onMenuButtonClick();
        void onContextMenuClick(gfx::Point pt);
        void doMenu(gfx::Point pt);
        void renderDisplayOptions();

        SpecBrowserProxy& m_proxy;
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        ui::EventLoop m_loop;
        ui::widgets::StringListbox m_list;
        ui::widgets::OptionGrid m_options;
        PlayerArray<String_t> m_playerNames;
        gsi::FilterAttribute m_sortOrder;
        int m_playerFilter;
        int m_current;
        bool m_withCustom;
    };
}

ChooseHullDialog::ChooseHullDialog(ui::Root& root, afl::string::Translator& tx, int current, SpecBrowserProxy& proxy, const PlayerArray<String_t> playerNames, bool withCustom)
    : m_proxy(proxy),
      m_root(root),
      m_translator(tx),
      m_loop(root),
      m_list(m_root.provider(), m_root.colorScheme()),
      m_options(0, 0, root),
      m_playerNames(playerNames),
      m_sortOrder(gsi::String_Name),
      m_playerFilter(0),
      m_current(current),
      m_withCustom(withCustom)
{
    // ex WHullSelectionDialog::WHullSelectionDialog
    proxy.sig_listChange.add(this, &ChooseHullDialog::onListChange);
    proxy.sig_filterChange.add(this, &ChooseHullDialog::onFilterChange);
    proxy.sig_sortChange.add(this, &ChooseHullDialog::onSortChange);
    m_list.setPreferredHeight(18);
    m_list.setPreferredWidth(20, false);
    m_list.sig_menuRequest.add(this, &ChooseHullDialog::onContextMenuClick);
    m_list.sig_itemDoubleClick.add(this, &ChooseHullDialog::onItemDoubleClick);
    m_options.addItem(ITEM_ID, '#', tx("Show"));
    m_options.sig_click.add(this, &ChooseHullDialog::onMenuButtonClick);
}

bool
ChooseHullDialog::run(const String_t& title)
{
    // ex WHullSelectionDialog::run
    afl::base::Deleter del;
    ui::Window& win = del.addNew(new ui::Window(title, m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));
    win.add(ui::widgets::FrameGroup::wrapWidget(del, m_root.colorScheme(), ui::LoweredFrame,
                                                del.addNew(new ui::widgets::ScrollbarContainer(m_list, m_root))));
    win.add(m_options);

    ui::widgets::StandardDialogButtons& btn = del.addNew(new ui::widgets::StandardDialogButtons(m_root, m_translator));
    win.add(btn);
    btn.addStop(m_loop);

    win.add(del.addNew(new ui::widgets::Quit(m_root, m_loop)));

    win.pack();
    m_root.centerWidget(win);
    m_root.add(win);
    return m_loop.run() != 0;
}

int32_t
ChooseHullDialog::getCurrent()
{
    m_list.getCurrentKey(m_current);
    return m_current;
}

void
ChooseHullDialog::onListChange(const gsi::ListContent& content, size_t /*index*/, game::spec::info::Page page)
{
    // Fetch current. If list is still empty, this is a nop.
    if (page == gsi::HullPage) {
        m_list.getCurrentKey(m_current);

        // Replace list
        util::StringList list;
        if (m_withCustom && m_playerFilter == 0) {
            list.add(0, m_translator("Custom Ship"));
        }
        for (size_t i = 0, n = content.content.size(); i < n; ++i) {
            list.add(content.content[i].id, content.content[i].name);
        }
        m_list.swapItems(list);
        m_list.setCurrentKey(m_current);
    }
}

void
ChooseHullDialog::onFilterChange(const gsi::FilterInfos_t& existing, const gsi::FilterInfos_t& /*available*/)
{
    // We'll only ever set one filter for now
    if (!existing.empty() && existing[0].elem.att == gsi::Value_Player) {
        m_playerFilter = existing[0].elem.value;
    } else {
        m_playerFilter = 0;
    }
    renderDisplayOptions();
}

void
ChooseHullDialog::onSortChange(gsi::FilterAttribute active, gsi::FilterAttributes_t /*available*/)
{
    m_sortOrder = active;
    renderDisplayOptions();
}

void
ChooseHullDialog::onItemDoubleClick()
{
    m_loop.stop(1);
}

void
ChooseHullDialog::onMenuButtonClick()
{
    // ex WHullSelectionDialog::onMenuButton
    doMenu(m_options.getAnchorPointForItem(ITEM_ID));
}

void
ChooseHullDialog::onContextMenuClick(gfx::Point pt)
{
    // ex WHullSelectionDialog::onMenuClick
    doMenu(pt);
}

void
ChooseHullDialog::doMenu(gfx::Point pt)
{
    // ex WHullSelectionDialog::doMenu
    // Build list widget
    ui::widgets::StringListbox list(m_root.provider(), m_root.colorScheme());
    list.addItem(0, m_translator("Show all ship types"));
    for (int i = 1; i <= game::MAX_PLAYERS; ++i) {
        if (!m_playerNames.get(i).empty()) {
            list.addItem(i, afl::string::Format("Show %s ship types", m_playerNames.get(i)));
        }
    }
    list.addItem(1000, m_translator("Unsorted"));
    list.addItem(1001, m_translator("Sort by name"));

    // Dialog
    ui::EventLoop loop(m_root);
    ui::widgets::MenuFrame frame(ui::layout::VBox::instance5, m_root, loop);
    int32_t choice = 0;
    if (frame.doMenu(list, pt) && list.getCurrentKey(choice)) {
        if (choice == 0) {
            // Show all
            if (m_playerFilter != 0) {
                m_proxy.eraseFilter(0);
            }

            // Update (and then render) ahead-of-time because this affects list building
            // and we don't know whether onListChange or onFilterChange arrives first.
            m_playerFilter = choice;
            renderDisplayOptions();
        } else if (choice >= 1 && choice <= game::MAX_PLAYERS) {
            // Show one
            gsi::FilterElement ele(gsi::Value_Player, choice, gsi::IntRange_t());
            if (m_playerFilter == 0) {
                m_proxy.addFilter(ele);
            } else {
                m_proxy.setFilter(0, ele);
            }

            // Update (and then render) ahead-of-time
            m_playerFilter = choice;
            renderDisplayOptions();
        } else if (choice == 1000) {
            // Sort by Id
            m_proxy.setSortOrder(gsi::Range_Id);
        } else if (choice == 1001) {
            // Sort by name
            m_proxy.setSortOrder(gsi::String_Name);
        }
    }
}

void
ChooseHullDialog::renderDisplayOptions()
{
    String_t value;
    if (m_playerFilter == 0) {
        value = m_translator("all ship types");
    } else {
        value = afl::string::Format(m_translator("%s ship types"), m_playerNames.get(m_playerFilter));
    }

    if (m_sortOrder == gsi::String_Name) {
        util::addListItem(value, ", ", m_translator("by name"));
    }

    m_options.findItem(ITEM_ID).setValue(value);
}

/*
 *  Main Entry Point
 */

bool
client::dialogs::chooseHull(ui::Root& root, const String_t& title, int32_t& hullNr, afl::string::Translator& tx, util::RequestSender<game::Session> gameSender, bool withCustom)
{
    // ex ccsim.pas:ChooseHull
    // Retrieve player list
    Downlink link(root, tx);
    PlayerArray<String_t> names = PlayerProxy(gameSender).getPlayerNames(link, game::Player::AdjectiveName);

    // Set up SpecBrowserProxy and dialog.
    // Must be in one go, without intervening wait, so the ChooseHullDialog can connect the SpecBrowserProxy's events before they arrive.
    SpecBrowserProxy proxy(gameSender, root.engine().dispatcher(), std::auto_ptr<gsi::PictureNamer>(new PictureNamer()));
    proxy.setSortOrder(gsi::String_Name);
    proxy.setPage(gsi::HullPage);
    ChooseHullDialog dlg(root, tx, hullNr, proxy, names, withCustom);
    if (dlg.run(title)) {
        hullNr = dlg.getCurrent();
        return true;
    } else {
        return false;
    }
}
