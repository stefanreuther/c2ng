/**
  *  \file u/t_util_configurationfileparser.cpp
  *  \brief Test for util::ConfigurationFileParser
  */

#include "util/configurationfileparser.hpp"

#include "t_util.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/string/nulltranslator.hpp"

/** Interface test. */
void
TestUtilConfigurationFileParser::testInterface()
{
    class Tester : public util::ConfigurationFileParser {
     public:
        Tester(afl::string::Translator& tx)
            : ConfigurationFileParser(tx), m_accum()
            { }
        virtual void handleAssignment(const String_t& /*fileName*/, int /*lineNr*/, const String_t& name, const String_t& value, const String_t& /*line*/)
            {
                m_accum += "handleAssignment(";
                m_accum += name;
                m_accum += ",";
                m_accum += value;
                m_accum += ")";
            }
        virtual void handleError(const String_t& /*fileName*/, int /*lineNr*/, const String_t& /*message*/)
            {
                m_accum += "handleError()";
            }
        virtual void handleIgnoredLine(const String_t& /*fileName*/, int /*lineNr*/, String_t /*line*/)
            {
                m_accum += "handleIgnoredLine()";
            }
        const String_t& get() const
            { return m_accum; }
     private:
        String_t m_accum;
    };
    afl::string::NullTranslator tx;
    Tester t(tx);

    // Verify defaults
    TS_ASSERT_EQUALS(t.isInSection(), true);
    TS_ASSERT_EQUALS(t.getSectionName(), "");
    TS_ASSERT_EQUALS(t.get(), "");

    // Configure
    t.setSection("PCONFIG", true);
    TS_ASSERT_EQUALS(t.isInSection(), true);
    TS_ASSERT_EQUALS(t.getSectionName(), "PCONFIG");

    // Parse a file
    afl::io::ConstMemoryStream ms(afl::string::toBytes("gamename = test\n"
                                                       "\n"
                                                       "bad\n"
                                                       "=bad2\n"
                                                       "%pcontrol\n"
                                                       "combat = skip\n"
                                                       ""));
    t.parseFile(ms);

    // Verify calls
    TS_ASSERT_EQUALS(t.get(), "handleAssignment(gamename,test)handleIgnoredLine()handleError()handleError()handleIgnoredLine()handleIgnoredLine()");
    TS_ASSERT_EQUALS(t.isInSection(), false);
}

