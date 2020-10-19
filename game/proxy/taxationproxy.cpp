/**
  *  \file game/proxy/taxationproxy.cpp
  *  \brief Class game::proxy::TaxationProxy
  */

#include "game/proxy/taxationproxy.hpp"
#include "afl/string/format.hpp"
#include "game/actions/preconditions.hpp"
#include "game/game.hpp"
#include "game/map/planet.hpp"
#include "game/map/universe.hpp"
#include "game/root.hpp"
#include "game/tables/happinesschangename.hpp"
#include "game/tables/nativeracename.hpp"
#include "game/turn.hpp"
#include "util/slaveobject.hpp"

using game::actions::TaxationAction;

/*
 *  Trampoline: contains the transaction and event responder
 */

class game::proxy::TaxationProxy::Trampoline : public util::SlaveObject<Session> {
 public:
    Trampoline(Id_t planetId, util::RequestSender<TaxationProxy> reply)
        : m_planetId(planetId),
          m_reply(reply),
          m_pSession(),
          m_action(),
          conn_change()
        { }

    void init(Session& session)
        {
            // Remember the session
            m_pSession = &session;

            // Create an action
            Game* g = session.getGame().get();
            Root* r = session.getRoot().get();
            if (g != 0 && r != 0) {
                Turn* t = g->getViewpointTurn().get();
                if (t != 0) {
                    game::map::Planet* p = t->universe().planets().get(m_planetId);
                    if (p != 0) {
                        m_action.reset(new TaxationAction(*p, r->hostConfiguration(), r->hostVersion()));
                    }
                }
            }

            // Set up signals
            if (m_action.get() != 0) {
                conn_change = m_action->sig_change.add(this, &Trampoline::onChange);
            }
        }

    void done(Session& /*session*/)
        {
            conn_change.disconnect();
            m_action.reset();
            m_pSession = 0;
        }

    void onChange()
        {
            class Reply : public util::Request<TaxationProxy> {
             public:
                Reply(Trampoline& tr)
                    { tr.describe(m_status); }

                void handle(TaxationProxy& proxy)
                    { proxy.sig_change.raise(m_status); }

             private:
                Status m_status;
            };
            m_reply.postNewRequest(new Reply(*this));
        }

    void describe(Status& out)
        {
            if (m_action.get() != 0) {
                out.valid = true;
                describe(out.colonists, *m_action, TaxationAction::Colonists);
                describe(out.natives,   *m_action, TaxationAction::Natives);
            } else {
                out.valid = false;
                out.colonists = AreaStatus();
                out.natives = AreaStatus();
            }
        }

    TaxationAction* action() const
        { return m_action.get(); }

 private:
    const Id_t m_planetId;
    util::RequestSender<TaxationProxy> m_reply;
    Session* m_pSession;
    std::auto_ptr<TaxationAction> m_action;
    afl::base::SignalConnection conn_change;

    void describe(AreaStatus& out, const TaxationAction& in, Area_t a)
        {
            afl::string::Translator& tx = m_pSession->translator();
            Root& root = game::actions::mustHaveRoot(*m_pSession);

            out.available = in.isAvailable(a);
            out.tax = in.getTax(a);
            out.change = in.getHappinessChange(a);
            out.changeLabel = game::tables::HappinessChangeName(tx)(out.change);
            out.description = in.describe(a, tx, root.userConfiguration().getNumberFormatter());
            if (a == TaxationAction::Colonists) {
                int owner = 0;
                in.planet().getOwner(owner);
                out.title = afl::string::Format(tx("%s colony"), root.playerList().getPlayerName(owner, Player::AdjectiveName));
            } else {
                out.title = afl::string::Format(tx("%s natives"), game::tables::NativeRaceName(tx)(in.planet().getNativeRace().orElse(0)));
            }
        }
};


/*
 *  TaxationProxy
 */

// Constructor.
game::proxy::TaxationProxy::TaxationProxy(util::RequestDispatcher& reply,
                                            util::RequestSender<Session> gameSender,
                                            Id_t planetId)
    : m_reply(reply, *this),
      m_trampoline(gameSender, new Trampoline(planetId, m_reply.getSender()))
{ }

// Destructor.
game::proxy::TaxationProxy::~TaxationProxy()
{
}

// Get status.
void
game::proxy::TaxationProxy::getStatus(WaitIndicator& link, Status& out)
{
    class Task : public util::SlaveRequest<Session, Trampoline> {
     public:
        Task(Status& out)
            : m_out(out)
            { }
        void handle(Session&, Trampoline& tr)
            { tr.describe(m_out); }
     private:
        Status& m_out;
    };
    Task t(out);
    if (!link.call(m_trampoline, t)) {
        out.valid = false;
    }
}

// Set number of buildings (mines + factories).
void
game::proxy::TaxationProxy::setNumBuildings(int n)
{
    class Task : public util::SlaveRequest<Session, Trampoline> {
     public:
        Task(int n)
            : m_numBuildings(n)
            { }
        void handle(Session&, Trampoline& tr)
            {
                if (TaxationAction* ta = tr.action()) {
                    ta->setNumBuildings(m_numBuildings);
                }
            }
     private:
        int m_numBuildings;
    };
    m_trampoline.postNewRequest(new Task(n));
}

// Set tax rate, limit to valid range.
void
game::proxy::TaxationProxy::setTaxLimited(Area_t a, int value)
{
    class Task : public util::SlaveRequest<Session, Trampoline> {
     public:
        Task(Area_t a, int value)
            : m_area(a), m_value(value)
            { }
        void handle(Session&, Trampoline& tr)
            {
                if (TaxationAction* ta = tr.action()) {
                    ta->setTaxLimited(m_area, m_value);
                }
            }
     private:
        Area_t m_area;
        int m_value;
    };
    m_trampoline.postNewRequest(new Task(a, value));
}

// Change tax rate for better/worse revenue.
void
game::proxy::TaxationProxy::changeRevenue(Area_t a, Direction_t d)
{
    class Task : public util::SlaveRequest<Session, Trampoline> {
     public:
        Task(Area_t a, Direction_t d)
            : m_area(a), m_direction(d)
            { }
        void handle(Session&, Trampoline& tr)
            {
                if (TaxationAction* ta = tr.action()) {
                    ta->changeRevenue(m_area, m_direction);
                }
            }
     private:
        Area_t m_area;
        Direction_t m_direction;
    };
    m_trampoline.postNewRequest(new Task(a, d));
}

// Change tax rate.
void
game::proxy::TaxationProxy::changeTax(Area_t a, int delta)
{
    class Task : public util::SlaveRequest<Session, Trampoline> {
     public:
        Task(Area_t a, int delta)
            : m_area(a), m_delta(delta)
            { }
        void handle(Session&, Trampoline& tr)
            {
                if (TaxationAction* ta = tr.action()) {
                    ta->changeTax(m_area, m_delta);
                }
            }
     private:
        Area_t m_area;
        int m_delta;
    };
    m_trampoline.postNewRequest(new Task(a, delta));
}

// Set safe-tax for areas.
void
game::proxy::TaxationProxy::setSafeTax(Areas_t as)
{
    class Task : public util::SlaveRequest<Session, Trampoline> {
     public:
        Task(Areas_t as)
            : m_areas(as)
            { }
        void handle(Session&, Trampoline& tr)
            {
                if (TaxationAction* ta = tr.action()) {
                    ta->setSafeTax(m_areas);
                }
            }
     private:
        Areas_t m_areas;
    };
    m_trampoline.postNewRequest(new Task(as));
}

// Revert tax rates.
void
game::proxy::TaxationProxy::revert(Areas_t as)
{
    class Task : public util::SlaveRequest<Session, Trampoline> {
     public:
        Task(Areas_t as)
            : m_areas(as)
            { }
        void handle(Session&, Trampoline& tr)
            {
                if (TaxationAction* ta = tr.action()) {
                    ta->revert(m_areas);
                }
            }
     private:
        Areas_t m_areas;
    };
    m_trampoline.postNewRequest(new Task(as));
}

// Commit transaction.
void
game::proxy::TaxationProxy::commit()
{
    class Task : public util::SlaveRequest<Session, Trampoline> {
     public:
        void handle(Session&, Trampoline& tr)
            {
                if (TaxationAction* ta = tr.action()) {
                    ta->commit();
                }
            }
    };
    m_trampoline.postNewRequest(new Task());
}
