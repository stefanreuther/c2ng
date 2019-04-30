/**
  *  \file server/play/beampacker.hpp
  */
#ifndef C2NG_SERVER_PLAY_BEAMPACKER_HPP
#define C2NG_SERVER_PLAY_BEAMPACKER_HPP

#include "server/play/packer.hpp"
#include "game/session.hpp"

namespace server { namespace play {

    class BeamPacker : public Packer {
     public:
        BeamPacker(game::Session& session);

        Value_t* buildValue() const;
        String_t getName() const;

     private:
        game::Session& m_session;
    };

} }

#endif
