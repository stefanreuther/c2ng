/**
  *  \file server/play/planetxypacker.hpp
  *  \brief Class server::play::PlanetXYPacker
  */
#ifndef C2NG_SERVER_PLAY_PLANETXYPACKER_HPP
#define C2NG_SERVER_PLAY_PLANETXYPACKER_HPP

#include "server/play/packer.hpp"
#include "game/session.hpp"

namespace server { namespace play {

    /** Packer for "obj/planetxy".
        Publishes planet core properties.
        The idea is that planetxy provides enough info to render a starchart.
        In addition, it defines the valid planet Id range. */
    class PlanetXYPacker : public Packer {
     public:
        /** Constructor.
            @param session Session */
        PlanetXYPacker(game::Session& session);

        // Packer:
        Value_t* buildValue() const;
        String_t getName() const;

     private:
        game::Session& m_session;
    };

} }

#endif
