/**
  *  \file game/turn.hpp
  *  \brief Class game::Turn
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
#include "game/alliance/container.hpp"

namespace game {

    /** Game turn.
        Stores information for a turn.
        - identifying information (turn number etc.)
        - universe
        - battles
        - messages
        - alliances
        - optional extras

        Turn owns all contained objects.

        Turn objects that are part of a Session are heap-allocated;
        code that refers to a Turn for an extended time period should point at it by Ptr/Ref. */
    class Turn : public afl::base::RefCounted {
     public:
        /** Constructor. */
        Turn();

        /** Destructor. */
        ~Turn();

        /** Set turn number.
            \param turnNumber turn number */
        void setTurnNumber(int turnNumber);

        /** Get turn number.
            \return turn number */
        int getTurnNumber() const;

        /** Set database turn number.
            \param turnNumber database turn number */
        void setDatabaseTurnNumber(int turnNumber);

        /** Get database turn number.
            The database turn number can differ from getTurnNumber() when the current turn has not yet been placed in the history database.
            \return database turn number */
        int getDatabaseTurnNumber() const;

        /** Set timestamp.
            \param ts Timestamp */
        void setTimestamp(const Timestamp& ts);

        /** Get timestamp.
            \return timestamp */
        Timestamp getTimestamp() const;

        /** Access universe.
            \return universe */
        game::map::Universe& universe();
        const game::map::Universe& universe() const;

        /** Set battle recordings.
            \param battles Battles */
        void setBattles(afl::base::Ptr<game::vcr::Database> battles);

        /** Get battle recordings.
            \return battles. Can be null */
        afl::base::Ptr<game::vcr::Database> getBattles() const;

        /** Access message inbox.
            \return message inbox */
        game::msg::Inbox& inbox();
        const game::msg::Inbox& inbox() const;

        /** Access message outbox.
            \return message outbox */
        game::msg::Outbox& outbox();
        const game::msg::Outbox& outbox() const;

        /** Access turn extras.
            \return turn extras */
        ExtraContainer<Turn>& extras();
        const ExtraContainer<Turn>& extras() const;

        /** Access alliances.
            \return alliances */
        game::alliance::Container& alliances();
        const game::alliance::Container& alliances() const;

        /** Notify listeners of all subobjects. */
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

        // FIXME: should be player-specific?
        game::alliance::Container m_alliances;

        // (Gen)
        // (data set)                          whose players' data we have
    };

}

#endif
