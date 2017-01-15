/**
  *  \file game/specificationloader.hpp
  */
#ifndef C2NG_GAME_SPECIFICATIONLOADER_HPP
#define C2NG_GAME_SPECIFICATIONLOADER_HPP

#include "afl/base/deletable.hpp"
#include "game/spec/shiplist.hpp"
#include "afl/base/refcounted.hpp"

namespace game {

    class Root;

    class SpecificationLoader : public afl::base::Deletable, public afl::base::RefCounted {
     public:
        virtual void loadShipList(game::spec::ShipList& list, Root& root) = 0;
    };

}

#endif
