/**
  *  \file game/tables/mineraldensityclassname.hpp
  */
#ifndef C2NG_GAME_TABLES_MINERALDENSITYCLASSNAME_HPP
#define C2NG_GAME_TABLES_MINERALDENSITYCLASSNAME_HPP

#include "afl/functional/mapping.hpp"
#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"

namespace game { namespace tables {

    class MineralDensityClassName : public afl::functional::Mapping<int32_t,String_t> {
     public:
        MineralDensityClassName(afl::string::Translator& tx);
        String_t get(int32_t density) const;
        virtual bool getFirstKey(int32_t& a) const;
        virtual bool getNextKey(int32_t& a) const;

     private:
        afl::string::Translator& m_translator;
    };

} }

#endif
