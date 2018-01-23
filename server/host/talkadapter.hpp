/**
  *  \file server/host/talkadapter.hpp
  *  \brief Class server::host::TalkAdapter
  */
#ifndef C2NG_SERVER_HOST_TALKADAPTER_HPP
#define C2NG_SERVER_HOST_TALKADAPTER_HPP

#include "server/host/talklistener.hpp"
#include "server/interface/talkforum.hpp"

namespace server { namespace host {

    /** Implementation of forum-related actions.
        TalkAdapter uses the TalkForum interface to talk to a c2talk instance */
    class TalkAdapter : public TalkListener {
     public:
        /** Constructor.
            \param forum TalkForum instance */
        explicit TalkAdapter(server::interface::TalkForum& forum);

        // TalkListener methods:
        virtual void handleGameStart(Game& game, server::interface::HostGame::Type gameType);
        virtual void handleGameEnd(Game& game, server::interface::HostGame::Type gameType);
        virtual void handleGameNameChange(Game& game, const String_t& newName);
        virtual void handleGameTypeChange(Game& game, server::interface::HostGame::State gameState, server::interface::HostGame::Type gameType);

     private:
        server::interface::TalkForum& m_forum;
    };

} }

#endif
