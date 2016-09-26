/**
  *  \file game/tables/nativeracename.hpp
  */
#ifndef C2NG_GAME_TABLES_NATIVERACENAME_HPP
#define C2NG_GAME_TABLES_NATIVERACENAME_HPP

#include "afl/functional/mapping.hpp"
#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"

namespace game { namespace tables {

    class NativeRaceName : public afl::functional::Mapping<int,String_t> {
     public:
        NativeRaceName(afl::string::Translator& tx);
        String_t get(int race) const;
        virtual bool getFirstKey(int& a) const;
        virtual bool getNextKey(int& a) const;

     private:
        afl::string::Translator& m_translator;
    };

} }

#endif
