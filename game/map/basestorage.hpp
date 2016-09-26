/**
  *  \file game/map/basestorage.hpp
  */
#ifndef C2NG_GAME_MAP_BASESTORAGE_HPP
#define C2NG_GAME_MAP_BASESTORAGE_HPP

#include <vector>
#include "game/types.hpp"

namespace game { namespace map {

    class BaseStorage {
     public:
        BaseStorage();

        ~BaseStorage();

        void set(int slot, IntegerProperty_t amount);

        IntegerProperty_t get(int slot) const;

        const IntegerProperty_t* at(int slot) const;

        IntegerProperty_t* at(int slot);

        bool isValid() const;

     private:
        std::vector<IntegerProperty_t> m_content;
    };

} }

#endif
