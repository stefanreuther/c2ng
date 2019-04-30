/**
  *  \file server/play/shipmissionpacker.hpp
  */
#ifndef C2NG_SERVER_PLAY_SHIPMISSIONPACKER_HPP
#define C2NG_SERVER_PLAY_SHIPMISSIONPACKER_HPP

#include "server/play/packer.hpp"
#include "game/session.hpp"

namespace server { namespace play {

    class ShipMissionPacker : public Packer {
     public:
        ShipMissionPacker(game::Session& session, game::Id_t shipId);

        virtual Value_t* buildValue() const;
        virtual String_t getName() const;

     private:
        game::Session& m_session;
        game::Id_t m_shipId;
    };

} }

#endif
