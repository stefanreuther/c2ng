/**
  *  \file test/util/configurationfileparsertest.cpp
  *  \brief Test for util::ConfigurationFileParser
  */

#include "util/configurationfileparser.hpp"

#include "afl/io/constmemorystream.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST("util.ConfigurationFileParser", a)
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
    a.checkEqual("01. isInSection", t.isInSection(), true);
    a.checkEqual("02. getSectionName", t.getSectionName(), "");
    a.checkEqual("03. get", t.get(), "");

    // Configure
    t.setSection("PCONFIG", true);
    a.checkEqual("11. isInSection", t.isInSection(), true);
    a.checkEqual("12. getSectionName", t.getSectionName(), "PCONFIG");

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
    a.checkEqual("21. get", t.get(), "handleAssignment(gamename,test)handleIgnoredLine()handleError()handleError()handleIgnoredLine()handleIgnoredLine()");
    a.checkEqual("22. isInSection", t.isInSection(), false);
}
