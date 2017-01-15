/**
  *  \file game/v3/commandextra.hpp
  */
#ifndef C2NG_GAME_V3_COMMANDEXTRA_HPP
#define C2NG_GAME_V3_COMMANDEXTRA_HPP

#include "game/extra.hpp"
#include "game/turn.hpp"
#include "afl/container/ptrmap.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/base/signalconnection.hpp"

namespace game { namespace v3 {

    class CommandContainer;
    class Command;

    class CommandExtra : public Extra {
     public:
        CommandExtra(Turn& parent);
        ~CommandExtra();

        CommandContainer& create(int player);
        CommandContainer* get(int player) const;

        static CommandExtra& create(Turn& parent);
        static CommandExtra* get(Turn& parent);

     private:
        Turn& m_parent;
        afl::container::PtrMap<int, CommandContainer> m_commandContainers;
        afl::container::PtrVector<afl::base::SignalConnection> m_signalConnections;

        void onCommandChange(Command& cmd, bool added);
    };

} }

#endif
