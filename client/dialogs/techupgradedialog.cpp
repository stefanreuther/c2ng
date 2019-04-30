/**
  *  \file client/dialogs/techupgradedialog.cpp
  */

#include "client/dialogs/techupgradedialog.hpp"
#include "afl/base/deleter.hpp"
#include "client/downlink.hpp"
#include "client/widgets/costdisplay.hpp"
#include "client/widgets/techbar.hpp"
#include "game/actions/preconditions.hpp"
#include "game/actions/techupgrade.hpp"
#include "game/exception.hpp"
#include "game/game.hpp"
#include "game/map/planetstorage.hpp"
#include "game/root.hpp"
#include "game/session.hpp"
#include "game/spec/shiplist.hpp"
#include "game/turn.hpp"
#include "ui/eventloop.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/widgets/focusablegroup.hpp"
#include "ui/widgets/focusiterator.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/window.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestsender.hpp"
#include "util/slaveobject.hpp"
#include "util/slaverequestsender.hpp"
#include "util/translation.hpp"
#include "client/proxy/configurationproxy.hpp"

using client::widgets::CostDisplay;
using game::NUM_TECH_AREAS;
using game::TechLevel;
using game::spec::Cost;
using afl::base::Ptr;

namespace {
    const char*const LOG_NAME = "client.dialogs.tech";

    /*
     *  Status structure: goes from Slave to Dialog
     */
    struct Status {
        Cost cost;                    // Total cost
        Cost available;               // Available resources
        Cost remaining;               // Remaining resources
        Cost missing;                 // Missing resources
        int min[NUM_TECH_AREAS];      // Minimum valid tech levels
        int max[NUM_TECH_AREAS];      // Maximum valid tech levels
        int current[NUM_TECH_AREAS];  // Current tech level
        bool valid;                   // true if transaction is valid and can be committed
    };

    /*
     *  Order structure: goes from Dialog to Slave
     */
    struct Order {
        int values[NUM_TECH_AREAS];
    };

    /*
     *  Slave: lives in game session, maintains the action/container.
     *
     *  In theory, we could support concurrency by sending unsolicited status updates.
     *  For now, we don't do that; Slave is passive.
     */
    class TechUpgradeSlave : public util::SlaveObject<game::Session> {
     public:
        explicit TechUpgradeSlave(game::Id_t pid)
            : m_pid(pid)
            { }

        virtual void init(game::Session& session);
        virtual void done(game::Session& session);

        bool getStatus(Status& st) const;
        void setOrder(const Order& o);
        void commit(game::Session& session);

     private:
        game::Id_t m_pid;

        Ptr<game::Turn> m_pTurn;
        Ptr<game::spec::ShipList> m_pShipList;
        Ptr<game::Root> m_pRoot;

        std::auto_ptr<game::CargoContainer> m_pContainer;
        std::auto_ptr<game::actions::TechUpgrade> m_pAction;
    };

    /*
     *  Dialog
     */
    class TechUpgradeDialog {
        friend class TechUpgradeSlave;
     public:
        TechUpgradeDialog(ui::Root& root, util::RequestSender<game::Session> gameSender, game::Id_t pid);

        void run();

     private:
        bool queryStatus(Status& st);
        void setStatus(const Status& st);
        void onChange();
        void onOK();

        /* Plumbing */
        ui::Root& m_root;
        ui::EventLoop m_loop;
        client::Downlink m_link;

        /* Target tech levels */
        afl::base::Observable<int32_t> m_techLevels[game::NUM_TECH_AREAS];

        /* Widgets */
        ui::widgets::StandardDialogButtons m_buttons;
        client::widgets::CostDisplay m_costDisplay;

        /* Communication */
        util::RequestReceiver<TechUpgradeDialog> m_reply;
        util::SlaveRequestSender<game::Session, TechUpgradeSlave> m_slave;

        /* Number of outstanding replies.
           We only update 'OK' button status if this is zero, to avoid flickering.
           Also, nonzero means that m_valid may not be current.
           Therefore, we don't allow confirming the dialog when replies are outstanding. */
        size_t m_numOutstandingReplies;

        /* Validity of transaction. Must be true to be allowed to 'OK' the dialog. */
        bool m_valid;
    };

}

/*
 *  TechUpgradeSlave
 */

void
TechUpgradeSlave::init(game::Session& session)
{
    try {
        // Surroundings
        m_pTurn = game::actions::mustHaveGame(session).getViewpointTurn();
        m_pShipList = session.getShipList();
        m_pRoot = session.getRoot();
        if (m_pTurn.get() == 0 || m_pShipList.get() == 0 || m_pRoot.get() == 0) {
            throw game::Exception(game::Exception::eUser);
        }

        // Fetch planet
        game::map::Planet* pPlanet = m_pTurn->universe().planets().get(m_pid);
        if (pPlanet == 0) {
            throw game::Exception(game::Exception::eNotPlaying);
        }

        // Create tech upgrade action (checks preconditions)
        m_pContainer.reset(new game::map::PlanetStorage(*pPlanet, session.interface(), m_pRoot->hostConfiguration()));
        m_pAction.reset(new game::actions::TechUpgrade(*pPlanet, *m_pContainer, *m_pShipList, *m_pRoot));
        m_pAction->setUndoInformation(m_pTurn->universe());
    }
    catch (std::exception& e) {
        session.log().write(afl::sys::LogListener::Warn, LOG_NAME, session.translator()("Failed to initialize dialog"), e);
        done(session);
    }
}

void
TechUpgradeSlave::done(game::Session& /*session*/)
{
    m_pTurn = 0;
    m_pShipList = 0;
    m_pRoot = 0;
    m_pContainer.reset();
    m_pAction.reset();
}

bool
TechUpgradeSlave::getStatus(Status& st) const
{
    if (m_pContainer.get() != 0 && m_pAction.get() != 0) {
        st.cost = m_pAction->costAction().getCost();
        st.available.set(Cost::Money, m_pContainer->getAmount(game::Element::Money));
        st.available.set(Cost::Supplies, m_pContainer->getAmount(game::Element::Supplies));
        st.remaining = m_pAction->costAction().getRemainingAmountAsCost();
        st.missing = m_pAction->costAction().getMissingAmountAsCost();
        for (size_t i = 0; i < NUM_TECH_AREAS; ++i) {
            st.min[i] = m_pAction->getMinTechLevel(TechLevel(i));
            st.max[i] = m_pAction->getMaxTechLevel(TechLevel(i));
            st.current[i] = m_pAction->getTechLevel(TechLevel(i));
        }
        st.valid = m_pAction->isValid();
        return true;
    } else {
        return false;
    }
}

void
TechUpgradeSlave::setOrder(const Order& o)
{
    if (m_pContainer.get() != 0 && m_pAction.get() != 0) {
        for (size_t i = 0; i < NUM_TECH_AREAS; ++i) {
            m_pAction->setTechLevel(TechLevel(i), o.values[i]);
        }
    }
}

void
TechUpgradeSlave::commit(game::Session& session)
{
    if (m_pContainer.get() != 0 && m_pAction.get() != 0) {
        try {
            m_pAction->commit();
        }
        catch (std::exception& e) {
            session.log().write(afl::sys::LogListener::Warn, LOG_NAME, session.translator()("Failed to commit transaction"), e);
        }
        done(session);
    }
}

/*
 *  TechUpgradeDialog
 */

TechUpgradeDialog::TechUpgradeDialog(ui::Root& root, util::RequestSender<game::Session> gameSender, game::Id_t pid)
    : m_root(root),
      m_loop(root),
      m_link(root),
      m_techLevels(),
      m_buttons(root),
      m_costDisplay(root, CostDisplay::Types_t(Cost::Money), client::proxy::ConfigurationProxy(gameSender).getNumberFormatter(m_link)),
      m_reply(root.engine().dispatcher(), *this),
      m_slave(gameSender, new TechUpgradeSlave(pid)),
      m_numOutstandingReplies(0),
      m_valid(false)
{ }

void
TechUpgradeDialog::run()
{
    // ex WTechUpgradeWindow::doWindow
    // Initialize and query initial state.
    // This may fail if the caller didn't honor preconditions.
    Status st;
    if (!queryStatus(st)) {
        return;
    }
    setStatus(st);

    // Build dialog
    static const char*const TECH_NAMES[] = {
        N_("Engines:"),
        N_("Hulls:"),
        N_("Beam Weapons:"),
        N_("Torpedoes:")
    };

    // Window [VBox]
    //   FocusableGroup [HBox]  \ 4x
    //     TechBar              /
    //   CostDisplay
    //   StandardDialogButtons
    afl::base::Deleter del;
    ui::Window& dlg(del.addNew(new ui::Window(_("Tech Upgrade"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5)));

    ui::widgets::FocusIterator& it(del.addNew(new ui::widgets::FocusIterator(ui::widgets::FocusIterator::Vertical | ui::widgets::FocusIterator::Tab)));
    for (size_t i = 0; i < NUM_TECH_AREAS; ++i) {
        m_techLevels[i].set(st.current[i]);
        m_techLevels[i].sig_change.add(this, &TechUpgradeDialog::onChange);
        client::widgets::TechBar& bar(del.addNew(new client::widgets::TechBar(m_root, m_techLevels[i], st.min[i], st.max[i], _(TECH_NAMES[i]))));
        ui::Widget& w = ui::widgets::FocusableGroup::wrapWidget(del, bar);
        dlg.add(w);
        it.add(w);
    }

    dlg.add(m_costDisplay);
    dlg.add(m_buttons);
    dlg.add(it);
    m_buttons.cancel().sig_fire.addNewClosure(m_loop.makeStop(0));
    m_buttons.ok().sig_fire.add(this, &TechUpgradeDialog::onOK);

    // Run
    dlg.pack();
    m_root.centerWidget(dlg);
    m_root.add(dlg);

    m_loop.run();
}

bool
TechUpgradeDialog::queryStatus(Status& st)
{
    class Query : public util::SlaveRequest<game::Session, TechUpgradeSlave> {
     public:
        Query(Status& st)
            : m_status(st), m_ok(false)
            { }
        virtual void handle(game::Session& /*session*/, TechUpgradeSlave& slave)
            { m_ok = slave.getStatus(m_status); }
        bool isOK() const
            { return m_ok; }
     private:
        Status& m_status;
        bool m_ok;
    };

    Query q(st);
    m_link.call(m_slave, q);
    return q.isOK();
}

void
TechUpgradeDialog::setStatus(const Status& st)
{
    // Update display
    m_costDisplay.setCost(st.cost);
    m_costDisplay.setAvailableAmount(st.available);
    m_costDisplay.setRemainingAmount(st.remaining);
    m_costDisplay.setMissingAmount(st.missing);

    // Update internal status
    m_valid = st.valid;
    if (m_numOutstandingReplies != 0) {
        --m_numOutstandingReplies;
    }

    // Update 'ok' button only when no replies are outstanding to avoid flicker
    if (m_numOutstandingReplies == 0) {
        m_buttons.ok().setState(ui::Widget::DisabledState, !m_valid);
    }
}

void
TechUpgradeDialog::onChange()
{
    // Response (Slave->Dialog)
    class Response : public util::Request<TechUpgradeDialog> {
     public:
        Response(const Status& st)
            : m_status(st)
            { }
        virtual void handle(TechUpgradeDialog& dlg)
            { dlg.setStatus(m_status); }
     private:
        Status m_status;
    };

    // Request (Dialog->Slave)
    class Request : public util::SlaveRequest<game::Session, TechUpgradeSlave> {
     public:
        Request(const Order& o, util::RequestSender<TechUpgradeDialog> reply)
            : m_order(o), m_reply(reply)
            { }
        virtual void handle(game::Session& /*session*/, TechUpgradeSlave& slave)
            {
                slave.setOrder(m_order);

                Status st;
                if (slave.getStatus(st)) {
                    m_reply.postNewRequest(new Response(st));
                }
            }
     private:
        Order m_order;
        util::RequestSender<TechUpgradeDialog> m_reply;
    };

    // Build order
    Order o;
    for (size_t i = 0; i < NUM_TECH_AREAS; ++i) {
        o.values[i] = m_techLevels[i].get();
    }

    // Send it
    ++m_numOutstandingReplies;
    m_slave.postNewRequest(new Request(o, m_reply.getSender()));
}

void
TechUpgradeDialog::onOK()
{
    // Request (Dialog->Slave)
    class Request : public util::SlaveRequest<game::Session, TechUpgradeSlave> {
     public:
        virtual void handle(game::Session& session, TechUpgradeSlave& slave)
            { slave.commit(session); }
    };

    if (m_numOutstandingReplies == 0 && m_valid) {
        // Asynchronous commit.
        // Destruction of the slave object is asynchronous as well and will be queued afterwards.
        m_slave.postNewRequest(new Request());
        m_loop.stop(0);
    }
}

void
client::dialogs::doTechUpgradeDialog(ui::Root& root, util::RequestSender<game::Session> gameSender, game::Id_t pid)
{
    TechUpgradeDialog(root, gameSender, pid).run();
}
