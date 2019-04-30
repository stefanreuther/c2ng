/**
  *  \file game/map/basestorage.hpp
  */
#ifndef C2NG_GAME_MAP_BASESTORAGE_HPP
#define C2NG_GAME_MAP_BASESTORAGE_HPP

#include <vector>
#include "game/types.hpp"
#include "util/vector.hpp"

namespace game { namespace map {

    class BaseStorage : public util::Vector<IntegerProperty_t, int> {
     public:
        BaseStorage();

        ~BaseStorage();

        bool isValid() const;
    };

} }

inline
game::map::BaseStorage::BaseStorage()
    : util::Vector<IntegerProperty_t, int>(1)
{ }

inline
game::map::BaseStorage::~BaseStorage()
{ }

#endif
