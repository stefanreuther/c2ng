/**
  *  \file server/play/mainpacker.hpp
  */
#ifndef C2NG_SERVER_PLAY_MAINPACKER_HPP
#define C2NG_SERVER_PLAY_MAINPACKER_HPP

#include "server/play/packer.hpp"
#include "game/session.hpp"
#include "game/extra.hpp"

namespace server { namespace play {

    class MainPacker : public Packer {
     public:
        MainPacker(game::Session& session);

        Value_t* buildValue() const;
        String_t getName() const;

     private:
        game::Session& m_session;
    };

    std::map<String_t, String_t>& getSessionProperties(game::Session& session);

} }

#endif
