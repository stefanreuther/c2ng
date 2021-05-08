/**
  *  \file interpreter/bytecodeobject.cpp
  *  \brief Class interpreter::BytecodeObject
  */

#include <cassert>
#include "interpreter/bytecodeobject.hpp"
#include "afl/data/scalarvalue.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/data/booleanvalue.hpp"
#include "afl/data/floatvalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/string/format.hpp"
#include "interpreter/compilationcontext.hpp"
#include "interpreter/world.hpp"
#include "interpreter/values.hpp"
#include "afl/base/optional.hpp"
#include "interpreter/error.hpp"

namespace {
    /** Find literal within data segment.
        \param dseg Data segment
        \param value Literal to find, must not be null
        \return index such that data[index] equals value; Nothing if none found. */
    afl::base::Optional<uint16_t> findLiteral(const afl::data::Segment& dseg, const afl::data::Value* value)
    {
        // Check at most 20 previous literals. This is to limit the amount of
        // time taken when compiling. As of 20100711, time taken and object file
        // sizes:                     core.q               selftest.q
        // - no literal merging    0.05s (28841 bytes)  0.08s (80985 bytes)
        // - max 20 literals       0.05s (25990 bytes)  0.11s (75894 bytes)
        // - full check            0.08s (24805 bytes)  0.90s (73186 bytes)
        // That is, we get roughly 2/3 of the savings at 1/30 of the cost.
        size_t first = 0;
        size_t last = dseg.size();
        if (last > 20) {
            first = last - 20;
        }

        if (const afl::data::IntegerValue* iv = dynamic_cast<const afl::data::IntegerValue*>(value)) {
            for (size_t i = first; i < last; ++i) {
                if (const afl::data::IntegerValue* iv2 = dynamic_cast<const afl::data::IntegerValue*>(dseg[i]))
                    if (iv2->getValue() == iv->getValue())
                        return uint16_t(i);
            }
            return afl::base::Nothing;
        } else if (const afl::data::FloatValue* fv = dynamic_cast<const afl::data::FloatValue*>(value)) {
            for (size_t i = first; i < last; ++i) {
                if (const afl::data::FloatValue* fv2 = dynamic_cast<const afl::data::FloatValue*>(dseg[i]))
                    if (fv2->getValue() == fv->getValue())
                        return uint16_t(i);
            }
            return afl::base::Nothing;
        } else if (const afl::data::StringValue* sv = dynamic_cast<const afl::data::StringValue*>(value)) {
            for (size_t i = first; i < last; ++i) {
                if (const afl::data::StringValue* sv2 = dynamic_cast<const afl::data::StringValue*>(dseg[i]))
                    if (sv2->getValue() == sv->getValue())
                        return uint16_t(i);
            }
            return afl::base::Nothing;
        } else {
            return afl::base::Nothing;
        }
    }

    template<typename T>
    uint16_t packIndex(T nativeIndex)
    {
        uint16_t packedIndex = uint16_t(nativeIndex);
        if (nativeIndex != packedIndex) {
            throw interpreter::Error::tooComplex();
        }
        return packedIndex;
    }
}




// Constructor.
interpreter::BytecodeObject::BytecodeObject()
    : m_literals(),
      m_names(),
      m_code(),
      m_numLabels(0),
      m_localVariables(),
      m_minArgs(0),
      m_maxArgs(0),
      m_isProcedure(true),
      m_isVarargs(false),
      m_subroutineName(),
      m_fileName(),
      m_origin(),
      m_lineNumbers()
{
    // ex IntBytecodeObject::IntBytecodeObject
}

// Destructor.
interpreter::BytecodeObject::~BytecodeObject()
{ }

// Add named argument.
void
interpreter::BytecodeObject::addArgument(String_t name, bool optional)
{
    m_localVariables.add(name);
    m_maxArgs = m_localVariables.getNumNames();
    if (!optional) {
        m_minArgs = m_localVariables.getNumNames();
    }
}

// Get subroutine name.
String_t
interpreter::BytecodeObject::getSubroutineName() const
{
    // ex IntBytecodeObject::getName
    return m_subroutineName;
}

// Set subroutine name.
void
interpreter::BytecodeObject::setSubroutineName(String_t name)
{
    // ex IntBytecodeObject::setName
    m_subroutineName = name;
}

// Get origin identifier.
String_t
interpreter::BytecodeObject::getOrigin() const
{
    return m_origin;
}

// Set origin identifier.
void
interpreter::BytecodeObject::setOrigin(const String_t& origin)
{
    m_origin = origin;
}

// Get file name.
String_t
interpreter::BytecodeObject::getFileName() const
{
    // ex IntBytecodeObject::getFileName
    return m_fileName;
}

// Set file name.
void
interpreter::BytecodeObject::setFileName(String_t fileName)
{
    // ex IntBytecodeObject::setFileName
    this->m_fileName = fileName;
}

// Remember current line number.
void
interpreter::BytecodeObject::addLineNumber(uint32_t line)
{
    // ex IntBytecodeObject::addLineNumber
    uint32_t address = uint32_t(m_code.size());

    if (m_lineNumbers.size() == 0
        || (line != m_lineNumbers[m_lineNumbers.size()-1] && address != m_lineNumbers[m_lineNumbers.size()-2]))
    {
        /* First pair, or new line at new address */
        m_lineNumbers.push_back(address);
        m_lineNumbers.push_back(line);
    } else if (address == m_lineNumbers[m_lineNumbers.size()-2]) {
        /* Same address as last pair, i.e. last line compiled to 0 instructions */
        m_lineNumbers[m_lineNumbers.size()-1] = line;
    } else {
        /* Same line as last pair, but different address, i.e. nested statement */
    }
}

// Add line/address pair.
void
interpreter::BytecodeObject::addLineNumber(uint32_t line, uint32_t pc)
{
    m_lineNumbers.push_back(pc);
    m_lineNumbers.push_back(line);
}


// Get line number for program counter.
uint32_t
interpreter::BytecodeObject::getLineNumber(PC_t pc) const
{
    // ex IntBytecodeObject::getLineNumber
    /* Slow and simple */
    if (m_lineNumbers.size() == 0 || pc < m_lineNumbers[0]) {
        return 0;
    }

    uint32_t i = 0;
    while (i+2 < m_lineNumbers.size() && pc >= m_lineNumbers[i+2]) {
        i += 2;
    }
    return m_lineNumbers[i+1];
}

// Make a new label for future reference.
interpreter::BytecodeObject::Label_t
interpreter::BytecodeObject::makeLabel()
{
    // ex IntBytecodeObject::makeLabel
    Label_t oldCount = m_numLabels;
    Label_t newCount = Label_t(m_numLabels + 1);
    if (newCount == 0) {
        throw Error::tooComplex();
    }
    m_numLabels = newCount;
    return oldCount;
}

// Add an instruction.
void
interpreter::BytecodeObject::addInstruction(Opcode::Major major, uint8_t minor, uint16_t arg)
{
    // ex IntBytecodeObject::addInstruction
    Opcode o;
    o.major = major;
    o.minor = minor;
    o.arg   = arg;
    m_code.push_back(o);
}

// Add a variable-referencing instruction.
void
interpreter::BytecodeObject::addVariableReferenceInstruction(Opcode::Major major, const String_t& name, const CompilationContext& cc)
{
    if (cc.hasFlag(CompilationContext::LocalContext)) {
        // Is it a local variable?
        afl::data::NameMap::Index_t ix = m_localVariables.getIndexByName(name);
        if (ix != afl::data::NameMap::nil) {
            addInstruction(major, Opcode::sLocal, packIndex(ix));
            return;
        }

        // Is it a global variable?
        if (cc.hasFlag(CompilationContext::AlsoGlobalContext)) {
            ix = cc.world().globalPropertyNames().getIndexByName(name);
            if (ix != afl::data::NameMap::nil) {
                addInstruction(major, Opcode::sShared, packIndex(ix));
                return;
            }
        }
    }

    // Shortcut not possible
    addInstruction(major, Opcode::sNamedVariable, addName(name));
}

// Place a label.
void
interpreter::BytecodeObject::addLabel(Label_t label)
{
    // ex IntBytecodeObject::addLabel
    addInstruction(Opcode::maJump, Opcode::jSymbolic, label);
}

// Insert a label in the middle of code.
void
interpreter::BytecodeObject::insertLabel(Label_t label, PC_t pc)
{
    if (pc <= m_code.size()) {
        // Insert label
        Opcode o;
        o.major = Opcode::maJump;
        o.minor = Opcode::jSymbolic;
        o.arg   = label;
        m_code.insert(m_code.begin() + pc, o);

        // Update debug information
        for (size_t i = 0, n = m_lineNumbers.size(); i < n; i += 2) {
            if (m_lineNumbers[i] >= pc) {
                ++m_lineNumbers[i];
            }
        }
    }
}

// Add jump instruction.
void
interpreter::BytecodeObject::addJump(uint8_t flags, Label_t label)
{
    // ex IntBytecodeObject::addJump
    addInstruction(Opcode::maJump, flags | Opcode::jSymbolic, label);
}

// Add push-literal instruction.
void
interpreter::BytecodeObject::addPushLiteral(const afl::data::Value* literal)
{
    // ex IntBytecodeObject::addPushLiteral
    // FIXME: can we use visit() instead of dynamic_cast here?

    // Is it empty?
    if (literal == 0) {
        addInstruction(Opcode::maPush, Opcode::sBoolean, uint16_t(-1));
        return;
    }

    // Is it a small integer literal?
    if (const afl::data::ScalarValue* sv = dynamic_cast<const afl::data::ScalarValue*>(literal)) {
        if (sv->getValue() >= -int32_t(0x7FFF) && sv->getValue() <= int32_t(0x7FFF)) {
            if (dynamic_cast<const afl::data::BooleanValue*>(sv) != 0) {
                addInstruction(Opcode::maPush, Opcode::sBoolean, uint16_t(sv->getValue()));
                return;
            }
            if (dynamic_cast<const afl::data::IntegerValue*>(sv) != 0) {
                addInstruction(Opcode::maPush, Opcode::sInteger, uint16_t(sv->getValue()));
                return;
            }
        }
    }

    // None of the above, so use general way
    uint16_t existing;
    if (findLiteral(m_literals, literal).get(existing)) {
        // Recycle existing literal
        addInstruction(Opcode::maPush, Opcode::sLiteral, existing);
    } else {
        // FIXME: check 16-bit range
        m_literals.pushBack(literal);
        addInstruction(Opcode::maPush, Opcode::sLiteral, uint16_t(m_literals.size()-1));
    }
}

// Add name (symbol) for later reference.
uint16_t
interpreter::BytecodeObject::addName(String_t name)
{
    // ex IntBytecodeObject::addName
    return packIndex(m_names.addMaybe(name));
}


// Check for potential call into user code.
bool
interpreter::BytecodeObject::hasUserCall() const
{
    // ex IntBytecodeObject::hasUserCall
    for (PC_t i = 0; i != m_code.size(); ++i) {
        if (m_code[i].major == Opcode::maIndirect
            || (m_code[i].major == Opcode::maSpecial
                && (m_code[i].minor == Opcode::miSpecialEvalStatement
                    || m_code[i].minor == Opcode::miSpecialEvalExpr
                    || m_code[i].minor == Opcode::miSpecialRunHook)))
        {
            return true;
        }
    }
    return false;
}

// Turn symbolic references into absolute references.
void
interpreter::BytecodeObject::relocate()
{
    // ex IntBytecodeObject::relocate
    std::vector<uint16_t> addresses(m_numLabels, uint16_t(-1));

    // Find existing labels
    PC_t outAdr = 0;
    for (PC_t i = 0; i != m_code.size(); ++i) {
        if (m_code[i].isLabel()) {
            /* It's a label. Those do not produce output. */
            if (m_code[i].minor & Opcode::jSymbolic) {
                /* Symbolic label. Note its address. */
                assert(m_code[i].arg < addresses.size());
                if (outAdr >= 0x10000) {
                    // Code too large, need to remain in symbolic mode
                    return;
                }
                addresses[m_code[i].arg] = static_cast<uint16_t>(outAdr);
            } else {
                /* Absolute label aka NOP */
            }
        } else {
            ++outAdr;
        }
    }

    // Turn symbolic jumps into absolute.
    std::vector<Opcode> oldCode;
    std::vector<uint32_t> oldDebug;
    m_code.swap(oldCode);
    m_lineNumbers.swap(oldDebug);
    uint32_t dbgIndex = 0;
    for (PC_t i = 0; i != oldCode.size(); ++i) {
        /* Update debug information */
        if (dbgIndex < oldDebug.size() && oldDebug[dbgIndex] == i) {
            addLineNumber(oldDebug[dbgIndex+1]);
            dbgIndex += 2;
        }
        /* Update code */
        if (oldCode[i].major == Opcode::maJump) {
            if (oldCode[i].isLabel()) {
                /* Label. Drop it. */
            } else if ((oldCode[i].minor & Opcode::jSymbolic) != 0) {
                /* Make it absolute */
                assert(oldCode[i].arg < addresses.size());
                Opcode c;
                c.major = oldCode[i].major;
                c.minor = uint8_t(oldCode[i].minor & ~Opcode::jSymbolic);
                c.arg   = addresses[oldCode[i].arg];
                m_code.push_back(c);
            } else {
                /* Already absolute? Should not happen. */
                m_code.push_back(oldCode[i]);
            }
        } else {
            m_code.push_back(oldCode[i]);
        }
    }
}

// Compact code.
void
interpreter::BytecodeObject::compact()
{
    // ex IntBytecodeObject::compact
    // Turn symbolic jumps into absolute.
    std::vector<Opcode> oldCode;
    std::vector<uint32_t> oldDebug;
    m_code.swap(oldCode);
    m_lineNumbers.swap(oldDebug);
    uint32_t dbgIndex = 0;
    for (PC_t i = 0; i != oldCode.size(); ++i) {
        /* Update debug information */
        if (dbgIndex < oldDebug.size() && oldDebug[dbgIndex] == i) {
            addLineNumber(oldDebug[dbgIndex+1]);
            dbgIndex += 2;
        }
        /* Update code */
        if (oldCode[i].major == Opcode::maJump && oldCode[i].minor == Opcode::jLabel) {
            /* Absolute label aka NOP. Drop it. */
        } else {
            /* Instruction. Keep it. */
            m_code.push_back(oldCode[i]);
        }
    }
}

// Copy local variables from another BCO.
void
interpreter::BytecodeObject::copyLocalVariablesFrom(const BytecodeObject& other)
{
    // ex IntBytecodeObject::copyLocals
    for (afl::data::NameMap::Index_t i = 0, e = other.m_localVariables.getNumNames(); i != e; ++i) {
        m_localVariables.add(other.m_localVariables.getNameByIndex(i));
    }
}

// Append code from another BCO.
void
interpreter::BytecodeObject::append(const BytecodeObject& other)
{
    // ex IntBytecodeObject::append
    // Remember base address of insertion
    PC_t absBase = m_code.size();
    Label_t symBase = m_numLabels;

    m_numLabels = packIndex(static_cast<uint32_t>(m_numLabels) + other.m_numLabels);
    m_code.reserve(m_code.size() + other.m_code.size());
    // FIXME: refuse if code size exceeds 16 bit range?

    // Copy the code
    for (PC_t i = 0; i != other.m_code.size(); ++i) {
        const Opcode& o = other.m_code[i];
        switch (const Opcode::Major maj = Opcode::Major(o.major)) {
         case Opcode::maPush:
         case Opcode::maPop:
         case Opcode::maStore:
         case Opcode::maFusedUnary:
         case Opcode::maFusedBinary:
         case Opcode::maFusedComparison2:
         case Opcode::maInplaceUnary:
            // Handle scope
            switch (Opcode::Scope(o.minor)) {
             case Opcode::sNamedVariable:
             case Opcode::sNamedShared:
                // Adjust name reference
                addInstruction(maj, o.minor, addName(other.getName(o.arg)));
                break;
             case Opcode::sLocal:
                // Adjust local by name
                // FIXME: this works as long as all locals are unique or compatible.
                // It will fail when we have overlapping locals that were intended to be unique, which could happen when inlining.
                // We should make up some rules about how the two BCOs have to be related.
                addInstruction(maj, o.minor, packIndex(m_localVariables.addMaybe(other.m_localVariables.getNameByIndex(o.arg))));
                break;
             case Opcode::sLiteral:
                // Adjust literal
                addPushLiteral(other.m_literals[o.arg]);
                break;
             case Opcode::sInteger:
             case Opcode::sBoolean:
             case Opcode::sStatic:
             case Opcode::sShared:
                // Copy verbatim
                m_code.push_back(o);
                break;
            }
            break;
         case Opcode::maBinary:
         case Opcode::maUnary:
         case Opcode::maTernary:
         case Opcode::maStack:
         case Opcode::maIndirect:
         case Opcode::maFusedComparison:
            // Copy verbatim
            m_code.push_back(o);
            break;
         case Opcode::maJump:
            // Adjust argument
            if (o.minor & Opcode::jSymbolic) {
                addInstruction(maj, o.minor, uint16_t(o.arg + symBase));
            } else {
                addInstruction(maj, o.minor, uint16_t(o.arg + absBase));
            }
            break;
         case Opcode::maMemref:
         case Opcode::maDim:
            // Adjust name reference
            addInstruction(maj, o.minor, addName(other.getName(o.arg)));
            break;
         case Opcode::maSpecial:
            switch (Opcode::Special(o.minor)) {
             case Opcode::miSpecialUncatch:
             case Opcode::miSpecialReturn:
             case Opcode::miSpecialWith:
             case Opcode::miSpecialEndWith:
             case Opcode::miSpecialFirstIndex:
             case Opcode::miSpecialNextIndex:
             case Opcode::miSpecialEndIndex:
             case Opcode::miSpecialEvalStatement:
             case Opcode::miSpecialEvalExpr:
             case Opcode::miSpecialLoad:
             case Opcode::miSpecialPrint:
             case Opcode::miSpecialAddHook:
             case Opcode::miSpecialRunHook:
             case Opcode::miSpecialThrow:
             case Opcode::miSpecialTerminate:
             case Opcode::miSpecialSuspend:
             case Opcode::miSpecialNewArray:
             case Opcode::miSpecialMakeList:
             case Opcode::miSpecialNewHash:
             case Opcode::miSpecialInstance:
             case Opcode::miSpecialResizeArray:
             case Opcode::miSpecialBind:
             case Opcode::miSpecialFirst:
             case Opcode::miSpecialNext:
                // Copy verbatim
                m_code.push_back(o);
                break;
             case Opcode::miSpecialDefSub:
             case Opcode::miSpecialDefShipProperty:
             case Opcode::miSpecialDefPlanetProperty:
                // Adjust name reference
                addInstruction(maj, o.minor, addName(other.getName(o.arg)));
                break;
            }
            break;
        }
    }

    // FIXME: copy debug information. Right now, this is not needed because there
    // is no debug information in BCOs we merge.
}

// Find jump target.
interpreter::BytecodeObject::PC_t
interpreter::BytecodeObject::getJumpTarget(uint8_t minor, uint16_t arg) const
{
    if (minor & Opcode::jSymbolic) {
        // Symbolic jump: find label
        for (PC_t i = 0; i != m_code.size(); ++i) {
            if (m_code[i].major == Opcode::maJump && m_code[i].minor == (Opcode::jSymbolic + Opcode::jLabel)) {
                if (m_code[i].arg == arg) {
                    return i;
                }
            }
        }
        return m_code.size();
    } else {
        return arg;
    }
}

// Format instruction in human-readable way.
String_t
interpreter::BytecodeObject::getDisassembly(PC_t index, const World& w) const
{
    String_t tpl = m_code[index].getDisassemblyTemplate();

    String_t result;
    for (String_t::size_type i = 0; i < tpl.size(); ++i) {
        if (tpl[i] == '\t') {
            result.append(12 - (result.size()%12), ' ');
        } else if (tpl[i] == '%' && i+1 < tpl.size()) {
            char mode = tpl[++i];
            uint16_t arg = m_code[index].arg;

            /* Append arg in raw form */
            if (mode == 'd') {
                result += afl::string::Format("%d", int16_t(arg));
            } else {
                result += afl::string::Format("%d", arg);
            }

            /* If we have a hint, append that as well */
            switch (mode) {
             case 'n':
                if (arg < m_names.getNumNames()) {
                    result += " <";
                    result += getName(arg);
                    result += ">";
                } else {
                    result += " !invalid";
                }
                break;
             case 'l': /* literal */
                result += " <";
                result += toString(m_literals[arg], true);
                result += ">";
                break;
             case 'd': /* signed */
             case 'u': /* unsigned */
             case 'T': /* static given by address */
                break;
             case 'G': /* shared given by address */
                if (arg < w.globalPropertyNames().getNumNames()) {
                    result += " <";
                    result += w.globalPropertyNames().getNameByIndex(arg);
                    result += ">";
                }
                break;
             case 'L': /* local given by address */
                if (arg < m_localVariables.getNumNames()) {
                    result += " <";
                    result += m_localVariables.getNameByIndex(arg);
                    result += ">";
                }
                break;
            }
        } else {
            result += tpl[i];
        }
    }

    return result;
}
