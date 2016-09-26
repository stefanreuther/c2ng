/**
  *  \file game/tables/mineralmassclassname.hpp
  */
#ifndef C2NG_GAME_TABLES_MINERALMASSCLASSNAME_HPP
#define C2NG_GAME_TABLES_MINERALMASSCLASSNAME_HPP

#include "afl/functional/mapping.hpp"
#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"

namespace game { namespace tables {

    class MineralMassClassName : public afl::functional::Mapping<int32_t,String_t> {
     public:
        MineralMassClassName(afl::string::Translator& tx);
        String_t get(int32_t mass) const;
        virtual bool getFirstKey(int32_t& a) const;
        virtual bool getNextKey(int32_t& a) const;

     private:
        afl::string::Translator& m_translator;
    };

} }

#endif
