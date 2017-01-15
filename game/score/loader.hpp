/**
  *  \file game/score/loader.hpp
  */
#ifndef C2NG_GAME_SCORE_LOADER_HPP
#define C2NG_GAME_SCORE_LOADER_HPP

#include "afl/io/stream.hpp"
#include "afl/string/translator.hpp"
#include "afl/charset/charset.hpp"

namespace game { namespace score {

    class TurnScoreList;

    class Loader {
     public:
        Loader(afl::string::Translator& tx, afl::charset::Charset& cs);

        void load(TurnScoreList& list, afl::io::Stream& in);

     private:
        afl::string::Translator& m_translator;
        afl::charset::Charset& m_charset;
    };

} }

#endif
