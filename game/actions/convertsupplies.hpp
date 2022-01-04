/**
  *  \file game/actions/convertsupplies.hpp
  *  \brief Class game::actions::ConvertSupplies
  */
#ifndef C2NG_GAME_ACTIONS_CONVERTSUPPLIES_HPP
#define C2NG_GAME_ACTIONS_CONVERTSUPPLIES_HPP

#include "afl/base/types.hpp"
#include "game/map/planet.hpp"
#include "game/map/universe.hpp"

namespace game { namespace actions {

    /** Converting supplies (sell/buy).

        This class allows to sell and buy (undo sell) supplies.

        For convenience, a particular amount of supplies/money can be reserved
        to prevent invalidating an open transaction.
        This is to implement the special case of selling supplies from the structure build screen.

        This action has no dynamic behaviour, i.e. it does not track state and does not forward changes on the planet. */
    class ConvertSupplies {
     public:
        /** Constructor.
            \param pl Planet to work on */
        explicit ConvertSupplies(game::map::Planet& pl);

        /** Set undo information.
            This enables the action to buy supplies.
            \param univ Universe */
        void setUndoInformation(const game::map::Universe& univ);

        /** Set reserved supplies.
            This reduces getMaxSuppliesToSell(), reserved supplies cannot be sold.
            \param amount Amount */
        void setReservedSupplies(int32_t amount);

        /** Set reserved money.
            This reduces getMaxSuppliesToBuy(), reserved money cannot be spent.
            \param amount Amount */
        void setReservedMoney(int32_t amount);

        /** Sell supplies.
            \param amount Maximum amount to sell (negative to buy, [-getMaxSuppliesToBuy(), getMaxSuppliesToSell])
            \param partial if true, allow partial operation; false: allow only complete operation
            \return Amount sold. With partial=false, either same as amount or 0 */
        int32_t sellSupplies(int32_t amount, bool partial);

        /** Buy supplies.
            \param amount Maximum amount to buy (negative to sell, [-getMaxSuppliesToSell(), getMaxSuppliesToBuy])
            \param partial if true, allow partial operation; false: allow only complete operation
            \return Amount bought. With partial=false, either same as amount or 0 */
        int32_t buySupplies(int32_t amount, bool partial);

        /** Get maximum possible amount to sell.
            \return amount */
        int32_t getMaxSuppliesToSell() const;

        /** Get maximum possible amount to buy.
            \return amount */
        int32_t getMaxSuppliesToBuy() const;

     private:
        game::map::Planet& m_planet;
        const game::map::Universe* m_pUniverse;
        int32_t m_reservedSupplies;
        int32_t m_reservedMoney;
    };

} }

#endif
