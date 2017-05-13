/**
  *  \file game/spec/hullfunctionlist.hpp
  *  \brief Class game::spec::HullFunctionList
  */
#ifndef C2NG_GAME_SPEC_HULLFUNCTIONLIST_HPP
#define C2NG_GAME_SPEC_HULLFUNCTIONLIST_HPP

#include <vector>
#include "game/playerset.hpp"
#include "game/spec/hullfunction.hpp"

namespace game { namespace spec {

    /** List of hull functions.
        This wraps a vector-of-HullFunction, and offers nice useful operations on it. */
    class HullFunctionList {
     public:
        typedef std::vector<HullFunction> Container_t;
        typedef Container_t::const_iterator Iterator_t;

        /** Default constructor.
            Makes a blank list. */
        HullFunctionList();

        /** Destructor. */
        ~HullFunctionList();

        /** Append new item at end.
            \param f new item */
        void add(const HullFunction& f);

        /** Clear list. */
        void clear();

        /** Simplify the list.
            This prepares the list for the user to see. */
        void simplify();

        /** Sort list for new ships.
            Brings it into a state that is useful for attaching it with a new (not-yet-built) ship.
            \param player player who's going to own the ship */
        void sortForNewShip(PlayerSet_t forPlayer);

        /*
         *  Container accessor interface
         */

        /** Get number of items in list. */
        size_t size() const;

        /** Get iterator to first item. */
        Iterator_t begin() const;

        /** Get iterator to one-past-end. */
        Iterator_t end() const;

        /** Indexed access.
            \param i Index, [0,size()) */
        const HullFunction& operator[](size_t index) const;

     private:
        Container_t m_data;
    };

} }

#endif
