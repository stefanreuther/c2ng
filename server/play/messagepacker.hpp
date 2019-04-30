/**
  *  \file server/play/messagepacker.hpp
  */
#ifndef C2NG_SERVER_PLAY_MESSAGEPACKER_HPP
#define C2NG_SERVER_PLAY_MESSAGEPACKER_HPP

#include "server/play/packer.hpp"
#include "game/session.hpp"

namespace server { namespace play {

    class MessagePacker : public Packer {
     public:
        MessagePacker(game::Session& session, int index);

        Value_t* buildValue() const;
        String_t getName() const;

     private:
        game::Session& m_session;
        int m_index;
    };

} }

#endif
