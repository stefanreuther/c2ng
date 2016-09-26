/**
  *  \file game/element.hpp
  */
#ifndef C2NG_GAME_ELEMENT_HPP
#define C2NG_GAME_ELEMENT_HPP

#include "afl/bits/smallset.hpp"

namespace game {

    struct Element {
        enum Type {
            Neutronium,              ///< Neutronium.
            Tritanium,               ///< Tritanium.
            Duranium,                ///< Duranium.
            Molybdenum,              ///< Molybdenum.
            Fighters,                ///< Fighters.
            Colonists,               ///< Colonist clans.
            Supplies,                ///< Supplies.
            Money,                   ///< Cash.
            FirstTorpedo             ///< Torps. \see getTorpTypeFromCargoType(), getCargoTypeFromTorpType()
        };

        static Type fromTorpedoType(int torpedoType);

        static bool isTorpedoType(Type t, int& torpedoType);
    };

    Element::Type& operator++(Element::Type& t);
    Element::Type& operator--(Element::Type& t);
    Element::Type operator++(Element::Type& t, int);
    Element::Type operator--(Element::Type& t, int);

//     typedef SmallSet<GCargoType> GCargoTypeSet;

//     /* bummer. Why can't C++ have these built-in? */
//     inline GCargoType operator++(GCargoType& t) { return t = GCargoType(t+1); }
//     inline GCargoType operator--(GCargoType& t) { return t = GCargoType(t-1); }
//     inline GCargoType operator++(GCargoType& t,int) { GCargoType rv = t; ++t; return rv; }
//     inline GCargoType operator--(GCargoType& t,int) { GCargoType rv = t; --t; return rv; }

//     /** Get torpedo type number from cargo type. */
//     inline int
//     getTorpTypeFromCargoType(GCargoType type)
//     {
//         ASSERT(type >= el_Torps && type < el_Torps + NUM_TORPS);
//         return type - el_Torps + 1;
//     }

//     /** Check whether cargo type corresponds to a torpedo type. */
//     inline int
//     isCargoTorpedo(GCargoType type)
//     {
//         return type >= el_Torps && type < el_Torps + NUM_TORPS;
//     }

//     /** Get cargo type from torpedo type.
//         \param type torpedo type, [1,NUM_TORPS] */
//     inline GCargoType
//     getCargoTypeFromTorpType(int type)
//     {
//         ASSERT(type > 0 && type <= NUM_TORPS);
//         return GCargoType(el_Torps + type - 1);
//     }

//     string_t getCargoName(GCargoType type);
//     string_t getCargoUnit(GCargoType type);

}

#endif
