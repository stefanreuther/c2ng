/**
  *  \file client/dialogs/flakvcrobject.cpp
  *  \brief FLAK VCR Object Dialog
  */

#include "client/dialogs/flakvcrobject.hpp"
#include "afl/base/deleter.hpp"
#include "client/dialogs/classicvcrobject.hpp"
#include "client/dialogs/hullspecification.hpp"
#include "client/downlink.hpp"
#include "client/widgets/combatunitlist.hpp"
#include "client/widgets/helpwidget.hpp"
#include "client/widgets/vcrobjectinfo.hpp"
#include "game/proxy/configurationproxy.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/imagebutton.hpp"
#include "ui/widgets/keyforwarder.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/scrollbarcontainer.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/window.hpp"

using game::proxy::VcrDatabaseProxy;
using client::widgets::CombatUnitList;
using afl::string::Format;

namespace {
    /*
     *  Dialog
     *
     *  Dynamic behaviour: observes a VcrDatabaseProxy that has been placed at the correct fight by the caller.
     *  - scrolling selects a unit (onListScroll)
     *  - proxy answers with onSideUpdate and onHullUpdate
     */

    class Dialog : private gfx::KeyEventConsumer {
     public:
        Dialog(ui::Root& root, afl::string::Translator& tx, util::RequestSender<game::Session> gameSender, VcrDatabaseProxy& proxy, util::NumberFormatter fmt);

        void init(game::proxy::WaitIndicator& ind, const game::vcr::BattleInfo& info, size_t initialUnit);
        game::Reference run(ui::Widget& help);
        void requestCurrent();
        void onListScroll();
        void onSideUpdate(const VcrDatabaseProxy::SideInfo& info);
        void onHullUpdate(const VcrDatabaseProxy::HullInfo& info);
        void onGoTo();
        void onHullSpecification();
        virtual bool handleKey(util::Key_t key, int prefix);
        void addToSimulation(bool after);

     private:
        // Integration:
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        VcrDatabaseProxy& m_proxy;
        util::RequestSender<game::Session> m_gameSender;

        // UI/Widgets
        CombatUnitList m_unitList;
        client::widgets::VcrObjectInfo m_objectInfo;
        ui::widgets::StaticText m_nameWidget;
        ui::widgets::StaticText m_subtitleWidget;
        ui::widgets::ImageButton m_image;
        ui::widgets::Button m_gotoButton;
        ui::widgets::Button m_specButton;
        ui::EventLoop m_loop;

        // Status
        int m_hullNr;
        game::Reference m_reference;
        afl::base::Optional<game::ShipQuery> m_shipQuery;

        // Events
        afl::base::SignalConnection conn_sideUpdate;
        afl::base::SignalConnection conn_hullUpdate;
    };
}


Dialog::Dialog(ui::Root& root, afl::string::Translator& tx, util::RequestSender<game::Session> gameSender, VcrDatabaseProxy& proxy, util::NumberFormatter fmt)
    : m_root(root),
      m_translator(tx),
      m_proxy(proxy),
      m_gameSender(gameSender),
      m_unitList(root),
      m_objectInfo(true, fmt, tx, root.provider()),
      m_nameWidget(String_t(), util::SkinColor::Static, "+", root.provider()),
      m_subtitleWidget(String_t(), util::SkinColor::Static, gfx::FontRequest(), root.provider()),
      m_image(String_t(), 0, root, gfx::Point(105, 93)),
      m_gotoButton(tx("Go to"), util::Key_Return, root),
      m_specButton("S", 's', root),
      m_loop(root),
      m_hullNr(0),
      m_reference(),
      m_shipQuery(),
      conn_sideUpdate(proxy.sig_sideUpdate.add(this, &Dialog::onSideUpdate)),
      conn_hullUpdate(proxy.sig_hullUpdate.add(this, &Dialog::onHullUpdate))
{
    m_nameWidget.setIsFlexible(true);
    m_subtitleWidget.setIsFlexible(true);
    m_gotoButton.sig_fire.add(this, &Dialog::onGoTo);
    m_specButton.sig_fire.add(this, &Dialog::onHullSpecification);
    m_unitList.sig_change.add(this, &Dialog::onListScroll);
}

/* Initialize (blocking data retrieval) */
void
Dialog::init(game::proxy::WaitIndicator& ind, const game::vcr::BattleInfo& info, size_t initialUnit)
{
    // Environment
    game::PlayerArray<String_t> names = m_proxy.getPlayerNames(ind, game::Player::AdjectiveName);
    game::TeamSettings teams;
    m_proxy.getTeamSettings(ind, teams);

    // Build the list
    size_t initialIndex = 0;
    for (size_t i = 0; i < info.groups.size(); ++i) {
        // Fleet
        const game::vcr::GroupInfo& g = info.groups[i];
        m_unitList.addItem(CombatUnitList::Fleet, i, Format(m_translator("%s fleet"), names.get(g.owner)), CombatUnitList::Flags_t() + CombatUnitList::Inaccessible, teams.getPlayerColor(g.owner));

        // Units
        for (size_t j = 0; j < g.numObjects; ++j) {
            size_t objIndex = g.firstObject + j;
            if (objIndex < info.units.size()) {
                if (initialUnit == objIndex) {
                    initialIndex = m_unitList.getNumItems();
                }
                m_unitList.addItem(CombatUnitList::Unit, objIndex, info.units[objIndex].text[0], CombatUnitList::Flags_t(), util::SkinColor::Static);
            }
        }
        m_unitList.setCurrentItem(initialIndex);
    }
}

/* Run dialog */
game::Reference
Dialog::run(ui::Widget& help)
{
    // Window (VBox)
    //   HBox 'main'
    //     CombatUnitList (framed, scrollbar)
    //     VBox 'content'
    //       HBox 'header'
    //         VBox 'label' (Title, Subtitle, Spacer)
    //         Image
    //       VcrObjectInfo
    //   HBox (Help, Spacer, Ins, Go to, Close)
    afl::base::Deleter del;
    ui::Window& win = del.addNew(new ui::Window(m_translator("Combat Information"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));

    ui::Group& mainGroup = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    mainGroup.add(ui::widgets::FrameGroup::wrapWidget(del, m_root.colorScheme(), ui::LoweredFrame, del.addNew(new ui::widgets::ScrollbarContainer(m_unitList, m_root))));

    ui::Group& contentGroup = del.addNew(new ui::Group(ui::layout::VBox::instance5));
    ui::Group& headerGroup = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    ui::Group& textGroup = del.addNew(new ui::Group(ui::layout::VBox::instance5));
    textGroup.add(m_nameWidget);
    textGroup.add(m_subtitleWidget);
    textGroup.add(del.addNew(new ui::Spacer()));
    headerGroup.add(textGroup);
    headerGroup.add(ui::widgets::FrameGroup::wrapWidget(del, m_root.colorScheme(), ui::LoweredFrame, m_image));
    contentGroup.add(headerGroup);
    contentGroup.add(m_objectInfo);
    mainGroup.add(contentGroup);
    win.add(mainGroup);

    ui::Group& buttonGroup = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    ui::widgets::Button& btnHelp  = del.addNew(new ui::widgets::Button(m_translator("Help"),      'h',              m_root));
    ui::widgets::Button& btnIns   = del.addNew(new ui::widgets::Button(m_translator("Ins - Sim"), util::Key_Insert, m_root));
    ui::widgets::Button& btnClose = del.addNew(new ui::widgets::Button(m_translator("Close"),     util::Key_Escape, m_root));
    buttonGroup.add(btnHelp);
    buttonGroup.add(del.addNew(new ui::Spacer()));
    buttonGroup.add(m_specButton);
    buttonGroup.add(btnIns);
    buttonGroup.add(m_gotoButton);
    buttonGroup.add(btnClose);
    win.add(buttonGroup);
    win.add(help);
    win.add(del.addNew(new ui::widgets::Quit(m_root, m_loop)));
    win.add(del.addNew(new ui::widgets::KeyForwarder(*this)));

    btnHelp.dispatchKeyTo(help);
    btnIns.dispatchKeyTo(*this);
    btnClose.sig_fire.addNewClosure(m_loop.makeStop(0));

    // Start up
    requestCurrent();

    win.pack();
    m_root.centerWidget(win);
    m_root.add(win);
    int result = m_loop.run();
    return result == 0 ? game::Reference() : m_reference;
}

/* Send request for current ship to proxy */
void
Dialog::requestCurrent()
{
    size_t side = 0;
    if (m_unitList.getCurrentShip(side)) {
        m_proxy.setSide(side, true);
    }
}

/* Callback: user scrolled */
void
Dialog::onListScroll()
{
    requestCurrent();
}

/* Callback: data for current ship */
void
Dialog::onSideUpdate(const VcrDatabaseProxy::SideInfo& info)
{
    m_nameWidget.setText(info.name);
    m_subtitleWidget.setText(info.subtitle);
    m_reference = info.reference;
    m_gotoButton.setState(ui::Widget::DisabledState, !m_reference.isSet());

    String_t tmp;
    int32_t hullNr;
    if (info.typeChoices.get(0, hullNr, tmp)) {
        m_hullNr = hullNr;
    } else {
        m_hullNr = 0;
    }
}

/* Callback: hull information for current ship */
void
Dialog::onHullUpdate(const VcrDatabaseProxy::HullInfo& info)
{
    m_image.setImage(info.imageName);
    if (const game::vcr::PlanetInfo* pi = info.planetInfo.get()) {
        m_objectInfo.setPlanetInfo(*pi);
    } else if (const game::vcr::ShipInfo* si = info.shipInfo.get()) {
        m_objectInfo.setShipInfo(*si);
    } else {
        m_objectInfo.clear();
    }

    m_shipQuery = info.shipQuery;
    m_specButton.setState(ui::Widget::DisabledState, !m_shipQuery.isValid());
}

/* "Go to" button */
void
Dialog::onGoTo()
{
    if (m_reference.isSet()) {
        m_loop.stop(1);
    }
}

/* "S" button */
void
Dialog::onHullSpecification()
{
    if (const game::ShipQuery* q = m_shipQuery.get()) {
        client::dialogs::showHullSpecification(*q, m_root, m_translator, m_gameSender);
    }
}

bool
Dialog::handleKey(util::Key_t key, int /*prefix*/)
{
    // ex WFlakVcrInfoWindow::handleEvent
    switch (key) {
     case util::Key_Insert:
        addToSimulation(true);
        return true;

     case util::Key_Insert + util::KeyMod_Shift:
        addToSimulation(false);
        return true;

     default:
        return false;
    }
}

void
Dialog::addToSimulation(bool after)
{
    if (m_hullNr != 0) {
        client::Downlink link(m_root, m_translator);
        client::dialogs::addToSimulation(link, m_proxy, m_hullNr, after, m_root, m_translator);
    }
}


/*
 *  Entry Point
 */

game::Reference
client::dialogs::doFlakVcrObjectInfoDialog(ui::Root& root,
                                           afl::string::Translator& tx,
                                           util::RequestSender<game::Session> gameSender,
                                           game::proxy::VcrDatabaseProxy& proxy,
                                           const game::vcr::BattleInfo& info,
                                           size_t initialUnit)
{
    Downlink link(root, tx);
    util::NumberFormatter fmt = game::proxy::ConfigurationProxy(gameSender).getNumberFormatter(link);

    client::widgets::HelpWidget help(root, tx, gameSender, "pcc2:vcrinfo");

    Dialog dlg(root, tx, gameSender, proxy, fmt);
    dlg.init(link, info, initialUnit);
    return dlg.run(help);
}
