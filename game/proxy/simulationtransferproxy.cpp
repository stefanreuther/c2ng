/**
  *  \file game/proxy/simulationtransferproxy.cpp
  *  \brief Class game::proxy::SimulationSetupProxy
  */

#include "game/proxy/simulationtransferproxy.hpp"
#include "game/game.hpp"
#include "game/root.hpp"
#include "game/sim/planet.hpp"
#include "game/sim/session.hpp"
#include "game/sim/sessionextra.hpp"
#include "game/sim/setup.hpp"
#include "game/sim/ship.hpp"
#include "game/sim/transfer.hpp"
#include "game/turn.hpp"
#include "util/request.hpp"

namespace {
    typedef util::Request<game::Session> Request_t;
}

game::proxy::SimulationTransferProxy::SimulationTransferProxy(util::RequestSender<Session> gameSender)
    : m_gameSender(gameSender)
{ }

game::proxy::SimulationTransferProxy::~SimulationTransferProxy()
{ }

bool
game::proxy::SimulationTransferProxy::hasObject(WaitIndicator& ind, Reference ref)
{
    class Task : public Request_t {
     public:
        Task(Reference ref)
            : m_reference(ref), m_result(false)
            { }
        virtual void handle(Session& session)
            { m_result = hasObject(session, m_reference); }
        bool getResult() const
            { return m_result; }
     private:
        Reference m_reference;
        bool m_result;
    };

    Task t(ref);
    ind.call(m_gameSender, t);
    return t.getResult();
}

bool
game::proxy::SimulationTransferProxy::copyObjectFromGame(WaitIndicator& ind, Reference ref)
{
    class Task : public Request_t {
     public:
        Task(Reference ref)
            : m_reference(ref), m_result(false)
            { }
        virtual void handle(Session& session)
            {
                m_result = copyObjectFromGame(session, m_reference);
                notify(session);
            }
        bool getResult() const
            { return m_result; }
     private:
        Reference m_reference;
        bool m_result;
    };

    Task t(ref);
    ind.call(m_gameSender, t);
    return t.getResult();
}

size_t
game::proxy::SimulationTransferProxy::copyObjectsFromGame(WaitIndicator& ind, const game::ref::List& list)
{
    class Task : public Request_t {
     public:
        Task(const game::ref::List& list)
            : m_list(list), m_result(0)
            { }
        virtual void handle(Session& session)
            {
                m_result = copyObjectsFromGame(session, m_list);
                notify(session);
            }
        size_t getResult() const
            { return m_result; }
     private:
        game::ref::List m_list;
        size_t m_result;
    };

    Task t(list);
    ind.call(m_gameSender, t);
    return t.getResult();
}

inline bool
game::proxy::SimulationTransferProxy::hasObject(Session& session, Reference ref)
{
    bool result = false;

    afl::base::Ref<game::sim::Session> sim = game::sim::getSimulatorSession(session);
    switch (ref.getType()) {
     case Reference::Ship:
        result = sim->setup().findShipById(ref.getId()) != 0;
        break;

     case Reference::Planet:
     case Reference::Starbase:
        if (const game::sim::Planet* pl = sim->setup().getPlanet()) {
            result = pl->getId() == ref.getId();
        }
        break;

     default:
        break;
    }
    return result;
}

bool
game::proxy::SimulationTransferProxy::copyObjectFromGame(Session& session, Reference ref)
{
    bool result = false;

    afl::base::Ref<game::sim::Session> sim = game::sim::getSimulatorSession(session);

    Game* g = session.getGame().get();
    Root* r = session.getRoot().get();
    game::spec::ShipList* sl = session.getShipList().get();
    if (g != 0 && r != 0 && sl != 0) {
        afl::base::Ptr<Turn> t = g->getViewpointTurn();
        if (t.get() != 0) {
            game::sim::Transfer transfer(g->shipScores(), g->planetScores(), *sl, r->hostConfiguration(), session.translator());
            switch (ref.getType()) {
             case Reference::Ship:
                if (game::map::Ship* in = t->universe().ships().get(ref.getId())) {
                    game::sim::Ship tmp;
                    result = transfer.copyShipFromGame(tmp, *in)
                        && sim->setup().addShip(tmp) != 0;
                }
                break;

             case Reference::Planet:
             case Reference::Starbase:
                if (game::map::Planet* in = t->universe().planets().get(ref.getId())) {
                    game::sim::Planet tmp;
                    result = transfer.copyPlanetFromGame(tmp, *in)
                        && sim->setup().addPlanet(tmp);
                }
                break;

             default:
                break;
            }
        }
    }
    return result;
}

inline size_t
game::proxy::SimulationTransferProxy::copyObjectsFromGame(Session& session, const game::ref::List& list)
{
    size_t result = 0;
    for (size_t i = 0, n = list.size(); i < n; ++i) {
        if (copyObjectFromGame(session, list[i])) {
            ++result;
        }
    }
    return result;
}

void
game::proxy::SimulationTransferProxy::notify(Session& session)
{
    game::sim::getSimulatorSession(session)->setup().notifyListeners();
}
