/**
  *  \file server/play/shippacker.hpp
  */
#ifndef C2NG_SERVER_PLAY_SHIPPACKER_HPP
#define C2NG_SERVER_PLAY_SHIPPACKER_HPP

#include "game/session.hpp"
#include "server/play/packer.hpp"

namespace server { namespace play {

    class ShipPacker : public Packer {
     public:
        ShipPacker(game::Session& session, int shipNr);

        Value_t* buildValue() const;
        String_t getName() const;

     private:
        game::Session& m_session;
        int m_shipNr;
    };

} }

#endif
