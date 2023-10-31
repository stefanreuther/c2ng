/**
  *  \file server/play/hullpacker.hpp
  *  \brief Class server::play::HullPacker
  */
#ifndef C2NG_SERVER_PLAY_HULLPACKER_HPP
#define C2NG_SERVER_PLAY_HULLPACKER_HPP

#include "server/play/packer.hpp"
#include "game/session.hpp"

namespace server { namespace play {

    /** Packer for "obj/hullX". */
    class HullPacker : public Packer {
     public:
        /** Constructor.
            @param shipList  Ship list (must be dynamically allocated)
            @param root      Root (must be dynamically allocated)
            @param hullNr    Hull number
            @see game::interface::HullContext */
        HullPacker(game::spec::ShipList& shipList, const game::Root& root, int hullNr);

        // Packer:
        Value_t* buildValue() const;
        String_t getName() const;

     private:
        game::spec::ShipList& m_shipList;
        const game::Root& m_root;
        int m_hullNr;
    };

    /** Pack a HullFunctionList.
        @param list HullFunctionList instance
        @return Newly-allocated data structure representing the HullFunctionList */
    Value_t* packHullFunctionList(const game::spec::HullFunctionList& list);

} }

#endif
