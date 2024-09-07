/**
  *  \file server/play/mainpacker.hpp
  *  \brief Class server::play::MainPacker
  */
#ifndef C2NG_SERVER_PLAY_MAINPACKER_HPP
#define C2NG_SERVER_PLAY_MAINPACKER_HPP

#include "server/play/packer.hpp"
#include "game/session.hpp"
#include "game/extra.hpp"

namespace server { namespace play {

    /** Packer for "obj/main".
        Publishes global properties.
        In addition, provides a way to publish ad-hoc key/value pairs. */
    class MainPacker : public Packer {
     public:
        /** Constructor.
            @param session Session */
        MainPacker(game::Session& session);

        // Packer:
        Value_t* buildValue() const;
        String_t getName() const;

     private:
        game::Session& m_session;
    };

    /** Access session's ad-hoc properties.
        Attaches a key/value store to the session as a session Extra.
        Values you store in it are published by MainPacker.
        @param session Session
        @return key/value store */
    std::map<String_t, String_t>& getSessionProperties(game::Session& session);

} }

#endif
