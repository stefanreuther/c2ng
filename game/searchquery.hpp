/**
  *  \file game/searchquery.hpp
  *  \brief Class game::SearchQuery
  */
#ifndef C2NG_GAME_SEARCHQUERY_HPP
#define C2NG_GAME_SEARCHQUERY_HPP

#include "afl/bits/smallset.hpp"
#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"
#include "interpreter/bytecodeobject.hpp"

namespace game {

    /** Representation of a search query.

        In c2ng, a search query is executed by a script.
        A SearchQuery object represents the user's search request,
        and can be passed around as needed.

        The SearchQuery object also allows the creation of the needed bytecode.
        We generate the bytecode directly without an intermediate textual script representation. */
    class SearchQuery {
        // ex TUserSearchQuery
     public:
        /** Kind of match. */
        enum MatchType {
            MatchName,           ///< Match Name/Id/Comment.
            MatchTrue,           ///< Match if expression is true.
            MatchFalse,          ///< Match if expression is false.
            MatchLocation        ///< Match location (X,Y).
        };

        /** Objects to search. */
        enum SearchObject {
            SearchShips,         ///< Search ships.
            SearchPlanets,       ///< Search planets.
            SearchBases,         ///< Search starbases (subset of planets).
            SearchUfos,          ///< Search Ufos.
            SearchOthers         ///< Search everything else (minefields, ion storms).
        };
        typedef afl::bits::SmallSet<SearchObject> SearchObjects_t;

        /** Constructor.
            Makes a blank query. */
        SearchQuery();

        /** Construct query from parameters.
            \param matchType desired match type
            \param objs      objects to search
            \param query     query string */
        SearchQuery(MatchType matchType, SearchObjects_t objs, String_t query);

        /** Destructor. */
        ~SearchQuery();

        /** Set of all object types.
            \return set of all object types */
        static SearchObjects_t allObjects();

        /** Set match type.
            \param matchType New match type */
        void setMatchType(MatchType matchType);

        /** Get match type.
            \return match type */
        MatchType getMatchType() const;

        /** Set set of objects.
            \param objs Objects */
        void setSearchObjects(SearchObjects_t objs);

        /** Get set of objects.
            \return Objects */
        SearchObjects_t getSearchObjects() const;

        /** Set query string.
            \param query query string */
        void setQuery(String_t query);

        /** Get query string.
            \return query string */
        String_t getQuery() const;

        /** Set limitation to played objects.
            \param flag True to limit search to played objects */
        void setPlayedOnly(bool flag);

        /** Get limitation to played objects.
            \return true if search is limited to played objects (default: false) */
        bool getPlayedOnly() const;

        /** Set optimisation level.
            This is used to optimize (or not) the generated code.
            \param level Optimisation level */
        void setOptimisationLevel(int level);

        /** Get search objects as string.
            \return string */
        String_t getSearchObjectsAsString() const;

        /** Compile search expression into code.
            Produces a BytecodeObject containing a function that takes a single parameter (the object)
            and returns a boolean value if that object matches the search query.

            This function is exposed mostly for testing.

            \param world Interpreter world
            \return BCO

            \throw interpreter::Error if search query fails to parse */
        interpreter::BCORef_t compileExpression(interpreter::World& world) const;

        /** Compile search query into code.
            Produces a BytecodeObject executing the entire search query.
            To execute the search query, run this BCO in a process, and examine its result
            (which will be a ReferenceListContext).

            The resulting code will invoke the driver CCUI$Search,
            passing it the compiled expression (compileExpression()) and other parameters.

            \param world Interpreter world
            \return BCO

            \throw interpreter::Error if search query fails to parse */
        interpreter::BCORef_t compile(interpreter::World& world) const;


        /** Format a SearchObjects_t into a string.
            \param objs Value to format
            \param tx   Translator
            \return human-readable, non-empty string */
        static String_t formatSearchObjects(SearchObjects_t objs, afl::string::Translator& tx);

     private:
        MatchType m_matchType;
        SearchObjects_t m_objects;
        bool m_playedOnly;
        String_t m_query;
        int m_optimisationLevel;
    };

}

// Set of all object types.
inline game::SearchQuery::SearchObjects_t
game::SearchQuery::allObjects()
{
    return SearchObjects_t()
        + SearchShips
        + SearchPlanets
        + SearchBases
        + SearchUfos
        + SearchOthers;
}

// Set match type.
inline void
game::SearchQuery::setMatchType(MatchType matchType)
{
    m_matchType = matchType;
}

// Get match type.
inline game::SearchQuery::MatchType
game::SearchQuery::getMatchType() const
{
    return m_matchType;
}

// Set set of objects.
inline void
game::SearchQuery::setSearchObjects(SearchObjects_t objs)
{
    m_objects = objs;
}

// Get set of objects.
inline game::SearchQuery::SearchObjects_t
game::SearchQuery::getSearchObjects() const
{
    return m_objects;
}

// Get limitation to played objects.
inline void
game::SearchQuery::setPlayedOnly(bool flag)
{
    m_playedOnly = flag;
}

// Get limitation to played objects.
inline bool
game::SearchQuery::getPlayedOnly() const
{
    return m_playedOnly;
}

// Set optimisation level.
inline void
game::SearchQuery::setOptimisationLevel(int level)
{
    m_optimisationLevel = level;
}

#endif
