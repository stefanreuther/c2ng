/**
  *  \file game/proxy/buildstructuresproxy.hpp
  *  \brief Class game::proxy::BuildStructuresProxy
  */
#ifndef C2NG_GAME_PROXY_BUILDSTRUCTURESPROXY_HPP
#define C2NG_GAME_PROXY_BUILDSTRUCTURESPROXY_HPP

#include "afl/base/signal.hpp"
#include "game/map/planet.hpp"
#include "game/proxy/waitindicator.hpp"
#include "game/session.hpp"
#include "game/spec/cost.hpp"
#include "util/requestsender.hpp"
#include "util/slaverequestsender.hpp"

namespace game { namespace proxy {

    /** Structure building proxy.

        Bidirectional, synchronous:
        - initialize (init())

        Bidirectional, asynchronous: modifications cause sig_statusChange to be raised
        - update() to request an update
        - addLimitCash(), doStandardAutoBuild() to modify the request
        - commit() */
    class BuildStructuresProxy {
     public:
        /** Header information.
            Reports general information about the action. */
        struct HeaderInfo {
            bool ok;                      /**< true if action successfully constructed, false if preconditions missing (not a playable planet). */
            bool hasBase;                 /**< true if planet has a base. */
            String_t planetName;          /**< Planet name. */
            String_t planetInfo;          /**< Planet information (temperature etc.) to use as subtitle. */

            HeaderInfo()
                : ok(false), hasBase(false), planetName(), planetInfo()
                { }
        };

        /** Information about a building. */
        struct BuildingInfo {
            int have;                     /**< Number present/orderer buildings. \see game::actions::BuildStructures::getNumBuildings() */
            int want;                     /**< Autobuild target. */
            int max;                      /**< Maximum number of buildings. \see game::actions::BuildStructures::getMaxBuildingsRuleLimit() */
            int speed;                    /**< Autobuild speed. */
            BuildingInfo()
                : have(0), want(0), max(0), speed(0)
                { }
        };

        /** Action status. */
        struct Status {
            BuildingInfo buildings[NUM_PLANETARY_BUILDING_TYPES];  /**< Information about buildings. Indexed by PlanetaryBuilding. */
            game::spec::Cost available;   /**< Available resources. */
            game::spec::Cost needed;      /**< Needed resources. */
            game::spec::Cost remaining;   /**< Remaining resources. */
        };

        /** Constructor.
            \param gameSender Game sender
            \param receiver   RequestDispatcher to receive updates in this thread */
        BuildStructuresProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& receiver);

        /** Destructor. */
        ~BuildStructuresProxy();

        /** Initialize.
            \param [in] link WaitIndicator
            \param [in] id Planet Id
            \param [out] info Header information */
        void init(WaitIndicator& link, Id_t id, HeaderInfo& info);

        /** Request an update.
            Causes sig_statusChange to be raised. */
        void update();

        /** Add structures, limited by resources.
            \param type building type
            \param count Number of buildings to add (negative to remove)
            \see game::actions::BuildStructures::addLimitCash() */
        void addLimitCash(PlanetaryBuilding type, int count);

        /** Autobuild.
            \see game::actions::BuildStructures::doStandardAutoBuild() */
        void doStandardAutoBuild();

        /** Modify auto-build settings.
            \param settings New settings
            \see game::map::Planet::applyAutobuildSettings() */
        void applyAutobuildSettings(const game::map::Planet::AutobuildSettings& settings);

        /** Commit this transaction. */
        void commit();

        /** Signal: status change (e.g.\ transaction modification).
            \param st New status */
        afl::base::Signal<void(const Status&)> sig_statusChange;

     private:
        struct Trampoline;
        util::RequestReceiver<BuildStructuresProxy> m_receiver;
        util::SlaveRequestSender<Session, Trampoline> m_sender;
    };

} }

#endif
