/**
  *  \file game/proxy/hullspecificationproxy.hpp
  *  \brief Class game::proxy::HullSpecificationProxy
  */
#ifndef C2NG_GAME_PROXY_HULLSPECIFICATIONPROXY_HPP
#define C2NG_GAME_PROXY_HULLSPECIFICATIONPROXY_HPP

#include "afl/base/signal.hpp"
#include "game/playerset.hpp"
#include "game/session.hpp"
#include "game/shipquery.hpp"
#include "game/spec/cost.hpp"
#include "game/spec/info/picturenamer.hpp"
#include "game/types.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestsender.hpp"

namespace game { namespace proxy {

    /** Hull specification access.

        Bidirectional, asynchronous:
        - retrieve information about a ship's hull */
    class HullSpecificationProxy {
     public:
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

            int pointsToBuild;
            int pointsForKilling;
            int pointsForScrapping;

            PlayerSet_t players;

            // Missing: points we have, hull functions

            HullSpecification()
                : name(), image(), hullId(0), mass(0), numEngines(0), techLevel(0), maxCrew(0),
                  maxCargo(0), maxFuel(0), maxBeams(0), maxLaunchers(0), numBays(0), mineHitDamage(0),
                  fuelBurnPerTurn(0), fuelBurnPerFight(0), cost(), pointsToBuild(), pointsForKilling(),
                  pointsForScrapping(), players()
                { }
        };

        /** Constructor.
            \param gameSender Game sender
            \param reply RequestDispatcher to send replies back
            \param picNamer PictureNamer (will be transferred to game thread) */
        HullSpecificationProxy(util::RequestSender<Session> gameSender, util::RequestDispatcher& reply, std::auto_ptr<game::spec::info::PictureNamer> picNamer);

        /** Set existing ship Id.
            Proxy will eventually produce a sig_update callback with that ship's data.
            \param id Ship Id */
        void setExistingShipId(Id_t id);

        // FIXME: setHullId() to show just a hull

        /** Signal: ship data to show.
            \param data Data */
        afl::base::Signal<void(const HullSpecification&)> sig_update;

     private:
        util::RequestSender<Session> m_gameSender;
        util::RequestReceiver<HullSpecificationProxy> m_reply;
        afl::base::Ptr<game::spec::info::PictureNamer> m_picNamer;

        static void sendReply(ShipQuery& q,
                              const game::spec::ShipList& shipList,
                              const Root& root,
                              int player,
                              game::spec::info::PictureNamer& picNamer,
                              util::RequestSender<HullSpecificationProxy> reply);
        static void sendReply(const HullSpecification& result,
                              util::RequestSender<HullSpecificationProxy> reply);
    };

} }

#endif
