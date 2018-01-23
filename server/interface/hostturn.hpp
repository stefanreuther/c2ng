/**
  *  \file server/interface/hostturn.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTTURN_HPP
#define C2NG_SERVER_INTERFACE_HOSTTURN_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/optional.hpp"
#include "afl/string/string.hpp"

namespace server { namespace interface {

    class HostTurn : public afl::base::Deletable {
     public:
        struct Result {
            int32_t state;
            String_t output;
            int32_t gameId;
            int32_t slot;
            int32_t previousState;
            String_t userId;

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


        // TRN content:Blob [GAME game:GID] [SLOT slot:Int] [MAIL email:Str] [INFO info:Str]
        virtual Result submit(const String_t& blob,
                              afl::base::Optional<int32_t> game,
                              afl::base::Optional<int32_t> slot,
                              afl::base::Optional<String_t> mail,
                              afl::base::Optional<String_t> info) = 0;

        // TRNMARKTEMP game:GID slot:Int flag:Int
        virtual void setTemporary(int32_t gameId, int32_t slot, bool flag) = 0;
    };

} }

#endif
