/**
  *  \file server/play/minefieldpacker.hpp
  */
#ifndef C2NG_SERVER_PLAY_MINEFIELDPACKER_HPP
#define C2NG_SERVER_PLAY_MINEFIELDPACKER_HPP

#include "game/session.hpp"
#include "server/play/packer.hpp"

namespace server { namespace play {

    class MinefieldPacker : public Packer {
     public:
        MinefieldPacker(game::Session& session);

        Value_t* buildValue() const;
        String_t getName() const;

     private:
        game::Session& m_session;
    };

} }

#endif
