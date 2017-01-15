/**
  *  \file interpreter/bytecodeobject.cpp
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

namespace {
    /** Find literal within data segment.
        \param dseg Data segment
        \param value Literal to find, must not be null
        \return index such that data[index] equals value. -1 if none found. */
    static int32_t
    findLiteral(const afl::data::Segment& dseg, const afl::data::Value* value)
    {
        // Check at most 20 previous literals. This is to limit the amount of
        // time taken when compiling. As of 20100711, time taken and object file
        // sizes:                     core.q               selftest.q
        // - no literal merging    0.05s (28841 bytes)  0.08s (80985 bytes)
        // - max 20 literals       0.05s (25990 bytes)  0.11s (75894 bytes)
        // - full check            0.08s (24805 bytes)  0.90s (73186 bytes)
        // That is, we get roughly 2/3 of the savings at 1/30 of the cost.
        uint32_t first = 0;
        if (dseg.size() > 20) {
            first = dseg.size() - 20;
        }

        if (const afl::data::IntegerValue* iv = dynamic_cast<const afl::data::IntegerValue*>(value)) {
            for (uint32_t i = first; i < dseg.size(); ++i) {
                if (const afl::data::IntegerValue* iv2 = dynamic_cast<const afl::data::IntegerValue*>(dseg[i]))
                    if (iv2->getValue() == iv->getValue())
                        return i;
            }
            return -1;
        } else if (const afl::data::FloatValue* fv = dynamic_cast<const afl::data::FloatValue*>(value)) {
            for (uint32_t i = first; i < dseg.size(); ++i) {
                if (const afl::data::FloatValue* fv2 = dynamic_cast<const afl::data::FloatValue*>(dseg[i]))
                    if (fv2->getValue() == fv->getValue())
                        return i;
            }
            return -1;
        } else if (const afl::data::StringValue* sv = dynamic_cast<const afl::data::StringValue*>(value)) {
            for (uint32_t i = first; i < dseg.size(); ++i) {
                if (const afl::data::StringValue* sv2 = dynamic_cast<const afl::data::StringValue*>(dseg[i]))
                    if (sv2->getValue() == sv->getValue())
                        return i;
            }
            return -1;
        } else {
            return -1;
        }
    }
}




// /** Construct blank BCO. */
interpreter::BytecodeObject::BytecodeObject()
    : m_data(),
      m_names(),
      m_code(),
      num_labels(0),
      m_localNames(),
      m_minArgs(0),
      m_maxArgs(0),
      m_isProcedure(true),
      m_isVarargs(false),
      m_name(),
      file_name(),
      line_numbers()
{
    // ex IntBytecodeObject::IntBytecodeObject
}

// /** Destructor. */
interpreter::BytecodeObject::~BytecodeObject()
{ }

// /** Add named argument.
//     \param name Name of argument
//     \param optional true if this argument is optional

//     addArgument(name, true) should not be followed by addArgument(name, false). */
void
interpreter::BytecodeObject::addArgument(String_t name, bool optional)
{
    m_localNames.add(name);
    m_maxArgs = m_localNames.getNumNames();
    if (!optional) {
        m_minArgs = m_localNames.getNumNames();
    }
}

// /** Get name of this BCO. Note that although this is a procedure name, it may not
//     correspond with the current content of that procedure. */
String_t
interpreter::BytecodeObject::getName() const
{
    // ex IntBytecodeObject::getName
    return m_name;
}

// /** Set name of this BCO. */
void
interpreter::BytecodeObject::setName(String_t name)
{
    // ex IntBytecodeObject::setName
    m_name = name;
}

// /** Get file name of this BCO. */
String_t
interpreter::BytecodeObject::getFileName() const
{
    // ex IntBytecodeObject::getFileName
    return file_name;
}

// /** Set file name of this BCO. */
void
interpreter::BytecodeObject::setFileName(String_t fileName)
{
    // ex IntBytecodeObject::setFileName
    this->file_name = fileName;
}

// /** Notice current line number for debugging. */
void
interpreter::BytecodeObject::addLineNumber(uint32_t line)
{
    // ex IntBytecodeObject::addLineNumber
    uint32_t address = m_code.size();

    if (line_numbers.size() == 0
        || (line != line_numbers[line_numbers.size()-1] && address != line_numbers[line_numbers.size()-2]))
    {
        /* First pair, or new line at new address */
        line_numbers.push_back(address);
        line_numbers.push_back(line);
    } else if (address == line_numbers[line_numbers.size()-2]) {
        /* Same address as last pair, i.e. last line compiled to 0 instructions */
        line_numbers[line_numbers.size()-1] = line;
    } else {
        /* Same line as last pair, but different address, i.e. nested statement */
    }
}

// add line number, for loading
void
interpreter::BytecodeObject::addLineNumber(uint32_t line, uint32_t pc)
{
    line_numbers.push_back(pc);
    line_numbers.push_back(line);
}


// /** Get line number for a particular PC. */
uint32_t
interpreter::BytecodeObject::getLineNumber(PC_t pc) const
{
    // ex IntBytecodeObject::getLineNumber
    /* Slow and simple */
    if (line_numbers.size() == 0 || pc < line_numbers[0]) {
        return 0;
    }

    uint32_t i = 0;
    while (i+2 < line_numbers.size() && pc >= line_numbers[i+2]) {
        i += 2;
    }
    return line_numbers[i+1];
}

// /** Add an instruction. */
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

// /** Add variable-referencing instruction.
//     \param major Major opcode
//     \param name  Name of referenced variable
//     \param cc    Compilation context */
void
interpreter::BytecodeObject::addVariableReferenceInstruction(Opcode::Major major, const String_t& name, const CompilationContext& cc)
{
    if (cc.hasFlag(CompilationContext::LocalContext)) {
        // Is it a local variable?
        afl::data::NameMap::Index_t ix = m_localNames.getIndexByName(name);
        if (ix != m_localNames.nil) {
            addInstruction(major, Opcode::sLocal, ix);
            return;
        }

        // Is it a global variable?
        if (cc.hasFlag(CompilationContext::AlsoGlobalContext)) {
            ix = cc.world().globalPropertyNames().getIndexByName(name);
            if (ix != m_localNames.nil) {
                addInstruction(major, Opcode::sShared, ix);
                return;
            }
        }
    }

    // Shortcut not possible
    addInstruction(major, Opcode::sNamedVariable, addName(name));
}

// /** Place a label. */
void
interpreter::BytecodeObject::addLabel(Label_t label)
{
    // ex IntBytecodeObject::addLabel
    addInstruction(Opcode::maJump, Opcode::jSymbolic, label);
}

// /** Add a jump. */
void
interpreter::BytecodeObject::addJump(uint8_t flags, Label_t label)
{
    // ex IntBytecodeObject::addJump
    addInstruction(Opcode::maJump, flags | Opcode::jSymbolic, label);
}

// /** Add a "push literal" instruction. Checks whether the literal can be
//     encoded as a single instruction; otherwise generates a new literal table slot. */
void
interpreter::BytecodeObject::addPushLiteral(afl::data::Value* literal)
{
    // ex IntBytecodeObject::addPushLiteral
    // FIXME: can we use visit() instead of dynamic_cast here?

    // Is it empty?
    if (literal == 0) {
        addInstruction(Opcode::maPush, Opcode::sBoolean, uint16_t(-1));
        return;
    }

    // Is it a small integer literal?
    if (afl::data::ScalarValue* sv = dynamic_cast<afl::data::ScalarValue*>(literal)) {
        if (sv->getValue() >= -int32_t(0x7FFF) && sv->getValue() <= int32_t(0x7FFF)) {
            if (dynamic_cast<afl::data::BooleanValue*>(sv) != 0) {
                addInstruction(Opcode::maPush, Opcode::sBoolean, sv->getValue());
                return;
            }
            if (dynamic_cast<afl::data::IntegerValue*>(sv) != 0) {
                addInstruction(Opcode::maPush, Opcode::sInteger, sv->getValue());
                return;
            }
        }
    }

    // None of the above, so use general way
    int32_t existing = findLiteral(m_data, literal);
    if (existing >= 0) {
        // Recycle existing literal
        addInstruction(Opcode::maPush, Opcode::sLiteral, existing);
    } else {
        // FIXME: check 16-bit range
        m_data.pushBack(literal);
        addInstruction(Opcode::maPush, Opcode::sLiteral, m_data.size()-1);
    }
}

// /** Check whether this BCO contains any instruction that potentially calls user code.
//     This can be
//     - callind, pushind, procind (for convenience, any maIndirect)
//     - sevalx
//     - sevals
//     - srunhook */
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

// /** Turn symbolic references into absolute references. Also eliminates
//     null operations (absolute labels). */
void
interpreter::BytecodeObject::relocate()
{
    // ex IntBytecodeObject::relocate
    // FIXME: can we degrade gracefully when there's more than 64k instructions?

    std::vector<PC_t> addresses(num_labels, PC_t(m_code.size()));

    // Find existing labels
    PC_t outAdr = 0;
    for (PC_t i = 0; i != m_code.size(); ++i) {
        if (m_code[i].isLabel()) {
            /* It's a label. Those do not produce output. */
            if (m_code[i].minor & Opcode::jSymbolic) {
                /* Symbolic label. Note its address. */
                assert(m_code[i].arg < addresses.size());
                addresses[m_code[i].arg] = outAdr;
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
    line_numbers.swap(oldDebug);
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
                c.minor = oldCode[i].minor & ~Opcode::jSymbolic;
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

// /** Compact code. This drops all NOPs. This is a subset of relocate()
//     used for optimisation. */
void
interpreter::BytecodeObject::compact()
{
    // ex IntBytecodeObject::compact
    // Turn symbolic jumps into absolute.
    std::vector<Opcode> oldCode;
    std::vector<uint32_t> oldDebug;
    m_code.swap(oldCode);
    line_numbers.swap(oldDebug);
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

// /** Copy local variables from another BCO. */
void
interpreter::BytecodeObject::copyLocalVariablesFrom(const BytecodeObject& other)
{
    // ex IntBytecodeObject::copyLocals
    for (afl::data::NameMap::Index_t i = 0, e = other.m_localNames.getNumNames(); i != e; ++i) {
        m_localNames.add(other.m_localNames.getNameByIndex(i));
    }
}

// /** Append another byte-code object.
//     Note that this trusts the BCO to contain no invalid opcodes. */
void
interpreter::BytecodeObject::append(const BytecodeObject& other)
{
    // ex IntBytecodeObject::append
    // Remember base address of insertion
    PC_t absBase = m_code.size();
    Label_t symBase = num_labels;

    num_labels += other.num_labels;
    m_code.reserve(m_code.size() + other.m_code.size());

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
                addInstruction(maj, o.minor, m_localNames.addMaybe(other.m_localNames.getNameByIndex(o.arg)));
                break;
             case Opcode::sLiteral:
                // Adjust literal
                addPushLiteral(other.m_data[o.arg]);
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
                addInstruction(maj, o.minor, o.arg + symBase);
            } else {
                addInstruction(maj, o.minor, o.arg + absBase);
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

// /** Find jump target. If the jump is symbolic, looks up the target label.
//     \param minor Minor opcode from the maJump instruction
//     \param arg Parameter */
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

// /** Get the human-readable value of the instruction at address \c index. */
String_t
interpreter::BytecodeObject::getDisassembly(PC_t index, const World& w) const
{
    String_t tpl;
    m_code[index].getDisassemblyTemplate(tpl);

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
                result += " <";
                result += getName(arg);
                result += ">";
                break;
             case 'l': /* literal */
                result += " <";
                result += toString(m_data[arg], true);
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
                if (arg < m_localNames.getNumNames()) {
                    result += " <";
                    result += m_localNames.getNameByIndex(arg);
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
