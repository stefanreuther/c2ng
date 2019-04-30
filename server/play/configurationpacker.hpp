/**
  *  \file server/play/configurationpacker.hpp
  */
#ifndef C2NG_SERVER_PLAY_CONFIGURATIONPACKER_HPP
#define C2NG_SERVER_PLAY_CONFIGURATIONPACKER_HPP

#include "server/play/packer.hpp"
#include "game/session.hpp"

namespace server { namespace play {

    class ConfigurationPacker : public Packer {
     public:
        ConfigurationPacker(game::Session& session, int slice);

        Value_t* buildValue() const;
        String_t getName() const;

     private:
        game::Session& m_session;
        int m_slice;
    };

} }

#endif
