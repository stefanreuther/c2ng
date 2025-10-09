/**
  *  \file game/proxy/simulationadaptorfromsession.hpp
  *  \brief Class game::proxy::SimulationAdaptorFromSession
  */
#ifndef C2NG_GAME_PROXY_SIMULATIONADAPTORFROMSESSION_HPP
#define C2NG_GAME_PROXY_SIMULATIONADAPTORFROMSESSION_HPP

#include "afl/base/closure.hpp"
#include "game/session.hpp"
#include "game/proxy/simulationadaptor.hpp"

namespace game { namespace proxy {

    /** Functor for converting a Session into a SimulationAdaptor.
        Use as `gameSender.makeTemporary(new SimulationAdaptorFromSession())`. */
    class SimulationAdaptorFromSession : public afl::base::Closure<SimulationAdaptor* (Session&)> {
     public:
        SimulationAdaptor* call(Session& session);
    };

} }

#endif
