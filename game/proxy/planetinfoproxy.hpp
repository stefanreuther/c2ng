/**
  *  \file game/proxy/planetinfoproxy.hpp
  *  \brief Class game::proxy::PlanetInfoProxy
  */
#ifndef C2NG_GAME_PROXY_PLANETINFOPROXY_HPP
#define C2NG_GAME_PROXY_PLANETINFOPROXY_HPP

#include "game/map/planetinfo.hpp"
#include "game/session.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestsender.hpp"
#include "util/slaverequestsender.hpp"

namespace game { namespace proxy {

    /** Asynchronous, bidirectional proxy for Planet Information.
        Provides bidirectional access to the functions of game/map/playerinfo.hpp.

        This proxy caches the information received from the game.
        Once populated, the information can be retrieved at any time.
        Before the first callback, information will be empty.

        - construct a PlanetInfoProxy.
        - attach listener to sig_change.
        - set parameters (setBuildingOverride, setAttackingClansOverride).
        - set planet Id (setPlanet).
        - from callback, obtain data (getMineralInfo, etc.)
        - changing the parameters now will provide further callbacks with updated information. */
    class PlanetInfoProxy {
     public:
        /** Shortcut for minerals.
            We use our own type for simplicity. */
        enum Mineral {
            Neutronium,                           ///< Neutronium.
            Tritanium,                            ///< Tritanium.
            Duranium,                             ///< Duranium.
            Molybdenum                            ///< Molybdenum.
        };
        static const size_t NUM_MINERALS = 4;     ///< Number of minerals.


        /** Constructor.
            \param gameSender Game sender
            \param receiver   RequestDispatcher to receive replies */
        PlanetInfoProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& receiver);

        /** Destructor. */
        ~PlanetInfoProxy();

        /** Set planet Id.
            If the planet Id is valid, this will produce a callback with new data.
            \param id Planet Id */
        void setPlanet(Id_t id);

        /** Set number of buildings.
            Affects getMineralInfo(), getBuildingEffectsInfo(), getDefenseEffectsInfo(), getGroundDefenseInfo().
            \param type Type
            \param amount Number of buildings (Nothing to use planet's amount) */
        void setBuildingOverride(PlanetaryBuilding type, IntegerProperty_t amount);

        /** Set number of attacking clans.
            \param n Number of clans. */
        void setAttackingClansOverride(int32_t n);

        /** Get mineral info.
            \param m Mineral to obtain information for
            \return result of last game::map::packPlanetMineralInfo() executed by proxy */
        const game::map::PlanetMineralInfo& getMineralInfo(Mineral m) const;

        /** Get climate info.
            \return result of last game::map::describePlanetClimate() executed by proxy */
        const afl::io::xml::Nodes_t& getClimateInfo() const;

        /** Get colony info.
            \return result of last game::map::describePlanetColony() executed by proxy */
        const afl::io::xml::Nodes_t& getColonyInfo() const;

        /** Get natives info.
            \return result of last game::map::describePlanetNatives() executed by proxy */
        const afl::io::xml::Nodes_t& getNativeInfo() const;

        /** Get building effects information.
            \return result of last game::map::describePlanetBuildingEffects() executed by proxy */
        const afl::io::xml::Nodes_t& getBuildingEffectsInfo() const;

        /** Get defense effects information.
            \return result of last game::map::describePlanetDefenseEffects() executed by proxy */
        const game::map::DefenseEffectInfos_t& getDefenseEffectsInfo() const;

        /** Get unload information.
            This information is initialized when the planet Id is set, and can be manipulated with setAttackingClansOverride().
            \return unload information */
        const game::map::UnloadInfo& getUnloadInfo() const;

        /** Get ground defense information.
            \return result of last game::map::packGroundDefenseInfo() executed by proxy */
        const game::map::GroundDefenseInfo& getGroundDefenseInfo() const;

        /** Signal: data change.
            Call our getters to obtain the data. */
        afl::base::Signal<void()> sig_change;

     private:
        game::map::PlanetMineralInfo m_mineralInfo[NUM_MINERALS];
        afl::io::xml::Nodes_t m_climateInfo;
        afl::io::xml::Nodes_t m_colonyInfo;
        afl::io::xml::Nodes_t m_nativeInfo;
        afl::io::xml::Nodes_t m_buildingEffectsInfo;
        game::map::DefenseEffectInfos_t m_defenseEffectsInfo;
        game::map::UnloadInfo m_unloadInfo;
        game::map::GroundDefenseInfo m_groundDefenseInfo;

        class Response;
        class Trampoline;
        util::RequestReceiver<PlanetInfoProxy> m_receiver;
        util::SlaveRequestSender<Session, Trampoline> m_sender;
    };

} }


#endif
