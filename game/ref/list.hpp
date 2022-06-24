/**
  *  \file game/ref/list.hpp
  */
#ifndef C2NG_GAME_REF_LIST_HPP
#define C2NG_GAME_REF_LIST_HPP

#include <vector>
#include "afl/bits/smallset.hpp"
#include "game/map/universe.hpp"
#include "game/reference.hpp"
#include "game/types.hpp"

namespace game { namespace ref {

    class SortPredicate;

    class List {
     public:
        enum Option {
            IncludeForeignShips,
            IncludePlanet,
            SafeShipsOnly
        };
        typedef afl::bits::SmallSet<Option> Options_t;

        typedef std::vector<Reference> Vector_t;
        typedef Vector_t::const_iterator Iterator_t;
        typedef afl::bits::SmallSet<Reference::Type> Types_t;

        List();
        ~List();

        void add(Reference ref);
        void add(Reference::Type type, const std::vector<Id_t> ids);

        /** Add all objects at a particular location.
            This is used for certain ship lists.
            \param univ          Universe
            \param pt            Location
            \param options       Options to select which objects to add
            \param excludeShipId Do not include this ship Id even if it is otherwise eligible */
        void addObjectsAt(game::map::Universe& univ, game::map::Point pt, Options_t options, Id_t excludeShipId);

        void clear();

        Reference operator[](size_t pos) const;
        void set(size_t pos, Reference ref);
        size_t size() const;
        Types_t getTypes() const;
        std::vector<Id_t> getIds(Reference::Type type) const;

        void sort(const SortPredicate& pred);

     private:
        Vector_t m_content;
    };


} }

#endif
