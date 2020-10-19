/**
  *  \file server/play/beampacker.hpp
  *  \brief Class server::play::BeamPacker
  */
#ifndef C2NG_SERVER_PLAY_BEAMPACKER_HPP
#define C2NG_SERVER_PLAY_BEAMPACKER_HPP

#include "server/play/packer.hpp"
#include "game/session.hpp"

namespace server { namespace play {

    /** Packer for "obj/beam". */
    class BeamPacker : public Packer {
     public:
        /** Constructor.
            \param session Session
            \see game::interface::BeamContext */
        explicit BeamPacker(game::Session& session);

        // Packer:
        virtual Value_t* buildValue() const;
        virtual String_t getName() const;

     private:
        game::Session& m_session;
    };

} }

#endif
