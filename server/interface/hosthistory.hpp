/**
  *  \file server/interface/hosthistory.hpp
  *  \brief Interface server::interface::HostHistory
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTHISTORY_HPP
#define C2NG_SERVER_INTERFACE_HOSTHISTORY_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/optional.hpp"
#include "afl/base/types.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/data/integerlist.hpp"
#include "afl/data/stringlist.hpp"
#include "afl/string/string.hpp"
#include "server/interface/hostgame.hpp"
#include "server/types.hpp"

namespace server { namespace interface {

    /** Host History interface.
        This interface allows access to historical turn data. */
    class HostHistory : public afl::base::Deletable {
     public:
        /** Event filter. */
        struct EventFilter {
            afl::base::Optional<int32_t> gameId;       ///< Return only events applying to this game.
            afl::base::Optional<String_t> userId;      ///< Return only events applying to this user Id.
            afl::base::Optional<int32_t> limit;        ///< Maximum number of events to return.
        };

        /** Event. */
        struct Event {
            Time_t time;                               ///< Event time.
            String_t eventType;                        ///< Event type.
            afl::base::Optional<int32_t> gameId;       ///< Game Id, if event applies to a game.
            afl::base::Optional<String_t> gameName;    ///< Game name, if event applies to a game.
            afl::base::Optional<String_t> userId;      ///< User Id, if event applies to a user.
            afl::base::Optional<int32_t> slotNumber;   ///< Slot number, if event applies to a slot number.
            afl::base::Optional<HostGame::State> gameState;  ///< Game state, if event is a game state change.

            Event()
                : time(), eventType(), gameId(), gameName(),
                  userId(), slotNumber(), gameState()
                { }
        };

        /** Turn filter. */
        struct TurnFilter {
            afl::base::Optional<int32_t> endTurn;      ///< Latest turn to return.
            afl::base::Optional<int32_t> limit;        ///< Maximum number of turns to return.
            afl::base::Optional<int32_t> startTime;    ///< Earliest time to return.
            afl::base::Optional<String_t> scoreName;   ///< Name of score to return.
            bool reportPlayers;                        ///< true to report players.
            bool reportStatus;                         ///< true to report slot status.

            TurnFilter()
                : endTurn(), limit(), startTime(), scoreName(), reportPlayers(), reportStatus()
                { }
        };

        /** Turn. */
        struct Turn {
            int turnNumber;                            ///< Turn number.
            afl::data::StringList_t slotPlayers;       ///< Players, starting with slot 1. Empty if not reported.
            afl::data::IntegerList_t slotStates;       ///< Slot states, starting with slot 1. Empty if not reported.
            afl::data::IntegerList_t slotScores;       ///< Slot scores, starting with slot 1. Empty if not reported.
            Time_t time;                               ///< Turn time.
            String_t timestamp;                        ///< Turn timestamp.
        };

        /** Get global events (HISTEVENTS).
            \param [in] filter Event selection
            \param [out] result Events */
        virtual void getEvents(const EventFilter& filter, afl::container::PtrVector<Event>& result) = 0;

        /** Get turn history (HISTTURN).
            \param [in] gameId game
            \param [in] filter data selection
            \param [out] result Turns */
        virtual void getTurns(int32_t gameId, const TurnFilter& filter, afl::container::PtrVector<Turn>& result) = 0;
    };

} }

#endif
