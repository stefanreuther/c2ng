/**
  *  \file server/play/torpedopacker.hpp
  *  \brief Class server::play::TorpedoPacker
  */
#ifndef C2NG_SERVER_PLAY_TORPEDOPACKER_HPP
#define C2NG_SERVER_PLAY_TORPEDOPACKER_HPP

#include "server/play/packer.hpp"
#include "game/session.hpp"

namespace server { namespace play {

    /** Packer for "obj/torp". */
    class TorpedoPacker : public Packer {
     public:
        /** Constructor.
            \param session Session
            \see game::interface::TorpedoContext */
        explicit TorpedoPacker(game::Session& session);

        virtual Value_t* buildValue() const;
        virtual String_t getName() const;

     private:
        game::Session& m_session;
    };

} }

#endif
