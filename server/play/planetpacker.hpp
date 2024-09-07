/**
  *  \file server/play/planetpacker.hpp
  *  \brief Class server::play::PlanetPacker
  */
#ifndef C2NG_SERVER_PLAY_PLANETPACKER_HPP
#define C2NG_SERVER_PLAY_PLANETPACKER_HPP

#include "game/session.hpp"
#include "server/play/packer.hpp"

namespace server { namespace play {

    /** Packer for "obj/planetX".
        Provides information about a planet. */
    class PlanetPacker : public Packer {
     public:
        /** Constructor.
            @param session  Session (must have ShipList, Root, Game)
            @param planetNr Planet Id */
        PlanetPacker(game::Session& session, int planetNr);

        // Packer:
        Value_t* buildValue() const;
        String_t getName() const;

     private:
        game::Session& m_session;
        const int m_planetNr;
    };

} }

#endif
