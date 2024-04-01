/**
  *  \file game/proxy/shipinfoproxy.hpp
  *  \brief Class game::proxy::ShipInfoProxy
  */
#ifndef C2NG_GAME_PROXY_SHIPINFOPROXY_HPP
#define C2NG_GAME_PROXY_SHIPINFOPROXY_HPP

#include "game/map/shipinfo.hpp"
#include "game/session.hpp"
#include "util/requestsender.hpp"

namespace game { namespace proxy {

    class WaitIndicator;

    /** Ship information access.
        Provides bidirectional, synchronous access to the functions from game/map/shipinfo.hpp. */
    class ShipInfoProxy {
     public:
        /** Result of getCargo(). */
        enum CargoStatus {
            NoCargo,             ///< No cargo information returned (e.g.\ ship did not exist).
            HistoryCargo,        ///< History cargo provided.
            CurrentShip          ///< Ship is current, history cargo information is meaningless.
        };

        /** Flag for getCargo: return game::proxy::packShipLastKnownCargo(). */
        static const int GetLastKnownCargo = 1;

        /** Flag for getCargo: return game::proxy::packShipMassRanges(). */
        static const int GetMassRanges     = 2;

        /** Constructor.
            @param gameSender Game sender */
        explicit ShipInfoProxy(const util::RequestSender<Session>& gameSender);

        /** Get cargo information.
            Returns the result of game::map::packShipLastKnownCargo() and/or game::map::packShipMassRanges(), as determined by the @c which argument.

            On error, returns NoCargo and leaves @c out empty.

            @param ind    [in,out] UI synchronisation
            @param shipId [in]     Ship Id
            @param which  [in]     Which information to return (bitset)
            @param out    [out]    Result
            @return Status of provided result */
        CargoStatus getCargo(WaitIndicator& ind, Id_t shipId, int which, game::map::ShipCargoInfos_t& out);

        /** Get experience information.
            Returns the result of game::map::packShipExperienceInfo().

            On error, returns an empty result structure.

            @param ind    [in,out] UI synchronisation
            @param shipId [in]     Ship Id
            @return Result */
        game::map::ShipExperienceInfo getExperienceInfo(WaitIndicator& ind, Id_t shipId);

     private:
        util::RequestSender<Session> m_gameSender;
    };

} }

#endif
