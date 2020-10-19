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

class game::proxy::BuildStarbaseProxy::Trampoline : public util::SlaveObject<Session> {
 public:
    virtual void init(Session&)
        { }

    virtual void done(Session&)
        {
            m_action.reset();
            m_container.reset();
        }

    void init(Session& session, Id_t id, Status& status)
        {
            try {
                // Preconditions
                Root& root = game::actions::mustHaveRoot(session);
                Game& game = game::actions::mustHaveGame(session);

                // Fetch planet
                game::map::Planet& planet = game::actions::mustExist(game.currentTurn().universe().planets().get(id));

                // Construct stuff
                bool wantBase = !planet.isBuildingBase();
                m_container.reset(new game::map::PlanetStorage(planet, root.hostConfiguration()));
                m_action.reset(new game::actions::BuildStarbase(planet, *m_container, wantBase, session.translator(), root.hostConfiguration()));

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
            catch (Exception& e) {
                status.mode = Error;
                status.errorMessage = e.getUserError();
            }
            catch (std::exception& e) {
                status.mode = Error;
                status.errorMessage = e.what();
            }
        }

    void commit(Session& session)
        {
            try {
                if (m_action.get() != 0) {
                    m_action->commit();
                }
            }
            catch (std::exception& e) {
                // FIXME: log it
                (void) e;
                (void) session;
            }
        }

 private:
    std::auto_ptr<CargoContainer> m_container;
    std::auto_ptr<game::actions::BuildStarbase> m_action;
};

game::proxy::BuildStarbaseProxy::BuildStarbaseProxy(util::RequestSender<Session> gameSender)
    : m_sender(gameSender, new Trampoline())
{ }

game::proxy::BuildStarbaseProxy::~BuildStarbaseProxy()
{ }

void
game::proxy::BuildStarbaseProxy::init(WaitIndicator& link, Id_t id, Status& status)
{
    class Task : public util::SlaveRequest<Session, Trampoline> {
     public:
        Task(Id_t id, Status& status)
            : m_id(id), m_status(status)
            { }
        virtual void handle(Session& session, Trampoline& tpl)
            { tpl.init(session, m_id, m_status); }
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
    class Task : public util::SlaveRequest<Session, Trampoline> {
     public:
        virtual void handle(Session& session, Trampoline& tpl)
            { tpl.commit(session); }
    };
    Task t;
    link.call(m_sender, t);
}
