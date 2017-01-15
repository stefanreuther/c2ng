/**
  *  \file u/t_util_configurationfileparser.cpp
  *  \brief Test for util::ConfigurationFileParser
  */

#include "util/configurationfileparser.hpp"

#include "t_util.hpp"

/** Interface test. */
void
TestUtilConfigurationFileParser::testInterface()
{
    class Tester : public util::ConfigurationFileParser {
     public:
        virtual void handleAssignment(const String_t& /*fileName*/, int /*lineNr*/, const String_t& /*name*/, const String_t& /*value*/, const String_t& /*line*/)
            { }
        virtual void handleError(const String_t& /*fileName*/, int /*lineNr*/, const String_t& /*message*/)
            { }
        virtual void handleIgnoredLine(const String_t& /*fileName*/, int /*lineNr*/, String_t /*line*/)
            { }
    };
    Tester t;
}

