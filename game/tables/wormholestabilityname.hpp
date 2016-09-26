/**
  *  \file game/tables/wormholestabilityname.hpp
  */
#ifndef C2NG_GAME_TABLES_WORMHOLESTABILITYNAME_HPP
#define C2NG_GAME_TABLES_WORMHOLESTABILITYNAME_HPP

#include "afl/functional/mapping.hpp"
#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"

namespace game { namespace tables {

    class WormholeStabilityName : public afl::functional::Mapping<int32_t,String_t> {
     public:
        WormholeStabilityName(afl::string::Translator& tx);
        String_t get(int32_t stab) const;
        virtual bool getFirstKey(int32_t& a) const;
        virtual bool getNextKey(int32_t& a) const;

     private:
        afl::string::Translator& m_translator;
    };

} }

#endif
