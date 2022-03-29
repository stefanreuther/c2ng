/**
  *  \file client/dialogs/objectselectiondialog.cpp
  */

#include "client/dialogs/objectselectiondialog.hpp"
#include "afl/base/refcounted.hpp"
#include "client/si/control.hpp"
#include "client/tiles/tilefactory.hpp"
#include "client/widgets/keymapwidget.hpp"
#include "game/interface/contextprovider.hpp"
#include "game/interface/iteratorcontext.hpp"
#include "game/interface/iteratorprovider.hpp"
#include "game/interface/planetcontext.hpp"
#include "game/interface/shipcontext.hpp"
#include "game/map/objectcursorfactory.hpp"
#include "game/map/simpleobjectcursor.hpp"
#include "game/proxy/cursorobserverproxy.hpp"
#include "game/proxy/objectlistener.hpp"
#include "game/turn.hpp"
#include "interpreter/contextreceiver.hpp"
#include "interpreter/error.hpp"
#include "interpreter/singlecontext.hpp"
#include "interpreter/values.hpp"
#include "ui/eventloop.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/window.hpp"
#include "util/translation.hpp"

namespace {
    /*
     *  Common state for the dialog.
     *  The CommonState object is created in the UI thread and may therefore not do anything during construction.
     *  All references to it are held by objects in the worker thread.
     */
    class CommonState : public afl::base::RefCounted {
     public:
        // Constructor.
        CommonState(int screenNumber, const String_t& keymapName)
            : m_screenNumber(screenNumber),
              m_keymapName(keymapName),
              m_game(),
              m_cursor(),
              conn_viewpointTurnChange()
            { }

        // Set game.
        // Called by ObjectCursorFactory when the ObjectObserver sets up its listener.
        // This opportunity is taken to create the cursor and set up everything.
        void setGame(afl::base::Ptr<game::Game> game)
            {
                // Keep game alive
                m_game = game;

                // Set up the cursor
                if (game::Game* g = game.get()) {
                    m_cursor.setObjectType(g->cursors().getTypeByNumber(m_screenNumber));
                    if (game::map::ObjectCursor* c = g->cursors().getCursorByNumber(m_screenNumber)) {
                        m_cursor.setCurrentIndex(c->getCurrentIndex());
                    }

                    // Set up change notification.
                    // This object has sufficient lifetime, so we can attach the listener here.
                    conn_viewpointTurnChange = g->sig_viewpointTurnChange.add(this, &CommonState::onViewpointTurnChange);
                }
            }

        // Access the cursor.
        // Should only be called after setGame (but is safe calling before; will just report empty).
        game::map::ObjectCursor& cursor()
            { return m_cursor; }

        // Get keymap name.
        const String_t& getKeymapName() const
            { return m_keymapName; }

     private:
        const int m_screenNumber;
        const String_t m_keymapName;
        afl::base::Ptr<game::Game> m_game;
        game::map::SimpleObjectCursor m_cursor;
        afl::base::SignalConnection conn_viewpointTurnChange;

        void onViewpointTurnChange()
            {
                if (m_game.get() != 0) {
                    m_cursor.setObjectType(m_game->cursors().getTypeByNumber(m_screenNumber));
                }
            }
    };

    /*
     *  Iterator Provider.
     *  This is needed to instantiate the UI.Iterator property.
     */
    class DialogIteratorProvider : public game::interface::IteratorProvider {
     public:
        DialogIteratorProvider(game::Session& session, afl::base::Ref<CommonState> state)
            : m_session(session), m_state(state)
            { }
        virtual game::map::ObjectCursor* getCursor()
            { return &m_state->cursor(); }
        virtual game::map::ObjectType* getType()
            { return m_state->cursor().getObjectType(); }
        virtual game::Session& getSession()
            { return m_session; }
        virtual void store(interpreter::TagNode& /*out*/)
            { throw interpreter::Error::notSerializable(); }
        virtual String_t toString()
            { return "#<iterator>"; }
     private:
        game::Session& m_session;
        afl::base::Ref<CommonState> m_state;
    };

    /*
     *  Context Provider.
     *  This class provides contexts to scripts run from the dialog.
     */
    class DialogContextProvider : public game::interface::ContextProvider {
     public:
        DialogContextProvider(afl::base::Ref<CommonState> state)
            : m_state(state)
            { }
        virtual void createContext(game::Session& session, interpreter::ContextReceiver& recv)
            {
                game::map::Object* obj = m_state->cursor().getCurrentObject();
                if (interpreter::Context* ctx = game::interface::createObjectContext(obj, session)) {
                    recv.pushNewContext(ctx);
                }
            }
     private:
        afl::base::Ref<CommonState> m_state;
    };

    /*
     *  Dialog Control.
     *  This object allows the dialog to interact with scripts.
     */
    class DialogControl : public client::si::Control {
     public:
        DialogControl(client::si::UserSide& side, ui::Root& root, ui::EventLoop& loop, afl::base::Ref<CommonState> state, client::si::OutputState& outputState)
            : Control(side),
              m_currentId(0),
              m_loop(loop),
              m_state(state),
              m_receiver(root.engine().dispatcher(), *this),
              m_outputState(outputState)
            { }
        virtual void handleStateChange(client::si::RequestLink2 link, client::si::OutputState::Target target)
            { dialogHandleStateChange(link, target, m_outputState, m_loop, 0); }
        virtual void handlePopupConsole(client::si::RequestLink2 link)
            { defaultHandlePopupConsole(link); }
        virtual void handleScanKeyboardMode(client::si::RequestLink2 link)
            { defaultHandleScanKeyboardMode(link); }
        virtual void handleEndDialog(client::si::RequestLink2 link, int code)
            { dialogHandleEndDialog(link, code, m_outputState, m_loop, code); }
        virtual void handleSetView(client::si::RequestLink2 link, String_t name, bool withKeymap)
            { defaultHandleSetView(link, name, withKeymap); }
        virtual void handleUseKeymap(client::si::RequestLink2 link, String_t name, int prefix)
            { defaultHandleUseKeymap(link, name, prefix); }
        virtual void handleOverlayMessage(client::si::RequestLink2 link, String_t text)
            { defaultHandleOverlayMessage(link, text); }
        virtual game::interface::ContextProvider* createContextProvider()
            { return new DialogContextProvider(m_state); }

        void attach(game::proxy::ObjectObserver& oop)
            {
                // FIXME: move this into a separate class
                class Updater : public util::Request<DialogControl> {
                 public:
                    Updater(int id)
                        : m_id(id)
                        { }
                    virtual void handle(DialogControl& ctl)
                        { ctl.m_currentId = m_id; }
                 private:
                    int m_id;
                };
                class Observer : public game::proxy::ObjectListener {
                 public:
                    Observer(util::RequestSender<DialogControl> sender)
                        : m_sender(sender)
                        { }
                    virtual void handle(game::Session& /*s*/, game::map::Object* obj)
                        {
                            int id = obj ? obj->getId() : 0;
                            m_sender.postNewRequest(new Updater(id));
                        }
                 private:
                    util::RequestSender<DialogControl> m_sender;
                };
                oop.addNewListener(new Observer(m_receiver.getSender()));
            }

        int m_currentId;
     private:
        ui::EventLoop& m_loop;
        afl::base::Ref<CommonState> m_state;
        util::RequestReceiver<DialogControl> m_receiver;
        client::si::OutputState& m_outputState;
    };

    /*
     *  Cursor Factory.
     *  This class initializes the cursor within the CommonState and provides it to the ObjectObserver.
     */
    class DialogCursorFactory : public game::map::ObjectCursorFactory {
     public:
        DialogCursorFactory(afl::base::Ref<CommonState> state)
            : m_state(state)
            { }
        game::map::ObjectCursor* getCursor(game::Session& s)
            {
                m_state->setGame(s.getGame());
                return &m_state->cursor();
            }
     private:
        afl::base::Ref<CommonState> m_state;
    };

    /*
     *  User Interface Property Accessor.
     *  Provides current UI state to scripts.
     */
    class DialogUserInterfaceProperties : public game::interface::UserInterfacePropertyAccessor {
     public:
        DialogUserInterfaceProperties(game::Session& session, afl::base::Ref<CommonState> state)
            : m_session(session),
              m_state(state)
            { m_session.uiPropertyStack().add(*this); }
        ~DialogUserInterfaceProperties()
            { m_session.uiPropertyStack().remove(*this); }

        virtual bool get(game::interface::UserInterfaceProperty prop, std::auto_ptr<afl::data::Value>& result)
            {
                switch (prop) {
                 case game::interface::iuiScreenNumber:
                    return false;
                 case game::interface::iuiScreenRegistered:
                    return false;
                 case game::interface::iuiIterator:
                    result.reset(new game::interface::IteratorContext(*new DialogIteratorProvider(m_session, m_state)));
                    return true;
                 case game::interface::iuiSimFlag:
                    result.reset(interpreter::makeBooleanValue(0));
                    return true;
                 case game::interface::iuiScanX:
                 case game::interface::iuiScanY:
                 case game::interface::iuiChartX:
                 case game::interface::iuiChartY:
                 case game::interface::iuiAutoTask:
                    result.reset();
                    return true;
                 case game::interface::iuiKeymap:
                    result.reset(interpreter::makeStringValue(m_state->getKeymapName()));
                    return true;
                }
                return false;
            }
        virtual bool set(game::interface::UserInterfaceProperty /*prop*/, const afl::data::Value* /*p*/)
            { return false; }
     private:
        game::Session& m_session;
        afl::base::Ref<CommonState> m_state;
    };


    class DialogUIPFromSession : public afl::base::Closure<DialogUserInterfaceProperties*(game::Session&)> {
     public:
        DialogUIPFromSession(const afl::base::Ref<CommonState>& state)
            : m_state(state)
            { }
        virtual DialogUserInterfaceProperties* call(game::Session& session)
            { return new DialogUserInterfaceProperties(session, m_state); }
     private:
        afl::base::Ref<CommonState> m_state;
    };
}

const client::dialogs::ObjectSelectionDialog client::dialogs::SHIP_SELECTION_DIALOG = {
    game::map::Cursors::ShipScreen,
    "SHIPSELECTIONDIALOG",
    "SHIPSELECTIONDIALOG",
    N_("Select Ship"),
    N_("You do not have any ships.\n\n<small>To build ships, use <kbd>B</kbd> on a starbase.</small>"),
};

const client::dialogs::ObjectSelectionDialog client::dialogs::PLANET_SELECTION_DIALOG = {
    game::map::Cursors::PlanetScreen,
    "PLANETSELECTIONDIALOG",
    "PLANETSELECTIONDIALOG",
    N_("Select Planet"),
    N_("You do not have any planets.\n\n<small>Unload colonists from a starship to a planet to colonize it.</small>")
};

const client::dialogs::ObjectSelectionDialog client::dialogs::BASE_SELECTION_DIALOG = {
    game::map::Cursors::BaseScreen,
    "BASESELECTIONDIALOG",
    "BASESELECTIONDIALOG",
    N_("Select Starbase"),
    N_("You do not have any starbases.\n\n<small>To build starbases, use <kbd>F8</kbd> on a planet.</small>")
};

int
client::dialogs::doObjectSelectionDialog(const ObjectSelectionDialog& def,
                                         client::si::UserSide& iface,
                                         client::si::Control& parentControl,
                                         client::si::OutputState& outputState)
{
    // ex WObjectSelectionDialog (sort-of), CSelectWindow.Init (sort-of), cscreen.pas:RunSelectWindow
    ui::Root& root = parentControl.root();
    afl::string::Translator& tx = parentControl.translator();

    // Create common state
    afl::base::Ref<CommonState> state(*new CommonState(def.screenNumber, def.keymapName));

    // Create ObjectObserver. This cause the CommonState to be initialized with the cursor we want.
    game::proxy::CursorObserverProxy oop(iface.gameSender(), std::auto_ptr<game::map::ObjectCursorFactory>(new DialogCursorFactory(state)));

    // Set up script controls
    ui::EventLoop loop(root);
    DialogControl ctl(iface, root, loop, state, outputState);
    util::RequestSender<DialogUserInterfaceProperties> dialogUIP(iface.gameSender().makeTemporary(new DialogUIPFromSession(state)));

    // Set up GUI
    afl::base::Deleter del;
    client::widgets::KeymapWidget& keys = del.addNew(new client::widgets::KeymapWidget(iface.gameSender(), root.engine().dispatcher(), ctl));
    keys.setKeymapName(def.keymapName);

    ui::Window& win = del.addNew(new ui::Window(tx.translateString(def.titleUT), root.provider(), root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));
    client::tiles::TileFactory(iface, keys, oop).createLayout(win, def.layoutName, del);
    ctl.attach(oop);

    ui::widgets::Button& btnOK     = del.addNew(new ui::widgets::Button(tx.translateString("OK"),     util::Key_Return, root));
    ui::widgets::Button& btnCancel = del.addNew(new ui::widgets::Button(tx.translateString("Cancel"), util::Key_Escape, root));
    btnOK.dispatchKeyTo(keys);
    btnCancel.dispatchKeyTo(keys);

    ui::Group& g = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    g.add(btnOK);
    g.add(btnCancel);
    g.add(del.addNew(new ui::Spacer()));
    win.add(g);
    win.add(keys);

    // Do it
    win.pack();
    root.centerWidget(win);
    root.add(win);

    int result = loop.run();
    return result != 0 ? ctl.m_currentId : 0;
}
