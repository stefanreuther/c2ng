/**
  *  \file interpreter/basetaskeditor.hpp
  *  \brief Class interpreter::BaseTaskEditor
  */
#ifndef C2NG_INTERPRETER_BASETASKEDITOR_HPP
#define C2NG_INTERPRETER_BASETASKEDITOR_HPP

#include <vector>
#include "afl/base/memory.hpp"
#include "afl/base/refcounted.hpp"
#include "afl/base/signal.hpp"
#include "afl/base/types.hpp"
#include "afl/data/stringlist.hpp"
#include "afl/string/string.hpp"
#include "interpreter/process.hpp"

namespace interpreter {

    /** Auto Task editor.
        Translates a process into an editable string list and back.

        An auto task is presented to the user as a list of commands (strings).
        A user program counter identifies the line currently being worked on.
        Execution may be beginning at that line, or have already started; see isInSubroutineCall().

        In addition, a BaseTaskEditor maintains a cursor.
        This is a feature of the task editor user interface;
        having it here reduces the number of abstractions to deal with.

        Internally, task commands are compiled into a sequence of interpreter instructions:
        - regular commands produce
            pushlit 'the command'
            pushlit CC$AUTOEXEC
            callind 1
        - the 'restart' command produces
             pushlit CC$AUTORECHECK
             callind 0
             j #0

        BaseTaskEditor converts to and from this format.

        You need to manage lifetime yourself, that is,
        - create the BaseTaskEditor;
        - load() a process;
        - save() a process;
        - restart the process.

        For more complex operations, you should use TaskEditor,
        which associates itself with the Process in a RAII fashion.

        To aid in lifetime management, BaseTaskEditor implements RefCounted. */
    class BaseTaskEditor : public afl::base::RefCounted {
     public:
        /** Shortcut for passing in a list of commands. */
        typedef afl::base::Memory<const String_t> Commands_t;

        /** Cursor behaviour for modifications. */
        enum CursorBehaviour {
            DefaultCursor,                ///< Default behaviour: if cursor is in modified range, place it at the beginning of the modification.
            PlaceCursorAfter              ///< Place cursor after modification.
        };

        /** Program counter behaviour for modifications. */
        enum PCBehaviour {
            DefaultPC,                    ///< Default behaviour: if PC is in modified range, place it at the beginning of the modification.
            PlacePCBefore                 ///< Place PC at the beginning of the modification.
        };

        /** Constructor. */
        BaseTaskEditor();

        /** Destructor. */
        ~BaseTaskEditor();

        /** Check whether task was changed.
            \return status */
        bool isChanged() const;

        /** Clear this editor (remove all commands). */
        void clear();

        /** Get number of instructions.
            \return number of instructions */
        size_t getNumInstructions() const;

        /** Get program counter.
            \return program counter (index into instruction list) */
        size_t getPC() const;

        /** Get cursor.
            \return cursor */
        size_t getCursor() const;

        /** Check for subroutine call.
            \retval false Task is at start of an instruction pointed to by program counter (execution has not started yet)
            \retval true  Task is inside the instruction pointed to by program counter (execution already began) */
        bool isInSubroutineCall() const;

        /** Access instructon.
            \param index Index [0,getPC())
            \return instruction text */
        const String_t& operator[](size_t index) const;

        /** Get all instructions.
            \param[out] out Instructions */
        void getAll(afl::data::StringList_t& out) const;

        /** Load from process.
            Replaces this BaseTaskEditor's content with the content of the given process.
            Caller is responsible for putting that process into an appropriate state.

            \param proc Process

            \retval true Conversion succeeded
            \retval false Conversion failed; this probably is not an auto task */
        bool load(const Process& proc);

        /** Save to process.
            Replaces the process' content with this BaseTaskEditor's.
            Caller is responsible for putting that process into an appropriate state,
            and re-executing it.

            \param proc Process */
        void save(Process& proc, bool salvageable) const;

        /** Update command list.
            Replace \c nold lines starting at \c pos by new \c lines.
            This can be used for insertion (nold=0), deletion (lines.empty()), or replacement in any combination.

            \param pos Position to modify [0,getNumInstructions()]
            \param nold Number of instructions to erase
            \param lines Instructions to insert
            \param cursor Cursor behaviour
            \param pc Program counter behaviour */
        void replace(size_t pos, size_t nold, Commands_t lines, CursorBehaviour cursor, PCBehaviour pc);

        /** Move commands.
            Moves the given lines.
            If the given range includes the cursor, moves that as well.

            \param from  Index of first element to move [0,getNumInstructions()]
            \param to    Insert moved elements before this index [0,getNumInstructions()]
            \param n     Number of elements to move

            If a position is out of range, call is ignored.
            If number is out of range, it is limited to maximum possible. */
        void move(size_t from, size_t to, size_t n);

        /** Set program counter.
            This will set the PC to the beginning (!isInSubroutineCall()) the specified instruction.
            \param newPC new PC [0,getNumInstructions()) */
        void setPC(size_t newPC);

        /** Set cursor.
            \param newCursor [0,getNumInstructions()] */
        void setCursor(size_t newCursor);

        /** Add command as current command.
            \param lines Commands to add */
        void addAsCurrent(Commands_t lines);

        /** Add command at end of task.
            \param lines Commands to add */
        void addAtEnd(Commands_t lines);

        /** Check whether a command is allowed in an auto task.
            Refuses commands that are syntactically invalid, and commands which are obviously not procedure calls.

            We have to refuse multi-line commands because they obviously will not work when wrapped into CC$AUTOEXEC calls line-by-line.
            To avoid the need to reliably distinguish one-line and multi-line, we refuse structural commands completely.
            This is the same restriction as in PCC 1.x, although for a different reason
            (in 1.x, structural commands affect the runtime context stack in a way the editor cannot handle).
            We refuse a few commands more than PCC 1.x, but the additional commands would not have worked in 1.x as well.

            \param cmd Command to check
            \return true iff command is valid */
        static bool isValidCommand(const String_t& cmd);

        /** Check for 'Restart' command.
            \param cmd Command to check
            \return true iff command is 'Restart' */
        static bool isRestartCommand(const String_t& cmd);

        /** Check for blank command.
            \param cmd Command to check
            \return true iff command line is blank */
        static bool isBlankCommand(const String_t& cmd);

        /** Signal: change.
            Invoked whenever the contained auto task code changes. */
        afl::base::Signal<void()> sig_change;

     private:
        // Auto task code.
        std::vector<String_t> m_code;

        // Current position. m_PC is the perceived program counter, m_localPC
        // is an (opaque) identifier for the relative position of the
        // program after m_PC. 0 means we're sitting exactly at m_PC, >0 means
        // we're inside the command.
        size_t m_PC;
        size_t m_localPC;
        size_t m_cursor;

        mutable bool m_changed;

        void clearContent();
        void checkSetPC(const Process& proc, Process::PC_t raw_pc, Process::PC_t length);
    };


}

#endif
