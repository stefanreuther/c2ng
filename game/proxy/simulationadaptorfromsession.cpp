/**
  *  \file game/proxy/simulationadaptorfromsession.cpp
  *  \brief Class game::proxy::SimulationAdaptorFromSession
  */

#include "game/proxy/simulationadaptorfromsession.hpp"
#include "game/sim/sessionextra.hpp"
#include "game/game.hpp"

game::proxy::SimulationAdaptor*
game::proxy::SimulationAdaptorFromSession::call(Session& session)
{
    class Adaptor : public SimulationAdaptor {
     public:
        Adaptor(Session& session)
            : m_session(session)
            { }
        game::sim::Session& simSession()
            { return *game::sim::getSimulatorSession(m_session); }
        virtual afl::base::Ptr<const Root> getRoot() const
            { return m_session.getRoot(); }
        virtual afl::base::Ptr<const game::spec::ShipList> getShipList() const
            { return m_session.getShipList(); }
        virtual const TeamSettings* getTeamSettings() const
            {
                Game* g = m_session.getGame().get();
                return g != 0 ? &g->teamSettings() : 0;
            }
        virtual afl::string::Translator& translator()
            { return m_session.translator(); }
        virtual afl::sys::LogListener& log()
            { return m_session.log(); }
        virtual afl::io::FileSystem& fileSystem()
            { return m_session.world().fileSystem(); }
        virtual bool isGameObject(const game::vcr::Object& obj) const
            {
                game::Game* g = m_session.getGame().get();
                return g != 0 && g->isGameObject(obj, getShipList()->hulls());
            }
        virtual util::RandomNumberGenerator& rng()
            { return m_session.rng(); }
        virtual size_t getNumProcessors() const
            { return m_session.getSystemInformation().numProcessors; }
     private:
        Session& m_session;
    };
    return new Adaptor(session);
}
