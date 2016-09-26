/**
  *  \file util/configurationfileparser.hpp
  */
#ifndef C2NG_UTIL_CONFIGURATIONFILEPARSER_HPP
#define C2NG_UTIL_CONFIGURATIONFILEPARSER_HPP

#include "util/fileparser.hpp"

namespace util {

    class ConfigurationFileParser : public FileParser {
     public:
        ConfigurationFileParser();

        void setSection(String_t sectionName, bool inSection);

        bool isInSection() const;
        const String_t& getSectionName() const;

        virtual void handleAssignment(const String_t& fileName, int lineNr, const String_t& name, const String_t& value, const String_t& line) = 0;
        virtual void handleError(const String_t& fileName, int lineNr, const String_t& message) = 0;

//     bool isInSection() const;
//     string_t getSectionName() const;

//     void process(int line_id, string_t line);

        // FileParser:
        virtual void handleLine(const String_t& fileName, int lineNr, String_t line);
        virtual void handleIgnoredLine(const String_t& fileName, int lineNr, String_t line) = 0;

     private:
        String_t m_sectionName;
        bool m_inSection;
    };

}

#endif
