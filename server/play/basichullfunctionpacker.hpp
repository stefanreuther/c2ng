/**
  *  \file server/play/basichullfunctionpacker.hpp
  *  \brief Class server::play::BasicHullFunctionPacker
  */
#ifndef C2NG_SERVER_PLAY_BASICHULLFUNCTIONPACKER_HPP
#define C2NG_SERVER_PLAY_BASICHULLFUNCTIONPACKER_HPP

#include "game/session.hpp"
#include "server/play/packer.hpp"

namespace server { namespace play {

    /** Packer for "obj/zab" (ship abilities). */
    class BasicHullFunctionPacker : public Packer {
     public:
        /** Constructor.
            \param session Session */
        explicit BasicHullFunctionPacker(game::Session& session);

        virtual Value_t* buildValue() const;
        virtual String_t getName() const;

     private:
        game::Session& m_session;
    };

} }

#endif
