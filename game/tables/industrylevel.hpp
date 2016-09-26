/**
  *  \file game/tables/industrylevel.hpp
  */
#ifndef C2NG_GAME_TABLES_INDUSTRYLEVEL_HPP
#define C2NG_GAME_TABLES_INDUSTRYLEVEL_HPP

#include "afl/functional/mapping.hpp"
#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"

namespace game { namespace tables {

    class IndustryLevel : public afl::functional::Mapping<int,String_t> {
     public:
        IndustryLevel(afl::string::Translator& tx);
        String_t get(int gov) const;
        virtual bool getFirstKey(int& a) const;
        virtual bool getNextKey(int& a) const;

     private:
        afl::string::Translator& m_translator;
    };

} }

#endif
