/**
  *  \file server/play/planetpacker.hpp
  */
#ifndef C2NG_SERVER_PLAY_PLANETPACKER_HPP
#define C2NG_SERVER_PLAY_PLANETPACKER_HPP

#include "game/session.hpp"
#include "server/play/packer.hpp"

namespace server { namespace play {

    class PlanetPacker : public Packer {
     public:
        PlanetPacker(game::Session& session, int planetNr);

        Value_t* buildValue() const;
        String_t getName() const;

     private:
        game::Session& m_session;
        int m_planetNr;
    };

} }

#endif
