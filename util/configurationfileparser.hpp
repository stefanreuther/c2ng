/**
  *  \file util/configurationfileparser.hpp
  */
#ifndef C2NG_UTIL_CONFIGURATIONFILEPARSER_HPP
#define C2NG_UTIL_CONFIGURATIONFILEPARSER_HPP

#include "afl/string/translator.hpp"
#include "util/fileparser.hpp"

namespace util {

    /** Configuration File Parser (pconfig.src alike).

        This class encapsulates the logic for parsing PCONFIG.SRC-alike configuration files.
        In a nutshell:
        - sections separated by "% sectionname"
        - comments starting with "\#"
        - assignments "key = value" in each section

        To parse a configuration file, derive a class from this and implement the virtual methods. */
    class ConfigurationFileParser : public FileParser {
     public:
        ConfigurationFileParser(afl::string::Translator& tx);

        void setSection(String_t sectionName, bool inSection);

        bool isInSection() const;
        const String_t& getSectionName() const;

        afl::string::Translator& translator() const;

        virtual void handleAssignment(const String_t& fileName, int lineNr, const String_t& name, const String_t& value, const String_t& line) = 0;
        virtual void handleError(const String_t& fileName, int lineNr, const String_t& message) = 0;

        // FileParser:
        virtual void handleLine(const String_t& fileName, int lineNr, String_t line);
        virtual void handleIgnoredLine(const String_t& fileName, int lineNr, String_t line) = 0;

     private:
        afl::string::Translator& m_translator;
        String_t m_sectionName;
        bool m_inSection;
    };

}


inline afl::string::Translator&
util::ConfigurationFileParser::translator() const
{
    return m_translator;
}


#endif
