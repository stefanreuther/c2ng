/**
  *  \file game/proxy/simulationexportadaptor.hpp
  *  \brief ExportAdaptor for Battle Simulation
  */
#ifndef C2NG_GAME_PROXY_SIMULATIONEXPORTADAPTOR_HPP
#define C2NG_GAME_PROXY_SIMULATIONEXPORTADAPTOR_HPP

#include "afl/base/closure.hpp"
#include "game/proxy/exportadaptor.hpp"
#include "game/proxy/simulationadaptor.hpp"

namespace game { namespace proxy {

    typedef afl::base::Closure<ExportAdaptor*(SimulationAdaptor&)> SimulationExportAdaptor_t;

    /** Make (creator for) simulation export adaptor.
        Use with RequestSender<SimulationAdaptor>::makeTemporary to create a RequestSender<ExportAdaptor>
        that exports the similation setup.

        @return newly-allocated closure
        @see game::interface::SimObjectContext */
    SimulationExportAdaptor_t* makeSimulationExportAdaptor();

} }

#endif
