/**
  *  \file u/t_interpreter_opcode.cpp
  *  \brief Test for interpreter::Opcode
  */

#include "interpreter/opcode.hpp"

#include "t_interpreter.hpp"

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
void
TestInterpreterOpcode::testPush()
{
    // pushloc 9
    Opcode a = make(Opcode::maPush, Opcode::sLocal, 9);
    TS_ASSERT(!a.is(Opcode::miSpecialUncatch));
    TS_ASSERT(!a.is(Opcode::miStackDup));
    TS_ASSERT( a.is(Opcode::maPush));
    TS_ASSERT(!a.is(interpreter::unInc));
    TS_ASSERT(!a.is(interpreter::biAdd));
    TS_ASSERT(!a.is(interpreter::teKeyAdd));
    TS_ASSERT(!a.isJumpOrCatch());
    TS_ASSERT(!a.isRegularJump());
    TS_ASSERT(!a.isLabel());
    TS_ASSERT_EQUALS(a.getExternalMajor(), Opcode::maPush);
    TS_ASSERT_EQUALS(a.getDisassemblyTemplate(), "pushloc\t%L");

    // pushint 42
    a = make(Opcode::maPush, Opcode::sInteger, 42);
    TS_ASSERT_EQUALS(a.getDisassemblyTemplate(), "pushint\t%d");

    // Out-of-range
    TS_ASSERT_EQUALS(make(Opcode::maPush, 222, 0).getDisassemblyTemplate(), "push?\t?");
}

/** Test binary operations. */
void
TestInterpreterOpcode::testBinary()
{
    // badd
    Opcode a = make(Opcode::maBinary, interpreter::biAdd, 0);
    TS_ASSERT(!a.is(Opcode::miSpecialUncatch));
    TS_ASSERT(!a.is(Opcode::miStackDup));
    TS_ASSERT( a.is(Opcode::maBinary));
    TS_ASSERT(!a.is(Opcode::maPush));
    TS_ASSERT(!a.is(interpreter::unInc));
    TS_ASSERT( a.is(interpreter::biAdd));
    TS_ASSERT(!a.is(interpreter::biSub));
    TS_ASSERT(!a.is(interpreter::teKeyAdd));
    TS_ASSERT(!a.isJumpOrCatch());
    TS_ASSERT(!a.isRegularJump());
    TS_ASSERT(!a.isLabel());
    TS_ASSERT_EQUALS(a.getExternalMajor(), Opcode::maBinary);
    TS_ASSERT_EQUALS(a.getDisassemblyTemplate(), "badd");

    // Out-of-range
    TS_ASSERT_EQUALS(make(Opcode::maBinary, 222, 0).getDisassemblyTemplate(), "b?");
}

/** Test unary operations. */
void
TestInterpreterOpcode::testUnary()
{
    // uval
    Opcode a = make(Opcode::maUnary, interpreter::unVal, 0);
    TS_ASSERT(!a.is(Opcode::miSpecialUncatch));
    TS_ASSERT(!a.is(Opcode::miStackDup));
    TS_ASSERT(!a.is(Opcode::maPush));
    TS_ASSERT( a.is(Opcode::maUnary));
    TS_ASSERT( a.is(interpreter::unVal));
    TS_ASSERT(!a.is(interpreter::unInc));
    TS_ASSERT(!a.is(interpreter::biSub));
    TS_ASSERT(!a.is(interpreter::teKeyAdd));
    TS_ASSERT(!a.isJumpOrCatch());
    TS_ASSERT(!a.isRegularJump());
    TS_ASSERT(!a.isLabel());
    TS_ASSERT_EQUALS(a.getExternalMajor(), Opcode::maUnary);
    TS_ASSERT_EQUALS(a.getDisassemblyTemplate(), "uval");

    // Out-of-range
    TS_ASSERT_EQUALS(make(Opcode::maUnary, 222, 0).getDisassemblyTemplate(), "u?");
}

/** Test ternary operations. */
void
TestInterpreterOpcode::testTernary()
{
    // tkeyadd
    Opcode a = make(Opcode::maTernary, interpreter::teKeyAdd, 0);
    TS_ASSERT(!a.is(Opcode::miSpecialUncatch));
    TS_ASSERT(!a.is(Opcode::miStackDup));
    TS_ASSERT(!a.is(Opcode::maPush));
    TS_ASSERT( a.is(Opcode::maTernary));
    TS_ASSERT(!a.is(interpreter::unVal));
    TS_ASSERT(!a.is(interpreter::biSub));
    TS_ASSERT( a.is(interpreter::teKeyAdd));
    TS_ASSERT(!a.isJumpOrCatch());
    TS_ASSERT(!a.isRegularJump());
    TS_ASSERT(!a.isLabel());
    TS_ASSERT_EQUALS(a.getExternalMajor(), Opcode::maTernary);
    TS_ASSERT_EQUALS(a.getDisassemblyTemplate(), "tkeyadd");

    // Out-of-range
    TS_ASSERT_EQUALS(make(Opcode::maTernary, 222, 0).getDisassemblyTemplate(), "t?");
}

/** Test jumps. */
void
TestInterpreterOpcode::testJump()
{
    // jep
    Opcode a = make(Opcode::maJump, Opcode::jIfEmpty | Opcode::jPopAlways, 850);
    TS_ASSERT(!a.is(Opcode::miSpecialUncatch));
    TS_ASSERT(!a.is(Opcode::miStackDup));
    TS_ASSERT(!a.is(Opcode::maPush));
    TS_ASSERT( a.is(Opcode::maJump));
    TS_ASSERT(!a.is(interpreter::unVal));
    TS_ASSERT(!a.is(interpreter::biSub));
    TS_ASSERT(!a.is(interpreter::teKeyAdd));
    TS_ASSERT( a.isJumpOrCatch());
    TS_ASSERT( a.isRegularJump());
    TS_ASSERT(!a.isLabel());
    TS_ASSERT_EQUALS(a.getExternalMajor(), Opcode::maJump);
    TS_ASSERT_EQUALS(a.getDisassemblyTemplate(), "jep\t#%u");

    // label
    a = make(Opcode::maJump, Opcode::jLabel | Opcode::jSymbolic, 850);
    TS_ASSERT(!a.isJumpOrCatch());
    TS_ASSERT(!a.isRegularJump());
    TS_ASSERT( a.isLabel());
    TS_ASSERT_EQUALS(a.getExternalMajor(), Opcode::maJump);
    TS_ASSERT_EQUALS(a.getDisassemblyTemplate(), "label\tsym%u");

    // catch
    a = make(Opcode::maJump, Opcode::jCatch, 32);
    TS_ASSERT( a.isJumpOrCatch());
    TS_ASSERT(!a.isRegularJump());
    TS_ASSERT(!a.isLabel());
    TS_ASSERT_EQUALS(a.getExternalMajor(), Opcode::maJump);
    TS_ASSERT_EQUALS(a.getDisassemblyTemplate(), "catch\t#%u");

    // jdz
    a = make(Opcode::maJump, Opcode::jDecZero, 55);
    TS_ASSERT( a.isJumpOrCatch());
    TS_ASSERT(!a.isRegularJump());
    TS_ASSERT(!a.isLabel());
    TS_ASSERT_EQUALS(a.getExternalMajor(), Opcode::maJump);
    TS_ASSERT_EQUALS(a.getDisassemblyTemplate(), "jdz\t#%u");

    // jtf
    a = make(Opcode::maJump, Opcode::jIfTrue | Opcode::jIfFalse, 55);
    TS_ASSERT( a.isJumpOrCatch());
    TS_ASSERT( a.isRegularJump());
    TS_ASSERT(!a.isLabel());
    TS_ASSERT_EQUALS(a.getExternalMajor(), Opcode::maJump);
    TS_ASSERT_EQUALS(a.getDisassemblyTemplate(), "jtf\t#%u");

    // j
    a = make(Opcode::maJump, Opcode::jAlways, 77);
    TS_ASSERT( a.isJumpOrCatch());
    TS_ASSERT( a.isRegularJump());
    TS_ASSERT(!a.isLabel());
    TS_ASSERT_EQUALS(a.getExternalMajor(), Opcode::maJump);
    TS_ASSERT_EQUALS(a.getDisassemblyTemplate(), "j\t#%u");

    // jneverp
    a = make(Opcode::maJump, Opcode::jPopAlways, 55);
    TS_ASSERT( a.isJumpOrCatch());
    TS_ASSERT(!a.isRegularJump());
    TS_ASSERT(!a.isLabel());
    TS_ASSERT_EQUALS(a.getExternalMajor(), Opcode::maJump);
    TS_ASSERT_EQUALS(a.getDisassemblyTemplate(), "jneverp\t#%u");

    // Out-of-range
    TS_ASSERT_EQUALS(make(Opcode::maJump, 111, 0).getDisassemblyTemplate(), "junknown\t#%u");
}

/** Test indirect operations (function call etc.). */
void
TestInterpreterOpcode::testIndirect()
{
    // callind 7
    Opcode a = make(Opcode::maIndirect, Opcode::miIMCall, 7);
    TS_ASSERT(!a.is(Opcode::miSpecialUncatch));
    TS_ASSERT(!a.is(Opcode::miStackDup));
    TS_ASSERT( a.is(Opcode::maIndirect));
    TS_ASSERT(!a.is(Opcode::maPush));
    TS_ASSERT(!a.is(interpreter::unVal));
    TS_ASSERT(!a.is(interpreter::biSub));
    TS_ASSERT(!a.is(interpreter::teKeyAdd));
    TS_ASSERT(!a.isJumpOrCatch());
    TS_ASSERT(!a.isRegularJump());
    TS_ASSERT(!a.isLabel());
    TS_ASSERT_EQUALS(a.getExternalMajor(), Opcode::maIndirect);
    TS_ASSERT_EQUALS(a.getDisassemblyTemplate(), "callind\t%u");

    // Formatting
    TS_ASSERT_EQUALS(make(Opcode::maIndirect, Opcode::miIMCall,  0).getDisassemblyTemplate(), "callind\t%u");
    TS_ASSERT_EQUALS(make(Opcode::maIndirect, Opcode::miIMLoad,  0).getDisassemblyTemplate(), "loadind\t%u");
    TS_ASSERT_EQUALS(make(Opcode::maIndirect, Opcode::miIMStore, 0).getDisassemblyTemplate(), "storeind\t%u");
    TS_ASSERT_EQUALS(make(Opcode::maIndirect, Opcode::miIMPop,   0).getDisassemblyTemplate(), "popind\t%u");

    TS_ASSERT_EQUALS(make(Opcode::maIndirect, Opcode::miIMCall  + Opcode::miIMRefuseFunctions, 0).getDisassemblyTemplate(), "procind\t%u");    // regular procedure call
    TS_ASSERT_EQUALS(make(Opcode::maIndirect, Opcode::miIMLoad  + Opcode::miIMRefuseFunctions, 0).getDisassemblyTemplate(), "ploadind\t%u");
    TS_ASSERT_EQUALS(make(Opcode::maIndirect, Opcode::miIMStore + Opcode::miIMRefuseFunctions, 0).getDisassemblyTemplate(), "pstoreind\t%u");
    TS_ASSERT_EQUALS(make(Opcode::maIndirect, Opcode::miIMPop   + Opcode::miIMRefuseFunctions, 0).getDisassemblyTemplate(), "ppopind\t%u");

    TS_ASSERT_EQUALS(make(Opcode::maIndirect, Opcode::miIMCall  + Opcode::miIMRefuseProcedures, 0).getDisassemblyTemplate(), "fcallind\t%u");
    TS_ASSERT_EQUALS(make(Opcode::maIndirect, Opcode::miIMLoad  + Opcode::miIMRefuseProcedures, 0).getDisassemblyTemplate(), "funcind\t%u");    // regular function call
    TS_ASSERT_EQUALS(make(Opcode::maIndirect, Opcode::miIMStore + Opcode::miIMRefuseProcedures, 0).getDisassemblyTemplate(), "fstoreind\t%u");
    TS_ASSERT_EQUALS(make(Opcode::maIndirect, Opcode::miIMPop   + Opcode::miIMRefuseProcedures, 0).getDisassemblyTemplate(), "fpopind\t%u");

    // Out-of-range
    TS_ASSERT_EQUALS(make(Opcode::maIndirect, 222, 0).getDisassemblyTemplate(), "?ind\t%u");
}

/** Test stack operation. */
void
TestInterpreterOpcode::testStack()
{
    // dup 5
    Opcode a = make(Opcode::maStack, Opcode::miStackDup, 5);
    TS_ASSERT(!a.is(Opcode::miSpecialUncatch));
    TS_ASSERT( a.is(Opcode::miStackDup));
    TS_ASSERT(!a.is(Opcode::miStackDrop));
    TS_ASSERT(!a.is(Opcode::maPush));
    TS_ASSERT(!a.is(interpreter::unVal));
    TS_ASSERT(!a.is(interpreter::biSub));
    TS_ASSERT(!a.is(interpreter::teKeyAdd));
    TS_ASSERT(!a.isJumpOrCatch());
    TS_ASSERT(!a.isRegularJump());
    TS_ASSERT(!a.isLabel());
    TS_ASSERT_EQUALS(a.getExternalMajor(), Opcode::maStack);
    TS_ASSERT_EQUALS(a.getDisassemblyTemplate(), "dup\t%u");
    
    // Out-of-range
    TS_ASSERT_EQUALS(make(Opcode::maStack, 222, 0).getDisassemblyTemplate(), "?\t%u");
}

/** Test pop operations. */
void
TestInterpreterOpcode::testPop()
{
    // popvar 9
    Opcode a = make(Opcode::maPop, Opcode::sNamedVariable, 9);
    TS_ASSERT(!a.is(Opcode::miSpecialUncatch));
    TS_ASSERT(!a.is(Opcode::miStackDup));
    TS_ASSERT( a.is(Opcode::maPop));
    TS_ASSERT(!a.is(Opcode::maPush));
    TS_ASSERT(!a.is(interpreter::unInc));
    TS_ASSERT(!a.is(interpreter::biAdd));
    TS_ASSERT(!a.is(interpreter::teKeyAdd));
    TS_ASSERT(!a.isJumpOrCatch());
    TS_ASSERT(!a.isRegularJump());
    TS_ASSERT(!a.isLabel());
    TS_ASSERT_EQUALS(a.getExternalMajor(), Opcode::maPop);
    TS_ASSERT_EQUALS(a.getDisassemblyTemplate(), "popvar\t%n");

    // Out-of-range
    TS_ASSERT_EQUALS(make(Opcode::maPop, 222, 0).getDisassemblyTemplate(), "pop?\t?");
}

/** Test store operations. */
void
TestInterpreterOpcode::testStore()
{
    // storetop 8
    Opcode a = make(Opcode::maStore, Opcode::sStatic, 8);
    TS_ASSERT(!a.is(Opcode::miSpecialUncatch));
    TS_ASSERT(!a.is(Opcode::miStackDup));
    TS_ASSERT( a.is(Opcode::maStore));
    TS_ASSERT(!a.is(Opcode::maPush));
    TS_ASSERT(!a.is(interpreter::unInc));
    TS_ASSERT(!a.is(interpreter::biAdd));
    TS_ASSERT(!a.is(interpreter::teKeyAdd));
    TS_ASSERT(!a.isJumpOrCatch());
    TS_ASSERT(!a.isRegularJump());
    TS_ASSERT(!a.isLabel());
    TS_ASSERT_EQUALS(a.getExternalMajor(), Opcode::maStore);
    TS_ASSERT_EQUALS(a.getDisassemblyTemplate(), "storetop\t%T");

    // Out-of-range
    TS_ASSERT_EQUALS(make(Opcode::maStore, 222, 0).getDisassemblyTemplate(), "store?\t?");
}

/** Test memory reference. */
void
TestInterpreterOpcode::testMemref()
{
    // loadmem 7
    Opcode a = make(Opcode::maMemref, Opcode::miIMLoad, 7);
    TS_ASSERT(!a.is(Opcode::miSpecialUncatch));
    TS_ASSERT(!a.is(Opcode::miStackDup));
    TS_ASSERT( a.is(Opcode::maMemref));
    TS_ASSERT(!a.is(Opcode::maPush));
    TS_ASSERT(!a.is(interpreter::unVal));
    TS_ASSERT(!a.is(interpreter::biSub));
    TS_ASSERT(!a.is(interpreter::teKeyAdd));
    TS_ASSERT(!a.isJumpOrCatch());
    TS_ASSERT(!a.isRegularJump());
    TS_ASSERT(!a.isLabel());
    TS_ASSERT_EQUALS(a.getExternalMajor(), Opcode::maMemref);
    TS_ASSERT_EQUALS(a.getDisassemblyTemplate(), "loadmem\t%n");

    // Formatting
    TS_ASSERT_EQUALS(make(Opcode::maMemref, Opcode::miIMCall,  0).getDisassemblyTemplate(), "callmem\t%n");  // not normally used
    TS_ASSERT_EQUALS(make(Opcode::maMemref, Opcode::miIMLoad,  0).getDisassemblyTemplate(), "loadmem\t%n");  // regular load
    TS_ASSERT_EQUALS(make(Opcode::maMemref, Opcode::miIMStore, 0).getDisassemblyTemplate(), "storemem\t%n"); // regular store
    TS_ASSERT_EQUALS(make(Opcode::maMemref, Opcode::miIMPop,   0).getDisassemblyTemplate(), "popmem\t%n");   // regular pop

    // Out-of-range
    TS_ASSERT_EQUALS(make(Opcode::maMemref, 222, 0).getDisassemblyTemplate(), "?mem\t%n");
}

/** Test "dim" operations. */
void
TestInterpreterOpcode::testDim()
{
    // storetop 8
    Opcode a = make(Opcode::maDim, Opcode::sShared, 8);
    TS_ASSERT(!a.is(Opcode::miSpecialUncatch));
    TS_ASSERT(!a.is(Opcode::miStackDup));
    TS_ASSERT( a.is(Opcode::maDim));
    TS_ASSERT(!a.is(Opcode::maPush));
    TS_ASSERT(!a.is(interpreter::unInc));
    TS_ASSERT(!a.is(interpreter::biAdd));
    TS_ASSERT(!a.is(interpreter::teKeyAdd));
    TS_ASSERT(!a.isJumpOrCatch());
    TS_ASSERT(!a.isRegularJump());
    TS_ASSERT(!a.isLabel());
    TS_ASSERT_EQUALS(a.getExternalMajor(), Opcode::maDim);
    TS_ASSERT_EQUALS(a.getDisassemblyTemplate(), "dimglob\t%n");

    // Out-of-range
    TS_ASSERT_EQUALS(make(Opcode::maDim, 222, 0).getDisassemblyTemplate(), "dim?\t%n");
}

/** Test specials. */
void
TestInterpreterOpcode::testSpecial()
{
    // sfirstindex
    Opcode a = make(Opcode::maSpecial, Opcode::miSpecialFirstIndex, 8);
    TS_ASSERT( a.is(Opcode::miSpecialFirstIndex));
    TS_ASSERT(!a.is(Opcode::miSpecialUncatch));
    TS_ASSERT(!a.is(Opcode::miStackDup));
    TS_ASSERT(!a.is(Opcode::maPush));
    TS_ASSERT(!a.is(interpreter::unInc));
    TS_ASSERT(!a.is(interpreter::biAdd));
    TS_ASSERT(!a.is(interpreter::teKeyAdd));
    TS_ASSERT(!a.isJumpOrCatch());
    TS_ASSERT(!a.isRegularJump());
    TS_ASSERT(!a.isLabel());
    TS_ASSERT_EQUALS(a.getExternalMajor(), Opcode::maSpecial);
    TS_ASSERT_EQUALS(a.getDisassemblyTemplate(), "sfirstindex");

    // sdefsub 42
    a = make(Opcode::maSpecial, Opcode::miSpecialDefSub, 42);
    TS_ASSERT( a.is(Opcode::miSpecialDefSub));
    TS_ASSERT_EQUALS(a.getDisassemblyTemplate(), "sdefsub\t%n");

    // Out-of-range
    TS_ASSERT_EQUALS(make(Opcode::maSpecial, 222, 0).getDisassemblyTemplate(), "s?");
}

/** Test fused-unary operation. */
void
TestInterpreterOpcode::testFusedUnary()
{
    // pushlit(u) [=first part of fused push+unary]
    Opcode a = make(Opcode::maFusedUnary, Opcode::sLiteral, 0);
    TS_ASSERT(!a.is(Opcode::miSpecialUncatch));
    TS_ASSERT(!a.is(Opcode::miStackDup));
    TS_ASSERT(!a.is(Opcode::maPush));
    TS_ASSERT(!a.is(Opcode::maUnary));
    TS_ASSERT( a.is(Opcode::maFusedUnary));
    TS_ASSERT(!a.is(interpreter::unInc));
    TS_ASSERT(!a.is(interpreter::biSub));
    TS_ASSERT(!a.is(interpreter::teKeyAdd));
    TS_ASSERT(!a.isJumpOrCatch());
    TS_ASSERT(!a.isRegularJump());
    TS_ASSERT(!a.isLabel());
    TS_ASSERT_EQUALS(a.getExternalMajor(), Opcode::maPush);
    TS_ASSERT_EQUALS(a.getDisassemblyTemplate(), "pushlit(u)\t%l");

    // Out-of-range
    TS_ASSERT_EQUALS(make(Opcode::maFusedUnary, 222, 0).getDisassemblyTemplate(), "push?(u)\t?");
}

/** Test fused-binary operation. */
void
TestInterpreterOpcode::testFusedBinary()
{
    // pushlit(b) [=first part of fused push+binary]
    Opcode a = make(Opcode::maFusedBinary, Opcode::sStatic, 0);
    TS_ASSERT(!a.is(Opcode::miSpecialUncatch));
    TS_ASSERT(!a.is(Opcode::miStackDup));
    TS_ASSERT(!a.is(Opcode::maPush));
    TS_ASSERT(!a.is(Opcode::maUnary));
    TS_ASSERT( a.is(Opcode::maFusedBinary));
    TS_ASSERT(!a.is(interpreter::unInc));
    TS_ASSERT(!a.is(interpreter::biSub));
    TS_ASSERT(!a.is(interpreter::teKeyAdd));
    TS_ASSERT(!a.isJumpOrCatch());
    TS_ASSERT(!a.isRegularJump());
    TS_ASSERT(!a.isLabel());
    TS_ASSERT_EQUALS(a.getExternalMajor(), Opcode::maPush);
    TS_ASSERT_EQUALS(a.getDisassemblyTemplate(), "pushtop(b)\t%T");

    // Out-of-range
    TS_ASSERT_EQUALS(make(Opcode::maFusedBinary, 222, 0).getDisassemblyTemplate(), "push?(b)\t?");
}

/** Test fused comparison. */
void
TestInterpreterOpcode::testFusedComparison()
{
    // bcmplt(j) [=first part of fused compare+jump]
    Opcode a = make(Opcode::maFusedComparison, interpreter::biCompareLT, 0);
    TS_ASSERT(!a.is(Opcode::miSpecialUncatch));
    TS_ASSERT(!a.is(Opcode::miStackDup));
    TS_ASSERT(!a.is(Opcode::maBinary));
    TS_ASSERT(!a.is(Opcode::maPush));
    TS_ASSERT( a.is(Opcode::maFusedComparison));
    TS_ASSERT(!a.is(interpreter::unInc));
    TS_ASSERT(!a.is(interpreter::biSub));
    TS_ASSERT(!a.is(interpreter::teKeyAdd));
    TS_ASSERT(!a.isJumpOrCatch());
    TS_ASSERT(!a.isRegularJump());
    TS_ASSERT(!a.isLabel());
    TS_ASSERT_EQUALS(a.getExternalMajor(), Opcode::maBinary);
    TS_ASSERT_EQUALS(a.getDisassemblyTemplate(), "bcmplt(j)");

    // Out-of-range
    TS_ASSERT_EQUALS(make(Opcode::maFusedComparison, 222, 0).getDisassemblyTemplate(), "b?(j)");
}

/** Test fused comparison (2). */
void
TestInterpreterOpcode::testFusedComparison2()
{
    // pushbool(b,j) [=first part of fused push+binary+jump]
    Opcode a = make(Opcode::maFusedComparison2, Opcode::sBoolean, 0);
    TS_ASSERT(!a.is(Opcode::miSpecialUncatch));
    TS_ASSERT(!a.is(Opcode::miStackDup));
    TS_ASSERT(!a.is(Opcode::maPush));
    TS_ASSERT(!a.is(Opcode::maUnary));
    TS_ASSERT( a.is(Opcode::maFusedComparison2));
    TS_ASSERT(!a.is(interpreter::unInc));
    TS_ASSERT(!a.is(interpreter::biSub));
    TS_ASSERT(!a.is(interpreter::teKeyAdd));
    TS_ASSERT(!a.isJumpOrCatch());
    TS_ASSERT(!a.isRegularJump());
    TS_ASSERT(!a.isLabel());
    TS_ASSERT_EQUALS(a.getExternalMajor(), Opcode::maPush);
    TS_ASSERT_EQUALS(a.getDisassemblyTemplate(), "pushbool(b,j)\t%d");

    // Out-of-range
    TS_ASSERT_EQUALS(make(Opcode::maFusedComparison2, 222, 0).getDisassemblyTemplate(), "push?(b,j)\t?");
}

/** Test in-place unary operation. */
void
TestInterpreterOpcode::testInplaceUnary()
{
    // pushloc(xu) [=first part of fused in-place push+unary]
    Opcode a = make(Opcode::maInplaceUnary, Opcode::sLocal, 3);
    TS_ASSERT(!a.is(Opcode::miSpecialUncatch));
    TS_ASSERT(!a.is(Opcode::miStackDup));
    TS_ASSERT(!a.is(Opcode::maPush));
    TS_ASSERT(!a.is(Opcode::maUnary));
    TS_ASSERT( a.is(Opcode::maInplaceUnary));
    TS_ASSERT(!a.is(interpreter::unInc));
    TS_ASSERT(!a.is(interpreter::biSub));
    TS_ASSERT(!a.is(interpreter::teKeyAdd));
    TS_ASSERT(!a.isJumpOrCatch());
    TS_ASSERT(!a.isRegularJump());
    TS_ASSERT(!a.isLabel());
    TS_ASSERT_EQUALS(a.getExternalMajor(), Opcode::maPush);
    TS_ASSERT_EQUALS(a.getDisassemblyTemplate(), "pushloc(xu)\t%L");

    // Out-of-range
    TS_ASSERT_EQUALS(make(Opcode::maInplaceUnary, 222, 0).getDisassemblyTemplate(), "push?(xu)\t?");
}

/** Test unknowns. */
void
TestInterpreterOpcode::testUnknown()
{
    // pushloc(xu) [=first part of fused in-place push+unary]
    Opcode a = make(77, 88, 99);
    TS_ASSERT(!a.is(Opcode::miSpecialUncatch));
    TS_ASSERT(!a.is(Opcode::miStackDup));
    TS_ASSERT(!a.is(Opcode::maPush));
    TS_ASSERT(!a.is(Opcode::maUnary));
    TS_ASSERT(!a.is(interpreter::unInc));
    TS_ASSERT(!a.is(interpreter::biSub));
    TS_ASSERT(!a.is(interpreter::teKeyAdd));
    TS_ASSERT(!a.isJumpOrCatch());
    TS_ASSERT(!a.isRegularJump());
    TS_ASSERT(!a.isLabel());
    TS_ASSERT_EQUALS(a.getExternalMajor(), 77);
    TS_ASSERT_EQUALS(a.getDisassemblyTemplate(), "unknown?\t%u");
}

