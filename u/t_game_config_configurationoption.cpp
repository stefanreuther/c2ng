/**
  *  \file u/t_game_config_configurationoption.cpp
  *  \brief Test for game::config::ConfigurationOption
  */

#include "game/config/configurationoption.hpp"

#include "t_game_config.hpp"

/** Interface test. */
void
TestGameConfigConfigurationOption::testIt()
{
    class Tester : public game::config::ConfigurationOption {
     public:
        virtual void set(String_t /*value*/)
            { }
        virtual String_t toString() const
            { return String_t(); }
    };
    Tester t;

    // Initial state
    TS_ASSERT_EQUALS(t.getSource(), t.Default);
    TS_ASSERT(!t.wasSet());
    TS_ASSERT(!t.isChanged());

    // Source
    t.setSource(t.User);
    TS_ASSERT_EQUALS(t.getSource(), t.User);
    TS_ASSERT(t.wasSet());

    // Change
    t.markChanged(false);
    TS_ASSERT(!t.isChanged());
    t.markChanged();
    TS_ASSERT(t.isChanged());
    t.markChanged(false);
    TS_ASSERT(!t.isChanged());
}

/** Test markUpdated. */
void
TestGameConfigConfigurationOption::testUpdate()
{
    class Tester : public game::config::ConfigurationOption {
     public:
        virtual void set(String_t /*value*/)
            { }
        virtual String_t toString() const
            { return String_t(); }
    };
    Tester t;

    TS_ASSERT_EQUALS(t.getSource(), t.Default);
    TS_ASSERT(!t.wasSet());

    t.markUpdated(t.User);
    TS_ASSERT_EQUALS(t.getSource(), t.User);
    TS_ASSERT(t.wasSet());
    TS_ASSERT(t.isChanged());
    t.markChanged(false);

    // Update to system is not a change
    t.markUpdated(t.System);
    TS_ASSERT_EQUALS(t.getSource(), t.User);
    TS_ASSERT(t.wasSet());
    TS_ASSERT(!t.isChanged());
}
