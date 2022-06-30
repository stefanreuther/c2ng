/**
  *  \file client/dialogs/turnlistdialog.cpp
  *  \brief Class client::dialogs::TurnListDialog
  */

#include "client/dialogs/turnlistdialog.hpp"
#include "afl/base/deleter.hpp"
#include "game/game.hpp"
#include "game/historyturn.hpp"
#include "game/interface/globalcommands.hpp"
#include "game/root.hpp"
#include "game/turn.hpp"
#include "game/turnloader.hpp"
#include "interpreter/bytecodeobject.hpp"
#include "interpreter/opcode.hpp"
#include "interpreter/process.hpp"
#include "interpreter/simpleprocedure.hpp"
#include "interpreter/values.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/window.hpp"
#include "util/rich/parser.hpp"

using client::widgets::TurnListbox;
using game::HistoryTurn;

namespace {
    /*
     *  Constants
     */

    /** Maximum number of turns to update metainformation for in one go.
        Limits the maximum time we spend performing I/O. */
    const int MAX_TURNS_TO_UPDATE = 10;

    /** Maximum number of turns to display at all.
        This is to limit that bogus data causes us to allocate unbounded memory.
        The longest games I've heard of have around 400 turns so this should be plenty.
        - Attax @ Blutmagie: 255 turns
        - Star Fleet Battles 4 @ Circus Maximus: 181 turns
        - Helios Sector @ Planets.nu: 367 turns
        - Winplan/HOST having special handling for games > 312 turns */
    const int MAX_TURNS_TO_DISPLAY = 1000;

    /** Turn activation grace period.
        If the user tries to load a turn that is not loaded yet, defer the activation by this time;
        if it becomes available in the meantime, activate it.
        This allows fluent usage using the keyboard even in the presence of network / I/O latencies. */
    const afl::sys::Timeout_t ACTIVATION_GRACE_PERIOD = 500;

    /** Convert HistoryTurn::Status into TurnListbox::Status. */
    TurnListbox::Status convertStatus(HistoryTurn::Status status)
    {
        switch (status) {
         case HistoryTurn::Unknown:           return TurnListbox::Unknown;
         case HistoryTurn::Unavailable:       return TurnListbox::Unavailable;
         case HistoryTurn::StronglyAvailable: return TurnListbox::StronglyAvailable;
         case HistoryTurn::WeaklyAvailable:   return TurnListbox::WeaklyAvailable;
         case HistoryTurn::Failed:            return TurnListbox::Failed;
         case HistoryTurn::Loaded:            return TurnListbox::Loaded;
        }
        return TurnListbox::Unknown;
    }

    /** Convert Timestamp into human-readable string. */
    String_t convertTimestamp(const game::Timestamp& ts)
    {
        if (ts.isValid()) {
            return ts.getDateAsString() + ", " + ts.getTimeAsString();
        } else {
            return String_t();
        }
    }

    /** Prepare list item for a history turn.
        \param content [out] List item will be appended here
        \param game [in] Game
        \param turnNumber [in] Turn number */
    void prepareListItem(std::vector<TurnListbox::Item>& content, const game::Game& game, int turnNumber)
    {
        content.push_back(TurnListbox::Item(turnNumber,
                                            convertTimestamp(game.previousTurns().getTurnTimestamp(turnNumber)),
                                            convertStatus(game.previousTurns().getTurnStatus(turnNumber))));
    }

    /*
     *  Initial (handleSetup()) response
     */
    class InitialResponse : public util::Request<client::dialogs::TurnListDialog> {
     public:
        InitialResponse(std::vector<TurnListbox::Item>& content, int turnNumber)
            : m_content(),
              m_turnNumber(turnNumber)
            { m_content.swap(content); }
        void handle(client::dialogs::TurnListDialog& dialog)
            { dialog.handleSetup(m_content, m_turnNumber); }
     private:
        std::vector<TurnListbox::Item> m_content;
        int m_turnNumber;
    };

    /*
     *  Initial request.
     *  Determines what to display and creates dialog initialisation data.
     */
    class InitialRequest : public util::Request<game::Session> {
     public:
        InitialRequest(util::RequestSender<client::dialogs::TurnListDialog> response)
            : m_response(response)
            { }
        void handle(game::Session& s)
            {
                std::vector<TurnListbox::Item> content;
                int activeTurn = 0;
                if (game::Game* g = s.getGame().get()) {
                    // Fetch current turn
                    int currentTurn = g->currentTurn().getTurnNumber();
                    activeTurn = g->getViewpointTurnNumber();

                    // Fetch status of all turns below that.
                    // Limit turn count to avoid bogus data overloading us.
                    int minTurn = (currentTurn <= MAX_TURNS_TO_DISPLAY ? 1 : currentTurn - MAX_TURNS_TO_DISPLAY);
                    for (int i = minTurn; i < currentTurn; ++i) {
                        prepareListItem(content, *g, i);
                    }

                    // Current turn
                    content.push_back(TurnListbox::Item(currentTurn, convertTimestamp(g->currentTurn().getTimestamp()), TurnListbox::Current));
                }
                m_response.postNewRequest(new InitialResponse(content, activeTurn));
            }
     private:
        util::RequestSender<client::dialogs::TurnListDialog> m_response;
    };

    /*
     *  Partial update (handleUpdate()) response
     */
    class UpdateResponse : public util::Request<client::dialogs::TurnListDialog> {
     public:
        UpdateResponse(std::vector<TurnListbox::Item>& content)
            : m_content()
            { m_content.swap(content); }
        void handle(client::dialogs::TurnListDialog& dialog)
            { dialog.handleUpdate(m_content); }
     private:
        std::vector<TurnListbox::Item> m_content;
    };

    /*
     *  Update request.
     *  Determines whether there are any turns that can be updated, and if so, updates their metainformation.
     */
    class UpdateRequest : public util::Request<game::Session> {
     public:
        /** Constructor.
            \param response Response channel
            \param firstTurn First turn we're interested in. If the dialog display turns 300 .. 1300, it makes no sense to update turn 100. */
        UpdateRequest(util::RequestSender<client::dialogs::TurnListDialog> response, int firstTurn)
            : m_response(response),
              m_firstTurn(firstTurn)
            { }
        void handle(game::Session& s)
            {
                std::vector<TurnListbox::Item> content;
                game::Game* g = s.getGame().get();
                game::Root* r = s.getRoot().get();
                if (g != 0 && r != 0 && r->getTurnLoader().get() != 0) {
                    int lastTurn = g->previousTurns().findNewestUnknownTurnNumber(g->currentTurn().getTurnNumber());
                    if (lastTurn >= m_firstTurn) {
                        // Update
                        int firstTurn = std::max(m_firstTurn, lastTurn - (MAX_TURNS_TO_UPDATE-1));
                        g->previousTurns().initFromTurnScores(g->scores(), firstTurn, lastTurn - firstTurn + 1);
                        g->previousTurns().initFromTurnLoader(*r->getTurnLoader(), *r, g->getViewpointPlayer(), firstTurn, lastTurn - firstTurn + 1);

                        // Build result
                        for (int i = firstTurn; i <= lastTurn; ++i) {
                            prepareListItem(content, *g, i);
                        }
                    }
                }
                m_response.postNewRequest(new UpdateResponse(content));
            }
     private:
        util::RequestSender<client::dialogs::TurnListDialog> m_response;
        const int m_firstTurn;
    };

    class LoadRequest : public util::Request<game::Session> {
     public:
        LoadRequest(util::RequestSender<client::dialogs::TurnListDialog> response, int turnNumber)
            : m_response(response),
              m_turnNumber(turnNumber)
            { }
        void handle(game::Session& s)
            {
                // Implemented as a helper process to re-use the IFHistoryLoadTurn command and its ability to suspend/interact;
                // otherwise, we'd have to implement storage for a suspended process.
                class Finalizer : public interpreter::Process::Finalizer {
                 public:
                    Finalizer(util::RequestSender<client::dialogs::TurnListDialog> response, game::Session& session, int turnNumber)
                        : m_response(response), m_session(session), m_turnNumber(turnNumber)
                        { }
                    void finalizeProcess(interpreter::Process& /*proc*/)
                        {
                            std::vector<TurnListbox::Item> content;
                            if (game::Game* g = m_session.getGame().get()) {
                                prepareListItem(content, *g, m_turnNumber);
                            }
                            m_response.postNewRequest(new UpdateResponse(content));
                        }
                 private:
                    util::RequestSender<client::dialogs::TurnListDialog> m_response;
                    game::Session& m_session;
                    const int m_turnNumber;
                };

                interpreter::Process& proc = s.processList().create(s.world(), "<LoadRequest>");
                interpreter::BCORef_t bco = interpreter::BytecodeObject::create(true);
                proc.pushNewValue(interpreter::makeIntegerValue(m_turnNumber));
                proc.pushNewValue(new interpreter::SimpleProcedure<game::Session&>(s, game::interface::IFHistoryLoadTurn));
                bco->addInstruction(interpreter::Opcode::maIndirect, interpreter::Opcode::miIMCall, 1);
                proc.pushFrame(bco, false);
                proc.setNewFinalizer(new Finalizer(m_response, s, m_turnNumber));

                uint32_t pgid = s.processList().allocateProcessGroup();
                s.processList().resumeProcess(proc, pgid);
                s.processList().startProcessGroup(pgid);
                s.processList().run();
                s.processList().removeTerminatedProcesses();
            }
     private:
        util::RequestSender<client::dialogs::TurnListDialog> m_response;
        const int m_turnNumber;
    };
}

/***************************** TurnListDialog ****************************/

// Constructor.
client::dialogs::TurnListDialog::TurnListDialog(ui::Root& root,
                                                afl::string::Translator& tx,
                                                util::RequestSender<game::Session> sender,
                                                int initialDelta)
    : m_state(LoadingInitial),
      m_initialDelta(initialDelta),
      m_root(root),
      m_translator(tx),
      m_sender(sender),
      m_receiver(root.engine().dispatcher(), *this),
      m_list(gfx::Point(12, 15), // FIXME: magic numbers
             root, tx),
      m_loop(root),
      m_activationTimer(root.engine().createTimer()),
      m_pendingActivation(false)
{
    // LoadingInitial: post an InitialRequest, will answer with handleSetup().
    m_sender.postNewRequest(new InitialRequest(m_receiver.getSender()));
    m_list.sig_change.add(this, &TurnListDialog::onScroll);
    m_list.sig_itemDoubleClick.add(this, &TurnListDialog::onOK);
    m_activationTimer->sig_fire.add(this, &TurnListDialog::onActivationTimer);
}

// Destructor.
client::dialogs::TurnListDialog::~TurnListDialog()
{ }

// Execute the dialog.
int
client::dialogs::TurnListDialog::run()
{
    afl::base::Deleter del;
    ui::Window&          win       = del.addNew(new ui::Window(m_translator.translateString("Turn History"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));
    ui::widgets::Button& btnOK     = del.addNew(new ui::widgets::Button(m_translator.translateString("OK"),     util::Key_Return, m_root));
    ui::widgets::Button& btnCancel = del.addNew(new ui::widgets::Button(m_translator.translateString("Cancel"), util::Key_Escape, m_root));

    win.add(m_list);

    ui::Group& g = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    g.add(del.addNew(new ui::Spacer()));
    g.add(btnOK);
    g.add(btnCancel);
    win.add(g);

    btnOK.sig_fire.add(this, &TurnListDialog::onOK);
    btnCancel.sig_fire.add(this, &TurnListDialog::onCancel);

    win.pack();
    m_root.centerWidget(win);
    m_root.add(win);

    // Run dialog and produce result
    bool ok = (m_loop.run() != 0);
    int result = 0;
    if (ok) {
        if (const TurnListbox::Item* p = m_list.getItem(m_list.getCurrentItem())) {
            result = p->turnNumber;
        }
    }
    return result;
}

// Callback: initial dialog setup.
void
client::dialogs::TurnListDialog::handleSetup(std::vector<TurnListbox::Item>& content, int turnNumber)
{
    // Configure list
    m_list.swapItems(content);
    if (const TurnListbox::Item* firstItem = m_list.getItem(0)) {
        // Place cursor on current turn
        m_list.setCurrentItem(turnNumber - firstItem->turnNumber + m_initialDelta,
                              m_initialDelta <= 0 ? TurnListbox::GoDown : TurnListbox::GoUp);
        // FIXME: should center display
    }

    // Request new data
    postNextRequest(true);
}

// Callback: partial data update.
void
client::dialogs::TurnListDialog::handleUpdate(std::vector<TurnListbox::Item>& content)
{
    bool did = false;

    // Process update if we can
    if (const TurnListbox::Item* firstItem = m_list.getItem(0)) {
        for (size_t i = 0, n = content.size(); i < n; ++i) {
            if (content[i].turnNumber >= firstItem->turnNumber) {
                m_list.setItem(content[i].turnNumber - firstItem->turnNumber, content[i]);
                did = true;
            }
        }
    }

    // Can we perform an activation?
    if (did && m_pendingActivation) {
        if (handleSelect()) {
            m_pendingActivation = false;
        }
    }

    // Request new data
    // If we updated some data, allow fetching more updates; otherwise, only allow fetching turns.
    postNextRequest(did);
}

/** Event: "OK" button pressed. */
void
client::dialogs::TurnListDialog::onOK()
{
    if (!handleSelect()) {
        // We tried to select, but the current turn was in an intermediate state.
        // Wind up timer.
        m_activationTimer->setInterval(ACTIVATION_GRACE_PERIOD);
        m_pendingActivation = true;
    } else {
        // OK, did it
        m_pendingActivation = false;
    }
}

/** Event: "Cancel" button pressed. */
void
client::dialogs::TurnListDialog::onCancel()
{
    m_loop.stop(0);
    m_pendingActivation = false;
}

/** Event: Position in turn listbox changed. */
void
client::dialogs::TurnListDialog::onScroll()
{
    if (m_state == NoMoreWork) {
        postNextRequest(false);
    }
    m_pendingActivation = false;
}

/** Event: Activation timer fired. */
void
client::dialogs::TurnListDialog::onActivationTimer()
{
    if (m_pendingActivation) {
        m_pendingActivation = false;
        handleSelect();
    }
}

/** Perform turn selection.
    If possible, either activates a turn (=ends the dialog) or explains why the turn cannot be activated.
    \retval true Request performed (turn activated or failed)
    \retval false Request cannot be performed yet. Caller should start activation timer. */
bool
client::dialogs::TurnListDialog::handleSelect()
{
    if (const TurnListbox::Item* p = m_list.getItem(m_list.getCurrentItem())) {
        switch (p->status) {
         case TurnListbox::Unknown:
         case TurnListbox::StronglyAvailable:
         case TurnListbox::WeaklyAvailable:
            // Will eventually become available
            return false;

         case TurnListbox::Unavailable:
            // Failed
            ui::dialogs::MessageBox(util::rich::Parser::parseXml(m_translator("This turn is not available.\n\n"
                                                                              "<small>Enable the backup functionality in <b>Options</b> to make PCC2 save copies of old result files.</small>")),
                                    m_translator("Turn History"),
                                    m_root)
                .doOkDialog(m_translator);
            return true;

         case TurnListbox::Failed:
            // Failed already
            ui::dialogs::MessageBox(m_translator("There was an error loading this turn."),
                                    m_translator("Turn History"),
                                    m_root)
                .doOkDialog(m_translator);
            return true;

         case TurnListbox::Loaded:
         case TurnListbox::Current:
         case TurnListbox::Active:
            // Success
            m_loop.stop(1);
            return true;
        }
    }
    return true;
}

/** Post next request to game session.
    \param allowUpdate true if it makes sense to ask for metadata updates (UpdateRequest); false if last request told us that no more metadata is available. */
void
client::dialogs::TurnListDialog::postNextRequest(bool allowUpdate)
{
    const TurnListbox::Item* currentItem = m_list.getItem(m_list.getCurrentItem());
    const TurnListbox::Item* firstItem = m_list.getItem(0);
    if (currentItem != 0 && (currentItem->status == TurnListbox::WeaklyAvailable || currentItem->status == TurnListbox::StronglyAvailable)) {
        // Cursor is on an item that is possibly available: load it
        m_sender.postNewRequest(new LoadRequest(m_receiver.getSender(), currentItem->turnNumber));
        m_state = LoadingTurn;
    } else if (firstItem != 0 && allowUpdate) {
        // Load more status data
        m_sender.postNewRequest(new UpdateRequest(m_receiver.getSender(), firstItem->turnNumber));
        m_state = LoadingStatus;
    } else {
        // Nothing to do
        m_state = NoMoreWork;
    }
}
