/**
  *  \file client/proxy/buildstructuresproxy.hpp
  */
#ifndef C2NG_CLIENT_PROXY_BUILDSTRUCTURESPROXY_HPP
#define C2NG_CLIENT_PROXY_BUILDSTRUCTURESPROXY_HPP

#include "util/requestsender.hpp"
#include "util/slaverequestsender.hpp"
#include "game/session.hpp"
#include "client/downlink.hpp"
#include "game/spec/cost.hpp"
#include "afl/base/signal.hpp"
#include "game/map/planet.hpp"

namespace client { namespace proxy {

    class BuildStructuresProxy {
     public:
        struct HeaderInfo {
            bool ok;
            bool hasBase;
            String_t planetName;
            String_t planetInfo;

            HeaderInfo()
                : ok(false), hasBase(false), planetName(), planetInfo()
                { }
        };

        struct BuildingInfo {
            int have;
            int want;
            int max;
            int speed;
            BuildingInfo()
                : have(0), want(0), max(0), speed(0)
                { }
        };

        struct Status {
            BuildingInfo buildings[game::NUM_PLANETARY_BUILDING_TYPES];
            game::spec::Cost available;
            game::spec::Cost needed;
            game::spec::Cost remaining;
        };

        BuildStructuresProxy(util::RequestSender<game::Session> gameSender, util::RequestDispatcher& receiver);

        ~BuildStructuresProxy();

        void init(Downlink& link, game::Id_t id, HeaderInfo& info);

        void update();

        void addLimitCash(game::PlanetaryBuilding type, int count);

        void doStandardAutoBuild();

        void applyAutobuildSettings(const game::map::Planet::AutobuildSettings& settings);

        void commit();

        afl::base::Signal<void(const Status&)> sig_statusChange;

     private:
        struct Trampoline;
        util::RequestReceiver<BuildStructuresProxy> m_receiver;
        util::SlaveRequestSender<game::Session, Trampoline> m_sender;
    };

} }

#endif
