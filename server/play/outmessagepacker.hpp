/**
  *  \file server/play/outmessagepacker.hpp
  *  \brief Class server::play::OutMessagePacker
  */
#ifndef C2NG_SERVER_PLAY_OUTMESSAGEPACKER_HPP
#define C2NG_SERVER_PLAY_OUTMESSAGEPACKER_HPP

#include "server/play/packer.hpp"
#include "game/session.hpp"
#include "game/types.hpp"

namespace server { namespace play {

    /** Packer for "obj/outmsgX": single outgoing message. */
    class OutMessagePacker : public Packer {
     public:
        /** Constructor.
            \param session Session
            \param id Message Id, see game::msg::Outbox::getMessageId() */
        OutMessagePacker(game::Session& session, game::Id_t id);

        // Packer:
        Value_t* buildValue() const;
        String_t getName() const;

     private:
        game::Session& m_session;
        game::Id_t m_id;
    };

} }

#endif
