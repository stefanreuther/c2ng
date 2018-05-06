/**
  *  \file game/msg/configuration.hpp
  */
#ifndef C2NG_GAME_MSG_CONFIGURATION_HPP
#define C2NG_GAME_MSG_CONFIGURATION_HPP

#include <set>
#include "afl/string/string.hpp"
#include "afl/io/directory.hpp"

namespace game { namespace msg {

    class Configuration {
     public:
        Configuration();

        ~Configuration();

        bool isHeadingFiltered(const String_t& heading) const;

        void toggleHeadingFiltered(const String_t& heading);

        void setHeadingFiltered(const String_t& heading, bool flag);

        void clear();

        // FIXME: PCC2 functions
        // bool empty() const;
        // iterator begin() const;
        // iterator end() const;

        void load(afl::io::Directory& dir, int playerNr);

        void save(afl::io::Directory& dir, int playerNr);

     private:
        // @change: we use a set<>, not a list<>
        typedef std::set<String_t> Filter_t;
        Filter_t m_filteredHeadings;
    };

} }

#endif
