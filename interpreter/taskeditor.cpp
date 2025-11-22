/**
  *  \file interpreter/taskeditor.cpp
  *  \brief Class interpreter::TaskEditor
  */

#include "interpreter/taskeditor.hpp"
#include "interpreter/error.hpp"

// Constructor.
interpreter::TaskEditor::TaskEditor(Process& proc, bool salvageable)
    : BaseTaskEditor(),
      m_process(proc),
      m_salvageable(salvageable)
{
    m_process.freeze(*this);
    if (!load(m_process)) {
        throw Error("Process cannot be edited");
    }
}

// Destructor.
interpreter::TaskEditor::~TaskEditor()
{
    if (isChanged()) {
        save(m_process, m_salvageable);
    }
    m_process.unfreeze();
}

// Access process.
interpreter::Process&
interpreter::TaskEditor::process() const
{
    return m_process;
}
