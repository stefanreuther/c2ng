/**
  *  \file game/interface/propertylist.hpp
  *  \brief Structure game::interface::PropertyList
  */
#ifndef C2NG_GAME_INTERFACE_PROPERTYLIST_HPP
#define C2NG_GAME_INTERFACE_PROPERTYLIST_HPP

#include <vector>
#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"
#include "game/map/object.hpp"
#include "interpreter/world.hpp"
#include "util/skincolor.hpp"

namespace game { namespace interface {

    /** Object Property List.
        Data structure to store a list of key/value pairs, representing properties and their values. */
    struct PropertyList {
        /** A single list item. */
        struct Info {
            String_t name;                         ///< Property name.
            String_t value;                        ///< Property value.
            util::SkinColor::Color valueColor;     ///< Suggested color for value.
            Info(const String_t& name, const String_t& value, util::SkinColor::Color valueColor)
                : name(name), value(value), valueColor(valueColor)
                { }
        };

        /** List title. */
        String_t title;

        /** List content. */
        std::vector<Info> infos;
    };


    /** Build list of properties for an object.
        If the object has no properties, @c out remains empty.
        @param [out] out    Result
        @param [in]  obj    Object to build list for
        @param [in]  world  Interpreter world (for property names)
        @param [in]  tx     Translator */
    void buildPropertyList(PropertyList& out, const game::map::Object* obj, const interpreter::World& world, afl::string::Translator& tx);

} }

#endif
