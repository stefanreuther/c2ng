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
      m_data(markerKind, color, "")
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
        set(Data(static_cast<uint8_t>(markerKindVal), static_cast<uint8_t>(colorVal), strTrim(value)));
    }
}

String_t
game::config::MarkerOption::toString() const
{
    // ex ConfigMarkerOption::toString
    return Format("%d,%d,%s", m_data.markerKind, m_data.color, m_data.note);
}

game::config::MarkerOption::Data&
game::config::MarkerOption::operator()()
{
    return m_data;
}

const game::config::MarkerOption::Data&
game::config::MarkerOption::operator()() const
{
    // ex ConfigMarkerOption::getColorIndex, ConfigMarkerOption::getMarkerKind, ConfigMarkerOption::getNote
    return m_data;
}

void
game::config::MarkerOption::set(const Data& data)
{
    // ex ConfigMarkerOption::setColorIndex, ConfigMarkerOption::setMarkerKind, ConfigMarkerOption::setNote
    if (m_data.markerKind != data.markerKind
        || m_data.color != data.color
        || m_data.note != data.note)
    {
        m_data = data;
        markChanged();
    }
}
