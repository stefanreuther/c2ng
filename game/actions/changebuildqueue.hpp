/**
  *  \file game/actions/changebuildqueue.hpp
  *  \brief Class game::actions::ChangeBuildQueue
  */
#ifndef C2NG_GAME_ACTIONS_CHANGEBUILDQUEUE_HPP
#define C2NG_GAME_ACTIONS_CHANGEBUILDQUEUE_HPP

#include "game/map/universe.hpp"
#include "game/types.hpp"
#include "util/randomnumbergenerator.hpp"

namespace game { namespace actions {

    /** Changing build queue priorities.
        Provides a simple view of the build queue,
        and allows changing priorities (distributing PBx fcodes).

        - create object
        - if desired, call setAvailableBuildPoints()
        - call increasePriority(), decreasePriority(), setPriority() as needed
        - call commit() to write back */
    class ChangeBuildQueue {
     public:
        /** Information about a build order in prepared format. */
        struct Info {
            Id_t planetId;                      ///< Planet Id.
            String_t planetName;                ///< Planet name.
            String_t actionName;                ///< Description of build order.
            String_t friendlyCode;              ///< Friendly code.
            int queuePosition;                  ///< Queue position (0 if not known or new order).
            LongProperty_t pointsRequired;      ///< Number of build points required to build this ship.
            LongProperty_t pointsAvailable;     ///< Number of build points available for this ship.
            bool hasPriority;                   ///< true if this build order has a priority FC.
            bool conflict;                      ///< true if this priority order conflicts with others (same FC).
            bool playable;                      ///< true if this slot can be modified.

            Info()
                : planetId(), planetName(), actionName(),
                  friendlyCode(), queuePosition(), pointsRequired(), pointsAvailable(),
                  hasPriority(), conflict(), playable()
                { }
        };

        /** Information about all build orders in prepared format. */
        typedef std::vector<Info> Infos_t;


        /** Constructor.
            \param univ Universe
            \param shipList Ship list
            \param host Host version
            \param config Host configuration
            \param rng Random number generator (required to generate fall-back friendly codes)
            \param viewpointPlayer Handle this player's build orders */
        explicit ChangeBuildQueue(game::map::Universe& univ,
                                  const game::spec::ShipList& shipList,
                                  game::HostVersion host,
                                  const game::config::HostConfiguration& config,
                                  util::RandomNumberGenerator& rng,
                                  int viewpointPlayer);

        /** Destructor. */
        ~ChangeBuildQueue();

        /** Set available build points.
            Call this to populate the "pointsAvailable" field.
            \param points Build points */
        void setAvailableBuildPoints(LongProperty_t points);

        /** Prepare data into output format.
            Data is reported sorted, highest priority first.
            \param result [out] Data
            \param tx Translator */
        void describe(Infos_t& result, afl::string::Translator& tx) const;

        /** Set priority of a build order.
            \param slot Slot number
            \param pri New priority (1-9 = PBx, 0 = no priority order) */
        void setPriority(size_t slot, int pri);

        /** Increase a slot's priority (build earlier).
            \param slot Slot number */
        void increasePriority(size_t slot);

        /** Decrease a slot's priority (build later).
            \param slot Slot number */
        void decreasePriority(size_t slot);

        /** Write all changes back to universe. */
        void commit();

     private:
        game::map::Universe& m_universe;
        const game::spec::ShipList& m_shipList;
        game::HostVersion m_host;
        const game::config::HostConfiguration& m_config;

        void init(util::RandomNumberGenerator& rng, int viewpointPlayer);

        void sort();
        void avoid(int setThis, int toThis, size_t slot);

        struct LocalInfo {
            Id_t planetId;              ///< Planet Id.
            Id_t cloningShipId;         ///< Cloning ship Id.
            String_t friendlyCode;      ///< Current friendly code.
            String_t oldFriendlyCode;   ///< Friendly code to revert to (old or random).
            int queuePosition;          ///< Current queue position if known. 0 for new orders.
            bool playable;

            LocalInfo(Id_t planetId, Id_t cloningShipId, String_t friendlyCode, String_t oldFriendlyCode, int queuePosition, bool playable)
                : planetId(planetId), cloningShipId(cloningShipId), friendlyCode(friendlyCode), oldFriendlyCode(oldFriendlyCode), queuePosition(queuePosition), playable(playable)
                { }
        };
        class Sorter;

        std::vector<LocalInfo> m_info;
        LongProperty_t m_availablePoints;
    };

} }

#endif
