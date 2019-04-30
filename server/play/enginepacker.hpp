/**
  *  \file server/play/enginepacker.hpp
  */
#ifndef C2NG_SERVER_PLAY_ENGINEPACKER_HPP
#define C2NG_SERVER_PLAY_ENGINEPACKER_HPP

#include "server/play/packer.hpp"
#include "game/session.hpp"

namespace server { namespace play {

    class EnginePacker : public Packer {
     public:
        EnginePacker(game::Session& session);

        Value_t* buildValue() const;
        String_t getName() const;

     private:
        game::Session& m_session;
    };

} }

#endif
