/**
  *  \file server/play/truehullpacker.hpp
  */
#ifndef C2NG_SERVER_PLAY_TRUEHULLPACKER_HPP
#define C2NG_SERVER_PLAY_TRUEHULLPACKER_HPP

#include "server/play/packer.hpp"
#include "game/session.hpp"

namespace server { namespace play {

    class TruehullPacker : public Packer {
     public:
        TruehullPacker(game::Session& session);

        Value_t* buildValue() const;
        String_t getName() const;

     private:
        game::Session& m_session;
    };

} }

#endif
