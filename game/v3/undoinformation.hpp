/**
  *  \file game/v3/undoinformation.hpp
  *  \brief Class game::v3::UndoInformation
  */
#ifndef C2NG_GAME_V3_UNDOINFORMATION_HPP
#define C2NG_GAME_V3_UNDOINFORMATION_HPP

#include "game/map/universe.hpp"
#include "game/map/basestorage.hpp"
#include "game/types.hpp"

namespace game { namespace v3 {

    class Reverter;

    /** Undo information for a planet, v3 version.
        This class computes information about a location in space, defined with a planet.
        Only planets can do things that could be undone;
        places in deep space that have only ships can only move things around.

        Conversions tracked by UndoInformation:
        - supply sale
        - buying ammo (torpedoes or fighters)
        - buying tech levels

        Conversions that do not interact with anything else and can always be undone to last-turn level:
        - buying planetary structures
        - buying starship components

        This class is used in the implementation of Reverter.

        To use,
        - construct an object
        - call set() to compute a location
        - call get methods to obtain all results
        This allows re-using one computation for multiple queries. */
    class UndoInformation {
     public:
        /** Default constructor.
            Makes a blank object. */
        UndoInformation();

        /** Initialize.
            If the given parameters do not refer to a played object or there is no undo information,
            sets default values (as if by clear()).
            \param univ Universe (used to access objects)
            \param shipList Ship list (used to access tech levels and hull mappings)
            \param config Host configuration
            \param reverter Reverter
            \param planetId Planet Id */
        void set(const game::map::Universe& univ,
                 const game::spec::ShipList& shipList,
                 const game::config::HostConfiguration& config,
                 const Reverter& reverter,
                 int planetId);

        /** Reset.
            Makes all methods return defaults ("no undo possible"). */
        void clear();

        /** Get number of torpedoes that can be sold.
            \param slot Torpedo type
            \return count */
        int getNumTorpedoesAllowedToSell(int slot) const;

        /** Get number of fighters that can be sold.
            \return count */
        int getNumFightersAllowedToSell() const;

        /** Get number of supplies that can be bought.
            \return count */
        int getSuppliesAllowedToBuy() const;

        /** Get minimum tech level permitted.
            You cannot reduce below a tech level if a component requiring it has been bought.
            \param level Tech level to query
            \return tech level */
        int getMinTechLevel(TechLevel level) const;
        
     private:
        game::map::BaseStorage m_torpedoesAllowedToSell;
        int32_t m_fightersAllowedToSell;
        int32_t m_suppliesAllowedToBuy;
        int m_minTechLevels[NUM_TECH_AREAS];
    };

} }

#endif
