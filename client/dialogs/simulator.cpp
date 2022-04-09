/**
  *  \file client/dialogs/simulator.cpp
  *  \brief Battle Simulator Main Dialog
  */

#include "client/dialogs/simulator.hpp"
#include "afl/string/format.hpp"
#include "client/dialogs/choosehull.hpp"
#include "client/dialogs/friendlycodedialog.hpp"
#include "client/dialogs/sessionfileselectiondialog.hpp"
#include "client/dialogs/simulationabilities.hpp"
#include "client/dialogs/simulationbasetorpedoes.hpp"
#include "client/dialogs/simulationconfiguration.hpp"
#include "client/dialogs/simulationflakratings.hpp"
#include "client/dialogs/simulationfleetcost.hpp"
#include "client/dialogs/simulationresult.hpp"
#include "client/downlink.hpp"
#include "client/si/outputstate.hpp"
#include "client/widgets/helpwidget.hpp"
#include "client/widgets/simulationlist.hpp"
#include "client/widgets/simulationobjectinfo.hpp"
#include "client/widgets/stoppablebusyindicator.hpp"
#include "game/proxy/simulationrunproxy.hpp"
#include "game/proxy/simulationsetupproxy.hpp"
#include "game/sim/ship.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/eventloop.hpp"
#include "ui/group.hpp"
#include "ui/layout/grid.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/rich/statictext.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/combobox.hpp"
#include "ui/widgets/decimalselector.hpp"
#include "ui/widgets/focusiterator.hpp"
#include "ui/widgets/inputline.hpp"
#include "ui/widgets/keydispatcher.hpp"
#include "ui/widgets/menuframe.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/scrollbarcontainer.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/widgets/stringlistbox.hpp"
#include "ui/window.hpp"

using afl::string::Format;
using client::Downlink;
using client::si::OutputState;
using client::si::RequestLink2;
using game::proxy::SimulationSetupProxy;
using game::sim::Configuration;
using game::sim::GameInterface;
using game::sim::Setup;
using game::Reference;
using ui::dialogs::MessageBox;

namespace {
    /*
     *  Limits
     *  (most limits provided by proxy)
     */

    const int MAX_DEFENSE = 600;
    const int MAX_MASS = 10000;

    const int MAX_SHIPS = 999;


    /*
     *  Operations on SimulationSetupProxy::Elements_t
     */

    struct CompareName {
        bool operator()(const SimulationSetupProxy::Element_t& a, const SimulationSetupProxy::Element_t& b) const
            { return a.second < b.second; }
    };

    void prependAggressivenessKeys(SimulationSetupProxy::Elements_t& elems)
    {
        for (size_t i = 0, n = elems.size(); i < n; ++i) {
            char ch;
            if (elems[i].first == game::sim::Ship::agg_Kill) {
                ch = '!';
            } else if (elems[i].first == game::sim::Ship::agg_Passive) {
                ch = '0';
            } else if (elems[i].first == game::sim::Ship::agg_NoFuel) {
                ch = 'Z';
            } else {
                ch = game::PlayerList::getCharacterFromPlayer(elems[i].first);
            }
            if (ch != '\0') {
                elems[i].second = Format("%c - %s", ch, elems[i].second);
            }
        }
    }

    void prependDigits(SimulationSetupProxy::Elements_t& elems)
    {
        for (size_t i = 0, n = elems.size(); i < n; ++i) {
            char ch = game::PlayerList::getCharacterFromPlayer(elems[i].first);
            if (ch != '\0') {
                elems[i].second = Format("%c - %s", ch, elems[i].second);
            }
        }
    }

    void sortAlphabetically(SimulationSetupProxy::Elements_t& elems, size_t startAt)
    {
        if (elems.size() > startAt) {
            std::sort(elems.begin() + startAt, elems.end(), CompareName());
        }
    }

    util::StringList convertList(const SimulationSetupProxy::Elements_t& elems)
    {
        util::StringList list;
        for (size_t i = 0, n = elems.size(); i < n; ++i) {
            list.add(elems[i].first, elems[i].second);
        }
        return list;
    }


    /*
     *  Canned Dialogs
     */

    bool doList(ui::Root& root, util::RequestSender<game::Session> /*gameSender*/, const SimulationSetupProxy::Elements_t& elems, int32_t& value, String_t title, String_t /*help*/, afl::string::Translator& tx)
    {
        ui::widgets::StringListbox list(root.provider(), root.colorScheme());
        for (size_t i = 0, n = elems.size(); i < n; ++i) {
            list.addItem(elems[i].first, elems[i].second);
        }
        list.setCurrentKey(value);

        ui::widgets::ScrollbarContainer cont(list, root);

        if (ui::widgets::doStandardDialog(title, String_t(), cont, true, root, tx)) {
            if (list.getCurrentKey(value)) {
                return true;
            }
        }
        return false;
    }

    bool doNumber(ui::Root& root, SimulationSetupProxy::Range_t range, int32_t& value, String_t title, String_t label, String_t /*help*/, afl::string::Translator& tx)
    {
        afl::base::Observable<int32_t> observableValue(value);
        ui::widgets::DecimalSelector sel(root, tx, observableValue, range.min(), range.max(), 10);
        if (ui::widgets::doStandardDialog(title, Format("%s [%d..%d]:", label, range.min(), range.max()), sel, true, root, tx)) {
            value = observableValue.get();
            return true;
        }
        return false;
    }

    /*
     *  Utilities
     */

    bool isEmptyOrUnit(const SimulationSetupProxy::Range_t& r)
    {
        return r.empty() || r.isUnit();
    }

    size_t getNumShips(const client::widgets::SimulationList& list)
    {
        size_t n = list.getNumItems();
        if (n > 0) {
            if (const client::widgets::SimulationList::ListItem_t* it = list.getItem(n-1)) {
                if (it->isPlanet) {
                    --n;
                }
            }
        }
        return n;
    }

    bool hasDisabledUnits(const client::widgets::SimulationList& list)
    {
        size_t n = list.getNumItems();
        if (n > 0) {
            if (const client::widgets::SimulationList::ListItem_t* it = list.getItem(n-1)) {
                if (it->disabled) {
                    return true;
                }
            }
        }
        return false;
    }

    bool runFirstSimulation(game::proxy::SimulationRunProxy& runner, ui::Root& root, afl::string::Translator& tx)
    {
        // ex WSimResultWindow::runFirstSimulation
        client::widgets::StoppableBusyIndicator stopper(root, tx);
        afl::base::SignalConnection conn1(stopper.sig_stop.add(&runner, &game::proxy::SimulationRunProxy::stop));
        afl::base::SignalConnection conn2(runner.sig_stop.add(&stopper, &client::widgets::StoppableBusyIndicator::stop));

        runner.runFinite(1);
        return stopper.run();
    }

    /*
     *  Dialog
     */

    class SimulatorDialog : public client::si::Control {
     public:
        SimulatorDialog(Control& parentControl, util::RequestSender<game::Session> gameSender, SimulationSetupProxy& proxy, ui::Root& root, OutputState& outputState, afl::string::Translator& tx);

        void run();
        void setListContent(const SimulationSetupProxy::ListItems_t& list);

     private:
        util::RequestSender<game::Session> m_gameSender;
        SimulationSetupProxy& m_proxy;
        ui::Root& m_root;
        OutputState& m_outputState;
        afl::string::Translator& m_translator;
        ui::widgets::KeyDispatcher m_keyDispatcher;
        client::widgets::SimulationList m_list;
        client::widgets::SimulationObjectInfo m_objectInfo;
        ui::widgets::Button m_runButton;
        ui::EventLoop m_loop;

        SimulationSetupProxy::Slot_t m_currentSlot;
        SimulationSetupProxy::ObjectInfo m_currentObject;

        // Control:
        virtual void handleStateChange(RequestLink2 link, OutputState::Target target);
        virtual void handleEndDialog(RequestLink2 link, int code);
        virtual void handlePopupConsole(RequestLink2 link);
        virtual void handleScanKeyboardMode(RequestLink2 link);
        virtual void handleSetView(RequestLink2 link, String_t name, bool withKeymap);
        virtual void handleUseKeymap(client::si::RequestLink2 link, String_t name, int prefix);
        virtual void handleOverlayMessage(RequestLink2 link, String_t text);
        virtual game::interface::ContextProvider* createContextProvider();

        // Event handlers
        void onListChange(const SimulationSetupProxy::ListItems_t& list);
        void onObjectChange(SimulationSetupProxy::Slot_t slot, const SimulationSetupProxy::ObjectInfo& info);
        void onListSelection();

        // Utilities
        int getReplicationLimit();
        bool isAtObject() const;
        bool isAtShip() const;
        bool isAtPlanet() const;
        bool isAtBase() const;
        bool canSwapUp() const;
        bool canSwapDown() const;

        // User entry points
        void onAddShip();
        void onAddPlanet();
        void onDelete();
        void onDeleteAll();
        void onLoad();
        void onSave();
        void onRun();
        void onToggleDisabled();
        void onReplicate();
        void onEditPrimary();
        void onEditSecondary();
        void onEditAggressivenessAmmo();
        void onEditDamageDefense();
        void onEditEngine();
        void onEditFriendlyCode();
        void onSetSequentialFriendlyCode();
        void onEditBaseFighters();
        void onEditBaseBeamLevel();
        void onEditCrew();
        void onEditId();
        void onEditFlakRatings();
        void onEditExperienceLevel();
        void onEditMass();
        void onEditName();
        void onEditOwner();
        void onEditPopulation();
        void onToggleRandomFriendlyCode();
        void onEditShieldBaseDefense();
        void onEditTypeBaseTorpedoLevel();
        void onToggleCloak();
        void onEditIntercept();
        void onEditAbilities();
        void onFleetCostSummary();
        void onEditConfiguration();
        void onUpdateThis();
        void onWriteBackThis();
        void onUpdateAll();
        void onWriteBackAll();
        void onSwapUp();
        void onSwapDown();
        void onContextMenu(gfx::Point pt);
        void onGoToShip();
        void onGoToPlanet();
        void onGoToBase();
        void onGoToReference(Reference ref);

        // Extra dialogs
        void editType(bool afterAdd, SimulationSetupProxy::Slot_t slot, int oldValue);
        void showLimitWarning();
    };
}

SimulatorDialog::SimulatorDialog(Control& parentControl, util::RequestSender<game::Session> gameSender, SimulationSetupProxy& proxy, ui::Root& root, OutputState& outputState, afl::string::Translator& tx)
    : Control(parentControl.interface()),
      m_gameSender(gameSender),
      m_proxy(proxy),
      m_root(root),
      m_outputState(outputState),
      m_translator(tx),
      m_keyDispatcher(),
      m_list(root, tx),
      m_objectInfo(root, m_keyDispatcher, tx),
      m_runButton(tx("Simulate!"), util::Key_Return, root),
      m_loop(root),
      m_currentSlot(-1U),
      m_currentObject()
{
    m_proxy.sig_listChange.add(this, &SimulatorDialog::onListChange);
    m_proxy.sig_objectChange.add(this, &SimulatorDialog::onObjectChange);
    m_list.sig_change.add(this, &SimulatorDialog::onListSelection);
    m_list.sig_menuRequest.add(this, &SimulatorDialog::onContextMenu);
    m_list.setFlag(ui::widgets::AbstractListbox::KeyboardMenu, true);
    m_runButton.sig_fire.add(this, &SimulatorDialog::onRun);
}

void
SimulatorDialog::run()
{
    afl::base::Deleter del;

    ui::Window& win = del.addNew(new ui::Window(m_translator("Battle Simulator"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));

    ui::Group& g1 = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    g1.add(del.addNew(new ui::widgets::ScrollbarContainer(m_list, m_root)));
    g1.add(m_objectInfo);
    win.add(g1);

    ui::Group& g2 = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    ui::widgets::Button& btnAddShip = del.addNew(new ui::widgets::Button(m_translator("Ins - Add Ship"), util::Key_Insert, m_root));
    ui::widgets::Button& btnAddPlanet = del.addNew(new ui::widgets::Button(m_translator("P - Add Planet"), 'p', m_root));
    ui::widgets::Button& btnDelete = del.addNew(new ui::widgets::Button(m_translator("Delete"), util::Key_Delete, m_root));
    ui::widgets::Button& btnLoad = del.addNew(new ui::widgets::Button(m_translator("Ctrl-R - Load"), 'r' + util::KeyMod_Ctrl, m_root));
    ui::widgets::Button& btnSave = del.addNew(new ui::widgets::Button(m_translator("Ctrl-S - Save"), 's' + util::KeyMod_Ctrl, m_root));
    g2.add(btnAddShip);
    g2.add(btnAddPlanet);
    g2.add(btnDelete);
    g2.add(btnLoad);
    g2.add(btnSave);
    g2.add(del.addNew(new ui::Spacer()));
    win.add(g2);

    ui::Group& g3 = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    ui::widgets::Button& btnClose = del.addNew(new ui::widgets::Button(m_translator("Close"), util::Key_Escape, m_root));
    ui::widgets::Button& btnHelp = del.addNew(new ui::widgets::Button(m_translator("Help"), 'h', m_root));
    g3.add(m_runButton);
    g3.add(btnClose);
    g3.add(del.addNew(new ui::Spacer()));
    g3.add(btnHelp);
    win.add(g3);

    client::widgets::HelpWidget& help = del.addNew(new client::widgets::HelpWidget(m_root, m_translator, m_gameSender, "pcc2:bsim"));
    win.add(help);
    win.add(m_keyDispatcher);
    win.add(del.addNew(new ui::widgets::Quit(m_root, m_loop)));

    btnAddShip.dispatchKeyTo(m_keyDispatcher);
    btnAddPlanet.sig_fire.add(this, &SimulatorDialog::onAddPlanet);
    btnDelete.dispatchKeyTo(m_keyDispatcher);
    btnLoad.sig_fire.add(this, &SimulatorDialog::onLoad);
    btnSave.sig_fire.add(this, &SimulatorDialog::onSave);
    btnClose.sig_fire.addNewClosure(m_loop.makeStop(0));
    btnHelp.dispatchKeyTo(help);

    m_keyDispatcher.add('.', this, &SimulatorDialog::onToggleDisabled);
    m_keyDispatcher.add('*', this, &SimulatorDialog::onReplicate);
    m_keyDispatcher.add('1', this, &SimulatorDialog::onEditPrimary);
    m_keyDispatcher.add('2', this, &SimulatorDialog::onEditSecondary);
    m_keyDispatcher.add('a', this, &SimulatorDialog::onEditAggressivenessAmmo);
    m_keyDispatcher.add('b', this, &SimulatorDialog::onEditBaseBeamLevel);
    m_keyDispatcher.add('c', this, &SimulatorDialog::onEditCrew);
    m_keyDispatcher.add('d', this, &SimulatorDialog::onEditDamageDefense);
    m_keyDispatcher.add('e', this, &SimulatorDialog::onEditEngine);
    m_keyDispatcher.add('f', this, &SimulatorDialog::onEditFriendlyCode);
    m_keyDispatcher.add('F', this, &SimulatorDialog::onSetSequentialFriendlyCode);
    m_keyDispatcher.add('g', this, &SimulatorDialog::onEditBaseFighters);
    m_keyDispatcher.add('i', this, &SimulatorDialog::onEditId);
    m_keyDispatcher.add('k', this, &SimulatorDialog::onEditFlakRatings);
    m_keyDispatcher.add('l', this, &SimulatorDialog::onEditExperienceLevel);
    m_keyDispatcher.add('m', this, &SimulatorDialog::onEditMass);
    m_keyDispatcher.add('n', this, &SimulatorDialog::onEditName);
    m_keyDispatcher.add('o', this, &SimulatorDialog::onEditOwner);
    m_keyDispatcher.add('p', this, &SimulatorDialog::onEditPopulation);
    m_keyDispatcher.add('r', this, &SimulatorDialog::onToggleRandomFriendlyCode);
    m_keyDispatcher.add('s', this, &SimulatorDialog::onEditShieldBaseDefense);
    m_keyDispatcher.add('t', this, &SimulatorDialog::onEditTypeBaseTorpedoLevel);
    m_keyDispatcher.add('u', this, &SimulatorDialog::onUpdateThis);
    m_keyDispatcher.add('v', this, &SimulatorDialog::onToggleCloak);
    m_keyDispatcher.add('w', this, &SimulatorDialog::onWriteBackThis);
    m_keyDispatcher.add('x', this, &SimulatorDialog::onEditIntercept);
    m_keyDispatcher.add('y', this, &SimulatorDialog::onEditAbilities);
    m_keyDispatcher.add('c'  | util::KeyMod_Ctrl, this, &SimulatorDialog::onFleetCostSummary);
    m_keyDispatcher.add('o'  | util::KeyMod_Ctrl, this, &SimulatorDialog::onEditConfiguration);
    m_keyDispatcher.add('u'  | util::KeyMod_Ctrl, this, &SimulatorDialog::onUpdateAll);
    m_keyDispatcher.add('w'  | util::KeyMod_Ctrl, this, &SimulatorDialog::onWriteBackAll);
    m_keyDispatcher.add(util::Key_Up     | util::KeyMod_Ctrl, this, &SimulatorDialog::onSwapUp);
    m_keyDispatcher.add(util::Key_Down   | util::KeyMod_Ctrl, this, &SimulatorDialog::onSwapDown);
    m_keyDispatcher.add(util::Key_Insert,                     this, &SimulatorDialog::onAddShip);
    m_keyDispatcher.add(util::Key_Insert | util::KeyMod_Ctrl, this, &SimulatorDialog::onReplicate);
    m_keyDispatcher.add(util::Key_Delete,                     this, &SimulatorDialog::onDelete);
    m_keyDispatcher.add(util::Key_Delete | util::KeyMod_Ctrl, this, &SimulatorDialog::onDeleteAll);
    m_keyDispatcher.add(util::Key_F1,                         this, &SimulatorDialog::onGoToShip);
    m_keyDispatcher.add(util::Key_F2,                         this, &SimulatorDialog::onGoToPlanet);
    m_keyDispatcher.add(util::Key_F3,                         this, &SimulatorDialog::onGoToBase);

    win.pack();
    m_root.centerWidget(win);
    m_root.add(win);
    m_loop.run();
}

void
SimulatorDialog::setListContent(const SimulationSetupProxy::ListItems_t& list)
{
    m_list.setContent(list);
    m_runButton.setState(ui::Widget::DisabledState, m_list.getNumItems() < 2);
    if (!list.empty()) {
        m_list.setCurrentItem(0);
        m_proxy.setSlot(0);
    } else {
        m_objectInfo.showIntroPage();
    }
}

void
SimulatorDialog::handleStateChange(RequestLink2 link, OutputState::Target target)
{
    dialogHandleStateChange(link, target, m_outputState, m_loop, 1);
}

void
SimulatorDialog::handleEndDialog(RequestLink2 link, int /*code*/)
{
    interface().continueProcess(link);
}

void
SimulatorDialog::handlePopupConsole(RequestLink2 link)
{
    defaultHandlePopupConsole(link);
}

void
SimulatorDialog::handleScanKeyboardMode(RequestLink2 link)
{
    defaultHandleScanKeyboardMode(link);
}

void
SimulatorDialog::handleSetView(RequestLink2 link, String_t name, bool withKeymap)
{
    defaultHandleSetView(link, name, withKeymap);
}

void
SimulatorDialog::handleUseKeymap(client::si::RequestLink2 link, String_t name, int prefix)
{
    defaultHandleUseKeymap(link, name, prefix);
}

void
SimulatorDialog::handleOverlayMessage(RequestLink2 link, String_t text)
{
    defaultHandleOverlayMessage(link, text);
}

game::interface::ContextProvider*
SimulatorDialog::createContextProvider()
{
    return 0;
}

void
SimulatorDialog::onListChange(const SimulationSetupProxy::ListItems_t& list)
{
    m_list.setContent(list);
    m_runButton.setState(ui::Widget::DisabledState, m_list.getNumItems() < 2);
    if (list.empty()) {
        m_currentSlot = -1U;
        m_objectInfo.showIntroPage();
    }
}

void
SimulatorDialog::onObjectChange(SimulationSetupProxy::Slot_t slot, const SimulationSetupProxy::ObjectInfo& info)
{
    if (slot == m_list.getCurrentItem()) {
        m_objectInfo.setContent(info);
        m_currentObject = info;
        m_currentSlot = slot;
    }
}

void
SimulatorDialog::onListSelection()
{
    if (m_list.getNumItems() > 0) {
        m_proxy.setSlot(m_list.getCurrentItem());
    }
}

int
SimulatorDialog::getReplicationLimit()
{
    size_t have = getNumShips(m_list);
    if (have >= MAX_SHIPS) {
        return 0;
    } else {
        return int(MAX_SHIPS - have);
    }
}

bool
SimulatorDialog::isAtObject() const
{
    // We may not examine m_currentObject if we don't currently have current data.
    // (Note that this may mean that fast input gets lost. Sorry.)
    return m_list.getCurrentItem() == m_currentSlot;
}

bool
SimulatorDialog::isAtShip() const
{
    return isAtObject() && !m_currentObject.isPlanet;
}

bool
SimulatorDialog::isAtPlanet() const
{
    return isAtObject() && m_currentObject.isPlanet;
}

bool
SimulatorDialog::isAtBase() const
{
    return isAtPlanet() && m_currentObject.hasBase;
}

bool
SimulatorDialog::canSwapUp() const
{
    size_t n = m_list.getCurrentItem();
    return n > 0 && n < getNumShips(m_list);
}

bool
SimulatorDialog::canSwapDown() const
{
    size_t n = m_list.getCurrentItem();
    size_t max = getNumShips(m_list);
    return n+1 < max;
}

void
SimulatorDialog::onAddShip()
{
    // ex WSimListWithHandler::addShip, CSimListbox.AddShip
    if (getReplicationLimit() > 0) {
        Downlink link(m_root, m_translator);
        SimulationSetupProxy::Slot_t slot = m_proxy.addShip(link, m_list.getCurrentItem(), 1);
        m_list.setCurrentItem(slot);

        // At this point in time, we will have received the new list, but not the new object data.
        // Instead of taking a guess, explicitly query the data (at the cost of one round-trip).
        SimulationSetupProxy::ObjectInfo info;
        if (m_proxy.getObject(link, slot, info)) {
            editType(true, slot, info.hullType.first);
        }
    } else {
        showLimitWarning();
    }
}

void
SimulatorDialog::onAddPlanet()
{
    Downlink link(m_root, m_translator);
    SimulationSetupProxy::Slot_t slot = m_proxy.addPlanet(link);
    m_list.setCurrentItem(slot);
}

void
SimulatorDialog::onDelete()
{
    // ex WSimListWithHandler::deleteShip, CSimListbox.DelShip
    // Just remove the object.
    // Proxy will respond with a list update (onListSelection, emitted upon any change to the list specs, including its length).
    // This will re-invoke setSlot() and therefore produce an update.
    size_t current = m_list.getCurrentItem();
    size_t total = m_list.getNumItems();
    if (current < total) {
        m_proxy.removeObject(current);
    }
}

void
SimulatorDialog::onDeleteAll()
{
    if (m_list.getNumItems() != 0) {
        if (MessageBox(m_translator("Clear this simulation arrangement?"),
                       m_translator("Battle Simulator"),
                       m_root).doYesNoDialog(m_translator))
        {
            m_proxy.clear();
        }
    }
}

void
SimulatorDialog::onLoad()
{
    // WSimScreen::loadFile, ccsim.pas:LoadBattleSetup
    Downlink link(m_root, m_translator);
    client::widgets::HelpWidget help(m_root, m_translator, m_gameSender, "pcc2:bsim");
    client::dialogs::SessionFileSelectionDialog dlg(m_root, m_translator, m_gameSender, m_translator("Load Simulation"));
    dlg.setPattern("*.ccb");
    dlg.setDefaultExtension("ccb");
    dlg.setHelpWidget(help);
    if (!dlg.runDefault(link)) {
        return;
    }

    String_t errorMessage;
    bool ok = m_proxy.load(link, dlg.getResult(), errorMessage);
    if (!ok) {
        MessageBox(
            Format(m_translator("Unable to load simulation.\n%s"), errorMessage),
            m_translator("Load Simulation"),
            m_root).doOkDialog(m_translator);
    }
    // FIXME: check ship list
    // if (!sim_state.isMatchingShipList())
    //     messageBox(_("This simulation seems to have been set up with a different ship list. "
    //                  "To regenerate the intended results, restart the Simulator from the correct "
    //                  "game directory."),
    //                _("Load Simulation"));
}

void
SimulatorDialog::onSave()
{
    // ex WSimScreen::saveFile, ccsim.pas:SaveBattleSetup
    Downlink link(m_root, m_translator);
    client::widgets::HelpWidget help(m_root, m_translator, m_gameSender, "pcc2:bsim");
    client::dialogs::SessionFileSelectionDialog dlg(m_root, m_translator, m_gameSender, m_translator("Save Simulation"));
    dlg.setPattern("*.ccb");
    dlg.setDefaultExtension("ccb");
    dlg.setHelpWidget(help);
    if (!dlg.runDefault(link)) {
        return;
    }

    String_t errorMessage;
    bool ok = m_proxy.save(link, dlg.getResult(), errorMessage);
    if (!ok) {
        MessageBox(
            Format(m_translator("Unable to save simulation.\n%s"), errorMessage),
            m_translator("Save Simulation"),
            m_root).doOkDialog(m_translator);
    }
}

void
SimulatorDialog::onRun()
{
    // ex WSimScreen::runSimulation
    // Do we allow to run the simulation?
    if (m_list.getNumItems() < 2) {
        return;
    }

    // First simulation
    game::proxy::SimulationRunProxy runner(m_proxy, m_root.engine().dispatcher());
    if (!runFirstSimulation(runner, m_root, m_translator)) {
        return;
    }

    if (runner.getNumBattles() == 0) {
        // No results. Figure out why and give a hopefully helpful hint.
        if (hasDisabledUnits(m_list)) {
            MessageBox(m_translator("There are no fights in this arrangement. You have disabled "
                                    "some ships, try re-enabling them using [.]."),
                       m_translator("Simulator"),
                       m_root).doOkDialog(m_translator);
        } else {
            MessageBox(m_translator("There are no fights in this arrangement. Did you set all "
                                    "owners and aggression settings correctly?"),
                       m_translator("Simulator"),
                       m_root).doOkDialog(m_translator);
        }
    } else {
        // OK
        using client::dialogs::SimulationResultStatus;
        SimulationResultStatus st = client::dialogs::doBattleSimulationResults(m_proxy, runner, m_root, m_translator, m_gameSender);

        switch (st.status) {
         case SimulationResultStatus::Nothing:
            break;

         case SimulationResultStatus::ScrollToSlot:
            m_list.setCurrentItem(st.slot);
            break;

         case SimulationResultStatus::GoToReference:
            MessageBox(st.reference.toString(m_translator), "ref", m_root).doOkDialog(m_translator);
            onGoToReference(st.reference);
            break;
        }
    }
}

void
SimulatorDialog::onToggleDisabled()
{
    // WSimListWithHandler::toggleEnabled
    m_proxy.toggleDisabled(m_list.getCurrentItem());
}

void
SimulatorDialog::onReplicate()
{
    // ex WSimListWithHandler::replicateShip, CSimListboxWithHandler.AskReplicateShip, CSimListbox.ReplicateShip
    if (isAtShip()) {
        int limit = getReplicationLimit();
        if (limit == 0) {
            showLimitWarning();
        } else {
            int32_t count = 0;
            if (doNumber(m_root, SimulationSetupProxy::Range_t(0, limit), count, m_translator("Replicate Ship"), m_translator("Number"), "pcc2:bsim", m_translator)) {
                Downlink link(m_root, m_translator);
                SimulationSetupProxy::Slot_t slot = m_proxy.addShip(link, m_list.getCurrentItem(), count);
                m_list.setCurrentItem(slot);
            }
        }
    }
}

void
SimulatorDialog::onEditPrimary()
{
    // ex WSimListWithHandler::editPrimaryWeapon, ccsim.pas:SelectPrimary
    if (isAtShip() && m_currentObject.allowPrimaryWeapons) {
        // Determine available choices
        Downlink link(m_root, m_translator);
        SimulationSetupProxy::PrimaryChoices info;
        m_proxy.getPrimaryChoices(link, m_currentSlot, info);
        prependDigits(info.beamTypes);
        if (info.beamTypes.empty()) {
            return;
        }

        // Values
        afl::base::Observable<int32_t> type, count;
        if (m_currentObject.numBeams == 0) {
            type.set(info.beamTypes.back().first);
        } else {
            type.set(m_currentObject.beamType.first);
        }
        count.set(m_currentObject.numBeams);

        // Combo box for type
        afl::base::Deleter del;
        ui::Widget& typedCombo = del.addNew(new ui::widgets::ComboBox(m_root, type, info.beamTypes.front().first, info.beamTypes.back().first, convertList(info.beamTypes)))
            .addButtons(del);

        // Decimal selector for count
        ui::Widget& countCombo = del.addNew(new ui::widgets::DecimalSelector(m_root, m_translator, count, info.numBeams.min(), info.numBeams.max(), 1))
            .addButtons(del, m_root);

        // Window
        ui::EventLoop loop(m_root);
        ui::Window& win = del.addNew(new ui::Window(m_translator("Set Primary Weapon"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));

        ui::Group& controls = del.addNew(new ui::Group(del.addNew(new ui::layout::Grid(2))));
        controls.add(del.addNew(new ui::widgets::StaticText(m_translator("Type:"), util::SkinColor::Static, "+", m_root.provider())));
        controls.add(typedCombo);
        controls.add(del.addNew(new ui::widgets::StaticText(m_translator("Count:"), util::SkinColor::Static, "+", m_root.provider())));
        controls.add(countCombo);

        ui::widgets::StandardDialogButtons& btn = del.addNew(new ui::widgets::StandardDialogButtons(m_root, m_translator));
        btn.addStop(loop);

        ui::widgets::FocusIterator& it = del.addNew(new ui::widgets::FocusIterator(ui::widgets::FocusIterator::Vertical));
        it.add(typedCombo);
        it.add(countCombo);

        win.add(controls);
        win.add(btn);
        win.add(it);
        win.add(del.addNew(new ui::widgets::Quit(m_root, loop)));
        win.pack();
        typedCombo.requestFocus();

        m_root.centerWidget(win);
        m_root.add(win);
        bool ok = loop.run();

        if (ok) {
            if (type.get() != 0 && count.get() != 0) {
                m_proxy.setNumBeams(m_currentSlot, count.get());
                m_proxy.setBeamType(m_currentSlot, type.get());
            } else {
                m_proxy.setNumBeams(m_currentSlot, 0);
                m_proxy.setBeamType(m_currentSlot, 0);
            }
        }
    }
}

void
SimulatorDialog::onEditSecondary()
{
    // ex WSimListWithHandler::editSecondaryWeapon, ccsim.pas:SelectSecondary
    if (isAtShip() && m_currentObject.allowSecondaryWeapons) {
        // Determine available choices
        Downlink link(m_root, m_translator);
        SimulationSetupProxy::SecondaryChoices info;
        m_proxy.getSecondaryChoices(link, m_currentSlot, info);

        // Determine kind of dialog
        if (!isEmptyOrUnit(info.numLaunchers)) {
            // Custom ship or torper, we can choose type and ammo. If it's a custom ship, also offer fighters.
            // Torpedo types must be valid
            prependDigits(info.torpedoTypes);
            if (info.torpedoTypes.empty()) {
                return;
            }

            // Do we offer fighters?
            const int32_t FIGHTERS = -99;
            if (!isEmptyOrUnit(info.numBays)) {
                info.torpedoTypes.push_back(SimulationSetupProxy::Element_t(FIGHTERS, m_translator("Fighters")));
            }

            // Modifyables
            afl::base::Observable<int32_t> type, count, ammo;
            if (m_currentObject.numBays != 0 && !isEmptyOrUnit(info.numBays)) {
                type.set(FIGHTERS);
                count.set(m_currentObject.numBays);
            } else if (m_currentObject.numLaunchers != 0) {
                type.set(m_currentObject.torpedoType.first);
                count.set(m_currentObject.numLaunchers);
            } else {
                type.set(info.torpedoTypes.back().first);
                count.set(0);
            }
            ammo.set(m_currentObject.ammo);

            // Combo box for type
            afl::base::Deleter del;
            ui::Widget& typedCombo = del.addNew(new ui::widgets::ComboBox(m_root, type, info.torpedoTypes.front().first, info.torpedoTypes.back().first, convertList(info.torpedoTypes)))
                .addButtons(del);

            // Decimal selector for count
            ui::Widget& countCombo = del.addNew(new ui::widgets::DecimalSelector(m_root, m_translator, count, info.numLaunchers.min(), info.numLaunchers.max(), 1))
                .addButtons(del, m_root);

            // Decimal selector for ammo
            ui::Widget& ammoCombo = del.addNew(new ui::widgets::DecimalSelector(m_root, m_translator, ammo, info.ammo.min(), info.ammo.max(), 10))
                .addButtons(del, m_root);

            // Window
            ui::EventLoop loop(m_root);
            ui::Window& win = del.addNew(new ui::Window(m_translator("Set Secondary Weapon"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));

            ui::Group& controls = del.addNew(new ui::Group(del.addNew(new ui::layout::Grid(2))));
            controls.add(del.addNew(new ui::widgets::StaticText(m_translator("Type:"), util::SkinColor::Static, "+", m_root.provider())));
            controls.add(typedCombo);
            controls.add(del.addNew(new ui::widgets::StaticText(m_translator("Count:"), util::SkinColor::Static, "+", m_root.provider())));
            controls.add(countCombo);
            controls.add(del.addNew(new ui::widgets::StaticText(m_translator("Ammo:"), util::SkinColor::Static, "+", m_root.provider())));
            controls.add(ammoCombo);

            ui::widgets::StandardDialogButtons& btn = del.addNew(new ui::widgets::StandardDialogButtons(m_root, m_translator));
            btn.addStop(loop);

            ui::widgets::FocusIterator& it = del.addNew(new ui::widgets::FocusIterator(ui::widgets::FocusIterator::Vertical));
            it.add(typedCombo);
            it.add(countCombo);
            it.add(ammoCombo);

            win.add(controls);
            win.add(btn);
            win.add(it);
            win.add(del.addNew(new ui::widgets::Quit(m_root, loop)));
            win.pack();
            typedCombo.requestFocus();

            m_root.centerWidget(win);
            m_root.add(win);
            bool ok = loop.run();

            if (ok) {
                if (type.get() != 0 && count.get() != 0) {
                    if (type.get() == FIGHTERS) {
                        m_proxy.setNumBays(m_currentSlot, count.get());
                        m_proxy.setTorpedoType(m_currentSlot, 0);
                        m_proxy.setNumLaunchers(m_currentSlot, 0);
                    } else {
                        m_proxy.setNumBays(m_currentSlot, 0);
                        m_proxy.setTorpedoType(m_currentSlot, type.get());
                        m_proxy.setNumLaunchers(m_currentSlot, count.get());
                    }
                } else {
                    m_proxy.setNumBays(m_currentSlot, 0);
                    m_proxy.setTorpedoType(m_currentSlot, 0);
                    m_proxy.setNumLaunchers(m_currentSlot, 0);
                }
                m_proxy.setAmmo(m_currentSlot, ammo.get());
            }
        } else if (info.numBays.isUnit() && info.numBays.max() > 0) {
            // Carrier, we can edit ammo (=number of fighters)
            int32_t ammo = m_currentObject.ammo;
            if (doNumber(m_root, info.ammo, ammo, m_translator("Set Number of Fighters"), m_translator("Fighters"), "pcc2:bsim", m_translator)) {
                m_proxy.setNumBays(m_currentSlot, info.numBays.max());
                m_proxy.setTorpedoType(m_currentSlot, 0);
                m_proxy.setNumLaunchers(m_currentSlot, 0);
                m_proxy.setAmmo(m_currentSlot, ammo);
            }
        } else {
            // Nothing
        }
    }
}

void
SimulatorDialog::onEditAggressivenessAmmo()
{
    if (isAtShip()) {
        // ex WSimListWithHandler::editAggressiveness (part)
        Downlink link(m_root, m_translator);
        SimulationSetupProxy::Elements_t elems;
        m_proxy.getAggressivenessChoices(link, elems);
        prependAggressivenessKeys(elems);

        int32_t value = m_currentObject.aggressiveness.first;
        if (doList(m_root, m_gameSender, elems, value, m_translator("Set Aggressiveness"), "pcc2:bsim", m_translator)) {
            m_proxy.setAggressiveness(m_currentSlot, value);
        }
    } else if (isAtBase()) {
        // ex WSimPlanetEditor::editBaseTorps
        Downlink link(m_root, m_translator);
        SimulationSetupProxy::Elements_t list;
        m_proxy.getNumBaseTorpedoes(link, m_currentSlot, list);
        if (client::dialogs::editSimulationBaseTorpedoes(m_root, m_gameSender, m_currentObject.baseTorpedoTech-1, list, m_translator)) {
            m_proxy.setNumBaseTorpedoes(m_currentSlot, list);
        }
    }
}

void
SimulatorDialog::onEditDamageDefense()
{
    if (isAtShip()) {
        // ex WSimListWithHandler::editDamage
        Downlink link(m_root, m_translator);
        SimulationSetupProxy::Range_t range = m_proxy.getDamageRange(link, m_currentSlot);
        int32_t value = m_currentObject.damage;
        if (doNumber(m_root, range, value, m_translator("Set Damage Level"), m_translator("Damage"), "pcc2:bsim", m_translator)) {
            m_proxy.setDamage(m_currentSlot, value);
        }
    } else if (isAtPlanet()) {
        // ex WSimPlanetEditor::editDefense
        int32_t value = m_currentObject.defense;
        if (doNumber(m_root, SimulationSetupProxy::Range_t(0, MAX_DEFENSE), value, m_translator("Set Defense"), m_translator("Defense"), "pcc2:simplanet", m_translator)) {
            m_proxy.setDefense(m_currentSlot, value);
        }
    }
}

void
SimulatorDialog::onEditEngine()
{
    // ex WSimListWithHandler::editEngine
    if (isAtShip()) {
        Downlink link(m_root, m_translator);
        SimulationSetupProxy::Elements_t elems;
        m_proxy.getEngineTypeChoices(link, elems);
        prependDigits(elems);

        int32_t value = m_currentObject.engineType.first;
        if (doList(m_root, m_gameSender, elems, value, m_translator("Set Engine Type"), "pcc2:bsim", m_translator)) {
            m_proxy.setEngineType(m_currentSlot, value);
        }
    }
}

void
SimulatorDialog::onEditFriendlyCode()
{
    // ex WSimListWithHandler::editFCode, WSimPlanetEditor::editFCode
    if (isAtObject()) {
        Downlink link(m_root, m_translator);
        game::spec::FriendlyCodeList::Infos_t list;
        m_proxy.getFriendlyCodeChoices(link, m_currentSlot, list);

        client::dialogs::FriendlyCodeDialog dlg(m_root, m_translator, m_translator("Set Friendly Code"), list, m_gameSender);
        dlg.setFriendlyCode(m_currentObject.friendlyCode);
        if (dlg.run()) {
            m_proxy.setFriendlyCode(m_currentSlot, dlg.getFriendlyCode());
        }
    }
}

void
SimulatorDialog::onSetSequentialFriendlyCode()
{
    // ex WSimListWithHandler::incrFCode (part)
    if (m_currentSlot == m_list.getCurrentItem() && m_currentSlot+1 < m_list.getNumItems()) {
        m_list.setCurrentItem(m_currentSlot+1);
        m_proxy.setSequentialFriendlyCode(m_currentSlot+1);
    }
}

void
SimulatorDialog::onEditBaseFighters()
{
    // ex WSimPlanetEditor::editBaseFighters
    if (isAtBase()) {
        Downlink link(m_root, m_translator);
        SimulationSetupProxy::Range_t range = m_proxy.getNumBaseFightersRange(link, m_currentSlot);
        int32_t value = m_currentObject.numBaseFighters;
        if (doNumber(m_root, range, value, m_translator("Set Base Fighters"), m_translator("Fighters"), "pcc2:bsim", m_translator)) {
            m_proxy.setNumBaseFighters(m_currentSlot, value);
        }
    }
}

void
SimulatorDialog::onEditBaseBeamLevel()
{
    // ex WSimPlanetEditor::editBaseBeamTech
    if (isAtPlanet()) {
        Downlink link(m_root, m_translator);
        SimulationSetupProxy::Elements_t elems;
        m_proxy.getBaseBeamLevelChoices(link, elems);
        prependDigits(elems);

        int32_t value = m_currentObject.baseBeamTech;
        if (doList(m_root, m_gameSender, elems, value, m_translator("Set Starbase Beam Tech"), "pcc2:simplanet", m_translator)) {
            m_proxy.setBaseBeamTech(m_currentSlot, value);
        }
    }
}

void
SimulatorDialog::onEditCrew()
{
    // ex WSimListWithHandler::editCrew
    if (isAtShip()) {
        Downlink link(m_root, m_translator);
        SimulationSetupProxy::Range_t range = m_proxy.getCrewRange(link, m_currentSlot);
        int32_t value = m_currentObject.crew;
        if (doNumber(m_root, range, value, m_translator("Set Crew"), m_translator("Crew"), "pcc2:bsim", m_translator)) {
            m_proxy.setCrew(m_currentSlot, value);
        }
    }
}

void
SimulatorDialog::onEditId()
{
    // ex WSimPlanetEditor::editId, WSimListWithHandler::editId, CSimListbox.SetId
    if (isAtObject()) {
        Downlink link(m_root, m_translator);
        SimulationSetupProxy::Range_t range = m_proxy.getIdRange(link, m_currentSlot);
        int32_t value = m_currentObject.id;

        while (doNumber(m_root, range, value, m_translator("Set Id"), m_translator("Id"), "pcc2:bsim", m_translator)) {
            // Dupe check
            if (!m_proxy.isDuplicateId(link, m_currentSlot, value)) {
                m_proxy.setId(m_currentSlot, value);
                break;
            }

            MessageBox(m_translator("This Id number is already in use in this simulation setup. Please choose another one."),
                       m_translator("Battle Simulator"),
                       m_root).doOkDialog(m_translator);
        }
    }
}

void
SimulatorDialog::onEditFlakRatings()
{
    if (isAtShip()) {
        // Set up
        client::dialogs::SimulationFlakRatings values;
        values.defaultFlakRating       = m_currentObject.defaultFlakRating;
        values.defaultFlakCompensation = m_currentObject.defaultFlakCompensation;
        if ((m_currentObject.flags & game::sim::Object::fl_RatingOverride) != 0) {
            values.useDefaults      = false;
            values.flakRating       = m_currentObject.flakRatingOverride;
            values.flakCompensation = m_currentObject.flakCompensationOverride;
        } else {
            values.useDefaults      = true;
            values.flakRating       = values.defaultFlakRating;
            values.flakCompensation = values.defaultFlakCompensation;
        }

        // Edit
        if (client::dialogs::editSimulationFlakRatings(m_root, values, m_translator)) {
            // Write back
            if (values.useDefaults) {
                m_proxy.setFlags(m_currentSlot, ~game::sim::Object::fl_RatingOverride, 0);
            } else {
                m_proxy.setFlags(m_currentSlot, ~game::sim::Object::fl_RatingOverride, game::sim::Object::fl_RatingOverride);
                m_proxy.setFlakRatingOverride(m_currentSlot, values.flakRating);
                m_proxy.setFlakCompensationOverride(m_currentSlot, values.flakCompensation);
            }
        }
    }
}

void
SimulatorDialog::onEditExperienceLevel()
{
    // ex WSimListWithHandler::editExperienceLevel, ccsim.pas:PickExperienceLevel
    if (isAtObject()) {
        Downlink link(m_root, m_translator);
        SimulationSetupProxy::Elements_t elems;
        m_proxy.getExperienceLevelChoices(link, elems);
        prependDigits(elems);

        int32_t value = m_currentObject.experienceLevel.first;
        if (doList(m_root, m_gameSender, elems, value, m_translator("Set Experience Level"), "pcc2:bsim", m_translator)) {
            m_proxy.setExperienceLevel(m_currentSlot, value);
        }
    }
}

void
SimulatorDialog::onEditMass()
{
    // ex WSimListWithHandler::editMass
    if (isAtShip() && m_currentObject.hullType.first == 0) {
        int32_t value = m_currentObject.mass;
        if (doNumber(m_root, SimulationSetupProxy::Range_t(1, MAX_MASS), value, m_translator("Set Mass"), m_translator("Mass"), "pcc2:bsim", m_translator)) {
            m_proxy.setMass(m_currentSlot, value);
        }
    }
}

void
SimulatorDialog::onEditName()
{
    if (isAtShip()) {
        // ex WSimListWithHandler::editName, ccsim.pas:RenameShip
        ui::widgets::InputLine input(20, m_root);
        input.setText(m_currentObject.name);
        if (input.doStandardDialog(m_translator("Set Ship Name"), m_translator("Name:"), m_translator)) {
            m_proxy.setName(m_currentSlot, input.getText());
        }
    } else if (isAtPlanet()) {
        // ex WSimPlanetEditor::editName, ccsim.pas:ChoosePlanetNameFromList
        Downlink link(m_root, m_translator);
        SimulationSetupProxy::Elements_t elems;
        m_proxy.getPlanetNameChoices(link, elems);
        sortAlphabetically(elems, 0);

        int32_t value = m_currentObject.id;
        if (doList(m_root, m_gameSender, elems, value, m_translator("Set Planet Name"), "pcc2:simplanet", m_translator)) {
            m_proxy.setId(m_currentSlot, value);
        }
    }
}

void
SimulatorDialog::onEditOwner()
{
    // ex WSimListWithHandler::editOwner
    if (isAtObject()) {
        Downlink link(m_root, m_translator);
        SimulationSetupProxy::Elements_t elems;
        m_proxy.getOwnerChoices(link, elems);
        prependDigits(elems);

        int32_t value = m_currentObject.owner.first;
        if (doList(m_root, m_gameSender, elems, value, m_translator("Set Owner Level"), "pcc2:bsim", m_translator)) {
            m_proxy.setOwner(m_currentSlot, value);
        }
    }
}

void
SimulatorDialog::onEditPopulation()
{
    // ex WSimPlanetEditor::editPopulation, CPlanetWindow.EditPopulation
    if (isAtPlanet()) {
        Downlink link(m_root, m_translator);
        SimulationSetupProxy::PopulationChoices info;
        m_proxy.getPopulationChoices(link, m_currentSlot, info);

        String_t advice = Format(m_translator("Enter population of the planet. PCC2 will then "
                                              "compute the maximum possible number of defense "
                                              "posts supported by that population. For example, "
                                              "%d clans support up to %d defense posts."),
                                 info.samplePopulation, info.sampleDefense);

        afl::base::Deleter del;
        ui::Window& win = del.addNew(new ui::Window(m_translator("Edit Population"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));
        win.add(del.addNew(new ui::rich::StaticText(advice, 30 * m_root.provider().getFont("")->getEmWidth(), m_root.provider())));

        afl::base::Observable<int32_t> observableValue(info.population);
        ui::widgets::DecimalSelector sel(m_root, m_translator, observableValue, info.range.min(), info.range.max(), 10);
        win.add(sel.addButtons(del, m_root));
        sel.requestFocus();

        ui::EventLoop loop(m_root);
        ui::widgets::StandardDialogButtons& btn = del.addNew(new ui::widgets::StandardDialogButtons(m_root, m_translator));
        win.add(btn);
        btn.addStop(loop);
        win.add(del.addNew(new ui::widgets::Quit(m_root, loop)));

        win.pack();
        m_root.centerWidget(win);
        m_root.add(win);
        if (loop.run() != 0) {
            m_proxy.setPopulation(m_currentSlot, observableValue.get());
        }
    } else {
        onAddPlanet();
    }
}

void
SimulatorDialog::onToggleRandomFriendlyCode()
{
    if (isAtObject()) {
        m_proxy.toggleRandomFriendlyCode(m_currentSlot);
    }
}

void
SimulatorDialog::onEditShieldBaseDefense()
{
    // ex WSimPlanetEditor::editBaseFighters
    if (isAtShip()) {
        // ex WSimListWithHandler::editShield
        Downlink link(m_root, m_translator);
        SimulationSetupProxy::Range_t range = m_proxy.getShieldRange(link, m_currentSlot);
        int32_t value = m_currentObject.shield;
        if (doNumber(m_root, range, value, m_translator("Set Shield Level"), m_translator("Shield"), "pcc2:bsim", m_translator)) {
            m_proxy.setShield(m_currentSlot, value);
        }
    } else if (isAtBase()) {
        // ex WSimPlanetEditor::editBaseDefense
        Downlink link(m_root, m_translator);
        SimulationSetupProxy::Range_t range = m_proxy.getBaseDefenseRange(link, m_currentSlot);
        int32_t value = m_currentObject.baseDefense;
        if (doNumber(m_root, range, value, m_translator("Set Base Defense"), m_translator("Defense"), "pcc2:bsim", m_translator)) {
            m_proxy.setBaseDefense(m_currentSlot, value);
        }
    }
}

void
SimulatorDialog::onEditTypeBaseTorpedoLevel()
{
    if (isAtShip()) {
        editType(false, m_currentSlot, m_currentObject.hullType.first);
    } else if (isAtBase()) {
        // ex WSimPlanetEditor::editBaseTorpTech
        Downlink link(m_root, m_translator);
        SimulationSetupProxy::Elements_t elems;
        m_proxy.getBaseTorpedoLevelChoices(link, elems);
        prependDigits(elems);

        int32_t value = m_currentObject.baseTorpedoTech;
        if (doList(m_root, m_gameSender, elems, value, m_translator("Set Starbase Torpedo Tech"), "pcc2:simplanet", m_translator)) {
            m_proxy.setBaseTorpedoTech(m_currentSlot, value);
        }
    }
}

void
SimulatorDialog::onToggleCloak()
{
    if (isAtShip()) {
        m_proxy.toggleCloak(m_currentSlot);
    }
}

void
SimulatorDialog::onEditIntercept()
{
    // ex WSimListWithHandler::editInterceptId
    if (isAtShip()) {
        Downlink link(m_root, m_translator);
        SimulationSetupProxy::Range_t range = m_proxy.getInterceptIdRange(link, m_currentSlot);
        int32_t value = m_currentObject.interceptId.first;

        if (doNumber(m_root, range, value, m_translator("Set Intercept-Attack Target"), m_translator("Id"), "pcc2:bsim", m_translator)) {
            m_proxy.setInterceptId(m_currentSlot, value);
        }
    }
}

void
SimulatorDialog::onEditAbilities()
{
    // ex WSimListWithHandler::editAbilities, editSimulatorShipFunctions, editSimulatorPlanetFunctions
    // ex ccsim.pas:EditShipAbilities
    if (isAtObject()) {
        Downlink link(m_root, m_translator);
        SimulationSetupProxy::AbilityChoices choices;
        m_proxy.getAbilityChoices(link, m_currentSlot, choices);

        if (client::dialogs::editSimulationAbilities(m_root, m_gameSender, choices, m_translator)) {
            m_proxy.setAbilities(m_currentSlot, choices);
        }
    }
}

void
SimulatorDialog::onFleetCostSummary()
{
    if (isAtObject()) {
        client::dialogs::showSimulationFleetCost(m_root, m_gameSender, m_proxy, m_translator);
    }
}

void
SimulatorDialog::onEditConfiguration()
{
    Downlink link(m_root, m_translator);
    Configuration config;
    m_proxy.getConfiguration(link, config);

    if (client::dialogs::editSimulationConfiguration(m_root, m_gameSender, config, m_translator)) {
        m_proxy.setConfiguration(config, Configuration::Areas_t(Configuration::MainArea));
    }
}

void
SimulatorDialog::onUpdateThis()
{
    // ex WSimListWithHandler::updateFromGame
    if (isAtObject() && m_currentObject.relation >= GameInterface::ReadOnly) {
        Downlink link(m_root, m_translator);
        m_proxy.copyFromGame(link, m_currentSlot, m_currentSlot+1);
    }
}

void
SimulatorDialog::onWriteBackThis()
{
    // ex WSimListWithHandler::writeToGame
    if (isAtObject() && m_currentObject.relation >= GameInterface::Playable) {
        Downlink link(m_root, m_translator);
        Setup::Status st = m_proxy.copyToGame(link, m_currentSlot, m_currentSlot+1);
        if (st.succeeded == 0) {
            MessageBox(m_translator("This unit's status could not be written back."),
                       m_translator("Battle Simulator"),
                       m_root).doOkDialog(m_translator);
        }
    }
}

void
SimulatorDialog::onUpdateAll()
{
    // ex WSimScreen::updateFromGame
    // FIXME: check whether there is a game?
    if (isAtObject()) {
        Downlink link(m_root, m_translator);
        Setup::Status st = m_proxy.copyFromGame(link, 0, m_list.getNumItems());

        afl::string::Translator& tx = m_translator;
        if (st.succeeded > 0) {
            MessageBox(Format(tx("%d unit%!1{s have%| has%} been updated from the game."), st.succeeded),
                       tx("Battle Simulator"),
                       m_root).doOkDialog(tx);
        } else if (st.failed > 0) {
            MessageBox(Format(tx("%d unit%!1{s%} could not be updated from the game."), st.failed),
                       tx("Battle Simulator"),
                       m_root).doOkDialog(tx);
        } else {
            MessageBox(tx("There are none of your units in this simulation."),
                       tx("Battle Simulator"),
                       m_root).doOkDialog(tx);
        }
    }
}

void
SimulatorDialog::onWriteBackAll()
{
    // ex WSimScreen::writeToGame
    // FIXME: check whether there is a game?
    if (isAtObject()) {
        Downlink link(m_root, m_translator);
        Setup::Status st = m_proxy.copyToGame(link, 0, m_list.getNumItems());

        afl::string::Translator& tx = m_translator;
        if (st.succeeded > 0) {
            MessageBox(Format(tx("%d unit%!1{s have%| has%} been written back to the game."), st.succeeded),
                       tx("Battle Simulator"),
                       m_root).doOkDialog(tx);
        } else if (st.failed > 0) {
            MessageBox(Format(tx("%d unit%!1{s%} could not be written back to the game."), st.failed),
                       tx("Battle Simulator"),
                       m_root).doOkDialog(tx);
        } else {
            MessageBox(tx("There are none of your units in this simulation."),
                       tx("Battle Simulator"),
                       m_root).doOkDialog(tx);
        }
    }
}

void
SimulatorDialog::onSwapUp()
{
    // ex WSimListWithHandler::swapUp
    if (canSwapUp()) {
        size_t n = m_list.getCurrentItem();
        m_proxy.swapShips(n, n-1);
        m_list.setCurrentItem(n-1);
    }
}

void
SimulatorDialog::onSwapDown()
{
    // ex WSimListWithHandler::swapDown
    if (canSwapDown()) {
        size_t n = m_list.getCurrentItem();
        m_proxy.swapShips(n+1, n);
        m_list.setCurrentItem(n+1);
    }
}

void
SimulatorDialog::onContextMenu(gfx::Point pt)
{
    // ex WSimScreen::onContextMenu, ccsim.pas:SortMenu
    if (getNumShips(m_list) == 0) {
        return;
    }

    enum {
        SortById,
        SortByOwner,
        SortByHull,
        SortByBattleOrder,
        SortByName,
        SwapUp,
        SwapDown
    };

    ui::widgets::StringListbox list(m_root.provider(), m_root.colorScheme());
    list.addItem(SortById,          m_translator("Sort by Id"));
    list.addItem(SortByOwner,       m_translator("Sort by Owner"));
    list.addItem(SortByHull,        m_translator("Sort by Hull"));
    list.addItem(SortByBattleOrder, m_translator("Sort by Battle Order"));
    list.addItem(SortByName,        m_translator("Sort by Name"));
    if (canSwapUp()) {
        list.addItem(SwapUp,        m_translator("Move up"));
    }
    if (canSwapDown()) {
        list.addItem(SwapDown,      m_translator("Move down"));
    }

    ui::EventLoop loop(m_root);
    if (ui::widgets::MenuFrame(ui::layout::HBox::instance0, m_root, loop).doMenu(list, pt)) {
        int32_t key = -1;
        if (list.getCurrentKey(key)) {
            switch (key) {
             case SortById:          m_proxy.sortShips(SimulationSetupProxy::SortById);          break;
             case SortByOwner:       m_proxy.sortShips(SimulationSetupProxy::SortByOwner);       break;
             case SortByHull:        m_proxy.sortShips(SimulationSetupProxy::SortByHull);        break;
             case SortByBattleOrder: m_proxy.sortShips(SimulationSetupProxy::SortByBattleOrder); break;
             case SortByName:        m_proxy.sortShips(SimulationSetupProxy::SortByName);        break;
             case SwapUp:            onSwapUp();                                                 break;
             case SwapDown:          onSwapDown();                                               break;
            }
        }
    }
}

void
SimulatorDialog::onGoToShip()
{
    // ex WSimListWithHandler::switchToGame
    if (isAtShip() && m_currentObject.relation >= GameInterface::Playable) {
        onGoToReference(Reference(Reference::Ship, m_currentObject.id));
    }
}

void
SimulatorDialog::onGoToPlanet()
{
    if (isAtPlanet() && m_currentObject.relation >= GameInterface::Playable) {
        onGoToReference(Reference(Reference::Planet, m_currentObject.id));
    }
}

void
SimulatorDialog::onGoToBase()
{
    if (isAtPlanet() && m_currentObject.relation >= GameInterface::Playable) {
        onGoToReference(Reference(Reference::Starbase, m_currentObject.id));
    }
}

void
SimulatorDialog::onGoToReference(game::Reference ref)
{
    executeGoToReferenceWait("(Battle Simulator)", ref);
}

void
SimulatorDialog::editType(bool afterAdd, SimulationSetupProxy::Slot_t slot, int oldValue)
{
    // ex WSimListWithHandler::editType
    // For a simple list, SimulationSetupProxy provides hull choices (getHullTypeChoices).
    // However, chooseHull() provides better UI.
    int32_t value = oldValue;
    if (client::dialogs::chooseHull(m_root, m_translator("Set Hull Type"), value, m_translator, m_gameSender, true)) {
        m_proxy.setHullType(slot, value, afterAdd);
    }
}

void
SimulatorDialog::showLimitWarning()
{
    MessageBox(
        m_translator("This simulation already contains the maximum possible number of ships."),
        m_translator("Battle Simulator"),
        m_root).doOkDialog(m_translator);
}


/*
 *  Main Entry Point
 */

void
client::dialogs::doBattleSimulator(client::si::UserSide& iface,
                                   client::si::Control& ctl,
                                   client::si::OutputState& outputState)
{
    // ex client/scr-sim.cc:doSimulator
    SimulationSetupProxy proxy(iface.gameSender(), ctl.root().engine().dispatcher());
    Downlink link(ctl.root(), ctl.translator());
    SimulationSetupProxy::ListItems_t list;
    proxy.getList(link, list);

    SimulatorDialog dlg(ctl, iface.gameSender(), proxy, ctl.root(), outputState, ctl.translator());
    dlg.setListContent(list);
    dlg.run();
}
