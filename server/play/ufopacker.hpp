/**
  *  \file server/play/ufopacker.hpp
  */
#ifndef C2NG_SERVER_PLAY_UFOPACKER_HPP
#define C2NG_SERVER_PLAY_UFOPACKER_HPP

#include "game/session.hpp"
#include "server/play/packer.hpp"

namespace server { namespace play {

    class UfoPacker : public Packer {
     public:
        UfoPacker(game::Session& session);

        Value_t* buildValue() const;
        String_t getName() const;

     private:
        game::Session& m_session;
    };

} }

#endif
