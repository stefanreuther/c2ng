/**
  *  \file u/t_game_interface_cargofunctions.cpp
  *  \brief Test for game::interface::CargoFunctions
  */

#include "game/interface/cargofunctions.hpp"

#include "t_game_interface.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "interpreter/error.hpp"
#include "interpreter/filevalue.hpp"
#include "interpreter/values.hpp"

/*
 *  Convenience Macros
 *
 *  Each requires 'seg' to be a afl::data::Segment with the parameters,
 *  and sets up 'session, args' as parameters for FUNC.
 */

#define CF_ASSERT_THROWS(FUNC)                          \
    afl::string::NullTranslator tx;                     \
    afl::io::NullFileSystem fs;                         \
    game::Session session(tx, fs);                      \
    interpreter::Arguments args(seg, 0, seg.size());    \
    TS_ASSERT_THROWS(FUNC, interpreter::Error);

#define CF_ASSERT_NULL(FUNC)                            \
    afl::string::NullTranslator tx;                     \
    afl::io::NullFileSystem fs;                         \
    game::Session session(tx, fs);                      \
    interpreter::Arguments args(seg, 0, seg.size());    \
    std::auto_ptr<afl::data::Value> result(FUNC);       \
    TS_ASSERT(result.get() == 0)

#define CF_ASSERT_STRING(FUNC, STR)                                     \
    afl::string::NullTranslator tx;                                     \
    afl::io::NullFileSystem fs;                                         \
    game::Session session(tx, fs);                                      \
    interpreter::Arguments args(seg, 0, seg.size());                    \
    std::auto_ptr<afl::data::Value> result(FUNC);                       \
    TS_ASSERT_EQUALS(interpreter::toString(result.get(), false), STR)

#define CF_ASSERT_INTEGER(FUNC, VAL)                                    \
    afl::string::NullTranslator tx;                                     \
    afl::io::NullFileSystem fs;                                         \
    game::Session session(tx, fs);                                      \
    int32_t iv;                                                         \
    interpreter::Arguments args(seg, 0, seg.size());                    \
    std::auto_ptr<afl::data::Value> result(FUNC);                       \
    TS_ASSERT_EQUALS(interpreter::checkIntegerArg(iv, result.get()), true); \
    TS_ASSERT_EQUALS(iv, VAL)


/** Test checkCargoSpecArg(). */
void
TestGameInterfaceCargoFunctions::testCheckCargoSpecArg()
{
    using game::CargoSpec;
    CargoSpec a;

    // Null -> false
    TS_ASSERT_EQUALS(game::interface::checkCargoSpecArg(a, 0), false);

    // Number (not a valid cargospec)
    afl::data::IntegerValue iv(42);
    TS_ASSERT_THROWS(game::interface::checkCargoSpecArg(a, &iv), interpreter::Error);

    // Some strings
    afl::data::StringValue sv1("");
    TS_ASSERT_EQUALS(game::interface::checkCargoSpecArg(a, &sv1), true);
    TS_ASSERT_EQUALS(a.isZero(), true);

    afl::data::StringValue sv2("30t 20ms");
    TS_ASSERT_EQUALS(game::interface::checkCargoSpecArg(a, &sv2), true);
    TS_ASSERT_EQUALS(a.isZero(), false);
    TS_ASSERT_EQUALS(a.get(CargoSpec::Tritanium), 30);
    TS_ASSERT_EQUALS(a.get(CargoSpec::Duranium), 0);
    TS_ASSERT_EQUALS(a.get(CargoSpec::Molybdenum), 20);
    TS_ASSERT_EQUALS(a.get(CargoSpec::Supplies), 20);
    TS_ASSERT_EQUALS(a.get(CargoSpec::Money), 0);

    afl::data::StringValue sv3("5m$ 1$");
    TS_ASSERT_EQUALS(game::interface::checkCargoSpecArg(a, &sv3), true);
    TS_ASSERT_EQUALS(a.isZero(), false);
    TS_ASSERT_EQUALS(a.get(CargoSpec::Tritanium), 0);
    TS_ASSERT_EQUALS(a.get(CargoSpec::Duranium), 0);
    TS_ASSERT_EQUALS(a.get(CargoSpec::Molybdenum), 5);
    TS_ASSERT_EQUALS(a.get(CargoSpec::Supplies), 0);
    TS_ASSERT_EQUALS(a.get(CargoSpec::Money), 6);

    afl::data::StringValue sv4("T4 D3 M2 9t");
    TS_ASSERT_EQUALS(game::interface::checkCargoSpecArg(a, &sv4), true);
    TS_ASSERT_EQUALS(a.isZero(), false);
    TS_ASSERT_EQUALS(a.get(CargoSpec::Tritanium), 13);
    TS_ASSERT_EQUALS(a.get(CargoSpec::Duranium), 3);
    TS_ASSERT_EQUALS(a.get(CargoSpec::Molybdenum), 2);
    TS_ASSERT_EQUALS(a.get(CargoSpec::Supplies), 0);
    TS_ASSERT_EQUALS(a.get(CargoSpec::Money), 0);
}

/** Test CAdd(). */
void
TestGameInterfaceCargoFunctions::testCAdd()
{
    // CAdd("10T", "5T 3M") = "15T 3M"
    {
        afl::data::Segment seg;
        seg.pushBackString("10T");
        seg.pushBackString("5T 3M");
        CF_ASSERT_STRING(game::interface::IFCAdd(session, args), "15T 3M");
    }

    // CAdd("") = ""
    {
        afl::data::Segment seg;
        seg.pushBackString("");
        CF_ASSERT_STRING(game::interface::IFCAdd(session, args), "");
    }

    // CAdd() = error
    {
        afl::data::Segment seg;
        CF_ASSERT_THROWS(game::interface::IFCAdd(session, args));
    }

    // CAdd("10T", null) = null
    {
        afl::data::Segment seg;
        seg.pushBackString("10T");
        seg.pushBackNew(0);
        CF_ASSERT_NULL(game::interface::IFCAdd(session, args));
    }
}

/** Test CCompare(). */
void
TestGameInterfaceCargoFunctions::testCCompare()
{
    // CCompare() = error
    {
        afl::data::Segment seg;
        CF_ASSERT_THROWS(game::interface::IFCCompare(session, args));
    }

    // CCompare("10T", null) = null
    {
        afl::data::Segment seg;
        seg.pushBackString("10T");
        seg.pushBackNew(0);
        CF_ASSERT_NULL(game::interface::IFCCompare(session, args));
    }

    // CCompare(null, "10T") = null
    {
        afl::data::Segment seg;
        seg.pushBackNew(0);
        seg.pushBackString("10T");
        CF_ASSERT_NULL(game::interface::IFCCompare(session, args));
    }

    // CCompare("10T", "10T") = true
    {
        afl::data::Segment seg;
        seg.pushBackString("10T");
        seg.pushBackString("10T");
        CF_ASSERT_INTEGER(game::interface::IFCCompare(session, args), 1);
    }

    // CCompare("11T", "10T") = true
    {
        afl::data::Segment seg;
        seg.pushBackString("11T");
        seg.pushBackString("10T");
        CF_ASSERT_INTEGER(game::interface::IFCCompare(session, args), 1);
    }

    // CCompare("10T", "11T") = true
    {
        afl::data::Segment seg;
        seg.pushBackString("10T");
        seg.pushBackString("11T");
        CF_ASSERT_INTEGER(game::interface::IFCCompare(session, args), 0);
    }
}

/** Test CDiv(). */
void
TestGameInterfaceCargoFunctions::testCDiv()
{
    // CDiv() = error
    {
        afl::data::Segment seg;
        CF_ASSERT_THROWS(game::interface::IFCDiv(session, args));
    }

    // CDiv("10T", null) = null
    {
        afl::data::Segment seg;
        seg.pushBackString("10T");
        seg.pushBackNew(0);
        CF_ASSERT_NULL(game::interface::IFCDiv(session, args));
    }

    // CDiv(null, "10T") = null
    {
        afl::data::Segment seg;
        seg.pushBackNew(0);
        seg.pushBackString("10T");
        CF_ASSERT_NULL(game::interface::IFCDiv(session, args));
    }

    // CDiv("25T", "10T") = 2
    {
        afl::data::Segment seg;
        seg.pushBackString("25T");
        seg.pushBackString("10T");
        CF_ASSERT_INTEGER(game::interface::IFCDiv(session, args), 2);
    }

    // CDiv("25T", 3) = "8T"
    {
        afl::data::Segment seg;
        seg.pushBackString("25T");
        seg.pushBackInteger(3);
        CF_ASSERT_STRING(game::interface::IFCDiv(session, args), "8T");
    }

    // CDiv("25T", 0) = error
    {
        afl::data::Segment seg;
        seg.pushBackString("25T");
        seg.pushBackInteger(0);
        CF_ASSERT_THROWS(game::interface::IFCDiv(session, args));
    }

    // CDiv("25T", "") = error
    {
        afl::data::Segment seg;
        seg.pushBackString("25T");
        seg.pushBackString("");
        CF_ASSERT_THROWS(game::interface::IFCDiv(session, args));
    }

    // CDiv("25T", object) = error
    {
        afl::data::Segment seg;
        seg.pushBackString("25T");
        seg.pushBackNew(new interpreter::FileValue(3));
        CF_ASSERT_THROWS(game::interface::IFCDiv(session, args));
    }
}

/** Test CExtract(). */
void
TestGameInterfaceCargoFunctions::testCExtract()
{
    // CExtract() = error
    {
        afl::data::Segment seg;
        CF_ASSERT_THROWS(game::interface::IFCExtract(session, args));
    }

    // CExtract("10T", null) = null
    {
        afl::data::Segment seg;
        seg.pushBackString("10T");
        seg.pushBackNew(0);
        CF_ASSERT_NULL(game::interface::IFCExtract(session, args));
    }

    // CExtract(null, "t") = null
    {
        afl::data::Segment seg;
        seg.pushBackNew(0);
        seg.pushBackString("t");
        CF_ASSERT_NULL(game::interface::IFCExtract(session, args));
    }

    // CExtract("10T 20M 30D", "tmm") = 30
    {
        afl::data::Segment seg;
        seg.pushBackString("10T 20M 30D");
        seg.pushBackString("tmm");
        CF_ASSERT_INTEGER(game::interface::IFCExtract(session, args), 30);
    }

    // CExtract("10T 20M 30D", "") = 0
    {
        afl::data::Segment seg;
        seg.pushBackString("10T 20M 30D");
        seg.pushBackString("");
        CF_ASSERT_INTEGER(game::interface::IFCExtract(session, args), 0);
    }

    // CExtract("10T", "q") = error
    {
        afl::data::Segment seg;
        seg.pushBackString("10T 20M 30D");
        seg.pushBackString("q");
        CF_ASSERT_THROWS(game::interface::IFCExtract(session, args));
    }
}

/** Test CMul(). */
void
TestGameInterfaceCargoFunctions::testCMul()
{
    // CMul() = error
    {
        afl::data::Segment seg;
        CF_ASSERT_THROWS(game::interface::IFCMul(session, args));
    }

    // CMul("10T", null) = null
    {
        afl::data::Segment seg;
        seg.pushBackString("10T");
        seg.pushBackNew(0);
        CF_ASSERT_NULL(game::interface::IFCMul(session, args));
    }

    // CMul(null, 7) = null
    {
        afl::data::Segment seg;
        seg.pushBackNew(0);
        seg.pushBackInteger(7);
        CF_ASSERT_NULL(game::interface::IFCMul(session, args));
    }

    // CMul("10T 20M 30D", 4) = "40T 120M 80D"
    {
        afl::data::Segment seg;
        seg.pushBackString("10T 20M 30D");
        seg.pushBackInteger(4);
        CF_ASSERT_STRING(game::interface::IFCMul(session, args), "40T 120D 80M");
    }
}

/** Test CRemove(). */
void
TestGameInterfaceCargoFunctions::testCRemove()
{
    // CRemove() = error
    {
        afl::data::Segment seg;
        CF_ASSERT_THROWS(game::interface::IFCRemove(session, args));
    }

    // CRemove("10T", null) = null
    {
        afl::data::Segment seg;
        seg.pushBackString("10T");
        seg.pushBackNew(0);
        CF_ASSERT_NULL(game::interface::IFCRemove(session, args));
    }

    // CRemove(null, "t") = null
    {
        afl::data::Segment seg;
        seg.pushBackNew(0);
        seg.pushBackString("t");
        CF_ASSERT_NULL(game::interface::IFCRemove(session, args));
    }

    // CRemove("10T 20M 40D 50S", "tmm") = "40D 50S"
    {
        afl::data::Segment seg;
        seg.pushBackString("10T 20M 40D 50S");
        seg.pushBackString("tmm");
        CF_ASSERT_STRING(game::interface::IFCRemove(session, args), "40D 50S");
    }

    // CRemove("10T 20M 30D", "") = "10T 30D 20M"
    {
        afl::data::Segment seg;
        seg.pushBackString("10T 20M 30D");
        seg.pushBackString("");
        CF_ASSERT_STRING(game::interface::IFCRemove(session, args), "10T 30D 20M");
    }

    // CRemove("10T", "q") = error
    {
        afl::data::Segment seg;
        seg.pushBackString("10T 20M 30D");
        seg.pushBackString("q");
        CF_ASSERT_THROWS(game::interface::IFCRemove(session, args));
    }
}

void
TestGameInterfaceCargoFunctions::testCSub()
{
    // CSub("10T 3M", "5T 3M") = "5T 3M"
    {
        afl::data::Segment seg;
        seg.pushBackString("10T 3M");
        seg.pushBackString("5T");
        CF_ASSERT_STRING(game::interface::IFCSub(session, args), "5T 3M");
    }

    // CSub() = error
    {
        afl::data::Segment seg;
        CF_ASSERT_THROWS(game::interface::IFCSub(session, args));
    }

    // CSub("10T", null) = null
    {
        afl::data::Segment seg;
        seg.pushBackString("10T");
        seg.pushBackNew(0);
        CF_ASSERT_NULL(game::interface::IFCSub(session, args));
    }

    // CSub(null, "10T") = null
    {
        afl::data::Segment seg;
        seg.pushBackNew(0);
        seg.pushBackString("10T");
        CF_ASSERT_NULL(game::interface::IFCSub(session, args));
    }

    // CSub("10T", "1T", "2T", "3T") = "4T"
    {
        afl::data::Segment seg;
        seg.pushBackString("10T");
        seg.pushBackString("1T");
        seg.pushBackString("2T");
        seg.pushBackString("3T");
        CF_ASSERT_STRING(game::interface::IFCSub(session, args), "4T");
    }
}

