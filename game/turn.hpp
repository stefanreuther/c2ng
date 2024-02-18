/**
  *  \file game/turn.hpp
  *  \brief Class game::Turn
  */
#ifndef C2NG_GAME_TURN_HPP
#define C2NG_GAME_TURN_HPP

#include "afl/base/refcounted.hpp"
#include "afl/container/ptrmap.hpp"
#include "game/alliance/container.hpp"
#include "game/extracontainer.hpp"
#include "game/map/universe.hpp"
#include "game/msg/inbox.hpp"
#include "game/msg/outbox.hpp"
#include "game/playerset.hpp"
#include "game/timestamp.hpp"
#include "game/vcr/database.hpp"

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

        /** Set players for which commands can be given.
            Primarily applies to data that is exchanged with the host, but not stored in map objects:
            - alliances
            - outgoing messages
            - data in turn extras (i.e. commands)

            If any map objects are Playable or better, this flag must be set.
            That aside, ability to edit map objects is controlled individually by their playability.

            This flag is usually set if this is the currentTurn() of a playable game;
            it is not set for allied or history turns, and read-only games.

            \param set Player set */
        void setCommandPlayers(PlayerSet_t set);

        /** Get set of players for which commands can be given.
            \return set
            \see setCommandPlayers */
        PlayerSet_t getCommandPlayers() const;

        /** Set players for which local data can be edited.
            Applies to data managed locally:
            - history
            - map drawings

            This flag is usually set for a player if this is the currentTurn() of a playable game.
            It can be set for finished games that still have a writable starchart file.

            Note that the change protection cannot be absolute;
            object properties (and thus, comments) can always be changed.

            The controlled data is not inherently player-specific.
            This is a player set for consistency with setCommandPlayers(),
            and controls whose files we access.

            \param set Player set */
        void setLocalDataPlayers(PlayerSet_t set);

        /** Get set of players for which local data can be edited.
            \return set
            \see setLocalDataPlayers */
        PlayerSet_t getLocalDataPlayers() const;

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
        PlayerSet_t m_commandPlayers;
        PlayerSet_t m_localDataPlayers;

        // FIXME: should be player-specific?
        game::alliance::Container m_alliances;
    };

}

inline void
game::Turn::setTurnNumber(int turnNumber)
{
    m_turnNumber = turnNumber;
}

inline int
game::Turn::getTurnNumber() const
{
    return m_turnNumber;
}

inline void
game::Turn::setCommandPlayers(PlayerSet_t set)
{
    m_commandPlayers = set;
}

inline game::PlayerSet_t
game::Turn::getCommandPlayers() const
{
    return m_commandPlayers;
}

inline void
game::Turn::setLocalDataPlayers(PlayerSet_t set)
{
    m_localDataPlayers = set;
}

inline game::PlayerSet_t
game::Turn::getLocalDataPlayers() const
{
    return m_localDataPlayers;
}

inline game::map::Universe&
game::Turn::universe()
{
    // ex GGameTurn::getCurrentUniverse, GGameTurn::getPreviousUniverse
    return m_universe;
}

inline const game::map::Universe&
game::Turn::universe() const
{
    // ex GGameTurn::getCurrentUniverse, GGameTurn::getPreviousUniverse
    return m_universe;
}

inline game::msg::Inbox&
game::Turn::inbox()
{
    return m_inbox;
}

inline const game::msg::Inbox&
game::Turn::inbox() const
{
    return m_inbox;
}

inline game::msg::Outbox&
game::Turn::outbox()
{
    return m_outbox;
}

inline const game::msg::Outbox&
game::Turn::outbox() const
{
    return m_outbox;
}

inline game::ExtraContainer<game::Turn>&
game::Turn::extras()
{
    return m_extras;
}

inline const game::ExtraContainer<game::Turn>&
game::Turn::extras() const
{
    return m_extras;
}

inline game::alliance::Container&
game::Turn::alliances()
{
    return m_alliances;
}

inline const game::alliance::Container&
game::Turn::alliances() const
{
    return m_alliances;
}

#endif
