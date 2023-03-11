/**
  *  \file util/configurationfileparser.hpp
  *  \brief Class util::ConfigurationFileParser
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
        /** Constructor.
            @param tx Translator (for error messages) */
        explicit ConfigurationFileParser(afl::string::Translator& tx);

        /** Set section to parse.
            @param sectionName   Section name (case-insensitive)
            @param inSection     If true, assume this section is active even without delimiter */
        void setSection(String_t sectionName, bool inSection);

        /** Check whether target section is currently active.
            @return true if section is active */
        bool isInSection() const;

        /** Get target section name.
            @return name */
        const String_t& getSectionName() const;

        /** Access Translator.
            @return translator */
        afl::string::Translator& translator() const;

        /** Handle an assignment to a value in the target section.
            @param fileName  File name
            @param lineNr    Line number
            @param name      Key
            @param value     Value
            @param line      Entire line */
        virtual void handleAssignment(const String_t& fileName, int lineNr, const String_t& name, const String_t& value, const String_t& line) = 0;

        /** Handle a syntax error (no "=", missing key).
            @param fileName  File name
            @param lineNr    Line number
            @param message   Error message (translated) */
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
