/**
  *  \file u/t_game_parser_datainterface.cpp
  *  \brief Test for game::parser::DataInterface
  */

#include "game/parser/datainterface.hpp"

#include "t_game_parser.hpp"

/** Interface test. */
void
TestGameParserDataInterface::testIt()
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

