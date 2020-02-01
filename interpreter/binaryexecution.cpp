/**
  *  \file interpreter/binaryexecution.cpp
  */

#include <cmath>
#include "interpreter/binaryexecution.hpp"
#include "interpreter/error.hpp"
#include "afl/base/countof.hpp"
#include "interpreter/values.hpp"
#include "afl/data/scalarvalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/data/floatvalue.hpp"
#include "afl/charset/utf8.hpp"
#include "afl/data/booleanvalue.hpp"
#include "afl/string/format.hpp"
#include "util/math.hpp"
#include "interpreter/binaryoperation.hpp"
#include "interpreter/keymapvalue.hpp"
#include "util/key.hpp"
#include "interpreter/callablevalue.hpp"
#include "afl/data/visitor.hpp"

using interpreter::getBooleanValue;
using interpreter::makeBooleanValue;
using interpreter::makeIntegerValue;
using interpreter::makeStringValue;
using interpreter::makeFloatValue;
using interpreter::Error;

namespace {
    /** Comparison result. */
    enum Comparison {
        cmpNull    = 1,         ///< Either argument is null.
        cmpLess    = 2,         ///< a < b.
        cmpEqual   = 4,         ///< a = b.
        cmpGreater = 8          ///< a > b.
    };

    /** Arithmetic status. */
    enum Arithmetic {
        ariBad,                 ///< Bad argument combination.
        ariNull,                ///< Either argument is null.
        ariInt,                 ///< Use integer arithmetic.
        ariFloat                ///< Use float arithmetic.
    };

    /** Arithmetic argument pair. */
    struct ArithmeticPair {
        int32_t ia, ib;         ///< Operands for integer arithmetic.
        double fa, fb;          ///< Operands for float arithmetic.
    };

    /** Check arguments for arithmetic.
        \param pair [out] Receives the operands to operate on
        \param a,b  [in] User-supplied operands
        \return argument status */
    Arithmetic checkArithmetic(ArithmeticPair& pair, const afl::data::Value* a, const afl::data::Value* b)
    {
        // ex ccexpr.pas:MakeComp
#if 1
        // Visitor version of type switch: 6% faster on "for i:=1 to 3000000 do j:=i+1" benchmark, 1.7k larger than dynamic_cast version
        class VBase : public afl::data::Visitor {
         public:
            VBase()
                : m_result(ariBad)
                { }
            virtual void visitString(const String_t& /*str*/)
                { m_result = ariBad; }
            virtual void visitBoolean(bool bv)
                { visitInteger(bv); }
            virtual void visitHash(const afl::data::Hash& /*hv*/)
                { m_result = ariBad; }
            virtual void visitVector(const afl::data::Vector& /*vv*/)
                { m_result = ariBad; }
            virtual void visitOther(const afl::data::Value& /*other*/)
                { m_result = ariBad; }
            virtual void visitNull()
                { m_result = ariNull; }
            virtual void visitError(const String_t& /*source*/, const String_t& /*str*/)
                { m_result = ariBad; }
            Arithmetic get() const
                { return m_result; }
         protected:
            Arithmetic m_result;
        };

        class VInt : public VBase {
         public:
            VInt(ArithmeticPair& pair, int32_t a)
                : m_pair(pair),
                  m_a(a)
                { }
            virtual void visitInteger(int32_t iv)
                {
                    m_pair.ia = m_a;
                    m_pair.ib = iv;
                    m_result = ariInt;
                }
            virtual void visitFloat(double fv)
                {
                    m_pair.fa = m_a;
                    m_pair.fb = fv;
                    m_result = ariFloat;
                }
         private:
            ArithmeticPair& m_pair;
            const int32_t m_a;
        };

        class VFloat : public VBase {
         public:
            VFloat(ArithmeticPair& pair, double a)
                : m_pair(pair),
                  m_a(a)
                { }
            virtual void visitInteger(int32_t iv)
                {
                    m_pair.fa = m_a;
                    m_pair.fb = iv;
                    m_result = ariFloat;
                }
            virtual void visitFloat(double fv)
                {
                    m_pair.fa = m_a;
                    m_pair.fb = fv;
                    m_result = ariFloat;
                }
         private:
            ArithmeticPair& m_pair;
            const double m_a;
        };

        class VPair : public VBase {
         public:
            VPair(ArithmeticPair& pair, const afl::data::Value* b)
                : m_pair(pair),
                  m_b(b)
                { }
            virtual void visitInteger(int32_t iv)
                {
                    VInt v(m_pair, iv);
                    v.visit(m_b);
                    m_result = v.get();
                }
            virtual void visitFloat(double fv)
                {
                    VFloat v(m_pair, fv);
                    v.visit(m_b);
                    m_result = v.get();
                }
         private:
            ArithmeticPair& m_pair;
            const afl::data::Value* m_b;
        };

        if (a == 0 || b == 0) {
            return ariNull;
        } else {
            VPair v(pair, b);
            v.visit(a);
            return v.get();
        }
#else
        bool afloat = false, bfloat = false;

        // Check for null
        if (a == 0 || b == 0)
            return ariNull;

        // Check a for numericness
        if (const afl::data::ScalarValue* iv = dynamic_cast<const afl::data::ScalarValue*>(a))
            pair.ia = iv->getValue();
        else if (const afl::data::FloatValue* fv = dynamic_cast<const afl::data::FloatValue*>(a))
            pair.fa = fv->getValue(), afloat = true;
        else
            return ariBad;

        // Check b for numericness
        if (const afl::data::ScalarValue* iv = dynamic_cast<const afl::data::ScalarValue*>(b))
            pair.ib = iv->getValue();
        else if (const afl::data::FloatValue* fv = dynamic_cast<const afl::data::FloatValue*>(b))
            pair.fb = fv->getValue(), bfloat = true;
        else
            return ariBad;

        // Promote if necessary
        if (afloat || bfloat) {
            if (!afloat)
                pair.fa = pair.ia;
            if (!bfloat)
                pair.fb = pair.ib;
            return ariFloat;
        } else {
            return ariInt;
        }
#endif
    }

    String_t convertCase(const afl::data::StringValue* sv, bool doit)
    {
        // ex ccexpr.pas:UpProc
        if (doit)
            return afl::string::strUCase(sv->getValue());
        else
            return sv->getValue();
    }

    /** Perform comparison. Compares two user-supplied parameters and returns
        comparison result. Caller converts this again to a user-visible value.
        \param a,b User-supplied parameters
        \return Result */
    Comparison compare(const afl::data::Value* a, const afl::data::Value* b, bool caseblind)
    {
        // ex ccexpr.pas:Compare
#if 1
        // Visitor version of type switch: again 6% faster on "for i:=1 to 3000000 do j:=i+1" benchmark, 1.8k larger than dynamic_cast version
        class VBase : public afl::data::Visitor {
         public:
            VBase()
                : m_result(cmpNull)
                { }
            virtual void visitBoolean(bool bv)
                { visitInteger(bv); }
            virtual void visitHash(const afl::data::Hash& /*hv*/)
                { fail(); }
            virtual void visitVector(const afl::data::Vector& /*vv*/)
                { fail(); }
            virtual void visitOther(const afl::data::Value& /*other*/)
                { fail(); }
            virtual void visitNull()
                { m_result = cmpNull; }
            virtual void visitError(const String_t& /*source*/, const String_t& /*str*/)
                { fail(); }
            Comparison get() const
                { return m_result; }
            void fail()
                { throw Error::typeError(); }
         protected:
            Comparison m_result;
        };

        class VInt : public VBase {
         public:
            VInt(int32_t a)
                : m_a(a)
                { }
            virtual void visitInteger(int32_t iv)
                {
                    m_result = (m_a == iv
                                ? cmpEqual
                                : m_a < iv
                                ? cmpLess
                                : cmpGreater);
                }
            virtual void visitFloat(double fv)
                {
                    m_result = (m_a == fv
                                ? cmpEqual
                                : m_a < fv
                                ? cmpLess
                                : cmpGreater);
                }
            virtual void visitString(const String_t& /*sv*/)
                { fail(); }
         private:
            const int32_t m_a;
        };

        class VFloat : public VBase {
         public:
            VFloat(double a)
                : m_a(a)
                { }
            virtual void visitInteger(int32_t iv)
                {
                    m_result = (m_a == iv
                                ? cmpEqual
                                : m_a < iv
                                ? cmpLess
                                : cmpGreater);
                }
            virtual void visitFloat(double fv)
                {
                    m_result = (m_a == fv
                                ? cmpEqual
                                : m_a < fv
                                ? cmpLess
                                : cmpGreater);
                }
            virtual void visitString(const String_t& /*sv*/)
                { fail(); }
         private:
            const double m_a;
        };

        class VString : public VBase {
         public:
            VString(const String_t& a, bool caseblind)
                : m_a(a),
                  m_caseblind(caseblind)
                { }
            virtual void visitInteger(int32_t /*iv*/)
                { fail(); }
            virtual void visitFloat(double /*fv*/)
                { fail(); }
            virtual void visitString(const String_t& sv)
                {
                    int tmp;
                    if (m_caseblind) {
                        tmp = afl::string::strCaseCompare(m_a, sv);
                    } else {
                        tmp = m_a.compare(sv);
                    }
                    m_result = (tmp == 0
                                ? cmpEqual
                                : tmp < 0
                                ? cmpLess
                                : cmpGreater);
                }
         private:
            const String_t& m_a;
            const bool m_caseblind;
        };

        class VPair : public VBase {
         public:
            VPair(bool caseblind, const afl::data::Value* b)
                : m_caseblind(caseblind),
                  m_b(b)
                { }
            virtual void visitInteger(int32_t iv)
                {
                    VInt v(iv);
                    v.visit(m_b);
                    m_result = v.get();
                }
            virtual void visitFloat(double fv)
                {
                    VFloat v(fv);
                    v.visit(m_b);
                    m_result = v.get();
                }
            virtual void visitString(const String_t& sv)
                {
                    VString v(sv, m_caseblind);
                    v.visit(m_b);
                    m_result = v.get();
                }
         private:
            const bool m_caseblind;
            const afl::data::Value*const m_b;
        };
        VPair v(caseblind, b);
        v.visit(a);
        return v.get();
#else
        ArithmeticPair p;
        switch (checkArithmetic(p, a, b)) {
         case ariNull:
            return cmpNull;
         case ariInt:
            return p.ia < p.ib
                          ? cmpLess
                          : p.ia == p.ib
                          ? cmpEqual
                          : cmpGreater;
         case ariFloat:
            return p.fa < p.fb
                          ? cmpLess
                          : p.fa == p.fb
                          ? cmpEqual
                          : cmpGreater;
         default:
            const afl::data::StringValue* sa = dynamic_cast<const afl::data::StringValue*>(a);
            const afl::data::StringValue* sb = dynamic_cast<const afl::data::StringValue*>(b);
            if (sa != 0 && sb != 0) {
                String_t ssa = convertCase(sa, caseblind);
                String_t ssb = convertCase(sb, caseblind);
                return ssa < ssb ? cmpLess : ssa == ssb ? cmpEqual : cmpGreater;
            }
            throw Error::typeError();
        }
#endif
    }

    /*
     *  Opcodes
     */


    afl::data::Value* FAnd(interpreter::World& /*world*/, const afl::data::Value* a, const afl::data::Value* b)
    {
        // ex ccexpr.pas:op_AND (part)
        // Logical And, ternary logic
        //   e_f_t
        // e|e f e
        // f|f f f
        // t|e f t
        int ba = getBooleanValue(a);
        int bb = getBooleanValue(b);
        if (ba > 0 && bb > 0)
            return makeBooleanValue(1);
        else if (ba == 0 || bb == 0)
            return makeBooleanValue(0);
        else
            return makeBooleanValue(-1);
    }

    afl::data::Value* FOr(interpreter::World& /*world*/, const afl::data::Value* a, const afl::data::Value* b)
    {
        // ex ccexpr.pas:op_OR (part)
        // Logical Or, ternary logic
        //   e_f_t
        // e|e e t
        // f|e f t
        // t|t t t
        int ba = getBooleanValue(a);
        int bb = getBooleanValue(b);
        if (ba > 0 || bb > 0)
            return makeBooleanValue(1);
        else if (ba == 0 && bb == 0)
            return makeBooleanValue(0);
        else
            return makeBooleanValue(-1);
    }

    afl::data::Value* FXor(interpreter::World& /*world*/, const afl::data::Value* a, const afl::data::Value* b)
    {
        // ex ccexpr.pas:op_XOR
        // Logical Xor, ternary logic
        //   e_f_t
        // e|e e e
        // f|e f t
        // t|e t f
        int ba = getBooleanValue(a);
        int bb = getBooleanValue(b);
        if (ba < 0 || bb < 0)
            return makeBooleanValue(-1);
        else
            return makeBooleanValue(ba ^ bb);
    }

    afl::data::Value* FAdd(interpreter::World& /*world*/, const afl::data::Value* a, const afl::data::Value* b)
    {
        // ex ccexpr.pas:op_PLUS
        // Arithmetical addition or string concatenation
        ArithmeticPair p;
        switch (checkArithmetic(p, a, b)) {
         case ariNull:
            return 0;
         case ariInt:
            return makeIntegerValue(p.ia + p.ib);
         case ariFloat:
            return makeFloatValue(p.fa + p.fb);
         default:
            const afl::data::StringValue* sa = dynamic_cast<const afl::data::StringValue*>(a);
            const afl::data::StringValue* sb = dynamic_cast<const afl::data::StringValue*>(b);
            if (sa != 0 && sb != 0) {
                return makeStringValue(sa->getValue() + sb->getValue());
            }
            throw Error::typeError();
        }
    }

    afl::data::Value* FSub(interpreter::World& /*world*/, const afl::data::Value* a, const afl::data::Value* b)
    {
        // ex ccexpr.pas:op_MINUS
        // Subtraction
        ArithmeticPair p;
        switch (checkArithmetic(p, a, b)) {
         case ariNull:
            return 0;
         case ariInt:
            return makeIntegerValue(p.ia - p.ib);
         case ariFloat:
            return makeFloatValue(p.fa - p.fb);
         default:
            throw Error::typeError(Error::ExpectNumeric);
        }
    }

    afl::data::Value* FMult(interpreter::World& /*world*/, const afl::data::Value* a, const afl::data::Value* b)
    {
        // ex ccexpr.pas:op_MUL
        // Multiplication
        ArithmeticPair p;
        switch (checkArithmetic(p, a, b)) {
         case ariNull:
            return 0;
         case ariInt:
            return makeIntegerValue(p.ia * p.ib);
         case ariFloat:
            return makeFloatValue(p.fa * p.fb);
         default:
            throw Error::typeError(Error::ExpectNumeric);
        }
    }

    afl::data::Value* FDivide(interpreter::World& /*world*/, const afl::data::Value* a, const afl::data::Value* b)
    {
        // ex ccexpr.pas:op_REALDIV
        // Division
        ArithmeticPair p;
        switch (checkArithmetic(p, a, b)) {
         case ariNull:
            return 0;
         case ariInt:
            if (p.ib == 0) {
                throw Error("Divide by zero");
            }
            if (p.ia % p.ib == 0) {
                return makeIntegerValue(p.ia / p.ib);
            } else {
                return makeFloatValue(double(p.ia) / double(p.ib));
            }
         case ariFloat:
            if (std::fabs(p.fb) < 1.0E-06)          // FIXME?
                throw Error("Divide by zero");
            return makeFloatValue(p.fa / p.fb);
         default:
            throw Error::typeError(Error::ExpectNumeric);
        }
    }

    afl::data::Value* FIntegerDivide(interpreter::World& /*world*/, const afl::data::Value* a, const afl::data::Value* b)
    {
        // ex ccexpr.pas:op_DIV
        // Integer division
        ArithmeticPair p;
        switch (checkArithmetic(p, a, b)) {
         case ariNull:
            return 0;
         case ariInt:
            if (p.ib == 0)
                throw Error("Divide by zero");
            return makeIntegerValue(p.ia / p.ib);
         default:
            throw Error::typeError(Error::ExpectInteger);
        }
    }

    afl::data::Value* FRemainder(interpreter::World& /*world*/, const afl::data::Value* a, const afl::data::Value* b)
    {
        // ex ccexpr.pas:op_MOD
        // Integer remainder
        ArithmeticPair p;
        switch (checkArithmetic(p, a, b)) {
         case ariNull:
            return 0;
         case ariInt:
            if (p.ib == 0)
                throw Error("Divide by zero");
            return makeIntegerValue(p.ia % p.ib);
         default:
            throw Error::typeError(Error::ExpectInteger);
        }
    }

    afl::data::Value* FPow(interpreter::World& /*world*/, const afl::data::Value* a, const afl::data::Value* b)
    {
        // ex ccexpr.pas:op_POW
        // Exponentiation
        if (a == 0 || b == 0)
            return 0;

        /* Second argument must be integer */
        const afl::data::ScalarValue* bi = dynamic_cast<const afl::data::ScalarValue*>(b);
        if (!bi)
            throw Error::typeError(Error::ExpectInteger);

        /* First argument must be integer or real */
        if (const afl::data::ScalarValue* ai = dynamic_cast<const afl::data::ScalarValue*>(a)) {
            /* Maximum value a, for which a^b yields an integer, starting with b=2 */
            static const uint16_t amax[] = {
                46340,              // 46340**2 = 2147395600, 46341**2 =      2147488281
                1290,               //  1290**3 = 2146689000,  1291**3 =      2151685171
                215,                //   215**4 = 2136750625,   216**4 =      2176782336
                73,                 //    73**5 = 2073071593,    74**5 =      2219006624
                35,                 //    35**6 = 1838265625,    36**6 =      2176782336
                21,                 //    21**7 = 1801088541,    22**7 =      2494357888
                14,                 //    14**8 = 1475789056,    15**8 =      2562890625
                10,                 //    10**9 = 1000000000,    11**9 =      2357947691
                8,                  //    8**10 = 1073741824,    9**10 =      3486784401
                7,                  //    7**11 = 1977326743,    8**11 =      8589934592
                5,                  //    5**12 =  244140625,    6**12 =      2176782336
                5,                  //    5**13 = 1220703125,    6**13 =     13060694016
                4,                  //    4**14 =  268435456,    5**14 =      6103515625
                4,                  //    4**15 = 1073741824,    5**15 =     30517578125
                3,                  //    3**16 =   43046721,    4**16 =      4294967296
                3,                  //    3**17 =  129140163,    4**17 =     17179869184
                3,                  //    3**18 =  387420489,    4**18 =     68719476736
                3,                  //    3**19 = 1162261467,    4**19 =    274877906944
                2,                  //    2**20 =    1048576,    3**20 =      3486784401
                2,                  //    2**21 =    2097152,    3**21 =     10460353203
                2,                  //    2**22 =    4194304,    3**22 =     31381059609
                2,                  //    2**23 =    8388608,    3**23 =     94143178827
                2,                  //    2**24 =   16777216,    3**24 =    282429536481
                2,                  //    2**25 =   33554432,    3**25 =    847288609443
                2,                  //    2**26 =   67108864,    3**26 =   2541865828329
                2,                  //    2**27 =  134217728,    3**27 =   7625597484987
                2,                  //    2**28 =  268435456,    3**28 =  22876792454961
                2,                  //    2**29 =  536870912,    3**29 =  68630377364883
                2,                  //    2**30 = 1073741824,    3**30 = 205891132094649
            };

            int32_t a = ai->getValue();
            int32_t b = bi->getValue();
            if (b == 0) {
                // a^0 is 1
                return makeIntegerValue(1);
            } else if (b > 0) {
                if (a == 0 || a == 1) {
                    // 0^b is 0, 1^b is 1
                    return makeIntegerValue(a);
                } else if (a == -1) {
                    // -1^b is 1 (even b) or -1 (odd b)
                    return makeIntegerValue((b & 1) ? -1 : 1);
                } else if (b == 1) {
                    // a^1 is a
                    return makeIntegerValue(a);
                } else if (b < int32_t(2+countof(amax)) && std::abs(a) <= int32_t(amax[b-2])) {
                    // can be computed in exact integers
                    int32_t result = 1;
                    while (b--) {
                        result *= a;
                    }
                    return makeIntegerValue(result);
                } else {
                    // overflows to float
                    return makeFloatValue(std::pow(double(a), double(b)));
                }
            } else {
                // fractional result
                return makeFloatValue(std::pow(double(a), double(b)));
            }
        } else if (const afl::data::FloatValue* af = dynamic_cast<const afl::data::FloatValue*>(a)) {
            return makeFloatValue(std::pow(af->getValue(), bi->getValue()));
        } else {
            throw Error::typeError(Error::ExpectNumeric);
        }
    }

    afl::data::Value* FConcat(interpreter::World& /*world*/, const afl::data::Value* a, const afl::data::Value* b)
    {
        // ex ccexpr.pas:op_HASH
        // Concatenation, null annihilates
        if (a == 0 || b == 0)
            return 0;
        else
            return makeStringValue(interpreter::toString(a, false) + interpreter::toString(b, false));
    }

    afl::data::Value* FConcatEmpty(interpreter::World& /*world*/, const afl::data::Value* a, const afl::data::Value* b)
    {
        // ex ccexpr.pas:op_CONCAT
        // Concatenation, null interpolates
        if (a == 0 && b == 0) {
            // @diff PCC 1.x does not special-case this, and returns "" for EMPTY & EMPTY.
            return 0;
        } else {
            String_t result;
            if (a != 0)
                result += interpreter::toString(a, false);
            if (b != 0)
                result += interpreter::toString(b, false);
            return makeStringValue(result);
        }
    }


    /****************** Optionally case-sensitive functions ******************/

    afl::data::Value* FCompareEQ(const afl::data::Value* a, const afl::data::Value* b, bool caseblind)
    {
        // ex ccexpr.pas:op_EQ
        // Compare for equality
        Comparison cmp = compare(a, b, caseblind);
        return (cmp == cmpNull)
            ? 0
            : makeBooleanValue(cmp == cmpEqual);
    }

    afl::data::Value* FCompareNE(const afl::data::Value* a, const afl::data::Value* b, bool caseblind)
    {
        // ex ccexpr.pas:op_NE
        // Compare for inequality
        Comparison cmp = compare(a, b, caseblind);
        return (cmp == cmpNull)
            ? 0
            : makeBooleanValue(cmp != cmpEqual);
    }

    afl::data::Value* FCompareLE(const afl::data::Value* a, const afl::data::Value* b, bool caseblind)
    {
        // ex ccexpr.pas:op_LE
        // Compare for less/equal
        Comparison cmp = compare(a, b, caseblind);
        return (cmp == cmpNull)
            ? 0
            : makeBooleanValue(cmp != cmpGreater);
    }

    afl::data::Value* FCompareLT(const afl::data::Value* a, const afl::data::Value* b, bool caseblind)
    {
        // ex ccexpr.pas:op_LT
        // Compare for less than
        Comparison cmp = compare(a, b, caseblind);
        return (cmp == cmpNull)
            ? 0
            : makeBooleanValue(cmp == cmpLess);
    }

    afl::data::Value* FCompareGE(const afl::data::Value* a, const afl::data::Value* b, bool caseblind)
    {
        // ex ccexpr.pas:op_GE
        // Compare for greater/equal
        Comparison cmp = compare(a, b, caseblind);
        return (cmp == cmpNull)
            ? 0
            : makeBooleanValue(cmp != cmpLess);
    }

    afl::data::Value* FCompareGT(const afl::data::Value* a, const afl::data::Value* b, bool caseblind)
    {
        // ex ccexpr.pas:op_GT
        // Compare for greater than
        Comparison cmp = compare(a, b, caseblind);
        return (cmp == cmpNull)
            ? 0
            : makeBooleanValue(cmp == cmpGreater);
    }

    afl::data::Value* FMin(const afl::data::Value* a, const afl::data::Value* b, bool caseblind)
    {
        // ex ccexpr.pas:op_MIN_func (sort-of)
        // Compute minimum
        Comparison cmp = compare(a, b, caseblind);
        if (cmp == cmpNull)
            return 0;
        else if (cmp == cmpLess)
            return a->clone();
        else
            return b->clone();
    }

    afl::data::Value* FMax(const afl::data::Value* a, const afl::data::Value* b, bool caseblind)
    {
        // ex ccexpr.pas:op_MAX_func (sort-of)
        // Compute maximum
        Comparison cmp = compare(a, b, caseblind);
        if (cmp == cmpNull)
            return 0;
        else if (cmp == cmpGreater)
            return a->clone();
        else
            return b->clone();
    }

    afl::data::Value* FFirstStr(const afl::data::Value* a, const afl::data::Value* b, bool caseblind)
    {
        // ex ccexpr.pas:op_FIRST_func
        // Split string at delimiter, return first part
        if (a == 0 || b == 0)
            return 0;

        const afl::data::StringValue* sa = dynamic_cast<const afl::data::StringValue*>(a);
        const afl::data::StringValue* sb = dynamic_cast<const afl::data::StringValue*>(b);
        if (sa == 0 || sb == 0)
            throw Error::typeError(Error::ExpectString);

        String_t ssa = sa->getValue();
        String_t::size_type apos = convertCase(sa, caseblind).find(convertCase(sb, caseblind));
        if (apos != String_t::npos)
            ssa.erase(apos);

        return makeStringValue(ssa);
    }

    afl::data::Value* FRestStr(const afl::data::Value* a, const afl::data::Value* b, bool caseblind)
    {
        // ex ccexpr.pas:op_REST_func
        // Split string at delimiter, return remainder
        if (a == 0 || b == 0)
            return 0;

        const afl::data::StringValue* sa = dynamic_cast<const afl::data::StringValue*>(a);
        const afl::data::StringValue* sb = dynamic_cast<const afl::data::StringValue*>(b);
        if (sa == 0 || sb == 0)
            throw Error::typeError(Error::ExpectString);

        String_t::size_type apos = convertCase(sa, caseblind).find(convertCase(sb, caseblind));
        if (apos != String_t::npos) {
            return makeStringValue(sa->getValue().substr(apos + sb->getValue().size()));
        } else {
            return 0;
        }
    }

    afl::data::Value* FFindStr(const afl::data::Value* a, const afl::data::Value* b, bool caseblind)
    {
        // ex ccexpr.pas:op_INSTR_func
        // Find substring
        if (a == 0 || b == 0)
            return 0;

        const afl::data::StringValue* sa = dynamic_cast<const afl::data::StringValue*>(a);
        const afl::data::StringValue* sb = dynamic_cast<const afl::data::StringValue*>(b);
        if (sa == 0 || sb == 0)
            throw Error::typeError(Error::ExpectString);

        String_t::size_type apos = convertCase(sa, caseblind).find(convertCase(sb, caseblind));
        if (apos != String_t::npos)
            return makeIntegerValue(static_cast<int32_t>(afl::charset::Utf8().byteToCharPos(sa->getValue(), apos)) + 1);
        else
            return makeIntegerValue(0);
    }

    afl::data::Value* FBitAnd(interpreter::World& /*world*/, const afl::data::Value* a, const afl::data::Value* b)
    {
        // ex ccexpr.pas:op_BITAND_func
        // Bitwise and
        ArithmeticPair p;
        switch (checkArithmetic(p, a, b)) {
         case ariNull:
            return 0;
         case ariInt:
            return makeIntegerValue(p.ia & p.ib);
         default:
            throw Error::typeError(Error::ExpectInteger);
        }
    }

    afl::data::Value* FBitOr(interpreter::World& /*world*/, const afl::data::Value* a, const afl::data::Value* b)
    {
        // ex ccexpr.pas:op_BITOR_func
        // Bitwise or
        ArithmeticPair p;
        switch (checkArithmetic(p, a, b)) {
         case ariNull:
            return 0;
         case ariInt:
            return makeIntegerValue(p.ia | p.ib);
         default:
            throw Error::typeError(Error::ExpectInteger);
        }
    }

    afl::data::Value* FBitXor(interpreter::World& /*world*/, const afl::data::Value* a, const afl::data::Value* b)
    {
        // ex ccexpr.pas:op_BITXOR_func
        // Bitwise Xor
        ArithmeticPair p;
        switch (checkArithmetic(p, a, b)) {
         case ariNull:
            return 0;
         case ariInt:
            return makeIntegerValue(p.ia ^ p.ib);
         default:
            throw Error::typeError(Error::ExpectInteger);
        }
    }

    afl::data::Value* FStr(interpreter::World& /*world*/, const afl::data::Value* a, const afl::data::Value* b)
    {
        // ex ccexpr.pas:StrProc, op_STR_func (part)
        // Stringification with precision
        if (a == 0 || b == 0)
            return 0;

        /* Check second arg */
        const afl::data::ScalarValue* bi = dynamic_cast<const afl::data::ScalarValue*>(b);
        if (bi == 0)
            throw Error::typeError(Error::ExpectInteger);
        if (bi->getValue() < 0)
            throw Error::rangeError();

        /* Check first arg */
        if (const afl::data::ScalarValue* ai = dynamic_cast<const afl::data::ScalarValue*>(a)) {
            /* Bool converts as-is, as does integer with precision 0 */
            if (dynamic_cast<const afl::data::BooleanValue*>(ai) != 0 || bi->getValue() == 0)
                return makeStringValue(interpreter::toString(ai, false));
            /* Convert integer as floating point */
            return makeStringValue(afl::string::Format(String_t(afl::string::Format("%%.%df", bi->getValue())).c_str(), double(ai->getValue())));
        } else if (const afl::data::FloatValue* af = dynamic_cast<const afl::data::FloatValue*>(a)) {
            /* Convert float */
            return makeStringValue(afl::string::Format(String_t(afl::string::Format("%%.%df", bi->getValue())).c_str(), double(af->getValue())));
        } else {
            throw Error::typeError(Error::ExpectNumeric);
        }
    }

    afl::data::Value* FAtan(interpreter::World& /*world*/, const afl::data::Value* a, const afl::data::Value* b)
    {
        // ex ccexpr.pas:op_ATAN_func
        // Arc-tangent
        double value;
        ArithmeticPair p;
        switch (checkArithmetic(p, a, b)) {
         case ariNull:
            return 0;
         case ariInt:
            p.fa = p.ia;
            p.fb = p.ib;
            /* FALLTHROUGH */
         case ariFloat:
            if (p.fa == 0 && p.fb == 0)
                return 0;
            value = std::atan2(p.fa, p.fb) * (180.0 / util::PI);
            if (value < 0)
                value += 360;
            return makeFloatValue(value);
         default:
            throw Error::typeError(Error::ExpectNumeric);
        }
    }

    afl::data::Value* FLCut(interpreter::World& /*world*/, const afl::data::Value* a, const afl::data::Value* b)
    {
        // Remove leftmost N characters
        if (a == 0 || b == 0)
            return 0;

        const afl::data::StringValue* sa = dynamic_cast<const afl::data::StringValue*>(a);
        const afl::data::ScalarValue* ib = dynamic_cast<const afl::data::ScalarValue*>(b);
        if (sa == 0 || ib == 0)
            throw Error::typeError();

        String_t ssa = sa->getValue();
        int32_t  iib = ib->getValue();
        if (iib > 0) {
            ssa = afl::charset::Utf8().substr(ssa, iib-1, String_t::npos);
        }
        return makeStringValue(ssa);
    }

    afl::data::Value* FRCut(interpreter::World& /*world*/, const afl::data::Value* a, const afl::data::Value* b)
    {
        // ex ccexpr.pas:op_LEFT_func
        // Remove after Nth character
        if (a == 0 || b == 0)
            return 0;

        const afl::data::StringValue* sa = dynamic_cast<const afl::data::StringValue*>(a);
        const afl::data::ScalarValue* ib = dynamic_cast<const afl::data::ScalarValue*>(b);
        if (sa == 0 || ib == 0)
            throw Error::typeError();

        String_t ssa = sa->getValue();
        int32_t  iib = ib->getValue();
        if (iib > 0) {
            ssa = afl::charset::Utf8().substr(ssa, 0, iib);
        } else {
            ssa.clear();
        }
        return makeStringValue(ssa);
    }

    afl::data::Value* FEndCut(interpreter::World& /*world*/, const afl::data::Value* a, const afl::data::Value* b)
    {
        // ex ccexpr.pas:op_RIGHT_func
        // Remove all but last N characters
        if (a == 0 || b == 0)
            return 0;

        const afl::data::StringValue* sa = dynamic_cast<const afl::data::StringValue*>(a);
        const afl::data::ScalarValue* ib = dynamic_cast<const afl::data::ScalarValue*>(b);
        if (sa == 0 || ib == 0)
            throw Error::typeError();

        String_t ssa = sa->getValue();
        int32_t  iib = ib->getValue();
        if (iib > 0) {
            size_t have = afl::charset::Utf8().length(ssa);
            if (size_t(iib) < have)
                ssa = afl::charset::Utf8().substr(ssa, have - iib, String_t::npos);
        } else {
            ssa.clear();
        }
        return makeStringValue(ssa);
    }

    afl::data::Value* FStrMult(interpreter::World& /*world*/, const afl::data::Value* a, const afl::data::Value* b)
    {
        // ex ccexpr.pas:op_STRING_func
        // Replicate string
        if (a == 0 || b == 0)
            return 0;

        const afl::data::ScalarValue* ia = dynamic_cast<const afl::data::ScalarValue*>(a);
        const afl::data::StringValue* sb = dynamic_cast<const afl::data::StringValue*>(b);
        if (ia == 0 || sb == 0)
            throw Error::typeError();

        int32_t  iia = ia->getValue();
        String_t ssb = sb->getValue();

        // FIXME: we should have some kind of limits to avoid overloading
        String_t result;
        while (iia > 0) {
            result += ssb;
            --iia;
        }

        return makeStringValue(result);
    }

    afl::data::Value* FKeyAddParent(interpreter::World& /*world*/, const afl::data::Value* a, const afl::data::Value* b)
    {
        if (a == 0 || b == 0)
            return 0;

        const interpreter::KeymapValue* keymap = dynamic_cast<const interpreter::KeymapValue*>(a);
        const interpreter::KeymapValue* parent = dynamic_cast<const interpreter::KeymapValue*>(b);
        if (keymap == 0 || parent == 0)
            throw Error::typeError(Error::ExpectKeymap);

        keymap->getKeymap()->addParent(*parent->getKeymap());
        return keymap->clone();
    }

    afl::data::Value* FKeyFind(interpreter::World& /*world*/, const afl::data::Value* a, const afl::data::Value* b)
    {
        if (a == 0 || b == 0)
            return 0;

        // Keymap
        const interpreter::KeymapValue* keymap = dynamic_cast<const interpreter::KeymapValue*>(a);
        if (keymap == 0)
            throw Error::typeError(Error::ExpectKeymap);

        // Key
        const afl::data::StringValue* keysym = dynamic_cast<const afl::data::StringValue*>(b);
        if (!keysym)
            throw Error::typeError(Error::ExpectString);

        uint32_t keyval = 0;
        if (!util::parseKey(keysym->getValue(), keyval)) {
            throw Error("Invalid key name");
        }

        // Generate result
        uint32_t cmd = keymap->getKeymap()->lookupCommand(keyval);
        if (!cmd)
            return 0;
        else
            return makeIntegerValue(cmd);
    }

    afl::data::Value* FArrayDim(interpreter::World& /*world*/, const afl::data::Value* a, const afl::data::Value* b)
    {
        if (a == 0 || b == 0)
            return 0;

        // Array
        const interpreter::CallableValue* av = dynamic_cast<const interpreter::CallableValue*>(a);
        if (av == 0) {
            throw Error::typeError(Error::ExpectArray);
        }

        // Index
        const afl::data::ScalarValue* iv = dynamic_cast<const afl::data::ScalarValue*>(b);
        if (iv == 0) {
            throw Error::typeError(Error::ExpectInteger);
        }
        int32_t n = iv->getValue();
        if (n <= 0 || n > av->getDimension(0)) {
            throw Error::rangeError();
        }

        return makeIntegerValue(av->getDimension(n));
    }

    template<afl::data::Value* (*Func)(const afl::data::Value*, const afl::data::Value*, bool), bool Value>
    afl::data::Value*
    Bind(interpreter::World& /*world*/, const afl::data::Value* a, const afl::data::Value* b)
    {
        return Func(a, b, Value);
    }

    afl::data::Value* (*const binary_ops[])(interpreter::World&,const afl::data::Value*,const afl::data::Value*) = {
        FAnd,
        FOr,
        FXor,
        FAdd,
        FSub,
        FMult,
        FDivide,
        FIntegerDivide,
        FRemainder,
        FPow,
        FConcat,
        FConcatEmpty,
        Bind<FCompareEQ,false>,
        Bind<FCompareEQ,true>,
        Bind<FCompareNE,false>,
        Bind<FCompareNE,true>,
        Bind<FCompareLE,false>,
        Bind<FCompareLE,true>,
        Bind<FCompareLT,false>,
        Bind<FCompareLT,true>,
        Bind<FCompareGE,false>,
        Bind<FCompareGE,true>,
        Bind<FCompareGT,false>,
        Bind<FCompareGT,true>,
        Bind<FMin,false>,
        Bind<FMin,true>,
        Bind<FMax,false>,
        Bind<FMax,true>,
        Bind<FFirstStr,false>,
        Bind<FFirstStr,true>,
        Bind<FRestStr,false>,
        Bind<FRestStr,true>,
        Bind<FFindStr,false>,
        Bind<FFindStr,true>,
        FBitAnd,
        FBitOr,
        FBitXor,
        FStr,
        FAtan,
        FLCut,
        FRCut,
        FEndCut,
        FStrMult,
        FKeyAddParent,
        FKeyFind,
        FArrayDim,
    };
}

// /** Execute binary operation.
//     \param op Operation, IntBinaryOperation
//     \param a,b User-supplied arguments taken from value stack
//     \return New value to push on value stack */
afl::data::Value*
interpreter::executeBinaryOperation(World& world, uint8_t op, const afl::data::Value* a, const afl::data::Value* b)
{
    // ex executeBinaryOperation
    if (op < countof(binary_ops)) {
        return binary_ops[op](world, a, b);
    } else {
        throw Error::internalError("invalid binary operation");
    }
}

// /** Execute a comparison operation.
//     \param op Operation, IntBinaryOperation
//     \param a,b User-supplied arguments taken from value stack
//     \return Comparison result, possible input to makeBooleanValue. */
int
interpreter::executeComparison(uint8_t op, const afl::data::Value* a, const afl::data::Value* b)
{
    // ex int/binary.h:executeComparison
    /* Figure out what to do */
    int mask;
    bool caseblind;
    switch (op) {
     case interpreter::biCompareEQ:    mask = cmpEqual;              caseblind = false;  break;
     case interpreter::biCompareEQ_NC: mask = cmpEqual;              caseblind = true;   break;
     case interpreter::biCompareNE:    mask = cmpGreater | cmpLess;  caseblind = false;  break;
     case interpreter::biCompareNE_NC: mask = cmpGreater | cmpLess;  caseblind = true;   break;
     case interpreter::biCompareLE:    mask = cmpLess | cmpEqual;    caseblind = false;  break;
     case interpreter::biCompareLE_NC: mask = cmpLess | cmpEqual;    caseblind = true;   break;
     case interpreter::biCompareLT:    mask = cmpLess;               caseblind = false;  break;
     case interpreter::biCompareLT_NC: mask = cmpLess;               caseblind = true;   break;
     case interpreter::biCompareGE:    mask = cmpGreater | cmpEqual; caseblind = false;  break;
     case interpreter::biCompareGE_NC: mask = cmpGreater | cmpEqual; caseblind = true;   break;
     case interpreter::biCompareGT:    mask = cmpGreater;            caseblind = false;  break;
     case interpreter::biCompareGT_NC: mask = cmpGreater;            caseblind = true;   break;
     default:
        throw Error::internalError("invalid binary operation");
    }

    /* Do it */
    Comparison result = compare(a, b, caseblind);
    if (result == cmpNull) {
        return -1;
    } else {
        return (result & mask) != 0;
    }
}
