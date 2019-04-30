/**
  *  \file server/play/vcrpacker.hpp
  */
#ifndef C2NG_SERVER_PLAY_VCRPACKER_HPP
#define C2NG_SERVER_PLAY_VCRPACKER_HPP

#include "game/session.hpp"
#include "server/play/packer.hpp"

namespace server { namespace play {

    class VcrPacker : public Packer {
     public:
        VcrPacker(game::Session& session);

        Value_t* buildValue() const;
        String_t getName() const;

     private:
        game::Session& m_session;
    };

} }

#endif
