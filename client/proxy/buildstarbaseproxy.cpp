/**
  *  \file client/proxy/buildstarbaseproxy.cpp
  */

#include "client/proxy/buildstarbaseproxy.hpp"
#include "game/actions/buildstarbase.hpp"
#include "game/exception.hpp"
#include "game/map/planetstorage.hpp"
#include "game/actions/preconditions.hpp"
#include "game/game.hpp"
#include "game/root.hpp"
#include "game/turn.hpp"

class client::proxy::BuildStarbaseProxy::Trampoline : public util::SlaveObject<game::Session> {
 public:
    virtual void init(game::Session&)
        { }

    virtual void done(game::Session&)
        {
            m_action.reset();
            m_container.reset();
        }

    void init(game::Session& session, game::Id_t id, Status& status)
        {
            try {
                // Preconditions
                game::Root& root = game::actions::mustHaveRoot(session);
                game::Game& game = game::actions::mustHaveGame(session);

                // Fetch planet
                game::map::Planet& planet = game::actions::mustExist(game.currentTurn().universe().planets().get(id));

                // Construct stuff
                bool wantBase = !planet.isBuildingBase();
                m_container.reset(new game::map::PlanetStorage(planet, session.interface(), root.hostConfiguration()));
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
            catch (game::Exception& e) {
                status.mode = Error;
                status.errorMessage = e.getUserError();
            }
            catch (std::exception& e) {
                status.mode = Error;
                status.errorMessage = e.what();
            }
        }

    void commit(game::Session& session)
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
    std::auto_ptr<game::CargoContainer> m_container;
    std::auto_ptr<game::actions::BuildStarbase> m_action;
};

client::proxy::BuildStarbaseProxy::BuildStarbaseProxy(util::RequestSender<game::Session> gameSender)
    : m_sender(gameSender, new Trampoline())
{ }

client::proxy::BuildStarbaseProxy::~BuildStarbaseProxy()
{ }

void
client::proxy::BuildStarbaseProxy::init(Downlink& link, game::Id_t id, Status& status)
{
    class Task : public util::SlaveRequest<game::Session, Trampoline> {
     public:
        Task(game::Id_t id, Status& status)
            : m_id(id), m_status(status)
            { }
        virtual void handle(game::Session& session, Trampoline& tpl)
            { tpl.init(session, m_id, m_status); }
     private:
        game::Id_t m_id;
        Status& m_status;
    };
    Task t(id, status);
    link.call(m_sender, t);
}

void
client::proxy::BuildStarbaseProxy::commit(Downlink& link)
{
    class Task : public util::SlaveRequest<game::Session, Trampoline> {
     public:
        virtual void handle(game::Session& session, Trampoline& tpl)
            { tpl.commit(session); }
    };
    Task t;
    link.call(m_sender, t);
}
