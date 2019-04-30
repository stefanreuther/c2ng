/**
  *  \file server/play/playerpacker.hpp
  */
#ifndef C2NG_SERVER_PLAY_PLAYERPACKER_HPP
#define C2NG_SERVER_PLAY_PLAYERPACKER_HPP

#include "server/play/packer.hpp"
#include "game/session.hpp"

namespace server { namespace play {

    class PlayerPacker : public Packer {
     public:
        PlayerPacker(game::Session& session);

        Value_t* buildValue() const;
        String_t getName() const;

     private:
        game::Session& m_session;
    };

} }

#endif
