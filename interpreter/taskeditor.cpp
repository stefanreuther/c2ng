/**
  *  \file interpreter/taskeditor.cpp
  *  \brief Class interpreter::TaskEditor
  */

#include "interpreter/taskeditor.hpp"
#include "afl/data/stringvalue.hpp"
#include "interpreter/bytecodeobject.hpp"
#include "interpreter/error.hpp"
#include "interpreter/keywords.hpp"
#include "interpreter/opcode.hpp"
#include "interpreter/process.hpp"
#include "interpreter/tokenizer.hpp"

using interpreter::BytecodeObject;
using interpreter::Opcode;

namespace {
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
        return opc.major == Opcode::maPush
            && (((opc.minor == Opcode::sNamedVariable
                  || opc.minor == Opcode::sNamedShared)
                 && bco.getName(opc.arg) == name));
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
}

// Constructor.
interpreter::TaskEditor::TaskEditor(Process& proc)
    : m_process(proc),
      m_code(),
      m_PC(0),
      m_localPC(0),
      m_cursor(0),
      m_changed(false)
{
    m_process.freeze(*this);
    if (!load()) {
        throw Error("Process cannot be edited");
    }
}

// Destructor.
interpreter::TaskEditor::~TaskEditor()
{
    if (m_changed) {
        save();
        m_changed = false;
    }
    m_process.unfreeze();
}

// Access process.
interpreter::Process&
interpreter::TaskEditor::process() const
{
    return m_process;
}

// Clear this editor.
void
interpreter::TaskEditor::clear()
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
interpreter::TaskEditor::getNumInstructions() const
{
    // ex IntAutoTaskEditor::getNumInstructions
    return m_code.size();
}

// Get program counter.
size_t
interpreter::TaskEditor::getPC() const
{
    // ex IntAutoTaskEditor::getPC
    return m_PC;
}

// Get cursor.
size_t
interpreter::TaskEditor::getCursor() const
{
    return m_cursor;
}

// Check for subroutine call.
bool
interpreter::TaskEditor::isInSubroutineCall() const
{
    // ex IntAutoTaskEditor::isInSubroutineCall
    return m_localPC != 0;
}

// Access instructon.
const String_t&
interpreter::TaskEditor::operator[](size_t index) const
{
    return m_code[index];
}

// Get all instructions.
void
interpreter::TaskEditor::getAll(afl::data::StringList_t& out) const
{
    for (size_t i = 0, n = m_code.size(); i < n; ++i) {
        out.push_back(m_code[i]);
    }
}

// Update command list.
void
interpreter::TaskEditor::replace(size_t pos, size_t nold, Commands_t lines, CursorBehaviour cursor, PCBehaviour pc)
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

// Set program counter.
void
interpreter::TaskEditor::setPC(size_t newPC)
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
interpreter::TaskEditor::setCursor(size_t newCursor)
{
    size_t effCursor = std::min(newCursor, m_code.size());
    if (effCursor != m_cursor) {
        m_cursor = effCursor;
        sig_change.raise();
    }
}

// Add command as current command.
void
interpreter::TaskEditor::addAsCurrent(Commands_t lines)
{
    replace(getPC(), 0, lines, PlaceCursorAfter, PlacePCBefore);
}

// Add command at end of task.
void
interpreter::TaskEditor::addAtEnd(Commands_t lines)
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
interpreter::TaskEditor::isValidCommand(const String_t& cmd)
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
interpreter::TaskEditor::isRestartCommand(const String_t& cmd)
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
interpreter::TaskEditor::isBlankCommand(const String_t& cmd)
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
interpreter::TaskEditor::clearContent()
{
    m_code.clear();
    m_PC = 0;
    m_localPC = 0;
    m_changed = false;
}

/** Load from process.
    Converts the process into an editable string list, stored in this object.
    \retval true Conversion succeeded
    \retval false Conversion failed; this probably is not an auto task */
bool
interpreter::TaskEditor::load()
{
    // ex IntAutoTaskEditor::loadFromProcess

    // There is no need to validate exception frames or contexts; those can
    // only be produced when the code contains appropriate
    // instructions, and if it does, it fails our syntax check.

    // Start empty.
    clearContent();

    // Must have at least one frame. Zero frames means an empty process, which is a valid auto task.
    if (m_process.getNumActiveFrames() < 1) {
        return true;
    }
    Process::Frame* frame = m_process.getOutermostFrame();
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
                checkSetPC(raw_pc, 3);
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
            checkSetPC(raw_pc, 3);
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

/** Save to process.
    Compiles this string list back into a process.
    The process will usually be an already existing one, which will be modified accordingly. */
void
interpreter::TaskEditor::save() const
{
    // ex IntAutoTaskEditor::saveToProcess
    // Generate new BCO
    BCORef_t bco = BytecodeObject::create(true);
    bco->setSubroutineName(m_process.getName());
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
    if (m_process.getNumActiveFrames() < 1) {
        // No frame at all. This means the process was newly created.
        m_process.pushFrame(bco, false);
    } else if (m_process.getNumActiveFrames() > 1 && m_localPC == 0) {
        // We're inside a call, but the new PC is outside. Drop all frames.
        while (m_process.getNumActiveFrames() > 1) {
            m_process.popFrame();
        }
        // popFrame does not pop the value stack. Since we're at the beginning
        // of an instruction sequence, the stack ought to be empty.
        while (m_process.getStackSize() > 0) {
            m_process.dropValue();
        }
    } else {
        // Don't change the frame sequence
    }

    // Fix up outermost frame
    Process::Frame* frame = m_process.getOutermostFrame();
    frame->pc = new_pc;
    frame->bco.reset(*bco);
}

/** Decompiler: Check and set program counter from parsed process.
    This will place the user PC at the end of /m_code/ when the real PC
    is within the range described by raw_pc/length. */
void
interpreter::TaskEditor::checkSetPC(Process::PC_t raw_pc, Process::PC_t length)
{
    // ex IntAutoTaskEditor::checkSetPC
    if (Process::Frame* f = m_process.getOutermostFrame()) {
        Process::PC_t real_pc = f->pc;
        bool found = false;
        if (m_process.getNumActiveFrames() == 1) {
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
