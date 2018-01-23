/**
  *  \file game/v3/outboxreader.hpp
  *  \brief Class game::v3::OutboxReader
  */
#ifndef C2NG_GAME_V3_OUTBOXREADER_HPP
#define C2NG_GAME_V3_OUTBOXREADER_HPP

#include "afl/base/deletable.hpp"
#include "afl/charset/charset.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"
#include "game/playerset.hpp"

namespace game { namespace v3 {

    /** Reading an outbox file.
        Derive from this class and implement the callback.
        Call loadOutbox() or loadOutbox35() to load an outbox file. */
    class OutboxReader : public afl::base::Deletable {
     public:
        /** Add message read from file.
            \param text Message text, decoded
            \param receivers Receiver set. The receiver set is produced in the same form as used in the file.
            That is, for 3.0. it will be single receivers; for 3.5, it can be multiple receivers.
            Consumer must implement deduplication if desired. */
        virtual void addMessage(String_t text, PlayerSet_t receivers) = 0;

        /** Load version 3.0 outbox (MESSx.DAT).
            \param s Stream to read from
            \param cs Character set
            \param tx Translator (used for error message texts)
            \throw afl::except::FileFormatException on file format errors */
        void loadOutbox(afl::io::Stream& s, afl::charset::Charset& cs, afl::string::Translator& tx);

        /** Load version 3.5 outbox (MESS35x.DAT).
            \param s Stream to read from
            \param cs Character set
            \param tx Translator (used for error message texts)
            \throw afl::except::FileFormatException on file format errors  */
        void loadOutbox35(afl::io::Stream& s, afl::charset::Charset& cs, afl::string::Translator& tx);
    };

} }

#endif
