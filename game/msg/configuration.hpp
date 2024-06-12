/**
  *  \file game/msg/configuration.hpp
  *  \brief Class game::msg::Configuration
  */
#ifndef C2NG_GAME_MSG_CONFIGURATION_HPP
#define C2NG_GAME_MSG_CONFIGURATION_HPP

#include <set>
#include "afl/charset/charset.hpp"
#include "afl/io/directory.hpp"
#include "afl/string/string.hpp"

namespace game { namespace msg {

    /** Message configuration.
        In-memory representation of the msgX.ini file.

        As of 20210327, represents the message filter. */
    class Configuration {
     public:
        /** Make empty configuration. */
        Configuration();

        /** Destructor. */
        ~Configuration();

        /** Check whether a message heading is filtered.
            \param heading Subject line to search for
            \return true iff filtered */
        bool isHeadingFiltered(const String_t& heading) const;

        /** Toggle whether a message heading is filtered.
            Adds line to filter if it is not yet there, removes it otherwise.
            \param heading Subject line to search for */
        void toggleHeadingFiltered(const String_t& heading);

        /** Set whether a message heading is filtered.
            \param heading Subject line to search for
            \param flag true to filter, false to remove from filter */
        void setHeadingFiltered(const String_t& heading, bool flag);

        /** Clear message configuration.
            Unfilters all messages. */
        void clear();

        /** Load configuration.
            \param dir      Directory to load from
            \param playerNr Player number
            \param charset  Character set */
        void load(afl::io::Directory& dir, int playerNr, afl::charset::Charset& charset);

        /** Save configuration.
            \param dir      Directory to save to
            \param playerNr Player number
            \param charset  Character set */
        void save(afl::io::Directory& dir, int playerNr, afl::charset::Charset& charset) const;

     private:
        // @change: we use a set<>, not a list<>
        typedef std::set<String_t> Filter_t;
        Filter_t m_filteredHeadings;
    };

} }

#endif
