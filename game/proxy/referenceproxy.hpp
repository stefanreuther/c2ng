/**
  *  \file game/proxy/referenceproxy.hpp
  *  \brief Class game::proxy::ReferenceProxy
  */
#ifndef C2NG_GAME_PROXY_REFERENCEPROXY_HPP
#define C2NG_GAME_PROXY_REFERENCEPROXY_HPP

#include "afl/base/optional.hpp"
#include "game/map/point.hpp"
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
            @return Name if reference is valid and name could be produced
            @see game::Session::getReferenceName() */
        afl::base::Optional<String_t> getReferenceName(WaitIndicator& ind, Reference ref, ObjectName which);

        /** Get position, given a reference.
            @param [in]  ind    WaitIndicator
            @param [in]  ref    Reference */
        afl::base::Optional<game::map::Point> getReferencePosition(WaitIndicator& ind, Reference ref);

     private:
        util::RequestSender<Session> m_gameSender;
    };

} }

#endif
