/**
  *  \file server/play/hullpacker.hpp
  */
#ifndef C2NG_SERVER_PLAY_HULLPACKER_HPP
#define C2NG_SERVER_PLAY_HULLPACKER_HPP

#include "server/play/packer.hpp"
#include "game/session.hpp"

namespace server { namespace play {

    class HullPacker : public Packer {
     public:
        HullPacker(game::Session& session, int hullNr);

        Value_t* buildValue() const;
        String_t getName() const;

     private:
        game::Session& m_session;
        int m_hullNr;
    };

    Value_t* packHullFunctionList(const game::spec::HullFunctionList& list);

} }

#endif
