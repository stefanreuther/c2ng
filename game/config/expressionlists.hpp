/**
  *  \file game/config/expressionlists.hpp
  *  \brief Class game::config::ExpressionLists
  */
#ifndef C2NG_GAME_CONFIG_EXPRESSIONLISTS_HPP
#define C2NG_GAME_CONFIG_EXPRESSIONLISTS_HPP

#include <vector>
#include "afl/io/directory.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"
#include "util/expressionlist.hpp"
#include "util/profiledirectory.hpp"

namespace game { namespace config {

    /** Expression lists.
        Stores the game-related expression lists:
        - least-recently-used expressions
        - predefined expressions */
    class ExpressionLists {
     public:
        /** Area (type of expression). */
        enum Area {
            ShipLabels,             ///< Ship labels. Flags are not used.
            PlanetLabels,           ///< Planet labels. Flags are not used.
            Search                  ///< Search expressions. Flags contain object and/or query type.
        };
        static const size_t NUM_AREAS = static_cast<size_t>(Search) + 1;

        /** Kinds of list.
            Note that this is the order in which lists appear in pack(). */
        enum Kind {
            Recent,                 ///< Least-recently-used list.
            Predefined              ///< Predefined list.
        };
        static const size_t NUM_KINDS = static_cast<size_t>(Predefined) + 1;

        /** Formatted item. */
        struct Item {
            String_t name;          ///< Name. Same as util::ExpressionList::Item::name, or heading.
            String_t flags;         ///< Flags. Same as util::ExpressionList::Item::flags.
            String_t value;         ///< Value. Same as util::ExpressionList::Item::value.
            bool isHeading;         ///< true if this is a heading.

            Item(const util::ExpressionList::Item& it)
                : name(it.name), flags(it.flags), value(it.value), isHeading(false)
                { }
            explicit Item(String_t heading)
                : name(heading), flags(), value(), isHeading(true)
                { }
        };
        typedef std::vector<Item> Items_t;

        /** Constructor. */
        ExpressionLists();

        /** Destructor. */
        ~ExpressionLists();

        /** Get list by parameters.
            \param a Area
            \param k Kind
            \return List */
        util::ExpressionList* get(Area a, Kind k);
        const util::ExpressionList* get(Area a, Kind k) const;

        /** Load least-recently-used file (lru.ini).
            This file is stored in the user profile.
            \param profile   User profile
            \param log       Logger
            \param tx        Translator */
        void loadRecentFiles(util::ProfileDirectory& profile, afl::sys::LogListener& log, afl::string::Translator& tx);

        /** Save least-recently-used file (lru.ini).
            This file is stored in the user profile.
            \param profile   User profile
            \param log       Logger
            \param tx        Translator */
        void saveRecentFiles(util::ProfileDirectory& profile, afl::sys::LogListener& log, afl::string::Translator& tx) const;

        /** Load predefined expression files.
            expr.ini is stored in the user profile, expr.cc/expr.usr in the game directory.
            \param profile   User profile
            \param dir       Game directory
            \param log       Logger
            \param tx        Translator */
        void loadPredefinedFiles(util::ProfileDirectory& profile, afl::io::Directory& dir, afl::sys::LogListener& log, afl::string::Translator& tx);

        /** Clear.
            Discards all content. */
        void clear();

        /** Pack information for one area.
            Builds the selection list for the user to choose from.
            The list will contain least-recently-used and predefined expressions,
            with divider headings if desired.
            \param [out] out  Result
            \param [in]  a    Area
            \param [in]  tx   Translator */
        void pack(Items_t& out, Area a, afl::string::Translator& tx) const;

        /** Add recent expression.
            \param a     Area
            \param flags Flags (including brackets)
            \param expr  Expression (used as name and value) */
        void pushRecent(Area a, String_t flags, String_t expr);

        /** Parse area name.
            \param [in]  area   Name (in upper case)
            \param [out] result Parsed Area value
            \return true on success */
        static bool parseArea(const String_t& area, Area& result);

     private:
        util::ExpressionList m_data[NUM_KINDS][NUM_AREAS];

        void clearAll(Kind k);
        static String_t getHeading(Area a, Kind k, afl::string::Translator& tx);
    };

} }

#endif
