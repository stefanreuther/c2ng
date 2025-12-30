/**
  *  \file test/ui/reshack/editortest.cpp
  *  \brief Test for ui::reshack::Editor
  */

#include "ui/reshack/editor.hpp"

#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("ui.reshack.Editor")
{
    class Tester : public ui::reshack::Editor {
     public:
        virtual String_t getName(afl::string::Translator& /*tx*/)
            { return String_t(); }
        virtual void edit(ui::reshack::Session& /*session*/)
            { }
        virtual void save(ui::reshack::Session& /*session*/)
            { }
        virtual bool hasUnsavedChanges()
            { return false; }
    };
    Tester t;
}

