/**
  *  \file client/dialogs/visualscandialog.cpp
  */

#include "client/dialogs/visualscandialog.hpp"
#include "client/downlink.hpp"
#include "client/proxy/hullspecificationproxy.hpp"
#include "client/proxy/objectlistener.hpp"
#include "client/proxy/playerproxy.hpp"
#include "client/proxy/referencelistproxy.hpp"
#include "client/proxy/referenceobserverproxy.hpp"
#include "client/tiles/visualscanheadertile.hpp"
#include "client/tiles/visualscanhullinfotile.hpp"
#include "client/tiles/visualscanshipinfotile.hpp"
#include "client/widgets/hullspecificationsheet.hpp"
#include "client/widgets/referencelistbox.hpp"
#include "game/game.hpp"
#include "game/map/ship.hpp"
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

using ui::widgets::FrameGroup;
using ui::widgets::AbstractButton;
using ui::widgets::Button;

namespace {
    FrameGroup& wrapWidget(afl::base::Deleter& del, ui::Widget& w, ui::Root& root)
    {
        FrameGroup& frame = FrameGroup::wrapWidget(del, root.colorScheme(), FrameGroup::NoFrame, w);
        frame.setFrameWidth(2);
        return frame;
    }
    
    class ListBuilder : public util::Request<game::Session> {
     public:
        ListBuilder(game::ref::List& list,
                    game::map::Point pos,
                    game::ref::List::Options_t options,
                    game::Id_t& excludeShip)
            : m_list(list), m_pos(pos), m_options(options), m_excludeShip(excludeShip)
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

                        // FIXME: if there is a planet, remember it
                    }
                }
                if (!excludeValid) {
                    m_excludeShip = 0;
                }
            }

     private:
        game::ref::List& m_list;
        game::map::Point m_pos;
        game::ref::List::Options_t m_options;
        game::Id_t& m_excludeShip;
    };

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
        virtual Initializer* clone() const
            { return new Initializer(m_list); }
     private:
        game::ref::List m_list;
    };

    class MarkTask : public util::Request<game::Session> {
     public:
        MarkTask(game::Reference ref)
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
        game::Reference m_reference;
    };
}

/*
 *  ShipData: ship information processed within Window (not in tiles)
 */

struct client::dialogs::VisualScanDialog::ShipData {
    String_t image;
    FrameGroup::Type imageFrame;
    FrameGroup::Type remoteFrame;
    ShipData()
        : image(), imageFrame(FrameGroup::NoFrame), remoteFrame(FrameGroup::NoFrame)
        { }
};


/*
 *  Listener: ObjectListener to produce ShipData (data not processed by tiles)
 */

class client::dialogs::VisualScanDialog::Listener : public client::proxy::ObjectListener {
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

class client::dialogs::VisualScanDialog::Window {
    friend class KeyHandler;
    friend class ListPeer;
    friend class SpecPeer;
 public:
    enum Mode {
        NormalMode,
        SpecMode,
        ListMode
    };

    Window(ui::Root& root, util::RequestSender<game::Session> gameSender, afl::string::Translator& tx, const game::ref::List& list);
    ~Window();

    bool run(String_t title, String_t okName);
    game::Reference getCurrentReference() const;
    void setCurrentReference(game::Reference ref);
    void setData(const ShipData& data);
    void browse(bool forward, bool marked);
    void setAllowRemoteControl(bool flag);
    void toggleMode(Mode mode);
    void setMode(Mode mode);
    void configurePeer(ui::Widget& w);

    afl::base::Signal<void(game::Reference)> sig_referenceChange;

 private:
    ui::Root& m_root;
    util::RequestSender<game::Session> m_gameSender;
    afl::string::Translator& m_translator;
    client::proxy::ReferenceListProxy m_listProxy;
    client::proxy::ReferenceObserverProxy m_observerProxy;
    client::proxy::HullSpecificationProxy m_specProxy;
    util::RequestReceiver<Window> m_reply;
    ui::EventLoop m_loop;

    game::Reference m_currentReference;
    game::ref::UserList m_userList;

    ui::Widget* m_pWindow;
    ui::widgets::ImageButton* m_pImage;
    FrameGroup* m_pImageFrame;
    FrameGroup* m_pRemoteFrame;
    AbstractButton* m_pListButton;
    AbstractButton* m_pSpecButton;

    bool m_allowRemoteControl;

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
            data.imageFrame = FrameGroup::YellowFrame;
        }

        // Remote frame
        // FIXME: missing (getRemoteControlColor, getNewRemoteControlState)
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
     // case SDLK_RETURN:
     // case SDLK_F1:
     //    if (allow_foreign || getCurrentObjectReference()->isPlayable(GObject::Playable))
     //        stop(1);
     //    return true;

     case util::Key_Escape:
        m_parent.m_loop.stop(0);
        return true;

     // case SDLK_INSERT:
     //    if (GShip* s = dynamic_cast<GShip*>(getCurrentObject()))
     //        doShipAddToSim(*s, true);
     //    else if (GPlanet* p = dynamic_cast<GPlanet*>(getCurrentObject()))
     //        doPlanetAddToSim(*p);
     //    return true;

     // case ss_Ctrl + SDLK_INSERT:
     //    for (int index = list.findNextIndex(0); index != 0; index = list.findNextIndex(index)) {
     //        if (GShip* sh = dynamic_cast<GShip*>(&list.getObjectByIndex(index))) {
     //            doShipAddToSim(*sh, false);
     //        } else if (GPlanet* pl = dynamic_cast<GPlanet*>(&list.getObjectByIndex(index))) {
     //            doPlanetAddToSim(*pl);
     //        } else {
     //            // nix
     //        }
     //    }
     //    return true;

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

     // FIXME -> case 'r':
     //    if (GShip* sh = dynamic_cast<GShip*>(getCurrentObject())) {
     //        doRemoteControl(*sh, getDisplayedTurn().getCommands(getPlayerId()));
     //        getCurrentUniverse()->doScreenUpdates();
     //    }
     //    return true;

     case 's':
        m_parent.toggleMode(Window::SpecMode);
        return true;

     case 'l':
     case 'L':
     case util::Key_Tab:
        m_parent.toggleMode(Window::ListMode);
        return true;

     // FIXME -> case ss_Ctrl + 'c':
     //    showCargoList();
     //    return true;

     // FIXME -> case 'c':
     //    showCargo();
     //    return true;

     // FIXME -> case 'h':
     // FIXME -> case ss_Alt + 'h':
     //    doHelp("pcc2:listship");
     //    return true;

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
    m_list.sig_menuRequest.add(&m_parent.m_listProxy, &client::proxy::ReferenceListProxy::onMenu);

    // Create it
    m_window.pack();
    parent.configurePeer(m_window);
    root.add(m_window);
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
      m_window(parent.m_translator.translateString("Ship Specification"), root.provider(), root.colorScheme(), ui::BLUE_DARK_WINDOW, ui::layout::HBox::instance0),
      m_specSheet(root,
                  false /* FIXME: hasPerTurnCosts */,
                  client::proxy::PlayerProxy(parent.m_gameSender).getAllPlayers(link),
                  client::proxy::PlayerProxy(parent.m_gameSender).getPlayerNames(link, game::Player::AdjectiveName))
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

client::dialogs::VisualScanDialog::Window::Window(ui::Root& root, util::RequestSender<game::Session> gameSender, afl::string::Translator& tx, const game::ref::List& list)
    : m_root(root),
      m_gameSender(gameSender),
      m_translator(tx),
      m_listProxy(root, gameSender, tx),
      m_observerProxy(gameSender),
      m_specProxy(gameSender, root.engine().dispatcher()),
      m_reply(root.engine().dispatcher(), *this),
      m_loop(root),
      m_currentReference(),
      m_userList(),
      m_pWindow(0),
      m_pImage(0),
      m_pImageFrame(0),
      m_pRemoteFrame(0),
      m_pListButton(0),
      m_pSpecButton(0),
      m_allowRemoteControl(false),
      m_mode(NormalMode),
      m_listPeer(),
      m_specPeer()
{
    m_listProxy.sig_listChange.add(this, &Window::onListChange);
    m_listProxy.setContentNew(std::auto_ptr<client::proxy::ReferenceListProxy::Initializer_t>(new Initializer(list)));
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
    FrameGroup& frmImage = wrapWidget(h, FrameGroup::wrapWidget(h, m_root.colorScheme(), FrameGroup::LoweredFrame, btnImage), m_root);
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
    m_root.moveWidgetToEdge(win, 0, 0, 5);
    m_root.add(win);
    return (m_loop.run() != 0);
}

inline game::Reference
client::dialogs::VisualScanDialog::Window::getCurrentReference() const
{
    // ex WVisualScanWindow::getCurrentObjectReference (sort-of)
    return m_currentReference;
}

void
client::dialogs::VisualScanDialog::Window::setCurrentReference(game::Reference ref)
{
    // ex WVisualScanWindow::setIndex (sort-of)
    if (ref != m_currentReference) {
        m_currentReference = ref;
        m_observerProxy.setReference(ref);
        m_specProxy.setExistingShipId(ref.getId());      // FIXME: validate that it's a ship
        sig_referenceChange.raise(ref);
    }
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
            Downlink link(m_root);
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
client::dialogs::VisualScanDialog::Window::onListChange(const game::ref::UserList& list)
{
    size_t pos;
    game::Reference ref = m_currentReference;
    if (!ref.isSet() || !list.find(m_currentReference, pos)) {
        for (size_t i = 0; i < list.size(); ++i) {
            if (const game::ref::UserList::Item* p = list.get(i)) {
                if (p->type == game::ref::UserList::ReferenceItem) {
                    ref = p->reference;
                    break;
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

client::dialogs::VisualScanDialog::VisualScanDialog(ui::Root& root, util::RequestSender<game::Session> gameSender, afl::string::Translator& tx)
    : m_root(root),
      m_gameSender(gameSender),
      m_translator(tx),
      m_title(tx.translateString("List Ships")),
      m_okName(tx.translateString("OK")),
      m_allowForeignShips(false),
      m_earlyExit(false),
      m_allowRemoteControl(true /* FIXME */)
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
client::dialogs::VisualScanDialog::loadCurrent(game::map::Point pos, game::ref::List::Options_t options, game::Id_t excludeShip)
{
    // Build initial list
    Downlink link(m_root);
    game::ref::List list;
    ListBuilder b(list, pos, options, excludeShip);
    link.call(m_gameSender, b);

    // Verify
    if (list.size() == 0) {
        String_t msg;

        if (!options.contains(game::ref::List::IncludeForeignShips)) {
            if (excludeShip != 0) {
                msg = m_translator.translateString("There is no other ship of ours at that position.");
            } else {
                msg = m_translator.translateString("There is no ship of ours at that position.");
            }
        } else {
            if (excludeShip != 0) {
                msg = m_translator.translateString("We can't locate another ship at that position.");
            } else {
                msg = m_translator.translateString("We can't locate a ship at that position.");
    // FIXME:      int pid = univ.getPlanetAt(pt);
    //             if (pid && !univ.getPlanet(pid).isPlayable(GObject::Playable))
    //                 /* this message must start with a space */
    //                 msg += format(_(" The planet %s may be hiding ships from our sensors."),
    //                               univ.getPlanet(pid).getName(GObject::PlainName));
            }
        }
        ui::dialogs::MessageBox(msg, m_translator.translateString("Scanner"), m_root).doOkDialog();
        return false;
    }

    // One object only? Bail out early if allowed.
    // FIXME: needs ListBuilder to report playability
    // if (n == 1 && (flags & ls_EarlyExit)) {
    //     GObjectList::size_type n = 1;
    //     while (!l.getObjectReferenceByIndex(n).isGameObject())
    //         ++n;
    //     if ((flags & ls_AllowForeignShips) || l.getObjectReferenceByIndex(n)->isPlayable(GObject::Playable))
    //         return l.getObjectReferenceByIndex(n);
    // }

    m_list = list;

    return true;
}

game::Reference
client::dialogs::VisualScanDialog::run()
{
    Window w(m_root, m_gameSender, m_translator, m_list);
    w.setAllowRemoteControl(m_allowRemoteControl);
                
    // FIXME: if (config.CPEnableRemote()) {
    //     button_r_cf = &createButton(group12, h, "R", 'r');
    // }

//     // /* Place initial cursor */
//     // /* FIXME: This works in control screens, but not in the starchart. */
//     // GObject* current = current_ship_selection.getCurrentObject();
//     // if (current != 0) {
//     //     int n = l.getIndexFor(*current);
//     //     if (n) {
//     //         selection.setCurrentIndex(n);
//     //     }
//     // }

//     // /* Do dialog. */
//     // window.setAcceptForeignShips((flags & ls_AllowForeignShips) != 0);

    bool ok = w.run(m_title, m_okName);
    if (ok) {
        return w.getCurrentReference();
    } else {
        return game::Reference();
    }
}
