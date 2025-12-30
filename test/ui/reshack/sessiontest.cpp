/**
  *  \file test/ui/reshack/sessiontest.cpp
  *  \brief Test for ui::reshack::Session
  */

#include "ui/reshack/session.hpp"

#include "afl/test/testrunner.hpp"
#include "afl/string/nulltranslator.hpp"
#include "gfx/nullengine.hpp"
#include "gfx/nullresourceprovider.hpp"
#include "ui/root.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "ui/res/manager.hpp"
#include "gfx/fontlist.hpp"

AFL_TEST("ui.reshack.Session", a)
{
    // Environment
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    gfx::NullEngine engine;
    gfx::NullResourceProvider provider;
    ui::Root root(engine, provider, gfx::WindowParameters());
    ui::res::Manager mgr;
    gfx::FontList fontList;

    // Session
    ui::reshack::Session testee(root, tx, fs, mgr, fontList);

    // Verify
    a.checkEqual("root",        &root,     &testee.root());
    a.checkEqual("tx",          &tx,       &testee.translator());
    a.checkEqual("fs",          &fs,       &testee.fileSystem());
    a.checkEqual("mgr",         &mgr,      &testee.manager());
    a.checkEqual("fontList",    &fontList, &testee.fontList());

    a.checkNonNull("clipboard", &testee.clipboard());
    a.checkNonNull("charNames", &testee.characterNames());

    testee.setPreviewText("foo");
    a.checkEqual("preview", testee.getPreviewText(), "foo");
}

