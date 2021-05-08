/**
  *  \file game/proxy/teamproxy.hpp
  *  \brief Class game::proxy::TeamProxy
  */
#ifndef C2NG_GAME_PROXY_TEAMPROXY_HPP
#define C2NG_GAME_PROXY_TEAMPROXY_HPP

#include "game/proxy/waitindicator.hpp"
#include "game/session.hpp"
#include "game/teamsettings.hpp"
#include "util/requestsender.hpp"

namespace game { namespace proxy {

    /** Team configuration proxy.
        Allows copying team information in and out of a game session.
        The UI can modify their copy and write back the result.

        Bidirectional, synchronous:
        - initialisation (retrieve team settings)

        Asynchronous:
        - modification (submit team settings)

        \see game::TeamSettings */
    class TeamProxy {
     public:
        /** Constructor.
            \param gameSender Sender */
        explicit TeamProxy(util::RequestSender<Session> gameSender);

        /** Initialize.
            \param [in]  link  WaitIndicator
            \param [out] out   TeamSettings object */
        void init(WaitIndicator& link, TeamSettings& out);

        /** Write back.
            This copies a TeamSettings object into the game session.
            This should be (a modified version of) the TeamSettings object produced by init().

            This function will overwrite the entire TeamSettings.
            Parallel editing means last writer wins,
            there is no tracking of individual edits.

            \param [in] in  TeamSettings object */
        void commit(const TeamSettings& in);

     private:
        util::RequestSender<Session> m_gameSender;
    };

} }

#endif
