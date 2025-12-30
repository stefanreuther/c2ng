/**
  *  \file ui/reshack/tool.cpp
  *  \brief Class ui::reshack::Tool
  */

#include "ui/reshack/tool.hpp"

ui::reshack::Tool::Tool(bool needsPreview, String_t name)
    : m_needsPreview(needsPreview),
      m_name(name)
{ }

ui::reshack::Tool::~Tool()
{ }

bool
ui::reshack::Tool::needsPreview() const
{
    // ex RHTool::needsPreview() const
    return m_needsPreview;
}


const String_t&
ui::reshack::Tool::getName() const
{
    // ex RHTool::getName() const
    return m_name;
}
