/**
  *  \file game/tables/headingname.hpp
  */
#ifndef C2NG_GAME_TABLES_HEADINGNAME_HPP
#define C2NG_GAME_TABLES_HEADINGNAME_HPP

#include "afl/functional/mapping.hpp"
#include "afl/string/string.hpp"

namespace game { namespace tables {

    class HeadingName : public afl::functional::Mapping<int,String_t> {
     public:
        HeadingName();
        String_t get(int heading) const;
        virtual bool getFirstKey(int& a) const;
        virtual bool getNextKey(int& a) const;
    };

} }

#endif
