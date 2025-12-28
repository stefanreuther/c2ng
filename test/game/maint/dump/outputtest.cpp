/**
  *  \file test/game/maint/dump/outputtest.cpp
  *  \brief Test for game::maint::dump::Output
  */

#include "game/maint/dump/output.hpp"

#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("game.maint.dump.Output")
{
    class Tester : public game::maint::dump::Output {
     public:
        virtual void startRecord(String_t /*header*/)
            { }
        virtual void addField(String_t /*name*/, String_t /*formattedValue*/)
            { }
        virtual void addUnparsedData(String_t /*formattedValue*/)
            { }
        virtual void endRecord()
            { }
    };
    Tester t;
}

