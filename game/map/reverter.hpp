/**
  *  \file game/map/reverter.hpp
  *  \brief Interface game::map::Reverter
  */
#ifndef C2NG_GAME_MAP_REVERTER_HPP
#define C2NG_GAME_MAP_REVERTER_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/optional.hpp"
#include "game/map/planet.hpp"
#include "game/map/point.hpp"
#include "game/types.hpp"

namespace game { namespace map {

    class LocationReverter;

    /** Undo information provider.
        Depending on the game type, different information is available to implement various "undo" operations.
        A TurnLoader has to provide a Reverter to make that information accessible.
        Depending on the implementation, different types of operations can be undone,
        and undo gets you back to different points in time (beginning of turn or beginning of session).

        Reverter is an optional component of Universe; lack of a reverter means nothing can be undone.
        In addition, each method can return a value to report that this area cannot be undone.
        All users need to deal with that. */
    class Reverter : public afl::base::Deletable {
     public:
        /** Get minimum number of buildings on a planet.
            This is the number at the beginning of the turn and determines how many can be sold for money back.
            \param planetId Planet Id
            \param building Building to query
            \return Minimum number. If unknown, nothing can be sold. */
        virtual afl::base::Optional<int> getMinBuildings(int planetId, PlanetaryBuilding building) const = 0;

        /** Get number of supplies that can be bought.
            \param planetId Planet Id
            \return Number of supplies. Return 0 to disable undo.
                    Can be more than money on planet if user already spent/transferred away the money. */
        virtual int getSuppliesAllowedToBuy(int planetId) const = 0;

        /** Get minimum tech level.
            Determines how many tech levels can be sold for money back.
            This is the level at the beginning of the turn unless the increased tech has already been used to buy things.
            \param planetId Planet Id
            \param techLevel Tech level area
            \return Minimum number. If unknown, tech cannot be lowered. */
        virtual afl::base::Optional<int> getMinTechLevel(int planetId, TechLevel techLevel) const = 0;

        /** Get minimum starship parts storage.
            Determines how many tech levels can be sold for money back.
            This is the number of parts at the beginning of the turn.
            \param planetId Planet Id
            \param techLevel Part area
            \param slot Slot (1..)
            \return Minimum number. If unknown, components cannot be sold. */
        virtual afl::base::Optional<int> getMinBaseStorage(int planetId, TechLevel area, int slot) const = 0;

        /** Get number of torpedoes allowed to be sold.
            This is the number of torpedoes bought this turn.
            \param planetId Planet Id
            \param slot Slot
            \return Number of torpedoes. Return 0 to disable undo.
                    Can be more than torpedoes on starbase if user already transferred away some. */
        virtual int getNumTorpedoesAllowedToSell(int planetId, int slot) const = 0;

        /** Get number of fighters allowed to be sold.
            This is the number of fighters bought this turn.
            \param planetId Planet Id
            \param slot Slot
            \return Number of fighters. Return 0 to disable undo.
                    Can be more than fighters on starbase if user already transferred away some. */
        virtual int getNumFightersAllowedToSell(int planetId) const = 0;

        /** Get previous friendly code for ship.
            This is used whenever an action needs to clear a friendly code to get a sensible value to fall back to.
            \param shipId Ship Id
            \return Friendly code. If unknown, caller must choose a fallback */
        virtual afl::base::Optional<String_t> getPreviousShipFriendlyCode(Id_t shipId) const = 0;

        /** Get previous friendly code for planet.
            This is used whenever an action needs to clear a friendly code to get a sensible value to fall back to.
            \param planetId Planet Id
            \return Friendly code. If unknown, caller must choose a fallback */
        virtual afl::base::Optional<String_t> getPreviousPlanetFriendlyCode(Id_t planetId) const = 0;

        /** Get previous mission for ship.
            This is used whenever an action needs to clear a mission to get a sensible value to fall back to.
            \param [in]  shipId Ship Id
            \param [out] m      Mission
            \param [out] i      Intercept parameter
            \param [out] t      Tow parameter
            \return true on success; false if no fallback known, caller must choose a fallback */
        virtual bool getPreviousShipMission(int shipId, int& m, int& i, int& t) const = 0;

        /** Get previous ship build oreder.
            This is used to determine whether a ship build order was changed.
            \param [in]  planetId Planet Id
            \param [out] result   Ship build order
            \return true on success; false if not known */
        virtual bool getPreviousShipBuildOrder(int planetId, ShipBuildOrder& result) const = 0;

        /** Prepare location reset.
            Location reset will reset (parts) of all units at a given location to their previous values.
            Because cargo can be transferred between units at a location, they can be reverted only as a group.

            This function can return null if location reset is not available.

            The returned LocationReverter is owned by the caller and should not exceed the lifetime of the Reverter.
            The underlying turn should not be structurally modified (i.e. new results loaded or unloaded) while the LocationReverter is active.

            \param pt Location
            \return newly-allocated LocationReverter. */
        virtual LocationReverter* createLocationReverter(Point pt) const = 0;
    };

} }

#endif
