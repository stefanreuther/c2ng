/**
  *  \file server/play/shipmissionpacker.hpp
  *  \brief Class server::play::ShipMissionPacker
  */
#ifndef C2NG_SERVER_PLAY_SHIPMISSIONPACKER_HPP
#define C2NG_SERVER_PLAY_SHIPMISSIONPACKER_HPP

#include "server/play/packer.hpp"
#include "game/session.hpp"

namespace server { namespace play {

    /** Packer for "query/shipmsnX".
        Publishes the list of missions, filtered for a ship. */
    class ShipMissionPacker : public Packer {
     public:
        /** Constructor.
            @param session Session
            @param shipId  Ship Id */
        ShipMissionPacker(game::Session& session, game::Id_t shipId);

        // Packer:
        virtual Value_t* buildValue() const;
        virtual String_t getName() const;

     private:
        game::Session& m_session;
        game::Id_t m_shipId;
    };

} }

#endif
