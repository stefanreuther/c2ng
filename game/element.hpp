/**
  *  \file game/element.hpp
  *  \brief Class game::Element
  */
#ifndef C2NG_GAME_ELEMENT_HPP
#define C2NG_GAME_ELEMENT_HPP

#include "afl/bits/smallset.hpp"
#include "afl/string/translator.hpp"

namespace game {

    // Must predeclare ShipList because otherwise we'll end up with a cyclic include dependency. FIXME.
    namespace spec { class ShipList; }

    /** Element (cargo type).
        Refers to an item that can be part of a cargo transfer or buy/sell transaction.
        The actual type is Element::Type; the class Element contains operations on that. */
    struct Element {
        /** Element type. */
        enum Type {
            Neutronium,              ///< Neutronium.
            Tritanium,               ///< Tritanium.
            Duranium,                ///< Duranium.
            Molybdenum,              ///< Molybdenum.
            Fighters,                ///< Fighters.
            Colonists,               ///< Colonist clans.
            Supplies,                ///< Supplies.
            Money,                   ///< Cash.
            FirstTorpedo             ///< Torps. \see fromTorpedoType(), isTorpedoType()
        };

        /** Make element type from torpedo type.
            \param torpedoType Torpedo type (>= 1), as used for ShipList::launchers().get().
            \return element type */
        static Type fromTorpedoType(int torpedoType);

        /** Check for torpedo type.
            \param t           [in] Element type
            \param torpedoType [out] Torpedo type (>= 1)
            \retval true t represented a torpedo type; \c torpedoType has been set
            \retval false t did not represent a torpedo type; \c torpedoType not modified */
        static bool isTorpedoType(Type t, int& torpedoType);

        /** Get name of an element type.
            \param t Element type
            \param tx Translator
            \param shipList Ship list
            \return name (empty if t is invalid) */
        static String_t getName(Type t, afl::string::Translator& tx, const game::spec::ShipList& shipList);

        /** Get unit of an element type.
            \param t Element type
            \param tx Translator
            \param shipList Ship list
            \return name (can be empty) */
        static String_t getUnit(Type t, afl::string::Translator& tx, const game::spec::ShipList& shipList);

        /** Get start.
            Use for iteration:
            <pre>for (Element::Type t = Element::begin(), e = Element::end(sl); t != e; ++t)</pre>
            \return start */
        static Type begin();

        /** Get limit.
            Use for iteration:
            <pre>for (Element::Type t = Element::begin(), e = Element::end(sl); t != e; ++t)</pre>
            \return limit */
        static Type end(const game::spec::ShipList& shipList);
    };

    Element::Type& operator++(Element::Type& t);
    Element::Type& operator--(Element::Type& t);
    Element::Type operator++(Element::Type& t, int);
    Element::Type operator--(Element::Type& t, int);

    /** Set of element types. */
    typedef afl::bits::SmallSet<Element::Type> ElementTypes_t;

}

inline game::Element::Type
game::Element::begin()
{
    return Neutronium;
}

#endif
