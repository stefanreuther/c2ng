/**
  *  \file server/interface/hostturn.hpp
  *  \brief Interface server::interface::HostTurn
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTTURN_HPP
#define C2NG_SERVER_INTERFACE_HOSTTURN_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/optional.hpp"
#include "afl/string/string.hpp"

namespace server { namespace interface {

    /** Interface for turn submission into a host server. */
    class HostTurn : public afl::base::Deletable {
     public:
        /** Result of a turn submission. */
        struct Result {
            int32_t state;                           /**< State of turn submission. */
            String_t output;                         /**< Turn checker output. */
            int32_t gameId;                          /**< Game Id. */
            int32_t slot;                            /**< Slot number. */
            int32_t previousState;                   /**< Previous turn state. */
            int32_t turnNumber;                      /**< Turn number. */
            String_t userId;                         /**< User Id. */
            String_t gameName;                       /**< Game name. */
            bool allowTemp;                          /**< True if turn can be marked temporary. */

            Result();
            ~Result();
        };

        static const int32_t MissingTurn  = 0;       /**< Turn not submitted. */
        static const int32_t GreenTurn    = 1;       /**< Turn submitted, green. */
        static const int32_t YellowTurn   = 2;       /**< Turn submitted, yellow. */
        static const int32_t RedTurn      = 3;       /**< Turn not submitted, last attempt was red. */
        static const int32_t BadTurn      = 4;       /**< Turn not submitted, last attempt was bad. */
        static const int32_t StaleTurn    = 5;       /**< Turn not submitted, last attempt was stale. */
        static const int32_t NeedlessTurn = 6;       /**< Turn not submitted, but not needed because player has zero score. */

        static const int32_t TemporaryTurnFlag = 16; /**< Turn submitted and marked temporary. */

        /** Submit a turn file (TRN).
            @param blob      Turn file data
            @param game      Submit turn to this game (empty for auto-detect).
            @param slot      Submit turn for this player (empty for auto-detect).
            @param mail      Sender email address
            @param info      Optional information for logging
            @return result of submission */
        virtual Result submit(const String_t& blob,
                              afl::base::Optional<int32_t> game,
                              afl::base::Optional<int32_t> slot,
                              afl::base::Optional<String_t> mail,
                              afl::base::Optional<String_t> info) = 0;

        /** Mark turn temporary (TRNMARKTEMP).
            @param gameId   Game
            @param slot     Slot number
            @param flag     true to mark temporary */
        virtual void setTemporary(int32_t gameId, int32_t slot, bool flag) = 0;
    };

} }

#endif
