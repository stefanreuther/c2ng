/**
  *  \file server/play/shippacker.hpp
  *  \brief Class server::play::ShipPacker
  */
#ifndef C2NG_SERVER_PLAY_SHIPPACKER_HPP
#define C2NG_SERVER_PLAY_SHIPPACKER_HPP

#include "game/session.hpp"
#include "server/play/packer.hpp"

namespace server { namespace play {

    /** Packer for "obj/shipX".
        Provides information about a ship. */
    class ShipPacker : public Packer {
     public:
        /** Constructor.
            @param session  Session (must have ShipList, Root, Game)
            @param shipNr   Ship Id */
        ShipPacker(game::Session& session, int shipNr);

        // Packer:
        Value_t* buildValue() const;
        String_t getName() const;

     private:
        game::Session& m_session;
        const int m_shipNr;
    };

} }

#endif
