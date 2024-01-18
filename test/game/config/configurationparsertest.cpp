/**
  *  \file test/game/config/configurationparsertest.cpp
  *  \brief Test for game::config::ConfigurationParser
  */

#include "game/config/configurationparser.hpp"

#include "afl/io/constmemorystream.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "game/config/configuration.hpp"
#include "game/config/integeroption.hpp"
#include "game/config/integervalueparser.hpp"

namespace {
    static const game::config::IntegerOptionDescriptor OPTION = {
        "Option",
        &game::config::IntegerValueParser::instance
    };

    struct Environment {
        afl::sys::Log log;
        afl::string::NullTranslator tx;
        game::config::Configuration config;
        game::config::ConfigurationParser parser;

        Environment()
            : log(), tx(), config(), parser(log, tx, config, game::config::ConfigurationOption::Game)
            {
                config[OPTION].set(10);
                config[OPTION].setSource(game::config::ConfigurationOption::Default);
            }
    };

    template<typename T>
    size_t count(afl::base::Enumerator<T>& e)
    {
        size_t n = 0;
        T tmp;
        while (e.getNextElement(tmp)) {
            ++n;
        }
        return n;
    }
}

/** Test normal config file parsing. */
AFL_TEST("game.config.ConfigurationParser:normal", a)
{
    const char*const FILE = {
        "option = 20\n"           // will be set as integer
        "other = x"               // will be created as string
    };
    afl::io::ConstMemoryStream ms(afl::string::toBytes(FILE));

    Environment env;
    env.parser.parseFile(ms);

    // Numeric option
    a.checkEqual("01. option value", env.config[OPTION](), 20);
    a.checkEqual("02. option source", env.config[OPTION].getSource(), game::config::ConfigurationOption::Game);

    // String option
    const game::config::ConfigurationOption* p = env.config.getOptionByName("other");
    a.checkNonNull("11. getOptionByName", p);
    a.checkEqual("12. toString", p->toString(), "x");

    // Must be two options
    a.checkEqual("21. count", count(*env.config.getOptions()), 2U);
}

/** Test config file parsing, error/null cases. */
AFL_TEST("game.config.ConfigurationParser:errors", a)
{
    // Neither of these cases should cause the configuration to be modified
    const char*const FILES[] = {
        "#comment\n",             // coverage. Why not.
        "syntax error\n",         // syntax error.
        "option = error",         // setting as integer will fail and thus be ignored
    };

    for (size_t i = 0; i < sizeof(FILES)/sizeof(FILES[0]); ++i) {
        afl::io::ConstMemoryStream ms(afl::string::toBytes(FILES[i]));

        Environment env;
        env.parser.parseFile(ms);

        // Existing option unchanged
        a.checkEqual("01. option value", env.config[OPTION](), 10);
        a.checkEqual("02. option source", env.config[OPTION].getSource(), game::config::ConfigurationOption::Default);

        // Must still be one option
        a.checkEqual("11. count", count(*env.config.getOptions()), 1U);
    }
}
