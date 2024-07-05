/**
  *  \file server/play/messagepacker.hpp
  *  \brief Class server::play::MessagePacker
  */
#ifndef C2NG_SERVER_PLAY_MESSAGEPACKER_HPP
#define C2NG_SERVER_PLAY_MESSAGEPACKER_HPP

#include "server/play/packer.hpp"
#include "game/session.hpp"

namespace server { namespace play {

    /** Packer for "obj/msgX".
        Publishes inbox messages. */
    class MessagePacker : public Packer {
     public:
        /** Constructor.
            @param session Session
            @param index   1-based index */
        MessagePacker(game::Session& session, int index);

        // Packer:
        Value_t* buildValue() const;
        String_t getName() const;

     private:
        game::Session& m_session;
        int m_index;
    };

} }

#endif
