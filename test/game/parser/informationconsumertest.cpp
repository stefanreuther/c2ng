/**
  *  \file test/game/parser/informationconsumertest.cpp
  *  \brief Test for game::parser::InformationConsumer
  */

#include "game/parser/informationconsumer.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("game.parser.InformationConsumer")
{
    class Tester : public game::parser::InformationConsumer {
     public:
        virtual void addMessageInformation(const game::parser::MessageInformation& /*info*/)
            { }
    };
    Tester t;
}
