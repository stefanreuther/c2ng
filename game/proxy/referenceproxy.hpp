/**
  *  \file game/proxy/referenceproxy.hpp
  *  \brief Class game::proxy::ReferenceProxy
  */
#ifndef C2NG_GAME_PROXY_REFERENCEPROXY_HPP
#define C2NG_GAME_PROXY_REFERENCEPROXY_HPP

#include "game/reference.hpp"
#include "game/session.hpp"
#include "game/types.hpp"
#include "util/requestsender.hpp"

namespace game { namespace proxy {

    class WaitIndicator;

    /** Synchronous, bidirectional proxy to access properties of a Reference. */
    class ReferenceProxy {
     public:
        /** Constructor.
            @param gameSender Game sender */
        explicit ReferenceProxy(util::RequestSender<Session> gameSender);

        /** Get name, given a reference.
            @param [in]  ind    WaitIndicator
            @param [in]  ref    Reference
            @param [in]  which  Which name to return
            @param [out] result Name returned here
            @retval true Name returned
            @retval false Invalid reference; name cannot be returned
            @see game::Session::getReferenceName() */
        bool getReferenceName(WaitIndicator& ind, Reference ref, ObjectName which, String_t& result);

     private:
        util::RequestSender<Session> m_gameSender;
    };

} }

#endif
