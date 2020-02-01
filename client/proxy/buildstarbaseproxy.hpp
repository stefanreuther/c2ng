/**
  *  \file client/proxy/buildstarbaseproxy.hpp
  */
#ifndef C2NG_CLIENT_PROXY_BUILDSTARBASEPROXY_HPP
#define C2NG_CLIENT_PROXY_BUILDSTARBASEPROXY_HPP

#include "client/downlink.hpp"
#include "game/types.hpp"
#include "game/spec/cost.hpp"
#include "util/slaverequestsender.hpp"
#include "util/requestsender.hpp"
#include "game/session.hpp"

namespace client { namespace proxy {

    class BuildStarbaseProxy {
     public:
        enum Mode {
            Error,
            CanBuild,
            CannotBuild,
            CanCancel
        };

        struct Status {
            Mode mode;
            game::spec::Cost available;
            game::spec::Cost cost;
            game::spec::Cost remaining;
            game::spec::Cost missing;
            String_t errorMessage;

            Status()
                : mode(Error),
                  available(), cost(), remaining(), missing(), errorMessage()
                { }
        };

        BuildStarbaseProxy(util::RequestSender<game::Session> gameSender);

        ~BuildStarbaseProxy();

        void init(Downlink& link, game::Id_t id, Status& status);

        void commit(Downlink& link);

     private:
        struct Trampoline;
        util::SlaveRequestSender<game::Session, Trampoline> m_sender;
    };

} }

#endif
