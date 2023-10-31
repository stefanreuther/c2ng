/**
  *  \file server/play/basichullfunctionpacker.hpp
  *  \brief Class server::play::BasicHullFunctionPacker
  */
#ifndef C2NG_SERVER_PLAY_BASICHULLFUNCTIONPACKER_HPP
#define C2NG_SERVER_PLAY_BASICHULLFUNCTIONPACKER_HPP

#include "game/spec/shiplist.hpp"
#include "server/play/packer.hpp"

namespace server { namespace play {

    /** Packer for "obj/zab" (ship abilities). */
    class BasicHullFunctionPacker : public Packer {
     public:
        /** Constructor.
            @param shipList ShipList instance */
        explicit BasicHullFunctionPacker(const game::spec::ShipList& shipList);

        // Packer:
        virtual Value_t* buildValue() const;
        virtual String_t getName() const;

     private:
        const game::spec::ShipList& m_shipList;
    };

} }

#endif
