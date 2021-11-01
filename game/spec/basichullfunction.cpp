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
    // ex hullfunc.pas:GetBasicFunctionName
    return m_name;
}

// Get function description.
const String_t&
game::spec::BasicHullFunction::getDescription() const
{
    // ex hullfunc.pas:GetBasicFunctionTitle
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

// Get damage limit for a function.
afl::base::Optional<int>
game::spec::BasicHullFunction::getDamageLimit(int /*forOwner*/, const game::config::HostConfiguration& config) const
{
    // ex GBasicHullFunction::getDamageLimit
    using game::config::HostConfiguration;
    switch (m_id) {
     case Cloak:
     case AdvancedCloak:
        return config[HostConfiguration::DamageLevelForCloakFail]();

     case LokiAnticloak:
     case AdvancedAntiCloak:
        return config[HostConfiguration::DamageLevelForAntiCloakFail]();

     case HeatsTo50:
     case CoolsTo50:
     case HeatsTo100:
        return config[HostConfiguration::DamageLevelForTerraformFail]();

     case Hyperdrive:
        return config[HostConfiguration::DamageLevelForHyperjumpFail]();

     case ImperialAssault:
        return 1;

     case FirecloudChunnel:
     case ChunnelSelf:
     case ChunnelOthers:
     case ChunnelTarget:
        return config[HostConfiguration::DamageLevelForChunnelFail]();

     default:
        return afl::base::Nothing;
    }
}
