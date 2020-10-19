/**
  *  \file game/v3/commandextra.hpp
  *  \brief Class game::v3::CommandExtra
  */
#ifndef C2NG_GAME_V3_COMMANDEXTRA_HPP
#define C2NG_GAME_V3_COMMANDEXTRA_HPP

#include "afl/base/signalconnection.hpp"
#include "afl/container/ptrmap.hpp"
#include "afl/container/ptrvector.hpp"
#include "game/extra.hpp"
#include "game/turn.hpp"

namespace game { namespace v3 {

    class CommandContainer;
    class Command;

    /** Storage of Command/CommandContainer for a game turn.

        Presence of a CommandExtra means game supports the Command feature.
        TurnLoader must call CommandExtra::create(Turn&) (and nobody else).

        A CommandContainer may or may not be present for a player.
        If no CommandContainer is present, game code may call create() to make one.

        In addition to storing the commands, CommandExtra forwards command changes to the affected units. */
    class CommandExtra : public Extra {
     public:
        /** Constructor.
            \param parent Turn */
        explicit CommandExtra(Turn& parent);

        /** Destructor. */
        ~CommandExtra();

        /** Create CommandContainer for a player.
            Call when you add or modify a command.
            \param player Player
            \return CommandContainer */
        CommandContainer& create(int player);

        /** Get CommandContainer for a player.
            Call when you retrieve commands.
            \param player Player
            \return CommandContainer if present, 0 if player has no commands */
        CommandContainer* get(int player) const;

        /** Create CommandContainer for a turn.
            Call when you're a TurnLoader.
            \param parent Turn
            \return CommandExtra */
        static CommandExtra& create(Turn& parent);

        /** Get CommandContainer for a turn.
            Call when you retrieve commands.
            \return CommandExtra if present, 0 if player has no commands */
        static CommandExtra* get(Turn& parent);
        static const CommandExtra* get(const Turn& parent);

        /** Get CommandContainer for a player, given a turn.
            Call when you retrieve commands.
            This is a shortcut for the other get() functions
            \param parent Turn
            \param player Player
            \return CommandContainer if present, 0 if player has no commands */
        static CommandContainer* get(Turn& parent, int player);
        static const CommandContainer* get(const Turn& parent, int player);

     private:
        Turn& m_parent;
        afl::container::PtrMap<int, CommandContainer> m_commandContainers;
        afl::container::PtrVector<afl::base::SignalConnection> m_signalConnections;

        void onCommandChange(Command& cmd, bool added);
    };

} }

#endif
