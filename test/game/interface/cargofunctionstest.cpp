/**
  *  \file test/game/interface/cargofunctionstest.cpp
  *  \brief Test for game::interface::CargoFunctions
  */

#include "game/interface/cargofunctions.hpp"

#include "afl/data/integervalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/error.hpp"
#include "interpreter/filevalue.hpp"
#include "interpreter/values.hpp"

/*
 *  Convenience Macros
 *
 *  Each requires 'seg' to be a afl::data::Segment with the parameters,
 *  and sets up 'args' as parameters for FUNC.
 */

#define CF_ASSERT_THROWS(a, FUNC)                       \
    interpreter::Arguments args(seg, 0, seg.size());    \
    AFL_CHECK_THROWS(a, FUNC, interpreter::Error);

#define CF_ASSERT_NULL(a, FUNC)                         \
    interpreter::Arguments args(seg, 0, seg.size());    \
    std::auto_ptr<afl::data::Value> result(FUNC);       \
    a.checkNull("result", result.get())

#define CF_ASSERT_STRING(a, FUNC, STR)                                  \
    interpreter::Arguments args(seg, 0, seg.size());                    \
    std::auto_ptr<afl::data::Value> result(FUNC);                       \
    a.checkEqual("toString", interpreter::toString(result.get(), false), STR)

#define CF_ASSERT_INTEGER(a, FUNC, VAL)                                 \
    int32_t iv;                                                         \
    interpreter::Arguments args(seg, 0, seg.size());                    \
    std::auto_ptr<afl::data::Value> result(FUNC);                       \
    a.checkEqual("checkIntegerArg", interpreter::checkIntegerArg(iv, result.get()), true); \
    a.checkEqual("value", iv, VAL)


/** Test checkCargoSpecArg(). */
AFL_TEST("game.interface.CargoFunctions:checkCargoSpecArg", a)
{
    using game::CargoSpec;
    CargoSpec arg;

    // Null -> false
    a.checkEqual("01. null", game::interface::checkCargoSpecArg(arg, 0), false);

    // Number (not a valid cargospec)
    afl::data::IntegerValue iv(42);
    AFL_CHECK_THROWS(a("11. int"), game::interface::checkCargoSpecArg(arg, &iv), interpreter::Error);

    // Some strings
    afl::data::StringValue sv1("");
    a.checkEqual("21", game::interface::checkCargoSpecArg(arg, &sv1), true);
    a.checkEqual("22", arg.isZero(), true);

    afl::data::StringValue sv2("30t 20ms");
    a.checkEqual("31", game::interface::checkCargoSpecArg(arg, &sv2), true);
    a.checkEqual("32", arg.isZero(), false);
    a.checkEqual("33", arg.get(CargoSpec::Tritanium), 30);
    a.checkEqual("34", arg.get(CargoSpec::Duranium), 0);
    a.checkEqual("35", arg.get(CargoSpec::Molybdenum), 20);
    a.checkEqual("36", arg.get(CargoSpec::Supplies), 20);
    a.checkEqual("37", arg.get(CargoSpec::Money), 0);

    afl::data::StringValue sv3("5m$ 1$");
    a.checkEqual("41", game::interface::checkCargoSpecArg(arg, &sv3), true);
    a.checkEqual("42", arg.isZero(), false);
    a.checkEqual("43", arg.get(CargoSpec::Tritanium), 0);
    a.checkEqual("44", arg.get(CargoSpec::Duranium), 0);
    a.checkEqual("45", arg.get(CargoSpec::Molybdenum), 5);
    a.checkEqual("46", arg.get(CargoSpec::Supplies), 0);
    a.checkEqual("47", arg.get(CargoSpec::Money), 6);

    afl::data::StringValue sv4("T4 D3 M2 9t");
    a.checkEqual("51", game::interface::checkCargoSpecArg(arg, &sv4), true);
    a.checkEqual("52", arg.isZero(), false);
    a.checkEqual("53", arg.get(CargoSpec::Tritanium), 13);
    a.checkEqual("54", arg.get(CargoSpec::Duranium), 3);
    a.checkEqual("55", arg.get(CargoSpec::Molybdenum), 2);
    a.checkEqual("56", arg.get(CargoSpec::Supplies), 0);
    a.checkEqual("57", arg.get(CargoSpec::Money), 0);
}

/*
 *  CAdd
 */

// CAdd("10T", "5T 3M") = "15T 3M"
AFL_TEST("game.interface.CargoFunctions:CAdd:str+str", a)
{
    afl::data::Segment seg;
    seg.pushBackString("10T");
    seg.pushBackString("5T 3M");
    CF_ASSERT_STRING(a, game::interface::IFCAdd(args), "15T 3M");
}

// CAdd("") = ""
AFL_TEST("game.interface.CargoFunctions:CAdd:blank", a)
{
    afl::data::Segment seg;
    seg.pushBackString("");
    CF_ASSERT_STRING(a, game::interface::IFCAdd(args), "");
}

// CAdd() = error
AFL_TEST("game.interface.CargoFunctions:CAdd:nullary", a)
{
    afl::data::Segment seg;
    CF_ASSERT_THROWS(a, game::interface::IFCAdd(args));
}

// CAdd("10T", null) = null
AFL_TEST("game.interface.CargoFunctions:CAdd:str+null", a)
{
    afl::data::Segment seg;
    seg.pushBackString("10T");
    seg.pushBackNew(0);
    CF_ASSERT_NULL(a, game::interface::IFCAdd(args));
}

/*
 *  CCompare
 */

// CCompare() = error
AFL_TEST("game.interface.CargoFunctions:CCompare:nullary", a)
{
    afl::data::Segment seg;
    CF_ASSERT_THROWS(a, game::interface::IFCCompare(args));
}

// CCompare("10T", null) = null
AFL_TEST("game.interface.CargoFunctions:CCompare:str+null", a)
{
    afl::data::Segment seg;
    seg.pushBackString("10T");
    seg.pushBackNew(0);
    CF_ASSERT_NULL(a, game::interface::IFCCompare(args));
}

// CCompare(null, "10T") = null
AFL_TEST("game.interface.CargoFunctions:CCompare:null+str", a)
{
    afl::data::Segment seg;
    seg.pushBackNew(0);
    seg.pushBackString("10T");
    CF_ASSERT_NULL(a, game::interface::IFCCompare(args));
}

// CCompare("10T", "10T") = true
AFL_TEST("game.interface.CargoFunctions:CCompare:str-equal", a)
{
    afl::data::Segment seg;
    seg.pushBackString("10T");
    seg.pushBackString("10T");
    CF_ASSERT_INTEGER(a, game::interface::IFCCompare(args), 1);
}

// CCompare("11T", "10T") = true
AFL_TEST("game.interface.CargoFunctions:CCompare:str-gt", a)
{
    afl::data::Segment seg;
    seg.pushBackString("11T");
    seg.pushBackString("10T");
    CF_ASSERT_INTEGER(a, game::interface::IFCCompare(args), 1);
}

// CCompare("10T", "11T") = false
AFL_TEST("game.interface.CargoFunctions:CCompare:str-lt", a)
{
    afl::data::Segment seg;
    seg.pushBackString("10T");
    seg.pushBackString("11T");
    CF_ASSERT_INTEGER(a, game::interface::IFCCompare(args), 0);
}

/*
 *  CDiv
 */

// CDiv() = error
AFL_TEST("game.interface.CargoFunctions:CDiv:nullary", a)
{
    afl::data::Segment seg;
    CF_ASSERT_THROWS(a, game::interface::IFCDiv(args));
}

// CDiv("10T", null) = null
AFL_TEST("game.interface.CargoFunctions:CDiv:str+null", a)
{
    afl::data::Segment seg;
    seg.pushBackString("10T");
    seg.pushBackNew(0);
    CF_ASSERT_NULL(a, game::interface::IFCDiv(args));
}

// CDiv(null, "10T") = null
AFL_TEST("game.interface.CargoFunctions:CDiv:null+str", a)
{
    afl::data::Segment seg;
    seg.pushBackNew(0);
    seg.pushBackString("10T");
    CF_ASSERT_NULL(a, game::interface::IFCDiv(args));
}

// CDiv("25T", "10T") = 2
AFL_TEST("game.interface.CargoFunctions:CDiv:str+str", a)
{
    afl::data::Segment seg;
    seg.pushBackString("25T");
    seg.pushBackString("10T");
    CF_ASSERT_INTEGER(a, game::interface::IFCDiv(args), 2);
}

// CDiv("25T", 3) = "8T"
AFL_TEST("game.interface.CargoFunctions:CDiv:str+num", a)
{
    afl::data::Segment seg;
    seg.pushBackString("25T");
    seg.pushBackInteger(3);
    CF_ASSERT_STRING(a, game::interface::IFCDiv(args), "8T");
}

// CDiv("25T", 0) = error
AFL_TEST("game.interface.CargoFunctions:CDiv:str+zero", a)
{
    afl::data::Segment seg;
    seg.pushBackString("25T");
    seg.pushBackInteger(0);
    CF_ASSERT_THROWS(a, game::interface::IFCDiv(args));
}

// CDiv("25T", "") = error
AFL_TEST("game.interface.CargoFunctions:CDiv:str+empty", a)
{
    afl::data::Segment seg;
    seg.pushBackString("25T");
    seg.pushBackString("");
    CF_ASSERT_THROWS(a, game::interface::IFCDiv(args));
}

// CDiv("25T", object) = error
AFL_TEST("game.interface.CargoFunctions:CDiv:str+object", a)
{
    afl::data::Segment seg;
    seg.pushBackString("25T");
    seg.pushBackNew(new interpreter::FileValue(3));
    CF_ASSERT_THROWS(a, game::interface::IFCDiv(args));
}

/*
 *  CExtract
 */

// CExtract() = error
AFL_TEST("game.interface.CargoFunctions:CExtract:nullary", a)
{
    afl::data::Segment seg;
    CF_ASSERT_THROWS(a, game::interface::IFCExtract(args));
}

// CExtract("10T", null) = null
AFL_TEST("game.interface.CargoFunctions:CExtract:str+null", a)
{
    afl::data::Segment seg;
    seg.pushBackString("10T");
    seg.pushBackNew(0);
    CF_ASSERT_NULL(a, game::interface::IFCExtract(args));
}

// CExtract(null, "t") = null
AFL_TEST("game.interface.CargoFunctions:CExtract:null+str", a)
{
    afl::data::Segment seg;
    seg.pushBackNew(0);
    seg.pushBackString("t");
    CF_ASSERT_NULL(a, game::interface::IFCExtract(args));
}

// CExtract("10T 20M 30D", "tmm") = 30
AFL_TEST("game.interface.CargoFunctions:CExtract:str+str", a)
{
    afl::data::Segment seg;
    seg.pushBackString("10T 20M 30D");
    seg.pushBackString("tmm");
    CF_ASSERT_INTEGER(a, game::interface::IFCExtract(args), 30);
}

// CExtract("10T 20M 30D", "") = 0
AFL_TEST("game.interface.CargoFunctions:CExtract:str+empty", a)
{
    afl::data::Segment seg;
    seg.pushBackString("10T 20M 30D");
    seg.pushBackString("");
    CF_ASSERT_INTEGER(a, game::interface::IFCExtract(args), 0);
}

// CExtract("10T", "q") = error
AFL_TEST("game.interface.CargoFunctions:CExtract:str+bad", a)
{
    afl::data::Segment seg;
    seg.pushBackString("10T 20M 30D");
    seg.pushBackString("q");
    CF_ASSERT_THROWS(a, game::interface::IFCExtract(args));
}

/*
 *  CMul
 */

// CMul() = error
AFL_TEST("game.interface.CargoFunctions:CMul:nullary", a)
{
    afl::data::Segment seg;
    CF_ASSERT_THROWS(a, game::interface::IFCMul(args));
}

// CMul("10T", null) = null
AFL_TEST("game.interface.CargoFunctions:CMul:str+null", a)
{
    afl::data::Segment seg;
    seg.pushBackString("10T");
    seg.pushBackNew(0);
    CF_ASSERT_NULL(a, game::interface::IFCMul(args));
}

// CMul(null, 7) = null
AFL_TEST("game.interface.CargoFunctions:CMul:null+str", a)
{
    afl::data::Segment seg;
    seg.pushBackNew(0);
    seg.pushBackInteger(7);
    CF_ASSERT_NULL(a, game::interface::IFCMul(args));
}

// CMul("10T 20M 30D", 4) = "40T 120M 80D"
AFL_TEST("game.interface.CargoFunctions:CMul:str+int", a)
{
    afl::data::Segment seg;
    seg.pushBackString("10T 20M 30D");
    seg.pushBackInteger(4);
    CF_ASSERT_STRING(a, game::interface::IFCMul(args), "40T 120D 80M");
}

/*
 *  CRemove
 */

// CRemove() = error
AFL_TEST("game.interface.CargoFunctions:CRemove:nullary", a)
{
    afl::data::Segment seg;
    CF_ASSERT_THROWS(a, game::interface::IFCRemove(args));
}

// CRemove("10T", null) = null
AFL_TEST("game.interface.CargoFunctions:CRemove:str+null", a)
{
    afl::data::Segment seg;
    seg.pushBackString("10T");
    seg.pushBackNew(0);
    CF_ASSERT_NULL(a, game::interface::IFCRemove(args));
}

// CRemove(null, "t") = null
AFL_TEST("game.interface.CargoFunctions:CRemove:null+str", a)
{
    afl::data::Segment seg;
    seg.pushBackNew(0);
    seg.pushBackString("t");
    CF_ASSERT_NULL(a, game::interface::IFCRemove(args));
}

// CRemove("10T 20M 40D 50S", "tmm") = "40D 50S"
AFL_TEST("game.interface.CargoFunctions:CRemove:str+str", a)
{
    afl::data::Segment seg;
    seg.pushBackString("10T 20M 40D 50S");
    seg.pushBackString("tmm");
    CF_ASSERT_STRING(a, game::interface::IFCRemove(args), "40D 50S");
}

// CRemove("10T 20M 30D", "") = "10T 30D 20M"
AFL_TEST("game.interface.CargoFunctions:CRemove:str+empty", a)
{
    afl::data::Segment seg;
    seg.pushBackString("10T 20M 30D");
    seg.pushBackString("");
    CF_ASSERT_STRING(a, game::interface::IFCRemove(args), "10T 30D 20M");
}

// CRemove("10T", "q") = error
AFL_TEST("game.interface.CargoFunctions:CRemove:str+bad", a)
{
    afl::data::Segment seg;
    seg.pushBackString("10T 20M 30D");
    seg.pushBackString("q");
    CF_ASSERT_THROWS(a, game::interface::IFCRemove(args));
}

/*
 *  CSub
 */

// CSub("10T 3M", "5T 3M") = "5T 3M"
AFL_TEST("game.interface.CargoFunctions:CSub:str+str", a)
{
    afl::data::Segment seg;
    seg.pushBackString("10T 3M");
    seg.pushBackString("5T");
    CF_ASSERT_STRING(a, game::interface::IFCSub(args), "5T 3M");
}

// CSub() = error
AFL_TEST("game.interface.CargoFunctions:CSub:nullary", a)
{
    afl::data::Segment seg;
    CF_ASSERT_THROWS(a, game::interface::IFCSub(args));
}

// CSub("10T", null) = null
AFL_TEST("game.interface.CargoFunctions:CSub:str+null", a)
{
    afl::data::Segment seg;
    seg.pushBackString("10T");
    seg.pushBackNew(0);
    CF_ASSERT_NULL(a, game::interface::IFCSub(args));
}

// CSub(null, "10T") = null
AFL_TEST("game.interface.CargoFunctions:CSub:null+str", a)
{
    afl::data::Segment seg;
    seg.pushBackNew(0);
    seg.pushBackString("10T");
    CF_ASSERT_NULL(a, game::interface::IFCSub(args));
}

// CSub("10T", "1T", "2T", "3T") = "4T"
AFL_TEST("game.interface.CargoFunctions:CSub:multiple", a)
{
    afl::data::Segment seg;
    seg.pushBackString("10T");
    seg.pushBackString("1T");
    seg.pushBackString("2T");
    seg.pushBackString("3T");
    CF_ASSERT_STRING(a, game::interface::IFCSub(args), "4T");
}

// CSub("10$", "5S") = "-5S 100$"
AFL_TEST("game.interface.CargoFunctions:CSub:underflow", a)
{
    afl::data::Segment seg;
    seg.pushBackString("10$");
    seg.pushBackString("5S");
    CF_ASSERT_STRING(a, game::interface::IFCSub(args), "-5S 10$");
}

// CSub("10S", "5$") = "5S"
AFL_TEST("game.interface.CargoFunctions:CSub:supply-sale", a)
{
    afl::data::Segment seg;
    seg.pushBackString("10S");
    seg.pushBackString("5$");
    CF_ASSERT_STRING(a, game::interface::IFCSub(args), "5S");
}

// CSub("-5S", "3$") = "-5S -3$"
AFL_TEST("game.interface.CargoFunctions:CSub:negative", a)
{
    afl::data::Segment seg;
    seg.pushBackString("-5S");
    seg.pushBackString("3$");
    CF_ASSERT_STRING(a, game::interface::IFCSub(args), "-5S -3$");
}
