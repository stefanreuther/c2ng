/**
  *  \file u/t_game_parser_informationconsumer.cpp
  *  \brief Test for game::parser::InformationConsumer
  */

#include "game/parser/informationconsumer.hpp"

#include "t_game_parser.hpp"

/** Interface test. */
void
TestGameParserInformationConsumer::testInterface()
{
    class Tester : public game::parser::InformationConsumer {
     public:
        virtual void addMessageInformation(const game::parser::MessageInformation& /*info*/)
            { }
    };
    Tester t;
}

