/**
  *  \file game/proxy/maintenanceproxy.hpp
  *  \brief Class game::proxy::MaintenanceProxy
  */
#ifndef C2NG_GAME_PROXY_MAINTENANCEPROXY_HPP
#define C2NG_GAME_PROXY_MAINTENANCEPROXY_HPP

#include "afl/base/signal.hpp"
#include "game/playerarray.hpp"
#include "game/playerset.hpp"
#include "game/proxy/maintenanceadaptor.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestsender.hpp"

namespace game { namespace proxy {

    class WaitIndicator;

    /** Directory Maintenance Proxy.
        This bidirectional proxy allows controlling directory maintenance operations: maketurn, unpack, sweep.

        All operations have a synchronous "prepare" action and an asynchronous "start" action.
        The "prepare" action provides initial parameters, and a validity flag.
        The "start" action must not be invoked if the validity flag is not set
        (this means the game side is not responsive).

        After the "start" action, messages will be generated using sig_message,
        completion will be signalled using sig_actionComplete.

        Access to the directory being worked on is provided by a MaintenanceAdaptor.
        MaintenanceProxy internally uses a game::v3::DirectoryScanner to interpret the directory.
        (Because directory maintenance is invoked from the browser,
        an alternative solution would be to retrieve this information from the browser.
        However, this would require a much larger interface.)

        Directory maintenance operations will not log to a system console;
        instead, they will produce messages using sig_message.
        User-interface shall render those to the player. */
    class MaintenanceProxy {
     public:
        /** Status of "maketurn" operation.
            For now, we always create all turn files as a group, and select the players internally.
            Therefore, the only status we need to track is validity. */
        struct MaketurnStatus {
            bool valid;                           ///< Validity flag.
            PlayerSet_t availablePlayers;         ///< Available players (HaveUnpacked).
            MaketurnStatus()
                : valid(), availablePlayers()
                { }
        };

        /** Status of "unpack" operation. */
        struct UnpackStatus {
            bool valid;                           ///< Validity flag.
            PlayerSet_t allPlayers;               ///< Set of all players.
            PlayerSet_t availablePlayers;         ///< Available players (selectable, HaveResult).
            PlayerSet_t selectedPlayers;          ///< Initially selected players (unpacked, HaveUnpacked).
            PlayerSet_t turnFilePlayers;          ///< Players that have a turn file (HaveTurn).
            PlayerArray<String_t> playerNames;    ///< Player names.
            UnpackStatus()
                : valid(), allPlayers(), availablePlayers(), selectedPlayers(), turnFilePlayers(), playerNames()
                { }
        };

        /** Status of "sweep" operation. */
        struct SweepStatus {
            bool valid;                           ///< Validity flag.
            PlayerSet_t allPlayers;               ///< All players (selectable).
            PlayerSet_t selectedPlayers;          ///< Initially selected players (conflicting races).
            PlayerArray<String_t> playerNames;    ///< Player names.
            SweepStatus()
                : valid(), allPlayers(), selectedPlayers(), playerNames()
                { }
        };

        /** Constructor.
            @param sender Sender
            \param reply  RequestDispatcher to receive updates in this thread */
        MaintenanceProxy(util::RequestSender<MaintenanceAdaptor> sender, util::RequestDispatcher& reply);

        /** Destructor. */
        ~MaintenanceProxy();

        /** Prepare "maketurn" operation.
            @param ind WaitIndicator for UI synchronisation
            @return MaketurnStatus structure */
        MaketurnStatus prepareMaketurn(WaitIndicator& ind);

        /** Start "maketurn" operation.
            The operation's completion will be signalled using sig_actionComplete.
            @param players Players for which to make turns */
        void startMaketurn(PlayerSet_t players);

        /** Prepare "unpack" operation.
            @param ind WaitIndicator for UI synchronisation
            @return UnpackStatus structure */
        UnpackStatus prepareUnpack(WaitIndicator& ind);

        /** Start "unpack" operation.
            The operation's completion will be signalled using sig_actionComplete.
            @param players        Players to unpack
            @param uncompileTurns true to unpack matching turn files */
        void startUnpack(PlayerSet_t players, bool uncompileTurns);

        /** Prepare "sweep" operation.
            @param ind WaitIndicator for UI synchronisation
            @return SweepStatus structure */
        SweepStatus prepareSweep(WaitIndicator& ind);

        /** Start "sweep" operation.
            The operation's completion will be signalled using sig_actionComplete.
            @param players        Players to sweep, see game::maint::Sweeper::setPlayers()
            @param eraseDatabase  Erase-database flag, see game::maint::Sweeper::setEraseDatabase() */
        void startSweep(PlayerSet_t players, bool eraseDatabase);

        /** Signal: action complete. */
        afl::base::Signal<void()> sig_actionComplete;

        /** Signal: status message.
            @param message Message text */
        afl::base::Signal<void(String_t)> sig_message;

     private:
        class Trampoline;
        class TrampolineFromAdaptor;

        util::RequestReceiver<MaintenanceProxy> m_receiver;
        util::RequestSender<Trampoline> m_sender;

        void emitActionComplete();
        void emitMessage(String_t msg);
    };

} }

#endif
