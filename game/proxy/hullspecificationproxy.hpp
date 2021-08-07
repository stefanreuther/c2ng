/**
  *  \file game/proxy/hullspecificationproxy.hpp
  *  \brief Class game::proxy::HullSpecificationProxy
  */
#ifndef C2NG_GAME_PROXY_HULLSPECIFICATIONPROXY_HPP
#define C2NG_GAME_PROXY_HULLSPECIFICATIONPROXY_HPP

#include "afl/base/signal.hpp"
#include "game/playerset.hpp"
#include "game/session.hpp"
#include "game/spec/cost.hpp"
#include "game/spec/info/picturenamer.hpp"
#include "game/spec/info/types.hpp"
#include "game/types.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestsender.hpp"

namespace game { namespace proxy {

    class WaitIndicator;

    /** Hull specification access.

        Bidirectional, asynchronous:
        - retrieve information about a ship's hull

        Bidirectional, synchronous:
        - retrieve weapon effects */
    class HullSpecificationProxy {
     public:
        /** Prepared information about a hull. */
        struct HullSpecification {
            String_t name;
            String_t image;

            int hullId;
            int mass;
            int numEngines;
            int techLevel;
            int maxCrew;
            int maxCargo;
            int maxFuel;
            int maxBeams;
            int maxLaunchers;
            int numBays;
            int mineHitDamage;
            int fuelBurnPerTurn;
            int fuelBurnPerFight;

            game::spec::Cost cost;

            int32_t pointsToBuild;
            int32_t pointsForKilling;
            int32_t pointsForScrapping;
            int32_t pointsAvailable;

            PlayerSet_t players;

            game::spec::info::Abilities_t abilities;

            HullSpecification()
                : name(), image(), hullId(0), mass(0), numEngines(0), techLevel(0), maxCrew(0),
                  maxCargo(0), maxFuel(0), maxBeams(0), maxLaunchers(0), numBays(0), mineHitDamage(0),
                  fuelBurnPerTurn(0), fuelBurnPerFight(0), cost(), pointsToBuild(), pointsForKilling(),
                  pointsForScrapping(), pointsAvailable(), players(), abilities()
                { }
        };

        /** Constructor.
            \param gameSender Game sender
            \param reply RequestDispatcher to send replies back
            \param picNamer PictureNamer (will be transferred to game thread; can be null) */
        HullSpecificationProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& reply, std::auto_ptr<game::spec::info::PictureNamer> picNamer);

        /** Set existing ship Id.
            Proxy will eventually produce a sig_update callback with that ship's data.
            \param id Ship Id */
        void setExistingShipId(Id_t id);

        // FIXME: setHullId() to show just a hull

        /** Get weapon effects.
            Returns the weapon effects for the previously-configured ship.
            \param [in]  ind    WaitIndicator for UI synchronisation
            \param [out] result Result */
        void describeWeaponEffects(WaitIndicator& ind, game::spec::info::WeaponEffects& result);

        /** Get hull function details.
            \param [in]  ind    WaitIndicator for UI synchronisation
            \param [out] result Result */
        void describeHullFunctionDetails(WaitIndicator& ind, game::spec::info::AbilityDetails_t& result);

        /** Signal: ship data to show.
            \param data Data */
        afl::base::Signal<void(const HullSpecification&)> sig_update;

     private:
        class Trampoline;
        class TrampolineFromSession;

        util::RequestReceiver<HullSpecificationProxy> m_reply;
        util::RequestSender<Trampoline> m_request;

        void sendUpdate(HullSpecification info);
    };

} }

#endif
