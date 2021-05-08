/**
  *  \file game/config/markeroption.cpp
  *  \brief Class game::config::MarkerOption
  */

#include "game/config/markeroption.hpp"
#include "afl/string/format.hpp"
#include "afl/string/string.hpp"
#include "afl/string/parse.hpp"
#include "game/map/drawing.hpp"

using afl::string::Format;
using afl::string::strSplit;
using afl::string::strToInteger;
using afl::string::strTrim;
using game::map::Drawing;

game::config::MarkerOption::MarkerOption(uint8_t markerKind, uint8_t color)
    : ConfigurationOption(),
      m_markerKind(markerKind),
      m_color(color),
      m_note()
{
    // ex ConfigMarkerOption::ConfigMarkerOption
}

game::config::MarkerOption::~MarkerOption()
{ }

// ConfigurationOption:
void
game::config::MarkerOption::set(String_t value)
{
    // ex ConfigMarkerOption::set
    String_t markerKindStr;
    String_t colorStr;
    int markerKindVal;
    int colorVal;
    if (strSplit(value, markerKindStr, value, ",")
        && strSplit(value, colorStr, value, ",")
        && strToInteger(markerKindStr, markerKindVal)
        && strToInteger(colorStr, colorVal)
        && markerKindVal >= 0
        && markerKindVal < Drawing::NUM_USER_MARKERS
        && colorVal >= 0
        && colorVal <= Drawing::NUM_USER_COLORS)
    {
        m_markerKind  = static_cast<uint8_t>(markerKindVal);
        m_color = static_cast<uint8_t>(colorVal);
        m_note  = strTrim(value);
    }
}

String_t
game::config::MarkerOption::toString() const
{
    // ex ConfigMarkerOption::toString
    return Format("%d,%d,%s", m_markerKind, m_color, m_note);
}

uint8_t
game::config::MarkerOption::getColor() const
{
    // ex ConfigMarkerOption::getColorIndex
    return m_color;
}

void
game::config::MarkerOption::setColor(uint8_t color)
{
    // ex ConfigMarkerOption::setColorIndex
    m_color = color;
}

uint8_t
game::config::MarkerOption::getMarkerKind() const
{
    // ex ConfigMarkerOption::getMarkerKind
    return m_markerKind;
}

void
game::config::MarkerOption::setMarkerKind(uint8_t markerKind)
{
    // ex ConfigMarkerOption::setMarkerKind
    m_markerKind = markerKind;
}

String_t
game::config::MarkerOption::getNote() const
{
    // ex ConfigMarkerOption::getNote
    return m_note;
}

void
game::config::MarkerOption::setNote(String_t note)
{
    // ex ConfigMarkerOption::setNote
    m_note = note;
}
