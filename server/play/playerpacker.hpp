/**
  *  \file server/play/playerpacker.hpp
  *  \brief Class server::play::PlayerPacker
  */
#ifndef C2NG_SERVER_PLAY_PLAYERPACKER_HPP
#define C2NG_SERVER_PLAY_PLAYERPACKER_HPP

#include "server/play/packer.hpp"
#include "game/session.hpp"

namespace server { namespace play {

    /** Packer for "obj/player". */
    class PlayerPacker : public Packer {
     public:
        /** Constructor.
            @param session Session
            @see game::interface::PlayerContext */
        explicit PlayerPacker(game::Session& session);

        // Packer:
        Value_t* buildValue() const;
        String_t getName() const;

     private:
        game::Session& m_session;
    };

} }

#endif
