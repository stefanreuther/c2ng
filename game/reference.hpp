/**
  *  \file game/reference.hpp
  */
#ifndef C2NG_GAME_REFERENCE_HPP
#define C2NG_GAME_REFERENCE_HPP

#include "game/types.hpp"
#include "game/map/point.hpp"
#include "afl/string/translator.hpp"

namespace game {

    class Reference {
     public:
        enum Type {
            // Misc
            Null,
            Special,
            Player,
            MapLocation,

            // Map objects
            Ship,
            Planet,
            Starbase,
            Storm,
            Minefield,
            Ufo,

            // Specification objects
            Hull,
            Engine,
            Beam,
            Torpedo
        };

        Reference();
        Reference(Type type, Id_t id);
        Reference(game::map::Point pt);

        bool isSet() const;
        Type getType() const;
        int getId() const;
        bool getPos(game::map::Point& pt) const;

        String_t toString(afl::string::Translator& tx) const;

        bool operator==(const Reference& other) const;
        bool operator!=(const Reference& other) const;

     private:
        Type m_type;
        int m_x;
        int m_y;
    };

}

#endif
