/**
  *  \file test/game/config/configurationoptiontest.cpp
  *  \brief Test for game::config::ConfigurationOption
  */

#include "game/config/configurationoption.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST("game.config.ConfigurationOption", a)
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
    a.checkEqual("01. getSource", t.getSource(), t.Default);
    a.check("02. wasSet", !t.wasSet());
    a.check("03. isChanged", !t.isChanged());

    // Source
    t.setSource(t.User);
    a.checkEqual("11. getSource", t.getSource(), t.User);
    a.check("12. wasSet", t.wasSet());

    // Change
    t.markChanged(false);
    a.check("21. isChanged", !t.isChanged());
    t.markChanged();
    a.check("22. isChanged", t.isChanged());
    t.markChanged(false);
    a.check("23. isChanged", !t.isChanged());
}

/** Test markUpdated. */
AFL_TEST("game.config.ConfigurationOption:markUpdated", a)
{
    class Tester : public game::config::ConfigurationOption {
     public:
        virtual void set(String_t /*value*/)
            { }
        virtual String_t toString() const
            { return String_t(); }
    };
    Tester t;

    a.checkEqual("01. getSource", t.getSource(), t.Default);
    a.check("02. wasSet", !t.wasSet());

    t.markUpdated(t.User);
    a.checkEqual("11. getSource", t.getSource(), t.User);
    a.check("12. wasSet", t.wasSet());
    a.check("13. isChanged", t.isChanged());
    t.markChanged(false);

    // Update to system is not a change
    t.markUpdated(t.System);
    a.checkEqual("21. getSource", t.getSource(), t.User);
    a.check("22. wasSet", t.wasSet());
    a.check("23. isChanged", !t.isChanged());
}
