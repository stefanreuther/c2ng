/**
  *  \file game/proxy/reverterproxy.hpp
  *  \brief Class game::proxy::ReverterProxy
  */
#ifndef C2NG_GAME_PROXY_REVERTERPROXY_HPP
#define C2NG_GAME_PROXY_REVERTERPROXY_HPP

#include "game/map/locationreverter.hpp"
#include "game/map/point.hpp"
#include "game/proxy/waitindicator.hpp"
#include "game/ref/userlist.hpp"
#include "game/session.hpp"
#include "util//requestsender.hpp"

namespace game { namespace proxy {

    /** Starbase building proxy.

        Bidirectional, synchronous:
        - set up and retrieve status (init())

        Asynchronous:
        - commit()

        \see game::map::Reverter, game::map::LocationReverter */
    class ReverterProxy {
     public:
        typedef game::map::LocationReverter::Modes_t Modes_t;

        struct Status {
            Modes_t modes;
            game::ref::UserList list;
        };

        /** Constructor.
            \param gameSender Game sender */
        ReverterProxy(util::RequestSender<Session> gameSender);

        /** Destructor. */
        ~ReverterProxy();

        /** Initialize.
            \param [in]  link   WaitIndicator
            \param [in]  pt     Position
            \param [out] status Transaction status */
        void init(WaitIndicator& link, game::map::Point pt, Status& status);

        /** Commit.
            \param modes Modes to use */
        void commit(Modes_t modes);

     private:
        struct Trampoline;
        class TrampolineFromSession;
        util::RequestSender<Trampoline> m_sender;
    };

} }

#endif
