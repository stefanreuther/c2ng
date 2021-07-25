/**
  *  \file client/dialogs/classicvcrobject.hpp
  *  \brief Classic VCR Object Information Dialog
  */
#ifndef C2NG_CLIENT_DIALOGS_CLASSICVCROBJECT_HPP
#define C2NG_CLIENT_DIALOGS_CLASSICVCROBJECT_HPP

#include "afl/string/translator.hpp"
#include "game/proxy/vcrdatabaseproxy.hpp"
#include "game/reference.hpp"
#include "game/session.hpp"
#include "ui/root.hpp"

namespace client { namespace dialogs {

    /** Show Classic VCR Object Information Dialog.
        Displays details about an object and allows user to switch sides.
        \param root        Root
        \param tx          Translator
        \param gameSender  Game sender (for ConfigurationProxy)
        \param proxy       VcrDatabaseProxy to use
        \param side        Initial side to show
        \return If user chose to go to an object's control screen, a reference to it. */
    game::Reference doClassicVcrObjectInfoDialog(ui::Root& root, afl::string::Translator& tx, util::RequestSender<game::Session> gameSender, game::proxy::VcrDatabaseProxy& proxy, size_t side);

    /** Add ship to simulation.
        This is a user interface macro to call VcrDatabaseProxy::addToSimulation(); it displays the result as a message box.
        \param ind    WaitIndicator
        \param proxy  Proxy
        \param hullNr Hull number (parameter to VcrDatabaseProxy::addToSimulation)
        \param after  After fight? (parameter to VcrDatabaseProxy::addToSimulation)
        \param root   UI root
        \param tx     Translator */
    void addToSimulation(game::proxy::WaitIndicator& ind, game::proxy::VcrDatabaseProxy& proxy, int hullNr, bool after, ui::Root& root, afl::string::Translator& tx);

} }


#endif
