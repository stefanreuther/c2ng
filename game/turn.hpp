/**
  *  \file game/turn.hpp
  */
#ifndef C2NG_GAME_TURN_HPP
#define C2NG_GAME_TURN_HPP

#include "game/map/universe.hpp"
#include "afl/container/ptrmap.hpp"
#include "game/msg/inbox.hpp"
#include "game/vcr/database.hpp"
#include "game/extracontainer.hpp"

namespace game {

    // This is the successor to GGameTurn.

    class Turn {
     public:
        Turn();
        ~Turn();

        game::map::Universe& universe();
        const game::map::Universe& universe() const;

        void setBattles(afl::base::Ptr<game::vcr::Database> battles);
        afl::base::Ptr<game::vcr::Database> getBattles() const;

        game::msg::Inbox& inbox();
        const game::msg::Inbox& inbox() const;

        ExtraContainer<Turn>& extras();
        const ExtraContainer<Turn>& extras() const;

        void notifyListeners();

     private:
        game::map::Universe m_universe;

        ExtraContainer<Turn> m_extras;

        game::msg::Inbox m_inbox;

        afl::base::Ptr<game::vcr::Database> m_battles;

        // (Inbox)
        // (Outbox)
        // (Alliances)
        // (Battles)
        // (Commands)
        // (Gen)
        // (data set)                          whose players' data we have
    };
    
}

#endif
