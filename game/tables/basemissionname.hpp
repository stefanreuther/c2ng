/**
  *  \file game/tables/basemissionname.hpp
  */
#ifndef C2NG_GAME_TABLES_BASEMISSIONNAME_HPP
#define C2NG_GAME_TABLES_BASEMISSIONNAME_HPP

#include "afl/functional/mapping.hpp"
#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"

namespace game { namespace tables {

    class BaseMissionName : public afl::functional::Mapping<int,String_t> {
     public:
        BaseMissionName(afl::string::Translator& tx);
        String_t get(int mission) const;
        virtual bool getFirstKey(int& a) const;
        virtual bool getNextKey(int& a) const;

     private:
        afl::string::Translator& m_translator;
    };

} }

#endif
