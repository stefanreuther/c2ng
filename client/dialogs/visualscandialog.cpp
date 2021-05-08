/**
  *  \file client/dialogs/visualscandialog.cpp
  *  \brief Class client::dialogs::VisualScanDialog
  */

#include "client/dialogs/visualscandialog.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/string/format.hpp"
#include "client/cargotransfer.hpp"
#include "client/dialogs/cargohistorydialog.hpp"
#include "client/dialogs/helpdialog.hpp"
#include "client/dialogs/inboxdialog.hpp"
#include "client/dialogs/referencesortorder.hpp"
#include "client/dialogs/simulationtransfer.hpp"
#include "client/picturenamer.hpp"
#include "client/si/control.hpp"
#include "client/si/outputstate.hpp"
#include "client/si/remotecontrol.hpp"
#include "client/tiles/visualscanheadertile.hpp"
#include "client/tiles/visualscanhullinfotile.hpp"
#include "client/tiles/visualscanshipinfotile.hpp"
#include "client/widgets/costsummarylist.hpp"
#include "client/widgets/hullspecificationsheet.hpp"
#include "client/widgets/referencelistbox.hpp"
#include "game/game.hpp"
#include "game/map/anyshiptype.hpp"
#include "game/map/movementpredictor.hpp"
#include "game/map/ship.hpp"
#include "game/map/shipinfo.hpp"
#include "game/proxy/configurationproxy.hpp"
#include "game/proxy/hullspecificationproxy.hpp"
#include "game/proxy/inboxadaptor.hpp"
#include "game/proxy/objectlistener.hpp"
#include "game/proxy/playerproxy.hpp"
#include "game/proxy/referencelistproxy.hpp"
#include "game/proxy/referenceobserverproxy.hpp"
#include "game/spec/costsummary.hpp"
#include "game/turn.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/invisiblewidget.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/res/resid.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/imagebutton.hpp"
#include "ui/widgets/quit.hpp"
#include "util/unicodechars.hpp"

using client::si::OutputState;
using client::widgets::CostSummaryList;
using game::Reference;
using game::spec::Cost;
using game::spec::CostSummary;
using ui::widgets::BaseButton;
using ui::widgets::Button;
using ui::widgets::FrameGroup;

namespace {
    FrameGroup& wrapWidget(afl::base::Deleter& del, ui::Widget& w, ui::Root& root)
    {
        FrameGroup& frame = FrameGroup::wrapWidget(del, root.colorScheme(), ui::NoFrame, w);
        frame.setFrameWidth(2);
        return frame;
    }

    bool hasRemoteControl(const game::Root& r)
    {
        return r.hostConfiguration()[game::config::HostConfiguration::CPEnableRemote]();
    }

    /*
     *  List builders
     */

    class ListBuilder : public util::Request<game::Session> {
     public:
        ListBuilder(game::ref::List& list,
                    game::map::Point pos,
                    game::ref::List::Options_t options,
                    game::Id_t& excludeShip)
            : m_list(list), m_pos(pos), m_options(options), m_excludeShip(excludeShip), m_initialShipId(0), m_isUniquePlayable(false), m_hasRemoteControl(false)
            { }

        virtual void handle(game::Session& session)
            {
                bool excludeValid = false;
                if (game::Game* g = session.getGame().get()) {
                    if (game::Turn* t = g->getViewpointTurn().get()) {
                        m_list.addObjectsAt(t->universe(), m_pos, m_options, m_excludeShip);

                        // Verify that the ship to be excluded is actually eligible.
                        // This is needed to pick the correct error message.
                        if (game::map::Ship* pShip = t->universe().ships().get(m_excludeShip)) {
                            game::map::Point excludePos;
                            excludeValid = (pShip->getPosition(excludePos) && excludePos == m_pos);
                        }

                        // Remember planet if it's empty
                        if (game::map::Planet* pPlanet = t->universe().planets().get(t->universe().findPlanetAt(m_pos))) {
                            if (!pPlanet->isPlayable(game::map::Object::Playable)) {
                                m_hidingPlanetName = pPlanet->getName(session.translator());
                            }
                        }

                        // Verify playability
                        // FIXME: right?
                        if (m_list.size() == 1) {
                            if (const game::map::Object* pObj = t->universe().getObject(m_list[0])) {
                                m_isUniquePlayable = (pObj->isPlayable(game::map::Object::ReadOnly));
                            }
                        }

                        // Initial cursor
                        m_initialShipId = g->cursors().currentShip().getCurrentIndex();
                    }
                }
                if (!excludeValid) {
                    m_excludeShip = 0;
                }

                if (game::Root* r = session.getRoot().get()) {
                    m_hasRemoteControl = ::hasRemoteControl(*r);
                }
            }

        const String_t& getHidingPlanetName() const
            { return m_hidingPlanetName; }

        bool isUniquePlayable() const
            { return m_isUniquePlayable; }

        bool hasRemoteControl() const
            { return m_hasRemoteControl; }

        game::Id_t getInitialShipId() const
            { return m_initialShipId; }

     private:
        game::ref::List& m_list;
        game::map::Point m_pos;
        game::ref::List::Options_t m_options;
        game::Id_t& m_excludeShip;
        game::Id_t m_initialShipId;
        String_t m_hidingPlanetName;
        bool m_isUniquePlayable;
        bool m_hasRemoteControl;
    };

    class NextBuilder : public util::Request<game::Session> {
     public:
        NextBuilder(game::ref::List& list,
                    game::map::Point pos,
                    game::Id_t fromShip,
                    game::ref::List::Options_t options)
            : m_list(list), m_pos(pos), m_fromShip(fromShip), m_options(options), m_hasRemoteControl(false)
            { }

        virtual void handle(game::Session& session)
            {
                // ex doListNextShips (part)
                game::Root* root = session.getRoot().get();
                game::spec::ShipList* list = session.getShipList().get();
                game::Game* g = session.getGame().get();
                if (root != 0 && list != 0 && g != 0) {
                    if (game::Turn* t = g->getViewpointTurn().get()) {
                        // Compute movement
                        const game::map::Universe& univ = t->universe();
                        game::map::MovementPredictor pred;
                        pred.computeMovement(univ, *g, *list, *root);

                        // If looking at a ship, resolve its position
                        bool posOK;
                        game::map::Point pos;
                        if (m_fromShip != 0) {
                            posOK = pred.getShipPosition(m_fromShip, pos);
                        } else {
                            posOK = true;
                            pos = m_pos;
                        }

                        // Build list
                        if (posOK) {
                            pos = univ.config().getCanonicalLocation(pos);

                            game::map::AnyShipType ty(const_cast<game::map::Universe&>(univ));
                            for (game::Id_t id = ty.findNextIndex(0); id != 0; id = ty.findNextIndex(id)) {
                                const game::map::Ship* sh = univ.ships().get(id);
                                game::map::Point shPos;

                                if (sh != 0
                                    && pred.getShipPosition(id, shPos)
                                    && univ.config().getCanonicalLocation(shPos) == pos
                                    && (m_options.contains(game::ref::List::IncludeForeignShips) || sh->isPlayable(game::map::Object::ReadOnly))
                                    && (!m_options.contains(game::ref::List::SafeShipsOnly) || sh->isReliablyVisible(0)))
                                {
                                    m_list.add(Reference(Reference::Ship, id));
                                }
                            }

                            // If list is not empty, AND we're coming from a ship, place scanner.
                            // (Otherwise, we're likely coming from a context where the scanner is already at the correct place.)
                            if (m_fromShip != 0) {
                                try {
                                    game::interface::UserInterfacePropertyStack& uiProps = session.uiPropertyStack();
                                    afl::data::IntegerValue xv(pos.getX());
                                    afl::data::IntegerValue yv(pos.getY());
                                    uiProps.set(game::interface::iuiScanX, &xv);
                                    uiProps.set(game::interface::iuiScanY, &yv);
                                }
                                catch (...) {
                                    // set() may fail; don't deprive user of this functionality then
                                }
                            }
                        }
                    }
                    m_hasRemoteControl = ::hasRemoteControl(*root);
                }
            }

        bool hasRemoteControl() const
            { return m_hasRemoteControl; }

     private:
        game::ref::List& m_list;
        game::map::Point m_pos;
        game::Id_t m_fromShip;
        game::ref::List::Options_t m_options;
        bool m_hasRemoteControl;
    };

    void buildCurrentCargoSummary(game::Session& session, game::ref::List& in, CostSummary& out)
    {
        if (game::Game* pGame = session.getGame().get()) {
            if (game::Turn* pTurn = pGame->getViewpointTurn().get()) {
                for (size_t i = 0, n = in.size(); i < n; ++i) {
                    if (const game::map::Ship* ship = dynamic_cast<const game::map::Ship*>(pTurn->universe().getObject(in[i]))) {
                        if (ship->isPlayable(game::map::Object::ReadOnly)) {
                            Cost cargo;
                            cargo.set(Cost::Tritanium,  ship->getCargo(game::Element::Tritanium).orElse(0));
                            cargo.set(Cost::Duranium,   ship->getCargo(game::Element::Duranium).orElse(0));
                            cargo.set(Cost::Molybdenum, ship->getCargo(game::Element::Molybdenum).orElse(0));
                            cargo.set(Cost::Supplies,   ship->getCargo(game::Element::Supplies).orElse(0));
                            cargo.set(Cost::Money,      ship->getCargo(game::Element::Money).orElse(0));
                            out.add(CostSummary::Item(ship->getId(), 1, ship->getName(game::LongName, session.translator(), session.interface()), cargo));
                        }
                    }
                }
            }
        }
    }

    void buildNextCargoSummary(game::Session& session, game::ref::List& in, CostSummary& out)
    {
        game::Root* root = session.getRoot().get();
        game::spec::ShipList* list = session.getShipList().get();
        game::Game* g = session.getGame().get();
        if (root != 0 && list != 0 && g != 0) {
            if (game::Turn* t = g->getViewpointTurn().get()) {
                // Compute movement
                const game::map::Universe& univ = t->universe();
                game::map::MovementPredictor pred;
                pred.computeMovement(univ, *g, *list, *root);

                // Build list
                for (size_t i = 0, n = in.size(); i < n; ++i) {
                    if (const game::map::Ship* ship = dynamic_cast<const game::map::Ship*>(univ.getObject(in[i]))) {
                        Cost cargo;
                        if (pred.getShipCargo(ship->getId(), cargo)) {
                            out.add(CostSummary::Item(ship->getId(), 1, ship->getName(game::LongName, session.translator(), session.interface()), cargo));
                        }
                    }
                }
            }
        }
    }

    /*
     *  Game-side implementation of "cargo transfer/history" function
     */

    class CargoRequest : public util::Request<game::Session> {
     public:
        enum Action {
            None,
            Transfer,
            Info
        };
        CargoRequest(Reference ref)
            : m_reference(ref), m_result(None)
            { }
        virtual void handle(game::Session& session)
            {
                game::Root* pRoot = session.getRoot().get();
                game::Game* pGame = session.getGame().get();
                game::spec::ShipList* pList =  session.getShipList().get();
                if (pRoot != 0 && pGame != 0) {
                    if (game::Turn* pTurn = pGame->getViewpointTurn().get()) {
                        if (const game::map::Ship* ship = dynamic_cast<const game::map::Ship*>(pTurn->universe().getObject(m_reference))) {
                            if (ship->isPlayable(game::map::Object::Playable)) {
                                m_result = Transfer;
                            } else {
                                util::NumberFormatter fmt = pRoot->userConfiguration().getNumberFormatter();
                                packShipLastKnownCargo(m_data, *ship, pTurn->getTurnNumber(), fmt, *pList, session.translator());
                                packShipMassRanges    (m_data, *ship,                         fmt, *pList, session.translator());
                                m_result = Info;
                            }
                        }
                    }
                }
            }
        Action getResult() const
            { return m_result; }
        const game::map::ShipCargoInfos_t& getCargoInformation() const
            { return m_data; }
     private:
        Reference m_reference;
        Action m_result;
        game::map::ShipCargoInfos_t m_data;
    };


    /*
     *  Initializer for ReferenceListProxy
     */

    class Initializer : public afl::base::Closure<void(game::Session&, game::ref::ListObserver&)> {
     public:
        Initializer(const game::ref::List& list)
            : m_list(list)
            { }
        virtual void call(game::Session& session, game::ref::ListObserver& obs)
            {
                obs.setSession(session);
                obs.setList(m_list);
            }
     private:
        game::ref::List m_list;
    };


    /*
     *  Implementation of "toggle selection"
     */

    class MarkTask : public util::Request<game::Session> {
     public:
        MarkTask(Reference ref)
            : m_reference(ref)
            { }

        virtual void handle(game::Session& session)
            {
                if (game::Game* pGame = session.getGame().get()) {
                    if (game::Turn* pTurn = pGame->getViewpointTurn().get()) {
                        if (game::map::Object* pObj = pTurn->universe().getObject(m_reference)) {
                            pObj->setIsMarked(!pObj->isMarked());
                            session.notifyListeners();
                        }
                    }
                }
            }

     private:
        Reference m_reference;
    };


    /*
     *  Implementation of "toggle remote"
     */

    class ToggleRemoteTask : public util::Request<game::Session> {
     public:
        ToggleRemoteTask(game::Id_t shipId)
            : m_shipId(shipId)
            { }

        virtual void handle(game::Session& session)
            {
                client::si::toggleRemoteControl(session, m_shipId);
                session.notifyListeners();
            }

     private:
        game::Id_t m_shipId;
    };
}

/*
 *  ShipData: ship information processed within Window (not in tiles)
 */

struct client::dialogs::VisualScanDialog::ShipData {
    String_t image;
    ui::FrameType imageFrame;
    ui::FrameType remoteFrame;
    afl::base::Optional<String_t> remoteQuestion;
    Reference ref;
    bool isPlayable;
    ShipData()
        : image(), imageFrame(ui::NoFrame), remoteFrame(ui::NoFrame), remoteQuestion(), ref(), isPlayable(false)
        { }
};

/*
 *  CargoSummaryBuilder
 */

class client::dialogs::VisualScanDialog::CargoSummaryBuilder : public util::Request<game::Session> {
 public:
    virtual ~CargoSummaryBuilder()
        { }
    virtual String_t getDialogTitle(afl::string::Translator& tx) = 0;

    // Keep it simple:
    game::ref::List m_list;
    CostSummary m_summary;
};

class client::dialogs::VisualScanDialog::CurrentSummaryBuilder : public CargoSummaryBuilder {
 public:
    virtual void handle(game::Session& session)
        { buildCurrentCargoSummary(session, m_list, m_summary); }
    virtual String_t getDialogTitle(afl::string::Translator& tx)
        { return tx("Cargo Summary"); }
};

class client::dialogs::VisualScanDialog::NextSummaryBuilder : public CargoSummaryBuilder {
 public:
    virtual void handle(game::Session& session)
        { buildNextCargoSummary(session, m_list, m_summary); }
    virtual String_t getDialogTitle(afl::string::Translator& tx)
        { return tx("Cargo Summary (Prediction)"); }
};


/*
 *  Listener: ObjectListener to produce ShipData (data not processed by tiles)
 */

class client::dialogs::VisualScanDialog::Listener : public game::proxy::ObjectListener {
 public:
    Listener(util::RequestSender<Window> reply)
        : m_reply(reply)
        { }
    virtual void handle(game::Session& session, game::map::Object* obj);
 private:
    util::RequestSender<Window> m_reply;
};


/*
 *  KeyHandler: one-trick-widget to handle all keys for the visual scan dialog
 */

class client::dialogs::VisualScanDialog::KeyHandler : public ui::InvisibleWidget {
 public:
    explicit KeyHandler(Window& parent)
        : InvisibleWidget(),
          m_parent(parent)
        { }
    virtual bool handleKey(util::Key_t key, int prefix);
 private:
    Window& m_parent;
};


/*
 *  ListPeer: representation of the "ship list" window
 */

class client::dialogs::VisualScanDialog::ListPeer {
 public:
    ListPeer(ui::Root& root, Window& parent);

 private:
    Window& m_parent;
    ui::Window m_window;
    client::widgets::ReferenceListbox m_list;
    afl::base::SignalConnection conn_listChange;
    afl::base::SignalConnection conn_referenceChange;

    void onMenu(gfx::Point pt);
    void onSelectionChange();
};


/*
 *  SpecPeer: representation of the "ship specification" window
 */

class client::dialogs::VisualScanDialog::SpecPeer {
 public:
    SpecPeer(ui::Root& root, Window& parent, Downlink& link);

 private:
    Window& m_parent;
    ui::Window m_window;
    client::widgets::HullSpecificationSheet m_specSheet;

    afl::base::SignalConnection conn_update;
};


/*
 *  Window: run-time representation (open dialog) of the visual scan dialog
 */

class client::dialogs::VisualScanDialog::Window : public client::si::Control {
    friend class KeyHandler;
    friend class ListPeer;
    friend class SpecPeer;
 public:
    enum Mode {
        NormalMode,
        SpecMode,
        ListMode
    };

    Window(client::si::UserSide& iface, ui::Root& root, util::RequestSender<game::Session> gameSender, afl::string::Translator& tx, const game::ref::List& list, CargoSummaryBuilder* csb, OutputState& outputState);
    ~Window();

    bool run(String_t title, String_t okName);
    Reference getCurrentReference() const;
    void setCurrentReference(Reference ref);
    void setInitialShipId(game::Id_t id);
    void setData(const ShipData& data);
    void browse(bool forward, bool marked);
    void setAllowRemoteControl(bool flag);
    void setAllowForeignShips(bool flag);
    void toggleMode(Mode mode);
    void setMode(Mode mode);
    void configurePeer(ui::Widget& w);
    void toggleRemoteControl();
    void showCargoList();
    void showCargo();
    bool canConfirm() const;

    // Control:
    virtual void handleStateChange(client::si::RequestLink2 link, OutputState::Target target);
    virtual void handleEndDialog(client::si::RequestLink2 link, int code);
    virtual void handlePopupConsole(client::si::RequestLink2 link);
    virtual void handleSetViewRequest(client::si::RequestLink2 link, String_t name, bool withKeymap);
    virtual void handleUseKeymapRequest(client::si::RequestLink2 link, String_t name, int prefix);
    virtual void handleOverlayMessageRequest(client::si::RequestLink2 link, String_t text);
    virtual client::si::ContextProvider* createContextProvider();

    afl::base::Signal<void(Reference)> sig_referenceChange;

 private:
    ui::Root& m_root;
    util::RequestSender<game::Session> m_gameSender;
    afl::string::Translator& m_translator;
    game::proxy::ReferenceListProxy m_listProxy;
    game::proxy::ReferenceObserverProxy m_observerProxy;
    game::proxy::HullSpecificationProxy m_specProxy;
    util::RequestReceiver<Window> m_reply;
    ui::EventLoop m_loop;
    OutputState& m_outputState;

    Reference m_currentReference;
    game::ref::UserList m_userList;
    const game::ref::List m_list;
    CargoSummaryBuilder*const m_cargoSummaryBuilder;
    game::Id_t m_initialShipId;

    ui::Widget* m_pWindow;
    ui::widgets::ImageButton* m_pImage;
    FrameGroup* m_pImageFrame;
    FrameGroup* m_pRemoteFrame;
    BaseButton* m_pListButton;
    BaseButton* m_pSpecButton;
    BaseButton* m_pOKButton;

    Reference m_playableReference;
    bool m_isPlayable;

    bool m_allowForeignShips;
    bool m_allowRemoteControl;
    afl::base::Optional<String_t> m_remoteQuestion;

    Mode m_mode;
    std::auto_ptr<ListPeer> m_listPeer;
    std::auto_ptr<SpecPeer> m_specPeer;

    void onListChange(const game::ref::UserList& list);
};

/*
 *  Implementation of Listener
 */

void
client::dialogs::VisualScanDialog::Listener::handle(game::Session& session, game::map::Object* obj)
{
    // ex WVisualScanWindow::onObjectChanged
    class Reply : public util::Request<Window> {
     public:
        Reply(const ShipData& data)
            : m_data(data)
            { }
        virtual void handle(Window& win)
            { win.setData(m_data); }
     private:
        ShipData m_data;
    };

    ShipData data;
    const game::map::Ship* pShip = dynamic_cast<game::map::Ship*>(obj);
    const game::spec::ShipList* pShipList = session.getShipList().get();
    if (pShipList != 0 && pShip != 0) {
        // Hull
        if (const game::spec::Hull* pHull = pShipList->hulls().get(pShip->getHull().orElse(0))) {
            data.image = ui::res::makeResourceId(ui::res::SHIP, pHull->getInternalPictureNumber(), pHull->getId());
        } else {
            data.image = RESOURCE_ID("nvc");
        }

        // Image frame
        if (pShip->isMarked()) {
            data.imageFrame = ui::YellowFrame;
        }

        // Remote control
        data.remoteFrame = client::si::getRemoteControlFrameColor(session, pShip->getId());
        data.remoteQuestion = client::si::getRemoteControlQuestion(session, pShip->getId());

        // Reference
        data.ref = Reference(Reference::Ship, pShip->getId());

        // Playability
        data.isPlayable = pShip->isPlayable(game::map::Object::Playable);
    }

    m_reply.postNewRequest(new Reply(data));
}


/*
 *  Implementation of KeyHandler
 */

bool
client::dialogs::VisualScanDialog::KeyHandler::handleKey(util::Key_t key, int /*prefix*/)
{
    // ex WVisualScanWindow::handleEvent
    switch (key) {
     case util::Key_Return:
     case util::Key_F1:
        if (m_parent.canConfirm()) {
            m_parent.m_loop.stop(1);
        }
        return true;

     case util::Key_Escape:
        m_parent.m_loop.stop(0);
        return true;

     case util::Key_Insert:
        addObjectToSimulation(m_parent.m_root, m_parent.m_gameSender, m_parent.getCurrentReference(), true, m_parent.m_translator);
        return true;

     case util::Key_Insert + util::KeyMod_Ctrl:
        addObjectsToSimulation(m_parent.m_root, m_parent.m_gameSender, m_parent.m_list, m_parent.m_translator);
        return true;

     case '+':
     case util::Key_Down:
     case util::Key_PgDn:
     case util::Key_WheelDown:
        m_parent.browse(true, false);
        return true;

     case util::KeyMod_Ctrl + '+':
     case util::KeyMod_Ctrl + util::Key_Down:
     case util::KeyMod_Ctrl + util::Key_PgDn:
     case util::KeyMod_Ctrl + util::Key_WheelDown:
        m_parent.browse(true, true);
        return true;

     case '-':
     case util::Key_Up:
     case util::Key_PgUp:
     case util::Key_WheelUp:
        m_parent.browse(false, false);
        return true;

     case util::KeyMod_Ctrl + '-':
     case util::KeyMod_Ctrl + util::Key_Up:
     case util::KeyMod_Ctrl + util::Key_PgUp:
     case util::KeyMod_Ctrl + util::Key_WheelUp:
        m_parent.browse(false, true);
        return true;

     case '.':
        m_parent.m_gameSender.postNewRequest(new MarkTask(m_parent.getCurrentReference()));
        return true;

     case 'r':
        m_parent.toggleRemoteControl();
        return true;

     case 's':
        m_parent.toggleMode(Window::SpecMode);
        return true;

     case 'l':
     case 'L':
     case util::Key_Tab:
        m_parent.toggleMode(Window::ListMode);
        return true;

     case 'm':
     case 'M':
        if (m_parent.getCurrentReference().getType() == Reference::Ship) {
            InboxDialog dlg(m_parent.m_translator("Messages"),
                            m_parent.m_gameSender.makeTemporary(game::proxy::makeShipInboxAdaptor(m_parent.getCurrentReference().getId())),
                            m_parent.interface(), m_parent.m_root, m_parent.m_translator);
            if (dlg.run(m_parent.m_outputState, "pcc2:msgin", m_parent.m_translator("No messages for this ship"))) {
                m_parent.m_loop.stop(0);
            }
        }
        return true;

     case util::KeyMod_Ctrl + 'c':
        m_parent.showCargoList();
        return true;

     case 'c':
        m_parent.showCargo();
        return true;

     case 'h':
     case util::KeyMod_Alt + 'h':
        doHelpDialog(m_parent.m_root, m_parent.m_translator, m_parent.m_gameSender, "pcc2:listship");
        return true;

     default:
        return false;
    }
}


/*
 *  Implementation of ListPeer
 */

client::dialogs::VisualScanDialog::ListPeer::ListPeer(ui::Root& root, Window& parent)
    : m_parent(parent),
      m_window(parent.m_translator.translateString("Ship List"), root.provider(), root.colorScheme(), ui::BLUE_WINDOW, ui::layout::HBox::instance0),
      m_list(root)
{
    // ex WVisualScanListWindow::init
    // Build widgets
    m_list.setNumLines(25);
    m_list.setWidth(300 /* FIXME */);
    m_window.add(m_list);

    // Initialize
    m_list.setContent(m_parent.m_userList);
    m_list.setCurrentReference(m_parent.getCurrentReference());

    // Connect signals
    m_list.sig_change.add(this, &ListPeer::onSelectionChange);
    conn_listChange = m_parent.m_listProxy.sig_listChange.add(&m_list, &client::widgets::ReferenceListbox::setContent);
    conn_referenceChange = m_parent.sig_referenceChange.add(&m_list, &client::widgets::ReferenceListbox::setCurrentReference);
    m_list.setFlag(ui::widgets::AbstractListbox::KeyboardMenu, true);
    m_list.sig_menuRequest.add(this, &ListPeer::onMenu);

    // Create it
    m_window.pack();
    parent.configurePeer(m_window);
    root.add(m_window);
}

void
client::dialogs::VisualScanDialog::ListPeer::onMenu(gfx::Point pt)
{
    Downlink link(m_parent.m_root, m_parent.m_translator);
    game::ref::Configuration order = m_parent.m_listProxy.getConfig(link);
    if (client::dialogs::doReferenceSortOrderMenu(order, pt, m_parent.m_root, m_parent.m_translator)) {
        m_parent.m_listProxy.setConfig(order);
    }
}


void
client::dialogs::VisualScanDialog::ListPeer::onSelectionChange()
{
    m_parent.setCurrentReference(m_list.getCurrentReference());
}


/*
 *  Implementation of SpecPeer
 */

client::dialogs::VisualScanDialog::SpecPeer::SpecPeer(ui::Root& root, Window& parent, Downlink& link)
    : m_parent(parent),
      m_window(parent.m_translator("Ship Specification"), root.provider(), root.colorScheme(), ui::BLUE_DARK_WINDOW, ui::layout::HBox::instance0),
      m_specSheet(root,
                  parent.m_translator,
                  false /* FIXME: hasPerTurnCosts */,
                  game::proxy::PlayerProxy(parent.m_gameSender).getAllPlayers(link),
                  game::proxy::PlayerProxy(parent.m_gameSender).getPlayerNames(link, game::Player::AdjectiveName),
                  game::proxy::ConfigurationProxy(parent.m_gameSender).getNumberFormatter(link))
{
    m_window.add(m_specSheet);
    m_window.pack();
    parent.configurePeer(m_window);
    root.add(m_window);

    conn_update = m_parent.m_specProxy.sig_update.add(&m_specSheet, &client::widgets::HullSpecificationSheet::setContent);
}


/*
 *  Implementation of Window
 */

client::dialogs::VisualScanDialog::Window::Window(client::si::UserSide& us, ui::Root& root, util::RequestSender<game::Session> gameSender, afl::string::Translator& tx, const game::ref::List& list, CargoSummaryBuilder* csb, OutputState& outputState)
    : Control(us, root, tx),
      m_root(root),
      m_gameSender(gameSender),
      m_translator(tx),
      m_listProxy(gameSender, root.engine().dispatcher()),
      m_observerProxy(gameSender),
      m_specProxy(gameSender, root.engine().dispatcher(), std::auto_ptr<game::spec::info::PictureNamer>(new client::PictureNamer())),
      m_reply(root.engine().dispatcher(), *this),
      m_loop(root),
      m_outputState(outputState),
      m_currentReference(),
      m_userList(),
      m_list(list),
      m_cargoSummaryBuilder(csb),
      m_initialShipId(0),
      m_pWindow(0),
      m_pImage(0),
      m_pImageFrame(0),
      m_pRemoteFrame(0),
      m_pListButton(0),
      m_pSpecButton(0),
      m_pOKButton(0),
      m_playableReference(),
      m_isPlayable(false),
      m_allowForeignShips(false),
      m_allowRemoteControl(false),
      m_remoteQuestion(),
      m_mode(NormalMode),
      m_listPeer(),
      m_specPeer()
{
    m_listProxy.sig_listChange.add(this, &Window::onListChange);
    m_listProxy.setContentNew(std::auto_ptr<game::proxy::ReferenceListProxy::Initializer_t>(new Initializer(list)));
}

client::dialogs::VisualScanDialog::Window::~Window()
{ }

bool
client::dialogs::VisualScanDialog::Window::run(String_t title, String_t okName)
{
    // ex WVisualScanWindow::init
    // Widget tree:
    //    Window [VBox]
    //      Group [HBox]      1
    //        Group [VBox]    11
    //          HeaderTile
    //          Group [HBox]  111
    //            Picture
    //            HullInfoTile
    //          ShipInfoTile
    //        Group [VBox]    12
    //          Buttons "Prev", "S", "L", "Next"
    //      Group [HBox]      2
    //        Buttons "OK", "Ins", "H", etc.

    // Button 'r' needs a color frame. To align all buttons in line, all buttons
    // in group 12 receive a WColorFrame. Since the color frame is 2 pixels, we
    // use a VBox layout with offset 1 to achieve the visual appearance of the
    // usual offset 5.

    afl::base::Deleter h;
    ui::Window win(title, m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5);
    m_pWindow = &win;

    ui::Group& group1   = h.addNew(new ui::Group(ui::layout::HBox::instance5));
    ui::Group& group11  = h.addNew(new ui::Group(ui::layout::VBox::instance5));
    ui::Group& group111 = h.addNew(new ui::Group(ui::layout::HBox::instance5));
    ui::Group& group12  = h.addNew(new ui::Group(h.addNew(new ui::layout::VBox(1))));
    ui::Group& group2   = h.addNew(new ui::Group(ui::layout::HBox::instance5));

    KeyHandler& keys = h.addNew(new KeyHandler(*this));
    win.add(keys);
    win.add(h.addNew(new ui::widgets::Quit(m_root, m_loop)));

    win.add(group1);
    win.add(group2);

    group1.add(group11);
    group1.add(group12);

    // Header tile: driven by m_observerProxy
    client::tiles::VisualScanHeaderTile& headerTile = h.addNew(new client::tiles::VisualScanHeaderTile(m_root));
    headerTile.attach(m_observerProxy);
    group11.add(headerTile);

    // Ship image: need to keep image widget and frame
    // Note that the image is wrapped twice.
    ui::widgets::ImageButton& btnImage = h.addNew(new ui::widgets::ImageButton(String_t(), '.', m_root, gfx::Point(105, 95)));
    FrameGroup& frmImage = wrapWidget(h, FrameGroup::wrapWidget(h, m_root.colorScheme(), ui::LoweredFrame, btnImage), m_root);
    group111.add(frmImage);
    btnImage.dispatchKeyTo(keys);
    m_pImage = &btnImage;
    m_pImageFrame = &frmImage;

    // Hull info tile: driven by m_observerProxy
    client::tiles::VisualScanHullInfoTile& hullTile = h.addNew(new client::tiles::VisualScanHullInfoTile(m_root));
    hullTile.attach(m_observerProxy);
    group111.add(hullTile);
    group11.add(group111);

    // Ship info tile: driven by m_observerProxy
    client::tiles::VisualScanShipInfoTile& shipTile = h.addNew(new client::tiles::VisualScanShipInfoTile(m_root));
    shipTile.attach(m_observerProxy);
    group11.add(shipTile);

    // "Previous" button
    Button& btnPrev = h.addNew(new Button(UTF_UP_ARROW, '-', m_root));
    group12.add(wrapWidget(h, btnPrev, m_root));
    btnPrev.dispatchKeyTo(keys);

    // "Remote" button. Need to keep frame
    if (m_allowRemoteControl) {
        Button& btnRemote = h.addNew(new Button("R", 'r', m_root));
        FrameGroup& frmRemote = wrapWidget(h, btnRemote, m_root);
        group12.add(frmRemote);
        btnRemote.dispatchKeyTo(keys);
        m_pRemoteFrame = &frmRemote;
    }

    // "Cargo" button
    Button& btnCargo = h.addNew(new Button("C", 'c', m_root));
    group12.add(wrapWidget(h, btnCargo, m_root));
    btnCargo.dispatchKeyTo(keys);

    group12.add(h.addNew(new ui::Spacer()));

    // "List" button. Need to keep button
    Button& btnList = h.addNew(new Button("L", 'l', m_root));
    group12.add(wrapWidget(h, btnList, m_root));
    btnList.dispatchKeyTo(keys);
    m_pListButton = &btnList;

    // "Spec" button. Need to keep button
    Button& btnSpec = h.addNew(new Button("S", 's', m_root));
    group12.add(wrapWidget(h, btnSpec, m_root));
    btnSpec.dispatchKeyTo(keys);
    m_pSpecButton = &btnSpec;

    // "Next" button.
    Button& btnNext = h.addNew(new Button(UTF_DOWN_ARROW, '+', m_root));
    group12.add(wrapWidget(h, btnNext, m_root));
    btnNext.dispatchKeyTo(keys);

    // Dialog buttons
    Button& btnOK = h.addNew(new Button(okName, util::Key_Return, m_root));
    btnOK.dispatchKeyTo(keys);
    m_pOKButton = &btnOK;
    Button& btnCancel = h.addNew(new Button(m_translator.translateString("ESC"), util::Key_Escape, m_root));
    btnCancel.dispatchKeyTo(keys);
    Button& btnAdd = h.addNew(new Button(m_translator.translateString("Add"), util::Key_Insert, m_root));
    btnAdd.dispatchKeyTo(keys);
    Button& btnHelp = h.addNew(new Button(m_translator.translateString("Help"), 'h', m_root));
    btnHelp.dispatchKeyTo(keys);

    group2.add(btnOK);
    group2.add(btnCancel);
    group2.add(btnAdd);
    group2.add(h.addNew(new ui::Spacer()));
    group2.add(btnHelp);

    // Make sure we are updated
    util::RequestReceiver<Window> reply(m_root.engine().dispatcher(), *this);
    m_observerProxy.addNewListener(new Listener(reply.getSender()));

    win.pack();
    m_root.moveWidgetToEdge(win, gfx::LeftAlign, gfx::TopAlign, 5);
    m_root.add(win);
    return (m_loop.run() != 0);
}

inline Reference
client::dialogs::VisualScanDialog::Window::getCurrentReference() const
{
    // ex WVisualScanWindow::getCurrentObjectReference (sort-of)
    return m_currentReference;
}

void
client::dialogs::VisualScanDialog::Window::setCurrentReference(Reference ref)
{
    // ex WVisualScanWindow::setIndex (sort-of)
    if (ref != m_currentReference) {
        m_currentReference = ref;
        m_observerProxy.setReference(ref);
        m_specProxy.setExistingShipId(ref.getId());      // FIXME: validate that it's a ship
        sig_referenceChange.raise(ref);
    }
}

inline void
client::dialogs::VisualScanDialog::Window::setInitialShipId(game::Id_t id)
{
    m_initialShipId = id;
}

void
client::dialogs::VisualScanDialog::Window::setData(const ShipData& data)
{
    if (m_pImage != 0) {
        m_pImage->setImage(data.image);
    }
    if (m_pImageFrame != 0) {
        m_pImageFrame->setType(data.imageFrame);
    }
    if (m_pRemoteFrame != 0) {
        m_pRemoteFrame->setType(data.remoteFrame);
        m_pRemoteFrame->setState(ui::Widget::DisabledState, !data.remoteQuestion.isValid());
    }

    m_playableReference = data.ref;
    m_isPlayable = data.isPlayable;
    m_remoteQuestion = data.remoteQuestion;

    if (m_playableReference == getCurrentReference()) {
        if (m_pOKButton != 0) {
            m_pOKButton->setState(ui::Widget::DisabledState, !canConfirm());
        }
    }
}

void
client::dialogs::VisualScanDialog::Window::browse(bool forward, bool marked)
{
    size_t limit = m_userList.size();
    size_t pos = 0;
    m_userList.find(m_currentReference, pos);
    for (size_t i = 0; i < limit; ++i) {
        if (forward) {
            ++pos;
            if (pos >= limit) {
                if (m_mode == ListMode) {
                    break;
                }
                pos = 0;
            }
        } else {
            if (pos == 0) {
                if (m_mode == ListMode) {
                    break;
                }
                pos = limit;
            }
            --pos;
        }
        if (const game::ref::UserList::Item* p = m_userList.get(pos)) {
            if (p->type == game::ref::UserList::ReferenceItem && (p->marked || !marked)) {
                setCurrentReference(p->reference);
                break;
            }
        }
    }
}

inline void
client::dialogs::VisualScanDialog::Window::setAllowRemoteControl(bool flag)
{
    m_allowRemoteControl = flag;
}

inline void
client::dialogs::VisualScanDialog::Window::setAllowForeignShips(bool flag)
{
    m_allowForeignShips = flag;
}

inline void
client::dialogs::VisualScanDialog::Window::toggleMode(Mode mode)
{
    if (m_mode == mode) {
        setMode(NormalMode);
    } else {
        setMode(mode);
    }
}

void
client::dialogs::VisualScanDialog::Window::setMode(Mode mode)
{
    // ex WVisualScanWindow::setMode
    if (mode != m_mode) {
        m_mode = mode;
        m_listPeer.reset();
        m_specPeer.reset();

        // Make new peer
        if (mode == SpecMode) {
            Downlink link(m_root, m_translator);
            m_specPeer.reset(new SpecPeer(m_root, *this, link));
            m_specProxy.setExistingShipId(m_currentReference.getId()); // FIXME: this re-triggers the signal. Can we do better?
        }
        if (mode == ListMode) {
            m_listPeer.reset(new ListPeer(m_root, *this));
        }

        if (m_pSpecButton != 0) {
            m_pSpecButton->setFlag(ui::HighlightedButton, mode == SpecMode);
        }
        if (m_pListButton != 0) {
            m_pListButton->setFlag(ui::HighlightedButton, mode == ListMode);
        }
    }
}

void
client::dialogs::VisualScanDialog::Window::configurePeer(ui::Widget& w)
{
    // The peer is not modal!
    w.setState(ui::Widget::ModalState, false);

    // Set position
    if (m_pWindow != 0) {
        gfx::Rectangle windowPos = m_pWindow->getExtent();
        gfx::Rectangle peerPos = w.getExtent();
        w.setExtent(gfx::Rectangle(windowPos.getRightX(), windowPos.getTopY(), peerPos.getWidth(), peerPos.getHeight()));
    }
}

void
client::dialogs::VisualScanDialog::Window::toggleRemoteControl()
{
    if (m_currentReference.getType() == Reference::Ship) {
        if (const String_t* q = m_remoteQuestion.get()) {
            if (ui::dialogs::MessageBox(*q, m_translator("Remote Control"), m_root).doYesNoDialog(m_translator)) {
                m_gameSender.postNewRequest(new ToggleRemoteTask(m_currentReference.getId()));
            }
        }
    }
}

void
client::dialogs::VisualScanDialog::Window::showCargoList()
{
    // ex WVisualScanWindow::showCargoList
    if (m_cargoSummaryBuilder != 0) {
        Downlink link(m_root, m_translator);
        m_cargoSummaryBuilder->m_list = m_list;
        m_cargoSummaryBuilder->m_summary.clear();
        link.call(m_gameSender, *m_cargoSummaryBuilder);

        const String_t title = m_cargoSummaryBuilder->getDialogTitle(m_translator);
        if (m_cargoSummaryBuilder->m_summary.getNumItems() == 0) {
            ui::dialogs::MessageBox(m_translator("This list does not include any of your ships."), title, m_root)
                .doOkDialog(m_translator);
            return;
        }

        // Show the dialog
        // VBox
        //   HBox
        //     VBox
        //       WBillDisplay
        //       WBillTotalDisplay
        //     UIScrollbar
        //   HBox
        //     "Export"
        //     UISpacer
        //     "OK"
        //     "Cancel"
        afl::base::Deleter del;
        ui::Window& win = del.addNew(new ui::Window(title, m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));
        CostSummaryList& list = del.addNew(
            new CostSummaryList(int(std::max(size_t(5), std::min(size_t(20), m_cargoSummaryBuilder->m_summary.getNumItems()))),
                                true,
                                CostSummaryList::TotalsFooter,
                                m_root.provider(), m_root.colorScheme(), m_translator));
        list.setContent(m_cargoSummaryBuilder->m_summary);
        win.add(list);

        // FIXME -> ui::widgets::Button& btnExport = del.addNew(new ui::widgets::Button(m_translator("E - Export"), m_root));
        ui::widgets::Button& btnOK     = del.addNew(new ui::widgets::Button(m_translator("OK"), util::Key_Return, m_root));
        ui::widgets::Button& btnCancel = del.addNew(new ui::widgets::Button(m_translator("Cancel"), util::Key_Escape, m_root));
        ui::EventLoop loop(m_root);
        btnOK.sig_fire.addNewClosure(loop.makeStop(1));
        btnCancel.sig_fire.addNewClosure(loop.makeStop(0));
        // FIXME -> btnExport.sig_fire.add(&dpy, &WBillDisplay::doExport);

        ui::Group& g = del.addNew(new ui::Group(ui::layout::HBox::instance5));
        // FIXME -> g.add(btnExport);
        g.add(del.addNew(new ui::Spacer()));
        g.add(btnOK);
        g.add(btnCancel);
        win.add(g);
        win.add(del.addNew(new ui::widgets::Quit(m_root, loop)));
        win.pack();

        // Place cursor on current ship
        size_t myIndex = 0;
        if (m_currentReference.getType() == Reference::Ship && m_cargoSummaryBuilder->m_summary.find(m_currentReference.getId(), &myIndex)) {
            list.setCurrentItem(myIndex);
        }

        m_root.centerWidget(win);
        m_root.add(win);
        if (loop.run() != 0) {
            if (const CostSummary::Item* it = m_cargoSummaryBuilder->m_summary.get(list.getCurrentItem())) {
                setCurrentReference(Reference(Reference::Ship, it->id));
            }
        }
    }
}

void
client::dialogs::VisualScanDialog::Window::showCargo()
{
    // ex WVisualScanWindow::showCargo
    // Determine status
    const Reference r = getCurrentReference();
    CargoRequest req(r);
    Downlink link(m_root, m_translator);
    link.call(m_gameSender, req);

    // Check action
    switch (req.getResult()) {
     case CargoRequest::None:
        break;
     case CargoRequest::Transfer:
        if (dynamic_cast<CurrentSummaryBuilder*>(m_cargoSummaryBuilder) != 0) {
            doShipCargoTransfer(m_root, m_gameSender, m_translator, r.getId());
        }
        break;
     case CargoRequest::Info:
        doCargoHistory(req.getCargoInformation(), m_root, m_translator);
        break;
    }
}

bool
client::dialogs::VisualScanDialog::Window::canConfirm() const
{
    return m_playableReference == getCurrentReference()
        && (m_allowForeignShips || m_isPlayable);
}

void
client::dialogs::VisualScanDialog::Window::handleStateChange(client::si::RequestLink2 link, OutputState::Target target)
{
    dialogHandleStateChange(link, target, m_outputState, m_loop, 0);
}

void
client::dialogs::VisualScanDialog::Window::handleEndDialog(client::si::RequestLink2 link, int code)
{
    dialogHandleEndDialog(link, code, m_outputState, m_loop, 0);
}

void
client::dialogs::VisualScanDialog::Window::handlePopupConsole(client::si::RequestLink2 link)
{
    defaultHandlePopupConsole(link);
}

void
client::dialogs::VisualScanDialog::Window::handleSetViewRequest(client::si::RequestLink2 link, String_t name, bool withKeymap)
{
    defaultHandleSetViewRequest(link, name, withKeymap);
}

void
client::dialogs::VisualScanDialog::Window::handleUseKeymapRequest(client::si::RequestLink2 link, String_t name, int prefix)
{
    defaultHandleUseKeymapRequest(link, name, prefix);
}

void
client::dialogs::VisualScanDialog::Window::handleOverlayMessageRequest(client::si::RequestLink2 link, String_t text)
{
    defaultHandleOverlayMessageRequest(link, text);
}

client::si::ContextProvider*
client::dialogs::VisualScanDialog::Window::createContextProvider()
{
    // FIXME: should be ship.
    return 0;
}


void
client::dialogs::VisualScanDialog::Window::onListChange(const game::ref::UserList& list)
{
    size_t pos;
    Reference ref = m_currentReference;
    if (!ref.isSet() || !list.find(m_currentReference, pos)) {
        Reference initialShip(Reference::Ship, m_initialShipId);
        if (list.find(initialShip, pos)) {
            ref = initialShip;
        } else {
            for (size_t i = 0; i < list.size(); ++i) {
                if (const game::ref::UserList::Item* p = list.get(i)) {
                    if (p->type == game::ref::UserList::ReferenceItem) {
                        ref = p->reference;
                        break;
                    }
                }
            }
        }
    }

    m_userList = list;
    setCurrentReference(ref);
}


/*
 *  VisualScanDialog
 */

client::dialogs::VisualScanDialog::VisualScanDialog(client::si::UserSide& iface, ui::Root& root, afl::string::Translator& tx)
    : m_userSide(iface),
      m_root(root),
      m_gameSender(iface.gameSender()),
      m_translator(tx),
      m_outputState(),
      m_title(tx.translateString("List Ships")),
      m_okName(tx.translateString("OK")),
      m_allowForeignShips(false),
      m_earlyExit(false),
      m_allowRemoteControl(true),
      m_canEarlyExit(false),
      m_initialShipId(0),
      m_list()
{ }

client::dialogs::VisualScanDialog::~VisualScanDialog()
{ }

void
client::dialogs::VisualScanDialog::setTitle(String_t title)
{
    m_title = title;
}

void
client::dialogs::VisualScanDialog::setOkName(String_t okName)
{
    m_okName = okName;
}

void
client::dialogs::VisualScanDialog::setAllowForeignShips(bool flag)
{
    m_allowForeignShips = flag;
}

void
client::dialogs::VisualScanDialog::setEarlyExit(bool flag)
{
    m_earlyExit = flag;
}

bool
client::dialogs::VisualScanDialog::loadCurrent(Downlink& link, game::map::Point pos, game::ref::List::Options_t options, game::Id_t excludeShip)
{
    // Build initial list
    game::ref::List list;
    ListBuilder b(list, pos, options, excludeShip);
    link.call(m_gameSender, b);
    m_canEarlyExit = b.isUniquePlayable();
    m_allowRemoteControl = b.hasRemoteControl();
    m_initialShipId = b.getInitialShipId();

    // Verify
    if (list.size() == 0) {
        String_t msg;

        if (!options.contains(game::ref::List::IncludeForeignShips)) {
            if (excludeShip != 0) {
                msg = m_translator("There is no other ship of ours at that position.");
            } else {
                msg = m_translator("There is no ship of ours at that position.");
            }
        } else {
            if (excludeShip != 0) {
                msg = m_translator("We can't locate another ship at that position.");
            } else {
                msg = m_translator("We can't locate a ship at that position.");
                if (!b.getHidingPlanetName().empty()) {
                    // This message must start with a space
                    msg += afl::string::Format(m_translator(" The planet %s may be hiding ships from our sensors."), b.getHidingPlanetName());
                }
            }
        }
        ui::dialogs::MessageBox(msg, m_translator("Scanner"), m_root).doOkDialog(m_translator);
        return false;
    }

    m_list = list;
    m_cargoSummaryBuilder.reset(new CurrentSummaryBuilder());

    return true;
}

bool
client::dialogs::VisualScanDialog::loadNext(Downlink& link, game::map::Point pos, game::Id_t fromShip, game::ref::List::Options_t options)
{
    // ex doListNextShips (part)
    // Build initial list
    game::ref::List list;
    NextBuilder b(list, pos, fromShip, options);
    link.call(m_gameSender, b);
    m_allowRemoteControl = b.hasRemoteControl();
    m_initialShipId = fromShip;

    // List empty? Show message.
    if (list.size() == 0) {
        ui::dialogs::MessageBox(m_translator("We can't find a ship that will be at this position next turn."), m_translator("Scanner"), m_root).doOkDialog(m_translator);
        return false;
    }

    m_list = list;
    m_cargoSummaryBuilder.reset(new NextSummaryBuilder());

    return true;
}

game::Reference
client::dialogs::VisualScanDialog::run()
{
    // One object only? Bail out early if allowed.
    if (m_earlyExit && m_canEarlyExit) {
        return m_list[0];
    }

    // Build window
    Window w(m_userSide, m_root, m_gameSender, m_translator, m_list, m_cargoSummaryBuilder.get(), m_outputState);
    w.setAllowRemoteControl(m_allowRemoteControl);
    w.setAllowForeignShips(m_allowForeignShips);
    w.setInitialShipId(m_initialShipId);

    bool ok = w.run(m_title, m_okName);
    if (ok) {
        return w.getCurrentReference();
    } else {
        return Reference();
    }
}

client::si::OutputState&
client::dialogs::VisualScanDialog::outputState()
{
    return m_outputState;
}
