/**
  *  \file server/play/ionstormpacker.hpp
  */
#ifndef C2NG_SERVER_PLAY_IONSTORMPACKER_HPP
#define C2NG_SERVER_PLAY_IONSTORMPACKER_HPP

#include "game/session.hpp"
#include "server/play/packer.hpp"

namespace server { namespace play {

    class IonStormPacker : public Packer {
     public:
        IonStormPacker(game::Session& session);

        Value_t* buildValue() const;
        String_t getName() const;

     private:
        game::Session& m_session;
    };

} }

#endif
