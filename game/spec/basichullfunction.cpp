/**
  *  \file game/spec/basichullfunction.cpp
  *  \brief Class game::spec::BasicHullFunction
  */

#include "game/spec/basichullfunction.hpp"

// Constructor.
game::spec::BasicHullFunction::BasicHullFunction(int id, String_t name)
    : m_id(id),
      m_name(name),
      m_description(),
      m_explanation(),
      m_pictureName(),
      m_impliedFunctionId(-1)
{ }

// Destructor.
game::spec::BasicHullFunction::~BasicHullFunction()
{ }

// Set function name.
void
game::spec::BasicHullFunction::setName(const String_t& name)
{
    m_name = name;
}

// Set short description of function.
void
game::spec::BasicHullFunction::setDescription(const String_t& description)
{
    m_description = description;
}

// Set explanation of function.
void
game::spec::BasicHullFunction::setExplanation(const String_t& explanation)
{
    m_explanation = explanation;
}

// Add to explanation.
void
game::spec::BasicHullFunction::addToExplanation(const String_t& explanation)
{
    if (!m_explanation.empty() && m_explanation[m_explanation.size()-1] != '\n') {
        m_explanation += '\n';
    }
    m_explanation += explanation;
}

// Set picture name.
void
game::spec::BasicHullFunction::setPictureName(const String_t& name)
{
    m_pictureName = name;
}

// Set implied function Id.
void
game::spec::BasicHullFunction::setImpliedFunctionId(int impliedFunctionId)
{
    // ex GBasicHullFunction::setImpliedDevice
    m_impliedFunctionId = impliedFunctionId;
}

// Get function Id.
int
game::spec::BasicHullFunction::getId() const
{
    return m_id;
}

// Get function name.
const String_t&
game::spec::BasicHullFunction::getName() const
{
    return m_name;
}

// Get function description.
const String_t&
game::spec::BasicHullFunction::getDescription() const
{
    if (m_description.empty()) {
        return m_name;
    } else {
        return m_description;
    }
}

// Get explanation.
const String_t&
game::spec::BasicHullFunction::getExplanation() const
{
    return m_explanation;
}

// Get picture name.
const String_t&
game::spec::BasicHullFunction::getPictureName() const
{
    return m_pictureName;
}

// Get implied function Id.
int
game::spec::BasicHullFunction::getImpliedFunctionId() const
{
    return m_impliedFunctionId;
}
