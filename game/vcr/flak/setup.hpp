/**
  *  \file game/vcr/flak/setup.hpp
  *  \brief Class game::vcr::flak::Setup
  */
#ifndef C2NG_GAME_VCR_FLAK_SETUP_HPP
#define C2NG_GAME_VCR_FLAK_SETUP_HPP

#include "afl/base/growablememory.hpp"
#include "afl/base/optional.hpp"
#include "afl/charset/charset.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/string/translator.hpp"
#include "game/map/point.hpp"
#include "game/vcr/flak/definitions.hpp"
#include "game/vcr/flak/object.hpp"
#include "game/vcr/flak/position.hpp"
#include "game/vcr/flak/structures.hpp"
#include "util/randomnumbergenerator.hpp"

namespace game { namespace vcr { namespace flak {

    class Environment;
    class Configuration;

    /** FLAK Battle Setup.
        Stores the initial status of a FLAK fight.
        Contains methods to prepare it, and load/save it from/to FLAKx.DAT files.

        This is a split of the original FlakBattle class to contain only code needed for managing and setting up battles,
        but none for playing it.

        This class includes some assertion checks that will throw exceptions.
        In particular, invoking getFleetByIndex(), getShipByIndex() with an out-of-range index will throw
        (unlike other classes where it will return null, relying on the caller to check). */
    class Setup {
        // ex FlakBattle
     public:
        /** Internal representation of a fleet. */
        struct Fleet {
            int player;
            int speed;
            int32_t x, y;
            size_t firstShipIndex;
            size_t numShips;
            size_t firstAttackListIndex;
            size_t numAttackListEntries;

            /** Default constructor. */
            Fleet()
                : player(), speed(), x(), y(), firstShipIndex(), numShips(), firstAttackListIndex(), numAttackListEntries()
                { }

            /** Construct from serialized form.
                \param data Serialized form loaded from file */
            Fleet(const structures::Fleet& data);
        };

        /** Typedef for attack list. */
        typedef std::vector<int16_t> AttackList_t;

        /** Fleet index.
            ex fleet_t */
        typedef size_t FleetIndex_t;

        /** Type for a ship number. */
        typedef size_t ShipIndex_t;


        /** Constructor.
            Make a blank setup. */
        Setup();

        /** Copy constructor.
            \param b Setup to copy from */
        Setup(const Setup& b);

        /** Destructor. */
        ~Setup();


        /*
         *  Fleets and Attack Lists
         */

        /** Add an empty fleet for the given player.
            You can then add ships to the fleet using addShip().
            \param player Player
            \return Fleet index */
        FleetIndex_t addFleet(int player);

        /** Get number of fleets.
            \return Number of fleets */
        FleetIndex_t getNumFleets() const;

        /** Start attack list for a fleet.
            All following calls to addAttackListEntry() apply to that fleet until endAttackList() is called.
            \param fleetNr Fleet index [0,getNumFleets()) */
        void startAttackList(FleetIndex_t fleetNr);

        /** End attack list for a fleet.
            \param fleetNr Fleet index [0,getNumFleets()) */
        void endAttackList(FleetIndex_t fleetNr);

        /** Add attack list entry.
            \param shipIndex Ship that can be attacked
            \param ratingBonus Rating bonus awarded for this attack */
        void addAttackListEntry(ShipIndex_t shipIndex, int16_t ratingBonus);

        /** Access fleet by index.
            \param number fleet index, [0, getNumFleets()).
            \return fleet
            \throw afl::except::AssertionFailedException if index out of range */
        Fleet& getFleetByIndex(FleetIndex_t number);
        const Fleet& getFleetByIndex(FleetIndex_t number) const;

        /** Access attack list.
            \return attack list */
        const AttackList_t& getAttackList() const;


        /*
         *  Ships
         */

        /** Add ship.
            The ship will be added to the most-recently added fleet.
            \pre there already is a fleet which matches the ship's owner
            \param ship Ship data
            \return number assigned to this ship (running index) */
        ShipIndex_t addShip(const Object& ship);

        /** Get number of ships.
            \return Number of ships */
        ShipIndex_t getNumShips() const;

        /** Access ship by index.
            \param number ship index, [0, getNumShips()).
            \return ship
            \throw afl::except::AssertionFailedException if index out of range */
        Object& getShipByIndex(size_t number);
        const Object& getShipByIndex(size_t number) const;


        /*
         *  Additional attributes
         */

        /** Get total time.
            \return total time */
        int32_t getTotalTime() const;

        /** Set total time.
            \param time Time */
        void setTotalTime(int32_t time);

        /** Get seed.
            \return seed */
        uint32_t getSeed() const;

        /** Set seed.
            \param seed Seed */
        void setSeed(uint32_t seed);

        /** Get position of this battle on the map.
            This information is transmitted with the file header and has no effect on the fight.
            \return position if known */
        afl::base::Optional<game::map::Point> getPosition() const;

        /** Set position.
            \param pos Position */
        void setPosition(game::map::Point pos);

        /** Get ambient flags.
            This information is transmitted with the file header and has no effect on the fight.
            \return flags */
        int32_t getAmbientFlags() const;

        /** Set ambient flags.
            \param flags Flags */
        void setAmbientFlags(int32_t flags);


        /*
         *  I/O
         */

        /** Save this battle into byte block.
            \param [out] s        Target buffer, will be grown
            \param [in]  charset  Charset */
        void save(afl::base::GrowableBytes_t& s, afl::charset::Charset& charset) const;

        /** Load this battle from a byte block.
            \param [in]  name     Name (for error messages)
            \param [in]  s        Buffer to read
            \param [in]  charset  Charset
            \param [in]  tx       Translator (for error messages)
            \throw afl::except::FileFormatException on error */
        void load(String_t name, afl::base::ConstBytes_t s, afl::charset::Charset& charset, afl::string::Translator& tx);


        /*
         *  Setup
         */

        /** Initialisation, main entry.
            After having set up a fight, call this routine to compute the derived information
            and remove units which are not needed for the fight.
            \param config  FLAK configuration
            \param env     Environment
            \param rng     Random number generator */
        void initAfterSetup(const Configuration& config, const Environment& env, util::RandomNumberGenerator& rng);

     private:
        AttackList_t m_attackList;
        afl::container::PtrVector<Fleet> m_fleets;
        afl::container::PtrVector<Object> m_objects;
        int32_t m_totalTime;
        uint32_t m_seed;
        game::map::Point m_position;
        int32_t m_ambientFlags;

        void removePassiveObjects();
        void computeFleetSpeeds(const Environment& env);
        void computeInitialPositions(const Configuration& config, util::RandomNumberGenerator& rng);
        void assignInitialPositions(int player, double angle, int32_t dist, const Configuration& config);
        void adjustStrengths(int adj_to, const Configuration& config);
    };

} } }

#endif
