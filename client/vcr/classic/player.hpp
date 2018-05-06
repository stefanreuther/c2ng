/**
  *  \file client/vcr/classic/player.hpp
  */
#ifndef C2NG_CLIENT_VCR_CLASSIC_PLAYER_HPP
#define C2NG_CLIENT_VCR_CLASSIC_PLAYER_HPP

#include "util/slaveobject.hpp"
#include "game/session.hpp"
#include "afl/base/deletable.hpp"
#include "util/requestsender.hpp"
#include "util/stringinstructionlist.hpp"
#include "game/vcr/classic/eventrecorder.hpp"
#include "game/vcr/classic/eventvisualizer.hpp"
#include "game/vcr/classic/algorithm.hpp"
#include "util/slaverequestsender.hpp"

namespace client { namespace vcr { namespace classic {

    class PlayerListener : public afl::base::Deletable {
     public:
        virtual void handleEvents(util::StringInstructionList& list, bool finish) = 0;
    };

    class Player : public util::SlaveObject<game::Session> {
     public:
        Player(util::RequestSender<PlayerListener> reply);

        virtual void init(game::Session& session);
        virtual void done(game::Session& session);

        void initRequest(game::Session& session, size_t index);
        void eventRequest(game::Session& session);
        void jumpRequest(game::Session& session, game::vcr::classic::Time_t time);

        static void sendInitRequest(util::SlaveRequestSender<game::Session,Player>& sender, size_t index);
        static void sendEventRequest(util::SlaveRequestSender<game::Session,Player>& sender);
        static void sendJumpRequest(util::SlaveRequestSender<game::Session,Player>& sender, game::vcr::classic::Time_t time);

     private:
        void sendResponse(game::Session& session, bool finish);

        util::RequestSender<PlayerListener> m_reply;
        game::vcr::classic::EventRecorder m_recorder;
        game::vcr::classic::EventVisualizer m_visualizer;
        std::auto_ptr<game::vcr::classic::Algorithm> m_algorithm;
        size_t m_index;
    };

} } }

#endif
