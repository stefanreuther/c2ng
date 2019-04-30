/**
  *  \file server/play/planetxypacker.hpp
  */
#ifndef C2NG_SERVER_PLAY_PLANETXYPACKER_HPP
#define C2NG_SERVER_PLAY_PLANETXYPACKER_HPP

#include "server/play/packer.hpp"
#include "game/session.hpp"

namespace server { namespace play {

    class PlanetXYPacker : public Packer {
     public:
        PlanetXYPacker(game::Session& session);

        Value_t* buildValue() const;
        String_t getName() const;

     private:
        game::Session& m_session;
    };

} }

#endif
