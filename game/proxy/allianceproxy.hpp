/**
  *  \file game/proxy/allianceproxy.hpp
  *  \brief Class game::proxy::AllianceProxy
  */
#ifndef C2NG_GAME_PROXY_ALLIANCEPROXY_HPP
#define C2NG_GAME_PROXY_ALLIANCEPROXY_HPP

#include "game/alliance/container.hpp"
#include "game/playerarray.hpp"
#include "game/playerset.hpp"
#include "game/session.hpp"
#include "util/requestsender.hpp"

namespace game { namespace proxy {

    class WaitIndicator;

    /** Alliance proxy.
        Allows retrieving and updating alliance information.

        Bidirectional, synchronous:
        - initialisation (retrieve alliance settings)

        Asynchronous:
        - modification (update the alliance settings)

        @see game::alliance::Container */
    class AllianceProxy {
     public:
        /** Status.
            Contains the alliance settings and, for convenience, player names. */
        struct Status {
            game::alliance::Container alliances;      ///< Alliance settings.
            PlayerArray<String_t> playerNames;        ///< Player names.
            PlayerSet_t players;                      ///< Set of available players.
            int viewpointPlayer;                      ///< Viewpoint player.

            Status()
                : alliances(), playerNames(), players(), viewpointPlayer()
                { }
        };

        /** Constructor.
            @param gameSender Game sender */
        explicit AllianceProxy(util::RequestSender<Session> gameSender);

        /** Get status (initialize).
            @param ind WaitIndicator for UI synchronisation
            @return status */
        Status getStatus(WaitIndicator& ind);

        /** Update alliances.
            Updates the game from the given alliance container.
            The update happens asynchronously in the background.
            @param alliances Alliances; should be same as getStatus().alliances, with own offers updated. */
        void setAlliances(const game::alliance::Container& alliances);

     private:
        util::RequestSender<Session> m_gameSender;
    };

} }

#endif
