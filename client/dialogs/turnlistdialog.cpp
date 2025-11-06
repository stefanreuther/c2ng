/**
  *  \file client/dialogs/turnlistdialog.cpp
  *  \brief Class client::dialogs::TurnListDialog
  */

#include "client/dialogs/turnlistdialog.hpp"
#include "afl/base/deleter.hpp"
#include "game/game.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/window.hpp"
#include "util/rich/parser.hpp"

using client::widgets::TurnListbox;
using game::proxy::HistoryTurnProxy;

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

    /** Convert HistoryTurnProxy::Status into TurnListbox::Status. */
    TurnListbox::Status convertStatus(HistoryTurnProxy::Status status)
    {
        switch (status) {
         case HistoryTurnProxy::Unknown:           return TurnListbox::Unknown;
         case HistoryTurnProxy::Unavailable:       return TurnListbox::Unavailable;
         case HistoryTurnProxy::StronglyAvailable: return TurnListbox::StronglyAvailable;
         case HistoryTurnProxy::WeaklyAvailable:   return TurnListbox::WeaklyAvailable;
         case HistoryTurnProxy::Failed:            return TurnListbox::Failed;
         case HistoryTurnProxy::Loaded:            return TurnListbox::Loaded;
         case HistoryTurnProxy::Current:           return TurnListbox::Current;
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

    TurnListbox::Item convertItem(const HistoryTurnProxy::Item& item)
    {
        return TurnListbox::Item(item.turnNumber, convertTimestamp(item.timestamp), convertStatus(item.status));
    }
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
      m_proxy(sender, root.engine().dispatcher()),
      m_list(gfx::Point(12, 15), // FIXME: magic numbers
             root, tx),
      m_loop(root),
      m_activationTimer(root.engine().createTimer()),
      m_pendingActivation(false)
{
    m_proxy.sig_setup.add(this, &TurnListDialog::onSetup);
    m_proxy.sig_update.add(this, &TurnListDialog::onUpdate);
    m_proxy.requestSetup(MAX_TURNS_TO_DISPLAY);
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
    ui::Window&          win       = del.addNew(new ui::Window(m_translator("Turn History"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));
    ui::widgets::Button& btnOK     = del.addNew(new ui::widgets::Button(m_translator("OK"),     util::Key_Return, m_root));
    ui::widgets::Button& btnCancel = del.addNew(new ui::widgets::Button(m_translator("Cancel"), util::Key_Escape, m_root));

    win.add(m_list);

    ui::Group& g = del.addNew(new ui::Group(ui::layout::HBox::instance5));
    g.add(del.addNew(new ui::Spacer()));
    g.add(btnOK);
    g.add(btnCancel);
    win.add(g);

    btnOK.sig_fire.add(this, &TurnListDialog::onOK);
    btnCancel.sig_fire.add(this, &TurnListDialog::onCancel);

    win.add(del.addNew(new ui::widgets::Quit(m_root, m_loop)));

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
client::dialogs::TurnListDialog::onSetup(const game::proxy::HistoryTurnProxy::Items_t& content, int turnNumber)
{
    // Configure list
    std::vector<TurnListbox::Item> items;
    for (size_t i = 0, n = content.size(); i < n; ++i) {
        items.push_back(convertItem(content[i]));
    }
    m_list.swapItems(items);
    m_list.setCurrentTurnNumber(turnNumber + m_initialDelta);
    m_list.setActiveTurnNumber(turnNumber);

    // Request new data
    postNextRequest(true);
}

// Callback: partial data update.
void
client::dialogs::TurnListDialog::onUpdate(const game::proxy::HistoryTurnProxy::Items_t& content)
{
    bool did = false;

    // Process update if we can
    if (const TurnListbox::Item* firstItem = m_list.getItem(0)) {
        for (size_t i = 0, n = content.size(); i < n; ++i) {
            if (content[i].turnNumber >= firstItem->turnNumber) {
                m_list.setItem(convertItem(content[i]));
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
        m_state = LoadingTurn;
        m_proxy.requestLoad(currentItem->turnNumber);
    } else if (firstItem != 0 && allowUpdate) {
        // Load more status data
        m_state = LoadingStatus;
        m_proxy.requestUpdate(firstItem->turnNumber, MAX_TURNS_TO_UPDATE);
    } else {
        // Nothing to do
        m_state = NoMoreWork;
    }
}
