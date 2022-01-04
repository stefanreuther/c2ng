/**
  *  \file game/proxy/buildstarbaseproxy.cpp
  *  \brief Class game::proxy::BuildStarbaseProxy
  */

#include "game/proxy/buildstarbaseproxy.hpp"
#include "game/actions/buildstarbase.hpp"
#include "game/actions/preconditions.hpp"
#include "game/exception.hpp"
#include "game/game.hpp"
#include "game/map/planetstorage.hpp"
#include "game/root.hpp"
#include "game/turn.hpp"

class game::proxy::BuildStarbaseProxy::Trampoline {
 public:
    Trampoline(Session& session)
        : m_session(session)
        { }

    void init(Id_t id, Status& status)
        {
            try {
                // Preconditions
                Root& root = game::actions::mustHaveRoot(m_session);
                Game& game = game::actions::mustHaveGame(m_session);

                // Fetch planet
                game::map::Planet& planet = game::actions::mustExist(game.currentTurn().universe().planets().get(id));

                // Construct stuff
                bool wantBase = !planet.isBuildingBase();
                m_container.reset(new game::map::PlanetStorage(planet, root.hostConfiguration()));
                m_action.reset(new game::actions::BuildStarbase(planet, *m_container, wantBase, root.hostConfiguration()));

                // Produce result
                if (wantBase) {
                    status.available = m_action->costAction().getAvailableAmountAsCost();
                    status.cost      = m_action->costAction().getCost();
                    status.remaining = m_action->costAction().getRemainingAmountAsCost();
                    status.missing   = m_action->costAction().getMissingAmountAsCost();
                    status.mode = m_action->isValid() ? CanBuild : CannotBuild;
                } else {
                    status.mode = CanCancel;
                }
            }
            catch (std::exception& e) {
                status.mode = Error;
                status.errorMessage = e.what();
            }
        }

    void commit()
        {
            try {
                if (m_action.get() != 0) {
                    m_action->commit();
                }
            }
            catch (std::exception& e) {
                // FIXME: log it
                (void) e;
            }
        }

 private:
    Session& m_session;
    std::auto_ptr<CargoContainer> m_container;
    std::auto_ptr<game::actions::BuildStarbase> m_action;
};

class game::proxy::BuildStarbaseProxy::TrampolineFromSession : public afl::base::Closure<Trampoline*(Session&)> {
 public:
    virtual Trampoline* call(Session& session)
        { return new Trampoline(session); }
};



game::proxy::BuildStarbaseProxy::BuildStarbaseProxy(util::RequestSender<Session> gameSender)
    : m_sender(gameSender.makeTemporary(new TrampolineFromSession()))
{ }

game::proxy::BuildStarbaseProxy::~BuildStarbaseProxy()
{ }

void
game::proxy::BuildStarbaseProxy::init(WaitIndicator& link, Id_t id, Status& status)
{
    class Task : public util::Request<Trampoline> {
     public:
        Task(Id_t id, Status& status)
            : m_id(id), m_status(status)
            { }
        virtual void handle(Trampoline& tpl)
            { tpl.init(m_id, m_status); }
     private:
        Id_t m_id;
        Status& m_status;
    };
    Task t(id, status);
    link.call(m_sender, t);
}

void
game::proxy::BuildStarbaseProxy::commit(WaitIndicator& link)
{
    class Task : public util::Request<Trampoline> {
     public:
        virtual void handle(Trampoline& tpl)
            { tpl.commit(); }
    };
    Task t;
    link.call(m_sender, t);
}
