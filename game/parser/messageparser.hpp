/**
  *  \file game/parser/messageparser.hpp
  */
#ifndef C2NG_GAME_PARSER_MESSAGEPARSER_HPP
#define C2NG_GAME_PARSER_MESSAGEPARSER_HPP

#include "afl/io/stream.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"

namespace game { namespace parser {

    class MessageInformation;
    class MessageTemplate;
    class DataInterface;

    class MessageParser {
     public:
        MessageParser();
        ~MessageParser();

        void load(afl::io::Stream& file, afl::string::Translator& tx, afl::sys::LogListener& log);

        void parseMessage(String_t theMessage, const DataInterface& iface, int turnNr, afl::container::PtrVector<MessageInformation>& info,
                          afl::string::Translator& tx, afl::sys::LogListener& log);

        size_t getNumTemplates() const;

     private:
        afl::container::PtrVector<MessageTemplate> m_templates;
    };

} }

inline size_t
game::parser::MessageParser::getNumTemplates() const
{
    // ex GMessageParser::getNumTemplates
    return m_templates.size();
}

// FIXME: remove (only needed for PCC2 test app)
// inline const GMessageTemplate&
// GMessageParser::getTemplate(uint32_t i) const
// {
//     return *message_templates[i];
// }

#endif
