/**
  *  \file u/t_gfx_windowparameters.cpp
  *  \brief Test for gfx::WindowParameters
  */

#include "gfx/windowparameters.hpp"

#include "t_gfx.hpp"
#include "afl/base/vectorenumerator.hpp"
#include "afl/except/commandlineexception.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/longcommandlineparser.hpp"

namespace {
    void processOptions(gfx::WindowParameters& pp, afl::base::Memory<const char*const> args)
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
            TS_ASSERT(option);

            // Calling handleOption outside TS_ASSERT's builtin catch() allows us to test "throw" usecases.
            bool ok = handleWindowParameterOption(pp, text, parser, tx);
            TS_ASSERT(ok);
        }
    }
}

/** Test initialisation. */
void
TestGfxWindowParameters::testInit()
{
    gfx::WindowParameters t;

    // Must have sensible defaults
    TS_ASSERT_LESS_THAN_EQUALS(1, t.size.getX());
    TS_ASSERT_LESS_THAN_EQUALS(1, t.size.getX());
    TS_ASSERT_LESS_THAN_EQUALS(1, t.bitsPerPixel);
    TS_ASSERT_EQUALS(t.disableGrab, false);
}

/** Test help. */
void
TestGfxWindowParameters::testHelp()
{
    afl::string::NullTranslator tx;
    String_t result = gfx::getWindowParameterHelp(tx);
    TS_ASSERT_DIFFERS(result, "");                         // not empty
    TS_ASSERT_DIFFERS(result.find('\t'), String_t::npos);  // must have tab (formatOptions syntax)
    TS_ASSERT_DIFFERS(result.find('\n'), String_t::npos);  // must have multiple lines
}

/** Test "-fullscreen". */
void
TestGfxWindowParameters::testFullScreen()
{
    const char*const ARGS[] = {"-fullscreen"};

    gfx::WindowParameters testee;
    processOptions(testee, ARGS);
    TS_ASSERT_EQUALS(testee.fullScreen, true);
}

/** Test "-windowed". */
void
TestGfxWindowParameters::testWindowed()
{
    const char*const ARGS[] = {"-windowed"};

    gfx::WindowParameters testee;
    processOptions(testee, ARGS);
    TS_ASSERT_EQUALS(testee.fullScreen, false);
}

/** Test "-nomousegrab". */
void
TestGfxWindowParameters::testDisableGrab()
{
    const char*const ARGS[] = {"-nomousegrab"};

    gfx::WindowParameters testee;
    processOptions(testee, ARGS);
    TS_ASSERT_EQUALS(testee.disableGrab, true);
}

/** Test "-bpp" with assignment. */
void
TestGfxWindowParameters::testBpp()
{
    const char*const ARGS[] = {"-bpp=8"};

    gfx::WindowParameters testee;
    processOptions(testee, ARGS);
    TS_ASSERT_EQUALS(testee.bitsPerPixel, 8);
}

/** Test "-bpp" with separate parameter. */
void
TestGfxWindowParameters::testBppVariant()
{
    const char*const ARGS[] = {"-bpp", "16"};

    gfx::WindowParameters testee;
    processOptions(testee, ARGS);
    TS_ASSERT_EQUALS(testee.bitsPerPixel, 16);
}

/** Test "-bpp", error case. */
void
TestGfxWindowParameters::testBppFail()
{
    const char*const ARGS[] = {"-bpp", "X"};

    gfx::WindowParameters testee;
    TS_ASSERT_THROWS(processOptions(testee, ARGS), afl::except::CommandLineException);
}

/** Test "-size" with a single dimension. */
void
TestGfxWindowParameters::testSizeSingle()
{
    const char*const ARGS[] = {"-size=1024"};

    gfx::WindowParameters testee;
    processOptions(testee, ARGS);
    TS_ASSERT_EQUALS(testee.size.getX(), 1024);
    TS_ASSERT_EQUALS(testee.size.getY(), 768);
}

/** Test "-size" with a two-dimensional value. */
void
TestGfxWindowParameters::testSizePair()
{
    const char*const ARGS[] = {"-size", "1900x1700"};

    gfx::WindowParameters testee;
    processOptions(testee, ARGS);
    TS_ASSERT_EQUALS(testee.size.getX(), 1900);
    TS_ASSERT_EQUALS(testee.size.getY(), 1700);
}

/** Test "-size", syntax error case. */
void
TestGfxWindowParameters::testSizeBad1()
{
    const char*const ARGS[] = {"-size", "Q"};

    gfx::WindowParameters testee;
    TS_ASSERT_THROWS(processOptions(testee, ARGS), afl::except::CommandLineException);
}

/** Test "-size", bad delimiter. */
void
TestGfxWindowParameters::testSizeBad2()
{
    const char*const ARGS[] = {"-size", "800%600"};

    gfx::WindowParameters testee;
    TS_ASSERT_THROWS(processOptions(testee, ARGS), afl::except::CommandLineException);
}

/** Test "-size", bad second dimension. */
void
TestGfxWindowParameters::testSizeBad3()
{
    const char*const ARGS[] = {"-size", "800x"};

    gfx::WindowParameters testee;
    TS_ASSERT_THROWS(processOptions(testee, ARGS), afl::except::CommandLineException);
}

/** Test "-size", trailing garbage. */
void
TestGfxWindowParameters::testSizeBad4()
{
    const char*const ARGS[] = {"-size", "800x600x"};

    gfx::WindowParameters testee;
    TS_ASSERT_THROWS(processOptions(testee, ARGS), afl::except::CommandLineException);
}

/** Test "-size", bad range. */
void
TestGfxWindowParameters::testSizeRange()
{
    const char*const ARGS[] = {"-size", "999999x999999"};

    gfx::WindowParameters testee;
    TS_ASSERT_THROWS(processOptions(testee, ARGS), afl::except::CommandLineException);
}

