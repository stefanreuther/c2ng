/**
  *  \file client/proxy/hullspecificationproxy.hpp
  */
#ifndef C2NG_CLIENT_PROXY_HULLSPECIFICATIONPROXY_HPP
#define C2NG_CLIENT_PROXY_HULLSPECIFICATIONPROXY_HPP

#include "afl/base/signal.hpp"
#include "game/playerset.hpp"
#include "game/session.hpp"
#include "game/spec/cost.hpp"
#include "game/types.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestsender.hpp"
#include "game/shipquery.hpp"

namespace client { namespace proxy {

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

            game::PlayerSet_t players;

            // Missing: points we have, hull functions
        };

        HullSpecificationProxy(util::RequestSender<game::Session> gameSender, util::RequestDispatcher& reply);

        void setExistingShipId(game::Id_t id);

        afl::base::Signal<void(const HullSpecification&)> sig_update;

     private:
        util::RequestSender<game::Session> m_gameSender;
        util::RequestReceiver<HullSpecificationProxy> m_reply;

        static void sendReply(game::ShipQuery& q,
                              const game::spec::ShipList& shipList,
                              const game::Root& root,
                              int player,
                              util::RequestSender<HullSpecificationProxy> reply);
        static void sendReply(const HullSpecification& result,
                              util::RequestSender<HullSpecificationProxy> reply);
    };

} }

#endif
