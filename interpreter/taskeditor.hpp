/**
  *  \file interpreter/taskeditor.hpp
  *  \brief Class interpreter::TaskEditor
  */
#ifndef C2NG_INTERPRETER_TASKEDITOR_HPP
#define C2NG_INTERPRETER_TASKEDITOR_HPP

#include "interpreter/basetaskeditor.hpp"

namespace interpreter {

    /** Auto Task editor.
        Translates a process into an editable string list and back.

        This class extends BaseTaskEditor with state management for the process in question.
        TaskEditor implements Process::Freezer and will freeze the process being edited.
        This means there can be at most one TaskEditor for each process,
        and only suspended processes can be edited.

        If you wish to edit a process that is already being edited,
        you can obtain the TaskEditor created by someone else.

        To manage lifetime, TaskEditor implements RefCounted (through BaseTaskEditor). */
    class TaskEditor : public BaseTaskEditor, public Process::Freezer {
     public:
        /** Constructor.
            \param proc Process. Needs to be suspended and not already have a TaskEditor
            \param salvageable Whether task should be salvageable (=have a CC$AutoSalvage command) */
        TaskEditor(Process& proc, bool salvageable);

        /** Destructor.
            This will update the process and mark it suspended again. */
        ~TaskEditor();

        /** Access process.
            \return process */
        Process& process() const;

     private:
        Process& m_process;
        bool m_salvageable;
    };

}

#endif
