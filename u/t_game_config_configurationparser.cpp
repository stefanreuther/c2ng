/**
  *  \file u/t_game_config_configurationparser.cpp
  *  \brief Test for game::config::ConfigurationParser
  */

#include "game/config/configurationparser.hpp"

#include "t_game_config.hpp"
#include "afl/sys/log.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/config/integeroption.hpp"
#include "game/config/integervalueparser.hpp"
#include "afl/io/constmemorystream.hpp"
#include "game/config/configuration.hpp"

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
void
TestGameConfigConfigurationParser::testNormal()
{
    const char*const FILE = {
        "option = 20\n"           // will be set as integer
        "other = x"               // will be created as string
    };
    afl::io::ConstMemoryStream ms(afl::string::toBytes(FILE));

    Environment env;
    env.parser.parseFile(ms);

    // Numeric option
    TS_ASSERT_EQUALS(env.config[OPTION](), 20);
    TS_ASSERT_EQUALS(env.config[OPTION].getSource(), game::config::ConfigurationOption::Game);

    // String option
    const game::config::ConfigurationOption* p = env.config.getOptionByName("other");
    TS_ASSERT(p != 0);
    TS_ASSERT_EQUALS(p->toString(), "x");

    // Must be two options
    TS_ASSERT_EQUALS(count(*env.config.getOptions()), 2U);
}

/** Test config file parsing, error/null cases. */
void
TestGameConfigConfigurationParser::testFail()
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
        TS_ASSERT_EQUALS(env.config[OPTION](), 10);
        TS_ASSERT_EQUALS(env.config[OPTION].getSource(), game::config::ConfigurationOption::Default);

        // Must still be one option
        TS_ASSERT_EQUALS(count(*env.config.getOptions()), 1U);
    }
}

