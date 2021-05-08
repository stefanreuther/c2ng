/**
  *  \file client/dialogs/simulationtransfer.cpp
  *  \brief Transferring Units into the Simulation (SimulationTransferProxy usecases)
  */

#include "client/dialogs/simulationtransfer.hpp"
#include "game/proxy/simulationtransferproxy.hpp"
#include "client/downlink.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "afl/string/format.hpp"

using afl::string::Format;
using game::Reference;
using game::proxy::SimulationTransferProxy;
using ui::dialogs::MessageBox;

void
client::dialogs::addObjectToSimulation(ui::Root& root, util::RequestSender<game::Session> gameSender, game::Reference ref, bool ask, afl::string::Translator& tx)
{
    // ex doShipAddToSim, doPlanetAddToSim
    SimulationTransferProxy proxy(gameSender);
    Downlink link(root, tx);
    if (ask && ref.getType() == Reference::Ship && proxy.hasObject(link, ref)) {
        // Traditionally, PCC honors the 'ask' flag only for ships.
        if (!MessageBox(Format(tx("The simulation already contains a ship with Id #%d. Replace it?"), ref.getId()),
                        tx("Add to Simulation"),
                        root).doYesNoDialog(tx)) {
            return;
        }
    }

    if (!proxy.copyObjectFromGame(link, ref)) {
        MessageBox(tx("Unit could not be added to simulation."),
                   tx("Add to Simulation"),
                   root).doOkDialog(tx);
    }
}

void
client::dialogs::addObjectsToSimulation(ui::Root& root, util::RequestSender<game::Session> gameSender, const game::ref::List& list, afl::string::Translator& tx)
{
    SimulationTransferProxy proxy(gameSender);
    Downlink link(root, tx);

    size_t n = proxy.copyObjectsFromGame(link, list);

    MessageBox(Format(tx("%d (of %d) units added to the simulation."), n, list.size()),
               tx("Add to Simulation"),
               root).doOkDialog(tx);
}
