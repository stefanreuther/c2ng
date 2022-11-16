/**
  *  \file game/v3/udata/messagebuilder.hpp
  *  \brief Class game::v3::udata::MessageBuilder
  */
#ifndef C2NG_GAME_V3_UDATA_MESSAGEBUILDER_HPP
#define C2NG_GAME_V3_UDATA_MESSAGEBUILDER_HPP

#include "afl/base/memory.hpp"
#include "afl/charset/charset.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/io/stream.hpp"
#include "afl/sys/loglistener.hpp"
#include "game/msg/inbox.hpp"

namespace game { namespace v3 { namespace udata {

    class NameProvider;

    /** Util.dat message builder.
        Converts util.dat into human-readable messages using configurable conversion templates.
        To use,
        - create a MessageBuilder
        - load conversion templates using loadDefinition()
        - convert util.dat into messages using loadFile(); repeatedly if required */
    class MessageBuilder {
     public:
        /** Constructor.
            @param provider   Name provider. Must live as long as the MessageBuilder.
            @param cs         Game character set.
            @param tx         Translator */
        MessageBuilder(NameProvider& provider, afl::charset::Charset& cs, afl::string::Translator& tx);

        /** Destructor. */
        ~MessageBuilder();

        /** Load definition file.
            @param in   Stream
            @param log  Logger (for error messages) */
        void loadDefinition(afl::io::Stream& in, afl::sys::LogListener& log);

        /** Load util.dat file.
            Each call converts a single file, no information is carried from one call to the next.

            @param [in]  in   Stream
            @param [out] out  Mailbox; messages are appended */
        void loadFile(afl::io::Stream& in, game::msg::Inbox& out) const;

     private:
        NameProvider& m_provider;
        afl::charset::Charset& m_charset;
        afl::string::Translator& m_translator;

        struct Description;
        afl::container::PtrVector<Description> m_descriptions;

        const Description* getDescriptionByType(uint16_t recordType) const;
        void addRecord(game::msg::Inbox& out, int turnNumber, uint16_t recordType, afl::base::ConstBytes_t data) const;
        String_t decodeRecord(afl::base::ConstBytes_t data, const String_t& tpl) const;
    };

} } }

#endif
