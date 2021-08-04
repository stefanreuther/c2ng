/**
  *  \file game/spec/info/utils.hpp
  *  \brief Utilities for common types in game::spec::info
  */
#ifndef C2NG_GAME_SPEC_INFO_UTILS_HPP
#define C2NG_GAME_SPEC_INFO_UTILS_HPP

#include "afl/string/translator.hpp"
#include "game/root.hpp"
#include "game/spec/info/types.hpp"
#include "game/spec/shiplist.hpp"

namespace game { namespace spec { namespace info {

    /** Get name of FilterAttribute.
        \param att Attribute
        \param tx Translator
        \return name */
    String_t toString(FilterAttribute att, afl::string::Translator& tx);

    /** Convert integer range to ExperienceLevelSet_t.
        \param r Range
        \return set */
    ExperienceLevelSet_t convertRangeToSet(IntRange_t r);

    /** Get available experience level range.
        Used as range for EditRangeLevel.
        \param root Root
        \return Level range */
    IntRange_t getLevelRange(const Root& root);

    /** Get available hull Id range.
        Used as range for EditValueHull.
        \param shipList Ship list
        \return Level range */
    IntRange_t getHullRange(const ShipList& shipList);

    /** Get available player Id range.
        Used as range for EditValuePlayer.
        \param root Root
        \return Level range */
    IntRange_t getPlayerRange(const Root& root);

    /** Get default range for a filter attribute.
        This is for use in attribute queries (EditRange) and does NOT consider configuration.
        \param att Attribute
        \return Range */
    IntRange_t getAttributeRange(FilterAttribute att);

} } }

#endif
