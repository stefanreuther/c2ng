/**
  *  \file server/play/flakconfigurationpacker.hpp
  *  \brief Class server::play::FlakConfigurationPacker
  */
#ifndef C2NG_SERVER_PLAY_FLAKCONFIGURATIONPACKER_HPP
#define C2NG_SERVER_PLAY_FLAKCONFIGURATIONPACKER_HPP

#include "server/play/packer.hpp"
#include "game/session.hpp"

namespace server { namespace play {

    /** Packer for "obj/flakconfig". */
    class FlakConfigurationPacker : public Packer {
     public:
        /** Constructor.
            \param session Session */
        FlakConfigurationPacker(game::Session& session);

        virtual Value_t* buildValue() const;
        virtual String_t getName() const;

     private:
        game::Session& m_session;
    };

} }

#endif
