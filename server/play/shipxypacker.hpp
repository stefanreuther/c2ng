/**
  *  \file server/play/shipxypacker.hpp
  *  \brief Class server::play::ShipXYPacker
  */
#ifndef C2NG_SERVER_PLAY_SHIPXYPACKER_HPP
#define C2NG_SERVER_PLAY_SHIPXYPACKER_HPP

#include "server/play/packer.hpp"
#include "game/session.hpp"

namespace server { namespace play {

    /** Packer for "obj/shipxy".
        Publishes ship core properties.
        The idea is that shipxy provides enough info to render a starchart.
        In addition, it defines the valid ship Id range. */
    class ShipXYPacker : public Packer {
     public:
        /** Constructor.
            @param session Session */
        ShipXYPacker(game::Session& session);

        // Packer:
        Value_t* buildValue() const;
        String_t getName() const;

     private:
        game::Session& m_session;
    };

} }

#endif
