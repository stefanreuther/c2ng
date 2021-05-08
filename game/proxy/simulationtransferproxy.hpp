/**
  *  \file game/proxy/simulationtransferproxy.hpp
  *  \brief Class game::proxy::SimulationSetupProxy
  */
#ifndef C2NG_GAME_PROXY_SIMULATIONTRANSFERPROXY_HPP
#define C2NG_GAME_PROXY_SIMULATIONTRANSFERPROXY_HPP

#include "game/proxy/waitindicator.hpp"
#include "game/ref/list.hpp"
#include "game/reference.hpp"
#include "game/session.hpp"
#include "game/types.hpp"
#include "util/requestsender.hpp"

namespace game { namespace proxy {

    /** Transferring units into a simulation.
        Whereas SimulationSetupProxy implements the view from the simulation (pull data from a game),
        this one implements the view from the game (push data into the simulation).

        Methods are synchronous, bidirectional to give status return. */
    class SimulationTransferProxy {
     public:
        explicit SimulationTransferProxy(util::RequestSender<Session> gameSender);
        ~SimulationTransferProxy();

        /** Check whether object is contained in simulation.
            \param ind  WaitIndicator for UI synchronisation
            \param ref  Reference to object to check
            \return true an object of the given type/Id is part of simulation */
        bool hasObject(WaitIndicator& ind, Reference ref);

        /** Copy object from game into simulation.
            \param ind  WaitIndicator for UI synchronisation
            \param ref  Reference to object
            \return true on success
            \see game::sim::Transfer::copyShipFromGame, game::sim::Transfer::copyPlanetFromGame */
        bool copyObjectFromGame(WaitIndicator& ind, Reference ref);

        /** Copy objects from game into simulation.
            \param ind  WaitIndicator for UI synchronisation
            \param list List of objects
            \return number of objects successfully copied */
        size_t copyObjectsFromGame(WaitIndicator& ind, const game::ref::List& list);

     private:
        util::RequestSender<Session> m_gameSender;

        static bool hasObject(Session& session, Reference ref);
        static bool copyObjectFromGame(Session& session, Reference ref);
        static size_t copyObjectsFromGame(Session& session, const game::ref::List& list);
        static void notify(Session& session);
    };

} }

#endif
