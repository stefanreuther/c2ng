/**
  *  \file game/reference.hpp
  *  \brief Class game::Reference
  */
#ifndef C2NG_GAME_REFERENCE_HPP
#define C2NG_GAME_REFERENCE_HPP

#include "afl/string/translator.hpp"
#include "game/map/point.hpp"
#include "game/types.hpp"

namespace game {

    /** Symbolic reference to an object.
        These symbolic references can be passed between game and UI domain, between turns, or even between games.

        This class' vocabulary has been chosen to minimize the number of abstractions required;
        it contains distinctions required by the PCC2 GUI level, but not by the game layer,
        and doesn't completely block all misuse. */
    class Reference {
     public:
        /** Type of the reference. */
        enum Type {
            /** Null reference. Does not point anywhere. */
            Null,

            /** Special reference. Not used by Reference itself; can be used by users to refer to special things
                (e.g. special menu items), distinguished by the Id. */
            Special,

            /** Player. Id is player number. */
            Player,

            /** Map location. This one is special in that it is constructed from a game::map::Point, not an Id. */
            MapLocation,

            /** Ship. Id is player number. */
            Ship,

            /** Planet. Id is planet number. */
            Planet,

            /** Starbase. Id is planet number. */
            Starbase,

            /** Ion storm. Id is storm number. */
            Storm,

            /** Minefield. Id is minefield number. */
            Minefield,

            /** Ufo.
                FIXME: clarify the meaning of type Ufo. Right now, we pass an Id, which can be ambiguous with Hans' ufos.
                PCC 1.x passed an index. */
            Ufo,

            /** Hull. Id is hull number. */
            Hull,

            /** Engine. Id is engine number. */
            Engine,

            /** Beam. Id is beam number. */
            Beam,

            /** Torpedo. Id is torpedo number. */
            Torpedo
        };


        /** Default constructor.
            Make empty reference.
            \post !isSet() */
        Reference();

        /** Construct from type and Id.
            \param type Type
            \param id Id
            \post getId() == id, getType() == type */
        Reference(Type type, Id_t id);

        /** Construct from position.
            \param pt Position
            \post getPos(...) == true */
        Reference(game::map::Point pt);

        /** Check validity.
            \return true if this has any type other than Null */
        bool isSet() const;

        /** Get type.
            \return type */
        Type getType() const;

        /** Get Id.
            \return Id. Unspecified value for getType() == MapLocation */
        int getId() const;

        /** Get position.
            \param [out] pt Position
            \retval true This reference was of type MapLocation, position returned
            \retval false This reference was not a location. */
        bool getPos(game::map::Point& pt) const;

        /** Format to string.
            This is just a simple formatter with no relation to a game.
            Prefer
            - game::Session::getReferenceName()
            - game::map::Universe::getObject(...)->getName(...)
            - game::spec::ShipList::getComponent(...)->getName(...)

            \param tx Translator
            \return name */
        String_t toString(afl::string::Translator& tx) const;

        /** Compare equality.
            \param other Other reference
            \return true if both references are identical */
        bool operator==(const Reference& other) const;

        /** Compare inequality.
            \param other Other reference
            \return true if both references are different */
        bool operator!=(const Reference& other) const;

     private:
        Type m_type;
        int m_x;
        int m_y;
    };

}

#endif
