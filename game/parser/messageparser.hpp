/**
  *  \file game/parser/messageparser.hpp
  *  \brief Class game::parser::MessageParser
  */
#ifndef C2NG_GAME_PARSER_MESSAGEPARSER_HPP
#define C2NG_GAME_PARSER_MESSAGEPARSER_HPP

#include "afl/container/ptrvector.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"

namespace game { namespace parser {

    class MessageInformation;
    class MessageTemplate;
    class DataInterface;

    /** Message parser.
        Used for extracting data from in-game messages.
        A MessageParser instance stores a set of templates that it applies to each messages given to it.
        The templates are loaded from a file (msgparse.ini). */
    class MessageParser {
     public:
        /** Default constructor.
            Makes an empty object that cannot parse anything. */
        MessageParser();

        /** Destructor. */
        ~MessageParser();

        /** Load definitions from file.
            New definitions will be appended to this MessageParser.
            \param [in]  file  File open for reading
            \param [in]  tx    Translator
            \param [in]  log   Logger */
        void load(afl::io::Stream& file, afl::string::Translator& tx, afl::sys::LogListener& log);

        /** Parse a message, main entry point.
            \param [in]  theMessage  Message text
            \param [in]  iface       Data interface (for names)
            \param [in]  turnNr      Turn number
            \param [out] info        Information will be appended here
            \param [in]  tx          Translator
            \param [in]  log         Logger */
        void parseMessage(String_t theMessage, const DataInterface& iface, int turnNr, afl::container::PtrVector<MessageInformation>& info, afl::string::Translator& tx, afl::sys::LogListener& log);

        /** Get number of templates.
            Mainly for testing purposes.
            \return number of templates */
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

#endif
