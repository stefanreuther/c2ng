/**
  *  \file interpreter/basetaskeditor.cpp
  *  \brief Class interpreter::BaseTaskEditor
  */

#include "interpreter/basetaskeditor.hpp"

#include "afl/data/stringvalue.hpp"
#include "interpreter/bytecodeobject.hpp"
#include "interpreter/error.hpp"
#include "interpreter/keywords.hpp"
#include "interpreter/opcode.hpp"
#include "interpreter/tokenizer.hpp"
#include "interpreter/world.hpp"

using interpreter::BytecodeObject;
using interpreter::Opcode;

namespace {
    const char*const LOG_NAME = "script.task";

    /* For decompilation: */

    /** Check for 'pushlit' instruction. */
    bool isPushLiteral(const BytecodeObject& bco, BytecodeObject::PC_t pc)
    {
        const Opcode& opc = bco(pc);
        return opc.major == Opcode::maPush
            && opc.minor == Opcode::sLiteral;
    }

    /** Check for instruction that pushes a global variable. Accepted encodings are:
        - pushvar NAME
        - pushgvar NAME

        @change PCC2 also accepts 'pushglob #NAME'. For simplicity, we don't accept that.
        So far, nobody creates 'pushglob' (in particular, we don't) because it doesn't
        play nicely with serialisation, and supporting it would mean we'd have to carry
        a World around. */
    bool isPushGlobal(const BytecodeObject& bco, BytecodeObject::PC_t pc, const char* name)
    {
        const Opcode& opc = bco(pc);
        const String_t* foundName;
        return opc.major == Opcode::maPush
            && (opc.minor == Opcode::sNamedVariable
                || opc.minor == Opcode::sNamedShared)
            && (foundName = bco.getNameByIndex(opc.arg)) != 0
            && *foundName == name;
    }

    /** Check for 'callind NARGS' instruction. */
    bool isCall(const BytecodeObject& bco, BytecodeObject::PC_t pc, uint16_t nargs)
    {
        const Opcode& opc = bco(pc);
        return opc.major == Opcode::maIndirect
            && (opc.minor & ~(Opcode::miIMRefuseFunctions | Opcode::miIMRefuseProcedures)) == Opcode::miIMCall
            && opc.arg == nargs;
    }

    /** Check for 'j #0' instruction. */
    bool isJump0(const BytecodeObject& bco, BytecodeObject::PC_t pc)
    {
        const Opcode& opc = bco(pc);
        return opc.major == Opcode::maJump
            && opc.minor == Opcode::jAlways
            && opc.arg == 0;
    }

    /** Update address for moving. */
    size_t updateAddress(size_t addr, size_t from, size_t to, size_t n)
    {
        if (from >= to) {
            //          aaaa                aaaa
            // to>      bbbb     becomes    cccc
            // from>    cccc                bbbb
            // from+n>  dddd                dddd
            if (addr >= from && addr < from+n) {
                // in 'cccc' range
                addr -= from;
                addr += to;
            } else if (addr >= to && addr < from) {
                // in 'bbbb' range
                addr += n;
            } else {
                // 'aaaa' or 'dddd', unaffected
            }
        } else {
            //          aaaa               aaaa
            // from>    bbbb     becomes   cccc
            // from+n>  cccc               bbbb
            // to>      dddd               dddd
            if (addr >= from && addr < from+n) {
                // in 'bbbb' range
                addr -= from;
                addr += to;
                addr -= n;
            } else if (addr >= from+n && addr < to) {
                // in 'cccc' range
                addr -= n;
            } else {
                // 'aaaa' or 'dddd', unaffected
            }
        }
        return addr;
    }
}

interpreter::BaseTaskEditor::BaseTaskEditor()
    : m_code(),
      m_PC(0),
      m_localPC(0),
      m_cursor(0),
      m_changed(false)
{ }

interpreter::BaseTaskEditor::~BaseTaskEditor()
{ }
// Check whether task was changed.
bool
interpreter::BaseTaskEditor::isChanged() const
{
    return m_changed;
}

// Clear this editor.
void
interpreter::BaseTaskEditor::clear()
{
    // ex IntAutoTaskEditor::clear
    if (!m_code.empty() || m_PC != 0 || m_localPC != 0) {
        clearContent();
        m_changed = true;
        sig_change.raise();
    }
}

// Get number of instructions.
size_t
interpreter::BaseTaskEditor::getNumInstructions() const
{
    // ex IntAutoTaskEditor::getNumInstructions
    return m_code.size();
}

// Get program counter.
size_t
interpreter::BaseTaskEditor::getPC() const
{
    // ex IntAutoTaskEditor::getPC
    return m_PC;
}

// Get cursor.
size_t
interpreter::BaseTaskEditor::getCursor() const
{
    return m_cursor;
}

// Check for subroutine call.
bool
interpreter::BaseTaskEditor::isInSubroutineCall() const
{
    // ex IntAutoTaskEditor::isInSubroutineCall
    return m_localPC != 0;
}

// Access instructon.
const String_t&
interpreter::BaseTaskEditor::operator[](size_t index) const
{
    return m_code[index];
}

// Get all instructions.
void
interpreter::BaseTaskEditor::getAll(afl::data::StringList_t& out) const
{
    for (size_t i = 0, n = m_code.size(); i < n; ++i) {
        out.push_back(m_code[i]);
    }
}

// Update command list.
void
interpreter::BaseTaskEditor::replace(size_t pos, size_t nold, Commands_t lines, CursorBehaviour cursor, PCBehaviour pc)
{
    // ex IntAutoTaskEditor::replace
    // FIXME: possible aliasing problem: "x.replace(pos, 0, Commands_t::fromSingleObject(x[i]), cursor, pc)"
    // will re-allocate and thus invalidate the reference.

    /* Set parameters / validate */
    const String_t*const line = lines.unsafeData();
    const size_t nnew = lines.size();
    pos = std::min(pos, m_code.size());
    nold = std::min(nold, m_code.size() - pos);

    /* Update vector size */
    if (nnew > nold) {
        m_code.insert(m_code.begin() + pos, nnew - nold, String_t());
    }
    if (nnew < nold) {
        m_code.erase(m_code.begin() + pos, m_code.begin() + pos + nold - nnew);
    }

    /* Update vector content */
    for (size_t i = 0; i < nnew; ++i) {
        m_code[pos + i] = line[i];
    }

    /* Handle PC */
    switch (pc) {
     case DefaultPC:
        if (m_PC >= pos && m_PC < pos+nold) {
            /* PC is within modified area */
            m_PC = pos;
            m_localPC = 0;
        } else if (m_PC >= pos+nold) {
            /* PC is after modified area */
            if (pos == 0 && m_code.size() <= nnew) {
                /* Special case: we have inserted after the end */
                m_PC = 0;
                m_localPC = 0;
            } else {
                m_PC -= nold;
                m_PC += nnew;
            }
        }
        break;

     case PlacePCBefore:
        m_PC = pos;
        m_localPC = 0;
        break;
    }

    /* Update cursor */
    switch (cursor) {
     case DefaultCursor:
        // Update cursor. We have the following cases:
        if (m_cursor < pos) {
            // cursor is before change; no modification
        } else if (m_cursor < pos+nold) {
            // cursor is within changed area: move to beginning of change
            m_cursor = pos;
        } else {
            // cursor is after changed area
            m_cursor -= nold;
            m_cursor += nnew;
        }
        break;

     case PlaceCursorAfter:
        m_cursor = pos + nnew;
        break;
    }

    m_changed = true;
    sig_change.raise();
}

// Move commands.
void
interpreter::BaseTaskEditor::move(size_t from, size_t to, size_t n)
{
    // Limit positions
    if (to > m_code.size() || from > m_code.size()) {
        return;
    }

    if (from <= to) {
        // Cannot move more than distance between from and to
        n = std::min(n, to - from);
    } else {
        // Cannot move more than remaining size
        n = std::min(n, m_code.size() - from);
    }

    // No-op?
    if (n == 0 || from == to) {
        return;
    }

    // Do it
    std::vector<String_t> tmp(m_code.begin() + from, m_code.begin() + (from+n));
    m_code.insert(m_code.begin() + to, tmp.begin(), tmp.end());
    if (from <= to) {
        m_code.erase(m_code.begin() + from, m_code.begin() + (from+n));
    } else {
        m_code.erase(m_code.begin() + (from+n), m_code.begin() + (from+2*n));
    }

    // Update PC
    m_PC = updateAddress(m_PC, from, to, n);
    m_cursor = updateAddress(m_cursor, from, to, n);

    m_changed = true;
    sig_change.raise();
}

// Set program counter.
void
interpreter::BaseTaskEditor::setPC(size_t newPC)
{
    // ex IntAutoTaskEditor::setPC
    if (newPC < m_code.size()) {
        if (m_PC != newPC || m_localPC != 0) {
            m_PC = newPC;
            m_localPC = 0;
            m_changed = true;
            sig_change.raise();
        }
    }
}

// Set cursor.
void
interpreter::BaseTaskEditor::setCursor(size_t newCursor)
{
    size_t effCursor = std::min(newCursor, m_code.size());
    if (effCursor != m_cursor) {
        m_cursor = effCursor;
        sig_change.raise();
    }
}

// Add command as current command.
void
interpreter::BaseTaskEditor::addAsCurrent(Commands_t lines)
{
    replace(getPC(), 0, lines, PlaceCursorAfter, PlacePCBefore);
}

// Add command at end of task.
void
interpreter::BaseTaskEditor::addAtEnd(Commands_t lines)
{
    // Insert at end. Insert before any Restart command. Skip blanks, too.
    size_t pos = getNumInstructions();
    while (pos > 0
           && (isRestartCommand(m_code[pos-1])
               || isBlankCommand(m_code[pos-1])))
    {
        --pos;
    }
    replace(pos, 0, lines, PlaceCursorAfter, DefaultPC);
}

// Check whether a command is allowed in an auto task.
bool
interpreter::BaseTaskEditor::isValidCommand(const String_t& cmd)
{
    // ex IntAutoTaskEditor::isValidCommand, autotask.pas:VerifyCommand
    try {
        Tokenizer tok(cmd);

        // Allow empty commands. Why not.
        if (tok.getCurrentToken() == tok.tEnd) {
            return true;
        }

        // Commands must start with an identifier.
        // (PCC 1.x also accepts anything starting with a non-identifier.)
        if (tok.getCurrentToken() != tok.tIdentifier) {
            return false;
        }

        // Reject commands that could possibly be multi-line.
        // This is a superset of the PCC 1.x restriction.
        switch (lookupKeyword(tok.getCurrentString())) {
         case kwBreak:          // not in 1.x
         case kwCase:
         case kwContinue:       // not in 1.x
         case kwDo:
         case kwElse:
         case kwEndFunction:    // not in 1.x
         case kwEndIf:
         case kwEndSelect:
         case kwEndSub:
         case kwEndTry:
         case kwEndWith:
         case kwFor:
         case kwForEach:
         case kwFunction:       // not in 1.x
         case kwIf:
         case kwLoop:
         case kwNext:
         case kwReturn:         // not in 1.x
         case kwSelect:
         case kwSub:
         case kwTry:
         case kwUntil:          // not in 1.x
         case kwWhile:          // not in 1.x
         case kwWith:
            return false;
         default:
            break;
        }

        // While we have a tokenizer handy, check that the whole line tokenizes well.
        while (tok.getCurrentToken() != tok.tEnd) {
            if (tok.getCurrentToken() == tok.tInvalid) {
                return false;
            }
            tok.readNextToken();
        }
        return true;
    }
    catch (Error&) {
        return false;
    }
}

// Check for 'Restart' command.
bool
interpreter::BaseTaskEditor::isRestartCommand(const String_t& cmd)
{
    // ex IntAutoTaskEditor::isRestartCommand
    try {
        Tokenizer tok(cmd);
        return tok.getCurrentToken() == tok.tIdentifier
            && tok.getCurrentString() == "RESTART"
            && tok.readNextToken() == tok.tEnd;
    }
    catch (...) {
        return false;
    }
}

// Check for blank command.
bool
interpreter::BaseTaskEditor::isBlankCommand(const String_t& cmd)
{
    // ex IntAutoTaskEditor::isBlankCommand
    try {
        Tokenizer tok(cmd);
        return tok.getCurrentToken() == tok.tEnd;
    }
    catch (...) {
        return false;
    }
}

void
interpreter::BaseTaskEditor::clearContent()
{
    m_code.clear();
    m_PC = 0;
    m_localPC = 0;
    m_changed = false;
}

// Load from process.
bool
interpreter::BaseTaskEditor::load(const Process& proc)
{
    // ex IntAutoTaskEditor::loadFromProcess

    // There is no need to validate exception frames or contexts; those can
    // only be produced when the code contains appropriate
    // instructions, and if it does, it fails our syntax check.

    // Start empty.
    clearContent();

    // Must have at least one frame. Zero frames means an empty process, which is a valid auto task.
    if (proc.getNumActiveFrames() < 1) {
        return true;
    }
    const Process::Frame* frame = proc.getOutermostFrame();
    BCORef_t bco = frame->bco;

    // Parse the code. Note that I try to avoid the assumption that
    // 'Restart' and regular commands are compiled to instructions
    // of the same length, so this could easier be changed later.
    BytecodeObject::PC_t raw_pc = 0;
    while (BytecodeObject::PC_t remaining = bco->getNumInstructions() - raw_pc) {
        // Check for regular command
        if (remaining >= 3
            && isPushLiteral(*bco, raw_pc)
            && isPushGlobal(*bco, raw_pc+1, "CC$AUTOEXEC")
            && isCall(*bco, raw_pc+2, 1))
        {
            afl::data::StringValue* sv = dynamic_cast<afl::data::StringValue*>(bco->getLiteral((*bco)(raw_pc).arg));
            if (sv != 0) {
                checkSetPC(proc, raw_pc, 3);
                m_code.push_back(sv->getValue());
                raw_pc += 3;
                continue;
            }
        }

        // Check for Restart command
        if (remaining >= 3
            && isPushGlobal(*bco, raw_pc, "CC$AUTORECHECK")
            && isCall(*bco, raw_pc+1, 0)
            && isJump0(*bco, raw_pc+2))
        {
            checkSetPC(proc, raw_pc, 3);
            m_code.push_back("Restart");
            raw_pc += 3;
            continue;
        }

        // When we're here, the instruction cannot be interpreted. Fail.
        clearContent();
        return false;
    }
    m_cursor = m_code.size();

    return true;
}

// Save to process.
void
interpreter::BaseTaskEditor::save(Process& proc) const
{
    // ex IntAutoTaskEditor::saveToProcess
    try {
        // Generate new BCO
        BCORef_t bco = BytecodeObject::create(true);
        bco->setSubroutineName(proc.getName());
        BytecodeObject::PC_t new_pc = 0;
        for (size_t i = 0; i < m_code.size(); ++i) {
            // Is this the new program counter?
            if (i == m_PC) {
                new_pc = bco->getNumInstructions() + m_localPC;
            }

            // Generate code
            if (isRestartCommand(m_code[i])) {
                // Encode restart operation
                bco->addInstruction(Opcode::maPush, Opcode::sNamedShared, bco->addName("CC$AUTORECHECK"));
                bco->addInstruction(Opcode::maIndirect, Opcode::miIMCall, 0);
                bco->addInstruction(Opcode::maJump, Opcode::jAlways, 0);
            } else {
                // Encode normal operation
                afl::data::StringValue sv(m_code[i]);
                bco->addPushLiteral(&sv);
                bco->addInstruction(Opcode::maPush, Opcode::sNamedShared, bco->addName("CC$AUTOEXEC"));
                bco->addInstruction(Opcode::maIndirect, Opcode::miIMCall, 1);
            }
        }

        // PC could be after end of task
        if (m_PC == m_code.size()) {
            new_pc = bco->getNumInstructions();
        }

        // Check active frames
        if (proc.getNumActiveFrames() < 1) {
            // No frame at all. This means the process was newly created.
            proc.pushFrame(bco, false);
        } else if (proc.getNumActiveFrames() > 1 && m_localPC == 0) {
            // We're inside a call, but the new PC is outside. Drop all frames.
            while (proc.getNumActiveFrames() > 1) {
                proc.popFrame();
            }
            // popFrame does not pop the value stack. Since we're at the beginning
            // of an instruction sequence, the stack ought to be empty.
            while (proc.getStackSize() > 0) {
                proc.dropValue();
            }
        } else {
            // Don't change the frame sequence
        }

        // Fix up outermost frame
        Process::Frame* frame = proc.getOutermostFrame();
        frame->pc = new_pc;
        frame->bco.reset(*bco);
    }
    catch (Error& e) {
        // We must not throw exceptions; we're doing this from the destructor.
        // This could happen if the auto-task is very long (>64k literals).
        // Given that auto-tasks are now scriptable, playful users can do this.
        proc.world().logListener().write(afl::sys::LogListener::Warn, LOG_NAME, "Error saving task", e);
    }
    m_changed = false;
}

/** Decompiler: Check and set program counter from parsed process.
    This will place the user PC at the end of /m_code/ when the real PC
    is within the range described by raw_pc/length. */
void
interpreter::BaseTaskEditor::checkSetPC(const Process& proc, Process::PC_t raw_pc, Process::PC_t length)
{
    // ex IntAutoTaskEditor::checkSetPC
    if (const Process::Frame* f = proc.getOutermostFrame()) {
        Process::PC_t real_pc = f->pc;
        bool found = false;
        if (proc.getNumActiveFrames() == 1) {
            // One frame.
            if (real_pc >= raw_pc && real_pc < raw_pc + length) {
                // real PC is within interval
                found = true;
            }
        } else {
            // More than one frame.
            if (real_pc > raw_pc && real_pc <= raw_pc + length) {
                // real PC is one-after an instruction within the interval
                found = true;
            }
        }

        if (found) {
            m_PC = m_code.size();
            m_localPC = real_pc - raw_pc;
        }
    }
}
