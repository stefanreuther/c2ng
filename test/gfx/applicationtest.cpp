/**
  *  \file test/gfx/applicationtest.cpp
  *  \brief Test for gfx::Application
  */

#include "gfx/application.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST("gfx.Application", a)
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
    a.checkEqual("01. translator", t.translator()("t"), "t");
    a.checkEqual("02. translator", tx("t"), "t");
    AFL_CHECK_SUCCEEDS(a("03. log"), t.log().write(afl::sys::LogListener::Trace, "ch", "tx"));
}
