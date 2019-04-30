/**
  *  \file server/play/shipxypacker.hpp
  */
#ifndef C2NG_SERVER_PLAY_SHIPXYPACKER_HPP
#define C2NG_SERVER_PLAY_SHIPXYPACKER_HPP

#include "server/play/packer.hpp"
#include "game/session.hpp"

namespace server { namespace play {

    class ShipXYPacker : public Packer {
     public:
        ShipXYPacker(game::Session& session);

        Value_t* buildValue() const;
        String_t getName() const;

     private:
        game::Session& m_session;
    };

} }

#endif
