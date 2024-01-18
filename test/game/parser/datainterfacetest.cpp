/**
  *  \file test/game/parser/datainterfacetest.cpp
  *  \brief Test for game::parser::DataInterface
  */

#include "game/parser/datainterface.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("game.parser.DataInterface")
{
    class Tester : public game::parser::DataInterface {
     public:
        virtual int getPlayerNumber() const
            { return 0; }
        virtual int parseName(Name /*which*/, const String_t& /*name*/) const
            { return 0; }
        virtual String_t expandRaceNames(String_t /*name*/) const
            { return String_t(); }
    };
    Tester t;
}
