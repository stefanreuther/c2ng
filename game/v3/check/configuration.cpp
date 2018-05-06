/**
  *  \file game/v3/check/configuration.cpp
  */

#include "game/v3/check/configuration.hpp"

game::v3::check::Configuration::Configuration()
    : m_resultMode(false),
      m_htmlMode(false),
      m_checksumsMode(false),
      m_handleMinus1Special(false),
      m_pickyMode(false)
{ }

void
game::v3::check::Configuration::setResultMode(bool enable)
{
    m_resultMode = enable;
}

bool
game::v3::check::Configuration::isResultMode() const
{
    return m_resultMode;
}

void
game::v3::check::Configuration::setHtmlMode(bool enable)
{
    m_htmlMode = enable;
}

bool
game::v3::check::Configuration::isHtmlMode() const
{
    return m_htmlMode;
}

void
game::v3::check::Configuration::setChecksumsMode(bool enable)
{
    m_checksumsMode = enable;
}

bool
game::v3::check::Configuration::isChecksumsMode() const
{
    return m_checksumsMode;
}

void
game::v3::check::Configuration::setHandleMinus1Special(bool enable)
{
    m_handleMinus1Special = enable;
}

bool
game::v3::check::Configuration::isHandleMinus1Special() const
{
    return m_handleMinus1Special;
}

void
game::v3::check::Configuration::setPickyMode(bool enable)
{
    m_pickyMode = enable;
}

bool
game::v3::check::Configuration::isPickyMode() const
{
    return m_pickyMode;
}
