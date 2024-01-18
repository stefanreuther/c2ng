/**
  *  \file test/gfx/windowparameterstest.cpp
  *  \brief Test for gfx::WindowParameters
  */

#include "gfx/windowparameters.hpp"

#include "afl/base/vectorenumerator.hpp"
#include "afl/except/commandlineexception.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/longcommandlineparser.hpp"
#include "afl/test/testrunner.hpp"

namespace {
    void processOptions(afl::test::Assert a, gfx::WindowParameters& pp, afl::base::Memory<const char*const> args)
    {
        afl::base::Ref<afl::base::VectorEnumerator<String_t> > argVec = *new afl::base::VectorEnumerator<String_t>();
        while (const char*const* arg = args.eat()) {
            argVec->add(*arg);
        }

        afl::sys::LongCommandLineParser parser(argVec);
        bool option;
        String_t text;
        afl::string::NullTranslator tx;
        while (parser.getNext(option, text)) {
            a.check("01. option", option);
            a.check("02. handleWindowParameterOption", handleWindowParameterOption(pp, text, parser, tx));
        }
    }
}

/** Test initialisation. */
AFL_TEST("gfx.WindowParameters:init", a)
{
    gfx::WindowParameters t;

    // Must have sensible defaults
    a.checkGreaterEqual("01. X size", t.size.getX(), 1);
    a.checkGreaterEqual("02. Y size", t.size.getX(), 1);
    a.checkGreaterEqual("03. bpp", t.bitsPerPixel, 1);
    a.checkEqual("04. disableGrab", t.disableGrab, false);
}

/** Test help. */
AFL_TEST("gfx.WindowParameters:getWindowParameterHelp", a)
{
    afl::string::NullTranslator tx;
    String_t result = gfx::getWindowParameterHelp(tx);
    a.checkDifferent("01. not empty",   result, "");                         // not empty
    a.checkDifferent("02. has tab",     result.find('\t'), String_t::npos);  // must have tab (formatOptions syntax)
    a.checkDifferent("03. new newline", result.find('\n'), String_t::npos);  // must have multiple lines
}

/** Test "-fullscreen". */
AFL_TEST("gfx.WindowParameters:option:fullscreen", a)
{
    const char*const ARGS[] = {"-fullscreen"};

    gfx::WindowParameters testee;
    processOptions(a, testee, ARGS);
    a.checkEqual("01. fullScreen", testee.fullScreen, true);
}

/** Test "-windowed". */
AFL_TEST("gfx.WindowParameters:option:windowed", a)
{
    const char*const ARGS[] = {"-windowed"};

    gfx::WindowParameters testee;
    processOptions(a, testee, ARGS);
    a.checkEqual("01. fullScreen", testee.fullScreen, false);
}

/** Test "-nomousegrab". */
AFL_TEST("gfx.WindowParameters:option:nomousegrab", a)
{
    const char*const ARGS[] = {"-nomousegrab"};

    gfx::WindowParameters testee;
    processOptions(a, testee, ARGS);
    a.checkEqual("01. disableGrab", testee.disableGrab, true);
}

/** Test "-bpp" with assignment. */
AFL_TEST("gfx.WindowParameters:option:bpp", a)
{
    const char*const ARGS[] = {"-bpp=8"};

    gfx::WindowParameters testee;
    processOptions(a, testee, ARGS);
    a.checkEqual("01. bitsPerPixel", testee.bitsPerPixel, 8);
}

/** Test "-bpp" with separate parameter. */
AFL_TEST("gfx.WindowParameters:option:bpp:separate", a)
{
    const char*const ARGS[] = {"-bpp", "16"};

    gfx::WindowParameters testee;
    processOptions(a, testee, ARGS);
    a.checkEqual("01. bitsPerPixel", testee.bitsPerPixel, 16);
}

/** Test "-bpp", error case. */
AFL_TEST("gfx.WindowParameters:option:bpp:error", a)
{
    const char*const ARGS[] = {"-bpp", "X"};

    gfx::WindowParameters testee;
    AFL_CHECK_THROWS(a, processOptions(a, testee, ARGS), afl::except::CommandLineException);
}

/** Test "-size" with a single dimension. */
AFL_TEST("gfx.WindowParameters:option:size:single", a)
{
    const char*const ARGS[] = {"-size=1024"};

    gfx::WindowParameters testee;
    processOptions(a, testee, ARGS);
    a.checkEqual("01. X", testee.size.getX(), 1024);
    a.checkEqual("02. Y", testee.size.getY(), 768);
}

/** Test "-size" with a two-dimensional value. */
AFL_TEST("gfx.WindowParameters:option:size:pair", a)
{
    const char*const ARGS[] = {"-size", "1900x1700"};

    gfx::WindowParameters testee;
    processOptions(a, testee, ARGS);
    a.checkEqual("01. X", testee.size.getX(), 1900);
    a.checkEqual("02. Y", testee.size.getY(), 1700);
}

/** Test "-size", syntax error case. */
AFL_TEST("gfx.WindowParameters:option:size:error:no-number", a)
{
    const char*const ARGS[] = {"-size", "Q"};

    gfx::WindowParameters testee;
    AFL_CHECK_THROWS(a, processOptions(a, testee, ARGS), afl::except::CommandLineException);
}

/** Test "-size", bad delimiter. */
AFL_TEST("gfx.WindowParameters:option:size:error:no-delimiter", a)
{
    const char*const ARGS[] = {"-size", "800%600"};

    gfx::WindowParameters testee;
    AFL_CHECK_THROWS(a, processOptions(a, testee, ARGS), afl::except::CommandLineException);
}

/** Test "-size", bad second dimension. */
AFL_TEST("gfx.WindowParameters:option:size:error:no-second-dimension", a)
{
    const char*const ARGS[] = {"-size", "800x"};

    gfx::WindowParameters testee;
    AFL_CHECK_THROWS(a, processOptions(a, testee, ARGS), afl::except::CommandLineException);
}

/** Test "-size", trailing garbage. */
AFL_TEST("gfx.WindowParameters:option:size:error:too-many-delimiters", a)
{
    const char*const ARGS[] = {"-size", "800x600x"};

    gfx::WindowParameters testee;
    AFL_CHECK_THROWS(a, processOptions(a, testee, ARGS), afl::except::CommandLineException);
}

/** Test "-size", bad range. */
AFL_TEST("gfx.WindowParameters:option:size:error:range", a)
{
    const char*const ARGS[] = {"-size", "999999x999999"};

    gfx::WindowParameters testee;
    AFL_CHECK_THROWS(a, processOptions(a, testee, ARGS), afl::except::CommandLineException);
}
