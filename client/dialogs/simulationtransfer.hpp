/**
  *  \file client/dialogs/simulationtransfer.hpp
  *  \brief Transferring Units into the Simulation (SimulationTransferProxy usecases)
  */
#ifndef C2NG_CLIENT_DIALOGS_SIMULATIONTRANSFER_HPP
#define C2NG_CLIENT_DIALOGS_SIMULATIONTRANSFER_HPP

#include "util/requestsender.hpp"
#include "game/session.hpp"
#include "ui/root.hpp"
#include "game/types.hpp"
#include "game/ref/list.hpp"
#include "game/reference.hpp"

namespace client { namespace dialogs {

    /** Add object to simulation (copyObjectFromGame), UI part.
        \param root UI root
        \param gameSender Game sender
        \param ref        Object to add
        \param ask        true to ask for replacement
        \param tx         Translator */
    void addObjectToSimulation(ui::Root& root, util::RequestSender<game::Session> gameSender, game::Reference ref, bool ask, afl::string::Translator& tx);

    /** Add objects to simulation (copyObjectsFromGame), UI part.
        \param root UI root
        \param gameSender Game sender
        \param list       List of objects to add
        \param tx         Translator */
    void addObjectsToSimulation(ui::Root& root, util::RequestSender<game::Session> gameSender, const game::ref::List& list, afl::string::Translator& tx);

} }

#endif
