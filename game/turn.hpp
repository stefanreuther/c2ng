/**
  *  \file game/turn.hpp
  */
#ifndef C2NG_GAME_TURN_HPP
#define C2NG_GAME_TURN_HPP

#include "afl/base/refcounted.hpp"
#include "afl/container/ptrmap.hpp"
#include "game/extracontainer.hpp"
#include "game/map/universe.hpp"
#include "game/msg/inbox.hpp"
#include "game/msg/outbox.hpp"
#include "game/timestamp.hpp"
#include "game/vcr/database.hpp"

namespace game {

    // This is the successor to GGameTurn.

    class Turn : public afl::base::RefCounted {
     public:
        Turn();
        ~Turn();

        void setTurnNumber(int turnNumber);
        int getTurnNumber() const;

        void setDatabaseTurnNumber(int turnNumber);
        int getDatabaseTurnNumber() const;

        void setTimestamp(const Timestamp& ts);
        Timestamp getTimestamp() const;

        game::map::Universe& universe();
        const game::map::Universe& universe() const;

        void setBattles(afl::base::Ptr<game::vcr::Database> battles);
        afl::base::Ptr<game::vcr::Database> getBattles() const;

        game::msg::Inbox& inbox();
        const game::msg::Inbox& inbox() const;

        game::msg::Outbox& outbox();
        const game::msg::Outbox& outbox() const;

        ExtraContainer<Turn>& extras();
        const ExtraContainer<Turn>& extras() const;

        void notifyListeners();

     private:
        game::map::Universe m_universe;

        ExtraContainer<Turn> m_extras;

        game::msg::Inbox m_inbox;
        game::msg::Outbox m_outbox;

        afl::base::Ptr<game::vcr::Database> m_battles;

        int m_turnNumber;
        int m_databaseTurnNumber;
        Timestamp m_timestamp;

        // (Alliances)
        // (Gen)
        // (data set)                          whose players' data we have
    };
    
}

#endif
