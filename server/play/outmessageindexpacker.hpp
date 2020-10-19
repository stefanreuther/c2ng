/**
  *  \file server/play/outmessageindexpacker.hpp
  *  \brief Class server::play::OutMessageIndexPacker
  */
#ifndef C2NG_SERVER_PLAY_OUTMESSAGEINDEXPACKER_HPP
#define C2NG_SERVER_PLAY_OUTMESSAGEINDEXPACKER_HPP

#include "server/play/packer.hpp"
#include "game/session.hpp"

namespace server { namespace play {

    /** Packer for "obj/outidx": outgoing message index. */
    class OutMessageIndexPacker : public Packer {
     public:
        /** Constructor.
            \param session Session */
        explicit OutMessageIndexPacker(game::Session& session);

        Value_t* buildValue() const;
        String_t getName() const;

     private:
        game::Session& m_session;
    };

} }

#endif
