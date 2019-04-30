/**
  *  \file client/proxy/screenhistoryproxy.hpp
  */
#ifndef C2NG_CLIENT_PROXY_SCREENHISTORYPROXY_HPP
#define C2NG_CLIENT_PROXY_SCREENHISTORYPROXY_HPP

#include "client/downlink.hpp"
#include "util/requestsender.hpp"
#include "game/session.hpp"
#include "client/screenhistory.hpp"
#include "afl/base/memory.hpp"
#include "afl/base/growablememory.hpp"

namespace client { namespace proxy {

    class ScreenHistoryProxy {
     public:
        ScreenHistoryProxy(util::RequestSender<game::Session> gameSender);

        bool validateReference(Downlink& link, ScreenHistory::Reference ref);

        void validateReferences(Downlink& link, afl::base::Memory<const ScreenHistory::Reference> refs, afl::base::GrowableMemory<bool>& result);

        String_t getReferenceName(Downlink& link, ScreenHistory::Reference ref);

        void getReferenceNames(Downlink& link, afl::base::Memory<const ScreenHistory::Reference> refs, afl::base::GrowableMemory<String_t>& result);

        bool activateReference(Downlink& link, ScreenHistory::Reference ref);

     private:
        util::RequestSender<game::Session> m_gameSender;
    };

} }

#endif
