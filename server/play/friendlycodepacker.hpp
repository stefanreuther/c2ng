/**
  *  \file server/play/friendlycodepacker.hpp
  *  \brief Class server::play::FriendlyCodePacker
  */
#ifndef C2NG_SERVER_PLAY_FRIENDLYCODEPACKER_HPP
#define C2NG_SERVER_PLAY_FRIENDLYCODEPACKER_HPP

#include "server/play/packer.hpp"
#include "game/session.hpp"

namespace server { namespace play {

    /** Packer for "obj/fcode" (friendly code list).
        Produces array of NAME, DESCRIPTION, FLAGS, RACES with friendly-code properties.
        @since PCC2 2.40.9 */
    class FriendlyCodePacker : public Packer {
     public:
        /** Constructor.
            \param session Session (must have Root, ShipList) */
        explicit FriendlyCodePacker(game::Session& session);

        // Packer:
        virtual Value_t* buildValue() const;
        virtual String_t getName() const;

     private:
        game::Session& m_session;
    };

} }

#endif
