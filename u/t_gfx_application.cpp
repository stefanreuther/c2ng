/**
  *  \file u/t_gfx_application.cpp
  *  \brief Test for gfx::Application
  */

#include "gfx/application.hpp"

#include "t_gfx.hpp"
#include "afl/string/nulltranslator.hpp"

/** Interface test. */
void
TestGfxApplication::testInterface()
{
    /* Interface instance */
    class Tester : public gfx::Application {
     public:
        Tester(afl::sys::Dialog& dialog, afl::string::Translator& tx, const String_t& title)
            : Application(dialog, tx, title)
            { }
        virtual void appMain(gfx::Engine& /*engine*/)
            { }
    };

    /* Environment mock */
    class NullDialog : public afl::sys::Dialog {
     public:
        virtual void showInfo(String_t /*info*/, String_t /*title*/)
            { }
        virtual void showError(String_t /*info*/, String_t /*title*/)
            { }
        virtual bool askYesNo(String_t /*info*/, String_t /*title*/)
            { return false; }
    };
    NullDialog d;
    afl::string::NullTranslator tx;

    /* Test it */
    Tester t(d, tx, String_t());
    TS_ASSERT_EQUALS(t.translator()("t"), "t");
    TS_ASSERT_EQUALS(tx("t"), "t");
    TS_ASSERT_THROWS_NOTHING(t.log().write(afl::sys::LogListener::Trace, "ch", "tx"));
}

