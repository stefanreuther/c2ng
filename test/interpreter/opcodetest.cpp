/**
  *  \file test/interpreter/opcodetest.cpp
  *  \brief Test for interpreter::Opcode
  */

#include "interpreter/opcode.hpp"
#include "afl/test/testrunner.hpp"

using interpreter::Opcode;

namespace {
    Opcode make(uint8_t ma, uint8_t mi, uint16_t arg)
    {
        Opcode result;
        result.major = ma;
        result.minor = mi;
        result.arg = arg;
        return result;
    }
}

/** Test "push" operations. */
AFL_TEST("interpreter.Opcode:maPush", a)
{
    // pushloc 9
    Opcode aa = make(Opcode::maPush, Opcode::sLocal, 9);
    a.check("01. miSpecialUncatch",           !aa.is(Opcode::miSpecialUncatch));
    a.check("02. miStackDup",                 !aa.is(Opcode::miStackDup));
    a.check("03. maPush",                      aa.is(Opcode::maPush));
    a.check("04. unInc",                      !aa.is(interpreter::unInc));
    a.check("05. biAdd",                      !aa.is(interpreter::biAdd));
    a.check("06. teKeyAdd",                   !aa.is(interpreter::teKeyAdd));
    a.check("07. isJumpOrCatch",              !aa.isJumpOrCatch());
    a.check("08. isRegularJump",              !aa.isRegularJump());
    a.check("09. isLabel",                    !aa.isLabel());
    a.checkEqual("10. getExternalMajor",       aa.getExternalMajor(), Opcode::maPush);
    a.checkEqual("11. getDisassemblyTemplate", aa.getDisassemblyTemplate(), "pushloc\t%L");

    // pushint 42
    aa = make(Opcode::maPush, Opcode::sInteger, 42);
    a.checkEqual("21. getDisassemblyTemplate", aa.getDisassemblyTemplate(), "pushint\t%d");

    // Out-of-range
    a.checkEqual("31. out-of-range", make(Opcode::maPush, 222, 0).getDisassemblyTemplate(), "push?\t?");
}

/** Test binary operations. */
AFL_TEST("interpreter.Opcode:maBinary", a)
{
    // badd
    Opcode aa = make(Opcode::maBinary, interpreter::biAdd, 0);
    a.check("01. miSpecialUncatch",           !aa.is(Opcode::miSpecialUncatch));
    a.check("02. miStackDup",                 !aa.is(Opcode::miStackDup));
    a.check("03. maBinary",                    aa.is(Opcode::maBinary));
    a.check("04. maPush",                     !aa.is(Opcode::maPush));
    a.check("05. unInc",                      !aa.is(interpreter::unInc));
    a.check("06. biAdd",                       aa.is(interpreter::biAdd));
    a.check("07. biSub",                      !aa.is(interpreter::biSub));
    a.check("08. teKeyAdd",                   !aa.is(interpreter::teKeyAdd));
    a.check("09. isJumpOrCatch",              !aa.isJumpOrCatch());
    a.check("10. isRegularJump",              !aa.isRegularJump());
    a.check("11. isLabel",                    !aa.isLabel());
    a.checkEqual("12. getExternalMajor",       aa.getExternalMajor(), Opcode::maBinary);
    a.checkEqual("13. getDisassemblyTemplate", aa.getDisassemblyTemplate(), "badd");

    // Out-of-range
    a.checkEqual("21. out-of-range", make(Opcode::maBinary, 222, 0).getDisassemblyTemplate(), "b?");
}

/** Test unary operations. */
AFL_TEST("interpreter.Opcode:maUnary", a)
{
    // uval
    Opcode aa = make(Opcode::maUnary, interpreter::unVal, 0);
    a.check("01. miSpecialUncatch",           !aa.is(Opcode::miSpecialUncatch));
    a.check("02. miStackDup",                 !aa.is(Opcode::miStackDup));
    a.check("03. maPush",                     !aa.is(Opcode::maPush));
    a.check("04. maUnary",                     aa.is(Opcode::maUnary));
    a.check("05. unVal",                       aa.is(interpreter::unVal));
    a.check("06. unInc",                      !aa.is(interpreter::unInc));
    a.check("07. biSub",                      !aa.is(interpreter::biSub));
    a.check("08. teKeyAdd",                   !aa.is(interpreter::teKeyAdd));
    a.check("09. isJumpOrCatch",              !aa.isJumpOrCatch());
    a.check("10. isRegularJump",              !aa.isRegularJump());
    a.check("11. isLabel",                    !aa.isLabel());
    a.checkEqual("12. getExternalMajor",       aa.getExternalMajor(), Opcode::maUnary);
    a.checkEqual("13. getDisassemblyTemplate", aa.getDisassemblyTemplate(), "uval");

    // Out-of-range
    a.checkEqual("21. out-of-range", make(Opcode::maUnary, 222, 0).getDisassemblyTemplate(), "u?");
}

/** Test ternary operations. */
AFL_TEST("interpreter.Opcode:maTernary", a)
{
    // tkeyadd
    Opcode aa = make(Opcode::maTernary, interpreter::teKeyAdd, 0);
    a.check("01. miSpecialUncatch",           !aa.is(Opcode::miSpecialUncatch));
    a.check("02. miStackDup",                 !aa.is(Opcode::miStackDup));
    a.check("03. maPush",                     !aa.is(Opcode::maPush));
    a.check("04. maTernary",                   aa.is(Opcode::maTernary));
    a.check("05. unVal",                      !aa.is(interpreter::unVal));
    a.check("06. biSub",                      !aa.is(interpreter::biSub));
    a.check("07. teKeyAdd",                    aa.is(interpreter::teKeyAdd));
    a.check("08. isJumpOrCatch",              !aa.isJumpOrCatch());
    a.check("09. isRegularJump",              !aa.isRegularJump());
    a.check("10. isLabel",                    !aa.isLabel());
    a.checkEqual("11. getExternalMajor",       aa.getExternalMajor(), Opcode::maTernary);
    a.checkEqual("12. getDisassemblyTemplate", aa.getDisassemblyTemplate(), "tkeyadd");

    // Out-of-range
    a.checkEqual("21. out-of-range", make(Opcode::maTernary, 222, 0).getDisassemblyTemplate(), "t?");
}

/** Test jumps. */
AFL_TEST("interpreter.Opcode:maJump", a)
{
    // jep
    Opcode aa = make(Opcode::maJump, Opcode::jIfEmpty | Opcode::jPopAlways, 850);
    a.check("01. miSpecialUncatch",           !aa.is(Opcode::miSpecialUncatch));
    a.check("02. miStackDup",                 !aa.is(Opcode::miStackDup));
    a.check("03. maPush",                     !aa.is(Opcode::maPush));
    a.check("04. maJump",                      aa.is(Opcode::maJump));
    a.check("05. unVal",                      !aa.is(interpreter::unVal));
    a.check("06. biSub",                      !aa.is(interpreter::biSub));
    a.check("07. teKeyAdd",                   !aa.is(interpreter::teKeyAdd));
    a.check("08. isJumpOrCatch",               aa.isJumpOrCatch());
    a.check("09. isRegularJump",               aa.isRegularJump());
    a.check("10. isLabel",                    !aa.isLabel());
    a.checkEqual("11. getExternalMajor",       aa.getExternalMajor(), Opcode::maJump);
    a.checkEqual("12. getDisassemblyTemplate", aa.getDisassemblyTemplate(), "jep\t#%u");

    // label
    aa = make(Opcode::maJump, Opcode::jLabel | Opcode::jSymbolic, 850);
    a.check("21. isJumpOrCatch",              !aa.isJumpOrCatch());
    a.check("22. isRegularJump",              !aa.isRegularJump());
    a.check("23. isLabel",                     aa.isLabel());
    a.checkEqual("24. getExternalMajor",       aa.getExternalMajor(), Opcode::maJump);
    a.checkEqual("25. getDisassemblyTemplate", aa.getDisassemblyTemplate(), "label\tsym%u");

    // catch
    aa = make(Opcode::maJump, Opcode::jCatch, 32);
    a.check("31. isJumpOrCatch",               aa.isJumpOrCatch());
    a.check("32. isRegularJump",              !aa.isRegularJump());
    a.check("33. isLabel",                    !aa.isLabel());
    a.checkEqual("34. getExternalMajor",       aa.getExternalMajor(), Opcode::maJump);
    a.checkEqual("35. getDisassemblyTemplate", aa.getDisassemblyTemplate(), "catch\t#%u");

    // jdz
    aa = make(Opcode::maJump, Opcode::jDecZero, 55);
    a.check("41. isJumpOrCatch",               aa.isJumpOrCatch());
    a.check("42. isRegularJump",              !aa.isRegularJump());
    a.check("43. isLabel",                    !aa.isLabel());
    a.checkEqual("44. getExternalMajor",       aa.getExternalMajor(), Opcode::maJump);
    a.checkEqual("45. getDisassemblyTemplate", aa.getDisassemblyTemplate(), "jdz\t#%u");

    // jtf
    aa = make(Opcode::maJump, Opcode::jIfTrue | Opcode::jIfFalse, 55);
    a.check("51. isJumpOrCatch",               aa.isJumpOrCatch());
    a.check("52. isRegularJump",               aa.isRegularJump());
    a.check("53. isLabel",                    !aa.isLabel());
    a.checkEqual("54. getExternalMajor",       aa.getExternalMajor(), Opcode::maJump);
    a.checkEqual("55. getDisassemblyTemplate", aa.getDisassemblyTemplate(), "jtf\t#%u");

    // j
    aa = make(Opcode::maJump, Opcode::jAlways, 77);
    a.check("61. isJumpOrCatch",               aa.isJumpOrCatch());
    a.check("62. isRegularJump",               aa.isRegularJump());
    a.check("63. isLabel",                    !aa.isLabel());
    a.checkEqual("64. getExternalMajor",       aa.getExternalMajor(), Opcode::maJump);
    a.checkEqual("65. getDisassemblyTemplate", aa.getDisassemblyTemplate(), "j\t#%u");

    // jneverp
    aa = make(Opcode::maJump, Opcode::jPopAlways, 55);
    a.check("71. isJumpOrCatch",               aa.isJumpOrCatch());
    a.check("72. isRegularJump",              !aa.isRegularJump());
    a.check("73. isLabel",                    !aa.isLabel());
    a.checkEqual("74. getExternalMajor",       aa.getExternalMajor(), Opcode::maJump);
    a.checkEqual("75. getDisassemblyTemplate", aa.getDisassemblyTemplate(), "jneverp\t#%u");

    // Out-of-range
    a.checkEqual("81. out-of-range", make(Opcode::maJump, 111, 0).getDisassemblyTemplate(), "junknown\t#%u");
}

/** Test indirect operations (function call etc.). */
AFL_TEST("interpreter.Opcode:maIndirect", a)
{
    // callind 7
    Opcode aa = make(Opcode::maIndirect, Opcode::miIMCall, 7);
    a.check("01. miSpecialUncatch",           !aa.is(Opcode::miSpecialUncatch));
    a.check("02. miStackDup",                 !aa.is(Opcode::miStackDup));
    a.check("03. maIndirect",                  aa.is(Opcode::maIndirect));
    a.check("04. maPush",                     !aa.is(Opcode::maPush));
    a.check("05. unVal",                      !aa.is(interpreter::unVal));
    a.check("06. biSub",                      !aa.is(interpreter::biSub));
    a.check("07. teKeyAdd",                   !aa.is(interpreter::teKeyAdd));
    a.check("08. isJumpOrCatch",              !aa.isJumpOrCatch());
    a.check("09. isRegularJump",              !aa.isRegularJump());
    a.check("10. isLabel",                    !aa.isLabel());
    a.checkEqual("11. getExternalMajor",       aa.getExternalMajor(), Opcode::maIndirect);
    a.checkEqual("12. getDisassemblyTemplate", aa.getDisassemblyTemplate(), "callind\t%u");

    // Formatting
    a.checkEqual("21. call",  make(Opcode::maIndirect, Opcode::miIMCall,  0).getDisassemblyTemplate(), "callind\t%u");
    a.checkEqual("22. load",  make(Opcode::maIndirect, Opcode::miIMLoad,  0).getDisassemblyTemplate(), "loadind\t%u");
    a.checkEqual("23. store", make(Opcode::maIndirect, Opcode::miIMStore, 0).getDisassemblyTemplate(), "storeind\t%u");
    a.checkEqual("24. pop",   make(Opcode::maIndirect, Opcode::miIMPop,   0).getDisassemblyTemplate(), "popind\t%u");

    a.checkEqual("31. call",  make(Opcode::maIndirect, Opcode::miIMCall  + Opcode::miIMRefuseFunctions, 0).getDisassemblyTemplate(), "procind\t%u");    // regular procedure call
    a.checkEqual("32. load",  make(Opcode::maIndirect, Opcode::miIMLoad  + Opcode::miIMRefuseFunctions, 0).getDisassemblyTemplate(), "ploadind\t%u");
    a.checkEqual("33. store", make(Opcode::maIndirect, Opcode::miIMStore + Opcode::miIMRefuseFunctions, 0).getDisassemblyTemplate(), "pstoreind\t%u");
    a.checkEqual("34. pop",   make(Opcode::maIndirect, Opcode::miIMPop   + Opcode::miIMRefuseFunctions, 0).getDisassemblyTemplate(), "ppopind\t%u");

    a.checkEqual("41. call",  make(Opcode::maIndirect, Opcode::miIMCall  + Opcode::miIMRefuseProcedures, 0).getDisassemblyTemplate(), "fcallind\t%u");
    a.checkEqual("42. load",  make(Opcode::maIndirect, Opcode::miIMLoad  + Opcode::miIMRefuseProcedures, 0).getDisassemblyTemplate(), "funcind\t%u");    // regular function call
    a.checkEqual("43. store", make(Opcode::maIndirect, Opcode::miIMStore + Opcode::miIMRefuseProcedures, 0).getDisassemblyTemplate(), "fstoreind\t%u");
    a.checkEqual("44. pop",   make(Opcode::maIndirect, Opcode::miIMPop   + Opcode::miIMRefuseProcedures, 0).getDisassemblyTemplate(), "fpopind\t%u");

    // Out-of-range
    a.checkEqual("51. out-of-range", make(Opcode::maIndirect, 222, 0).getDisassemblyTemplate(), "?ind\t%u");
}

/** Test stack operation. */
AFL_TEST("interpreter.Opcode:maStack", a)
{
    // dup 5
    Opcode aa = make(Opcode::maStack, Opcode::miStackDup, 5);
    a.check("01. miSpecialUncatch",           !aa.is(Opcode::miSpecialUncatch));
    a.check("02. miStackDup",                  aa.is(Opcode::miStackDup));
    a.check("03. miStackDrop",                !aa.is(Opcode::miStackDrop));
    a.check("04. maPush",                     !aa.is(Opcode::maPush));
    a.check("05. unVal",                      !aa.is(interpreter::unVal));
    a.check("06. biSub",                      !aa.is(interpreter::biSub));
    a.check("07. teKeyAdd",                   !aa.is(interpreter::teKeyAdd));
    a.check("08. isJumpOrCatch",              !aa.isJumpOrCatch());
    a.check("09. isRegularJump",              !aa.isRegularJump());
    a.check("10. isLabel",                    !aa.isLabel());
    a.checkEqual("11. getExternalMajor",       aa.getExternalMajor(), Opcode::maStack);
    a.checkEqual("12. getDisassemblyTemplate", aa.getDisassemblyTemplate(), "dup\t%u");

    // Out-of-range
    a.checkEqual("21. out-of-range", make(Opcode::maStack, 222, 0).getDisassemblyTemplate(), "?\t%u");
}

/** Test pop operations. */
AFL_TEST("interpreter.Opcode:maPop", a)
{
    // popvar 9
    Opcode aa = make(Opcode::maPop, Opcode::sNamedVariable, 9);
    a.check("01. miSpecialUncatch",           !aa.is(Opcode::miSpecialUncatch));
    a.check("02. miStackDup",                 !aa.is(Opcode::miStackDup));
    a.check("03. maPop",                       aa.is(Opcode::maPop));
    a.check("04. maPush",                     !aa.is(Opcode::maPush));
    a.check("05. unInc",                      !aa.is(interpreter::unInc));
    a.check("06. biAdd",                      !aa.is(interpreter::biAdd));
    a.check("07. teKeyAdd",                   !aa.is(interpreter::teKeyAdd));
    a.check("08. isJumpOrCatch",              !aa.isJumpOrCatch());
    a.check("09. isRegularJump",              !aa.isRegularJump());
    a.check("10. isLabel",                    !aa.isLabel());
    a.checkEqual("11. getExternalMajor",       aa.getExternalMajor(), Opcode::maPop);
    a.checkEqual("12. getDisassemblyTemplate", aa.getDisassemblyTemplate(), "popvar\t%n");

    // Out-of-range
    a.checkEqual("21. out-of-range", make(Opcode::maPop, 222, 0).getDisassemblyTemplate(), "pop?\t?");
}

/** Test store operations. */
AFL_TEST("interpreter.Opcode:maStore", a)
{
    // storetop 8
    Opcode aa = make(Opcode::maStore, Opcode::sStatic, 8);
    a.check("01. miSpecialUncatch",           !aa.is(Opcode::miSpecialUncatch));
    a.check("02. miStackDup",                 !aa.is(Opcode::miStackDup));
    a.check("03. maStore",                     aa.is(Opcode::maStore));
    a.check("04. maPush",                     !aa.is(Opcode::maPush));
    a.check("05. unInc",                      !aa.is(interpreter::unInc));
    a.check("06. biAdd",                      !aa.is(interpreter::biAdd));
    a.check("07. teKeyAdd",                   !aa.is(interpreter::teKeyAdd));
    a.check("08. isJumpOrCatch",              !aa.isJumpOrCatch());
    a.check("09. isRegularJump",              !aa.isRegularJump());
    a.check("10. isLabel",                    !aa.isLabel());
    a.checkEqual("11. getExternalMajor",       aa.getExternalMajor(), Opcode::maStore);
    a.checkEqual("12. getDisassemblyTemplate", aa.getDisassemblyTemplate(), "storetop\t%T");

    // Out-of-range
    a.checkEqual("21. out-of-range", make(Opcode::maStore, 222, 0).getDisassemblyTemplate(), "store?\t?");
}

/** Test memory reference. */
AFL_TEST("interpreter.Opcode:maMemref", a)
{
    // loadmem 7
    Opcode aa = make(Opcode::maMemref, Opcode::miIMLoad, 7);
    a.check("01. miSpecialUncatch",           !aa.is(Opcode::miSpecialUncatch));
    a.check("02. miStackDup",                 !aa.is(Opcode::miStackDup));
    a.check("03. maMemref",                    aa.is(Opcode::maMemref));
    a.check("04. maPush",                     !aa.is(Opcode::maPush));
    a.check("05. unVal",                      !aa.is(interpreter::unVal));
    a.check("06. biSub",                      !aa.is(interpreter::biSub));
    a.check("07. teKeyAdd",                   !aa.is(interpreter::teKeyAdd));
    a.check("08. isJumpOrCatch",              !aa.isJumpOrCatch());
    a.check("09. isRegularJump",              !aa.isRegularJump());
    a.check("10. isLabel",                    !aa.isLabel());
    a.checkEqual("11. getExternalMajor",       aa.getExternalMajor(), Opcode::maMemref);
    a.checkEqual("12. getDisassemblyTemplate", aa.getDisassemblyTemplate(), "loadmem\t%n");

    // Formatting
    a.checkEqual("21. call",  make(Opcode::maMemref, Opcode::miIMCall,  0).getDisassemblyTemplate(), "callmem\t%n");  // not normally used
    a.checkEqual("22. load",  make(Opcode::maMemref, Opcode::miIMLoad,  0).getDisassemblyTemplate(), "loadmem\t%n");  // regular load
    a.checkEqual("23. store", make(Opcode::maMemref, Opcode::miIMStore, 0).getDisassemblyTemplate(), "storemem\t%n"); // regular store
    a.checkEqual("24. pop",   make(Opcode::maMemref, Opcode::miIMPop,   0).getDisassemblyTemplate(), "popmem\t%n");   // regular pop

    // Out-of-range
    a.checkEqual("31. out-of-range", make(Opcode::maMemref, 222, 0).getDisassemblyTemplate(), "?mem\t%n");
}

/** Test "dim" operations. */
AFL_TEST("interpreter.Opcode:maDim", a)
{
    // storetop 8
    Opcode aa = make(Opcode::maDim, Opcode::sShared, 8);
    a.check("01. miSpecialUncatch",           !aa.is(Opcode::miSpecialUncatch));
    a.check("02. miStackDup",                 !aa.is(Opcode::miStackDup));
    a.check("03. maDim",                       aa.is(Opcode::maDim));
    a.check("04. maPush",                     !aa.is(Opcode::maPush));
    a.check("05. unInc",                      !aa.is(interpreter::unInc));
    a.check("06. biAdd",                      !aa.is(interpreter::biAdd));
    a.check("07. teKeyAdd",                   !aa.is(interpreter::teKeyAdd));
    a.check("08. isJumpOrCatch",              !aa.isJumpOrCatch());
    a.check("09. isRegularJump",              !aa.isRegularJump());
    a.check("10. isLabel",                    !aa.isLabel());
    a.checkEqual("11. getExternalMajor",       aa.getExternalMajor(), Opcode::maDim);
    a.checkEqual("12. getDisassemblyTemplate", aa.getDisassemblyTemplate(), "dimglob\t%n");

    // Out-of-range
    a.checkEqual("21. out-of-range", make(Opcode::maDim, 222, 0).getDisassemblyTemplate(), "dim?\t%n");
}

/** Test specials. */
AFL_TEST("interpreter.Opcode:maSpecial", a)
{
    // sfirstindex
    Opcode aa = make(Opcode::maSpecial, Opcode::miSpecialFirstIndex, 8);
    a.check("01. miSpecialFirstIndex",         aa.is(Opcode::miSpecialFirstIndex));
    a.check("02. miSpecialUncatch",           !aa.is(Opcode::miSpecialUncatch));
    a.check("03. miStackDup",                 !aa.is(Opcode::miStackDup));
    a.check("04. maPush",                     !aa.is(Opcode::maPush));
    a.check("05. unInc",                      !aa.is(interpreter::unInc));
    a.check("06. biAdd",                      !aa.is(interpreter::biAdd));
    a.check("07. teKeyAdd",                   !aa.is(interpreter::teKeyAdd));
    a.check("08. isJumpOrCatch",              !aa.isJumpOrCatch());
    a.check("09. isRegularJump",              !aa.isRegularJump());
    a.check("10. isLabel",                    !aa.isLabel());
    a.checkEqual("11. getExternalMajor",       aa.getExternalMajor(), Opcode::maSpecial);
    a.checkEqual("12. getDisassemblyTemplate", aa.getDisassemblyTemplate(), "sfirstindex");

    // sdefsub 42
    aa = make(Opcode::maSpecial, Opcode::miSpecialDefSub, 42);
    a.check("21. miSpecialDefSub",  aa.is(Opcode::miSpecialDefSub));
    a.checkEqual("22. getDisassemblyTemplate", aa.getDisassemblyTemplate(), "sdefsub\t%n");

    // Out-of-range
    a.checkEqual("31. out-of-range", make(Opcode::maSpecial, 222, 0).getDisassemblyTemplate(), "s?");
}

/** Test fused-unary operation. */
AFL_TEST("interpreter.Opcode:maFusedUnary", a)
{
    // pushlit(u) [=first part of fused push+unary]
    Opcode aa = make(Opcode::maFusedUnary, Opcode::sLiteral, 0);
    a.check("01. miSpecialUncatch",           !aa.is(Opcode::miSpecialUncatch));
    a.check("02. miStackDup",                 !aa.is(Opcode::miStackDup));
    a.check("03. maPush",                     !aa.is(Opcode::maPush));
    a.check("04. maUnary",                    !aa.is(Opcode::maUnary));
    a.check("05. maFusedUnary",                aa.is(Opcode::maFusedUnary));
    a.check("06. unInc",                      !aa.is(interpreter::unInc));
    a.check("07. biSub",                      !aa.is(interpreter::biSub));
    a.check("08. teKeyAdd",                   !aa.is(interpreter::teKeyAdd));
    a.check("09. isJumpOrCatch",              !aa.isJumpOrCatch());
    a.check("10. isRegularJump",              !aa.isRegularJump());
    a.check("11. isLabel",                    !aa.isLabel());
    a.checkEqual("12. getExternalMajor",       aa.getExternalMajor(), Opcode::maPush);
    a.checkEqual("13. getDisassemblyTemplate", aa.getDisassemblyTemplate(), "pushlit(u)\t%l");

    // Out-of-range
    a.checkEqual("21. out-of-range", make(Opcode::maFusedUnary, 222, 0).getDisassemblyTemplate(), "push?(u)\t?");
}

/** Test fused-binary operation. */
AFL_TEST("interpreter.Opcode:maFusedBinary", a)
{
    // pushlit(b) [=first part of fused push+binary]
    Opcode aa = make(Opcode::maFusedBinary, Opcode::sStatic, 0);
    a.check("01. miSpecialUncatch",           !aa.is(Opcode::miSpecialUncatch));
    a.check("02. miStackDup",                 !aa.is(Opcode::miStackDup));
    a.check("03. maPush",                     !aa.is(Opcode::maPush));
    a.check("04. maUnary",                    !aa.is(Opcode::maUnary));
    a.check("05. maFusedBinary",               aa.is(Opcode::maFusedBinary));
    a.check("06. unInc",                      !aa.is(interpreter::unInc));
    a.check("07. biSub",                      !aa.is(interpreter::biSub));
    a.check("08. teKeyAdd",                   !aa.is(interpreter::teKeyAdd));
    a.check("09. isJumpOrCatch",              !aa.isJumpOrCatch());
    a.check("10. isRegularJump",              !aa.isRegularJump());
    a.check("11. isLabel",                    !aa.isLabel());
    a.checkEqual("12. getExternalMajor",       aa.getExternalMajor(), Opcode::maPush);
    a.checkEqual("13. getDisassemblyTemplate", aa.getDisassemblyTemplate(), "pushtop(b)\t%T");

    // Out-of-range
    a.checkEqual("21. out-of-range", make(Opcode::maFusedBinary, 222, 0).getDisassemblyTemplate(), "push?(b)\t?");
}

/** Test fused comparison. */
AFL_TEST("interpreter.Opcode:maFusedComparison", a)
{
    // bcmplt(j) [=first part of fused compare+jump]
    Opcode aa = make(Opcode::maFusedComparison, interpreter::biCompareLT, 0);
    a.check("01. miSpecialUncatch",           !aa.is(Opcode::miSpecialUncatch));
    a.check("02. miStackDup",                 !aa.is(Opcode::miStackDup));
    a.check("03. maBinary",                   !aa.is(Opcode::maBinary));
    a.check("04. maPush",                     !aa.is(Opcode::maPush));
    a.check("05. maFusedComparison",           aa.is(Opcode::maFusedComparison));
    a.check("06. unInc",                      !aa.is(interpreter::unInc));
    a.check("07. biSub",                      !aa.is(interpreter::biSub));
    a.check("08. teKeyAdd",                   !aa.is(interpreter::teKeyAdd));
    a.check("09. isJumpOrCatch",              !aa.isJumpOrCatch());
    a.check("10. isRegularJump",              !aa.isRegularJump());
    a.check("11. isLabel",                    !aa.isLabel());
    a.checkEqual("12. getExternalMajor",       aa.getExternalMajor(), Opcode::maBinary);
    a.checkEqual("13. getDisassemblyTemplate", aa.getDisassemblyTemplate(), "bcmplt(j)");

    // Out-of-range
    a.checkEqual("21. out-of-range", make(Opcode::maFusedComparison, 222, 0).getDisassemblyTemplate(), "b?(j)");
}

/** Test fused comparison (2). */
AFL_TEST("interpreter.Opcode:maFusedComparison2", a)
{
    // pushbool(b,j) [=first part of fused push+binary+jump]
    Opcode aa = make(Opcode::maFusedComparison2, Opcode::sBoolean, 0);
    a.check("01. miSpecialUncatch",           !aa.is(Opcode::miSpecialUncatch));
    a.check("02. miStackDup",                 !aa.is(Opcode::miStackDup));
    a.check("03. maPush",                     !aa.is(Opcode::maPush));
    a.check("04. maUnary",                    !aa.is(Opcode::maUnary));
    a.check("05. maFusedComparison2",          aa.is(Opcode::maFusedComparison2));
    a.check("06. unInc",                      !aa.is(interpreter::unInc));
    a.check("07. biSub",                      !aa.is(interpreter::biSub));
    a.check("08. teKeyAdd",                   !aa.is(interpreter::teKeyAdd));
    a.check("09. isJumpOrCatch",              !aa.isJumpOrCatch());
    a.check("10. isRegularJump",              !aa.isRegularJump());
    a.check("11. isLabel",                    !aa.isLabel());
    a.checkEqual("12. getExternalMajor",       aa.getExternalMajor(), Opcode::maPush);
    a.checkEqual("13. getDisassemblyTemplate", aa.getDisassemblyTemplate(), "pushbool(b,j)\t%d");

    // Out-of-range
    a.checkEqual("21. out-of-range", make(Opcode::maFusedComparison2, 222, 0).getDisassemblyTemplate(), "push?(b,j)\t?");
}

/** Test in-place unary operation. */
AFL_TEST("interpreter.Opcode:maInplaceUnary", a)
{
    // pushloc(xu) [=first part of fused in-place push+unary]
    Opcode aa = make(Opcode::maInplaceUnary, Opcode::sLocal, 3);
    a.check("01. miSpecialUncatch",           !aa.is(Opcode::miSpecialUncatch));
    a.check("02. miStackDup",                 !aa.is(Opcode::miStackDup));
    a.check("03. maPush",                     !aa.is(Opcode::maPush));
    a.check("04. maUnary",                    !aa.is(Opcode::maUnary));
    a.check("05. maInplaceUnary",              aa.is(Opcode::maInplaceUnary));
    a.check("06. unInc",                      !aa.is(interpreter::unInc));
    a.check("07. biSub",                      !aa.is(interpreter::biSub));
    a.check("08. teKeyAdd",                   !aa.is(interpreter::teKeyAdd));
    a.check("09. isJumpOrCatch",              !aa.isJumpOrCatch());
    a.check("10. isRegularJump",              !aa.isRegularJump());
    a.check("11. isLabel",                    !aa.isLabel());
    a.checkEqual("12. getExternalMajor",       aa.getExternalMajor(), Opcode::maPush);
    a.checkEqual("13. getDisassemblyTemplate", aa.getDisassemblyTemplate(), "pushloc(xu)\t%L");

    // Out-of-range
    a.checkEqual("21. out-of-range", make(Opcode::maInplaceUnary, 222, 0).getDisassemblyTemplate(), "push?(xu)\t?");
}

/** Test unknowns. */
AFL_TEST("interpreter.Opcode:unknown", a)
{
    // pushloc(xu) [=first part of fused in-place push+unary]
    Opcode aa = make(77, 88, 99);
    a.check("01. miSpecialUncatch",           !aa.is(Opcode::miSpecialUncatch));
    a.check("02. miStackDup",                 !aa.is(Opcode::miStackDup));
    a.check("03. maPush",                     !aa.is(Opcode::maPush));
    a.check("04. maUnary",                    !aa.is(Opcode::maUnary));
    a.check("05. unInc",                      !aa.is(interpreter::unInc));
    a.check("06. biSub",                      !aa.is(interpreter::biSub));
    a.check("07. teKeyAdd",                   !aa.is(interpreter::teKeyAdd));
    a.check("08. isJumpOrCatch",              !aa.isJumpOrCatch());
    a.check("09. isRegularJump",              !aa.isRegularJump());
    a.check("10. isLabel",                    !aa.isLabel());
    a.checkEqual("11. getExternalMajor",       aa.getExternalMajor(), 77);
    a.checkEqual("12. getDisassemblyTemplate", aa.getDisassemblyTemplate(), "unknown?\t%u");
}
