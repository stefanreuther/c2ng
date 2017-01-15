/**
  *  \file u/t_util_fileparser.cpp
  *  \brief Test for util::FileParser
  */

#include "util/fileparser.hpp"

#include "t_util.hpp"

/** Interface test. */
void
TestUtilFileParser::testInterface()
{
    class Tester : public util::FileParser {
     public:
        Tester()
            : FileParser(";")
            { }
        virtual void handleLine(const String_t& /*fileName*/, int /*lineNr*/, String_t /*line*/)
            { }
        virtual void handleIgnoredLine(const String_t& /*fileName*/, int /*lineNr*/, String_t /*line*/)
            { }
    };
    Tester t;
}

