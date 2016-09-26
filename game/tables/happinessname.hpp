/**
  *  \file game/tables/happinessname.hpp
  */
#ifndef C2NG_GAME_TABLES_HAPPINESSNAME_HPP
#define C2NG_GAME_TABLES_HAPPINESSNAME_HPP

#include "afl/functional/mapping.hpp"
#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"

namespace game { namespace tables {

    class HappinessName : public afl::functional::Mapping<int,String_t> {
     public:
        HappinessName(afl::string::Translator& tx);
        String_t get(int happy) const;
        virtual bool getFirstKey(int& a) const;
        virtual bool getNextKey(int& a) const;

     private:
        afl::string::Translator& m_translator;
    };

} }

#endif
