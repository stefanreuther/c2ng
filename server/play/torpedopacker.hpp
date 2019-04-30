/**
  *  \file server/play/torpedopacker.hpp
  */
#ifndef C2NG_SERVER_PLAY_TORPEDOPACKER_HPP
#define C2NG_SERVER_PLAY_TORPEDOPACKER_HPP

#include "server/play/packer.hpp"
#include "game/session.hpp"

namespace server { namespace play {

    class TorpedoPacker : public Packer {
     public:
        TorpedoPacker(game::Session& session);

        Value_t* buildValue() const;
        String_t getName() const;

     private:
        game::Session& m_session;
    };

} }

#endif
