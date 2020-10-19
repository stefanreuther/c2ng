/**
  *  \file server/play/configurationpacker.hpp
  *  \brief Class server::play::ConfigurationPacker
  */
#ifndef C2NG_SERVER_PLAY_CONFIGURATIONPACKER_HPP
#define C2NG_SERVER_PLAY_CONFIGURATIONPACKER_HPP

#include "server/play/packer.hpp"
#include "game/session.hpp"

namespace server { namespace play {

    /** Packer for "obj/cfgX". */
    class ConfigurationPacker : public Packer {
     public:
        /** Constructor.
            \param session Session
            \param slice   Slice (0=everything, others=subset) */
        ConfigurationPacker(game::Session& session, int slice);

        virtual Value_t* buildValue() const;
        virtual String_t getName() const;

     private:
        game::Session& m_session;
        int m_slice;
    };

} }

#endif
