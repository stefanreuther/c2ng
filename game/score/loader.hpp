/**
  *  \file game/score/loader.hpp
  *  \brief Class game::score::Loader
  */
#ifndef C2NG_GAME_SCORE_LOADER_HPP
#define C2NG_GAME_SCORE_LOADER_HPP

#include "afl/charset/charset.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/translator.hpp"

namespace game { namespace score {

    class TurnScoreList;

    /** Score file I/O. */
    class Loader {
     public:
        /** Constructor.
            \param tx Translator (used for error messages)
            \param cs Character set (used for loading strings from score files) */
        Loader(afl::string::Translator& tx, afl::charset::Charset& cs);

        /** Load PCC2 score file (score.cc).
            \param list [out] Result
            \param in Stream */
        void load(TurnScoreList& list, afl::io::Stream& in);

        /** Load PCC1 score file (stat.cc).
            \param list [out] Result
            \param in Stream */
        void loadOldFile(TurnScoreList& list, afl::io::Stream& in);

        /** Save PCC2 score file (score.cc).
            \param list [in] Result
            \param out Stream */
        void save(const TurnScoreList& list, afl::io::Stream& out);

     private:
        afl::string::Translator& m_translator;
        afl::charset::Charset& m_charset;
    };

} }

#endif
