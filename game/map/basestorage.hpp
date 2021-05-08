/**
  *  \file game/map/basestorage.hpp
  *  \brief Class game::map::BaseStorage
  */
#ifndef C2NG_GAME_MAP_BASESTORAGE_HPP
#define C2NG_GAME_MAP_BASESTORAGE_HPP

#include "game/types.hpp"
#include "util/vector.hpp"

namespace game { namespace map {

    /** Base storage.
        Used for implementing actual base storage access.
        This is now mostly a util::Vector.
        - accepts indexes starting at 1.
        - size() is one-past-last element, i.e. for a v3 starbase, the hull storage has size()=21.

        As a guideline, only the loader will create slots in BaseStorage. */
    class BaseStorage : public util::Vector<IntegerProperty_t, int> {
     public:
        /** Constructor. */
        BaseStorage();

        /** Destructor. */
        ~BaseStorage();

        /** Check validity.
            \return true if this contains any valid value (=base data not empty) */
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
