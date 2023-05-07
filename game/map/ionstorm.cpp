/**
  *  \file game/map/ionstorm.cpp
  *  \brief Class game::map::IonStorm
  *
  *  FIXME: as of 20200815, setters do not mark an IonStorm changed.
  *  The trivial solution, using Updater, does not work because IntegerProperty_t
  *  is not comparable.
  */

#include <cmath>
#include "game/map/ionstorm.hpp"
#include "afl/string/format.hpp"
#include "util/math.hpp"

namespace {
    using game::map::IonStorm;
    using game::map::Point;

    void getSingleForecast(IonStorm::Forecast_t& result, const IonStorm& ion, int dh, int warp, int uncertainity)
    {
        // ex drawPrediction, chartdlg.pas::DrawStormPrediction
        int radius, voltage, heading;
        Point center;
        if (!ion.getRadius().get(radius) || !ion.getVoltage().get(voltage) || !ion.getHeading().get(heading) || !ion.getPosition().get(center)) {
            return;
        }

        // Storm size may enforce different speed
        if (radius < 200) {
            warp = 6;
        }
        if (voltage > 250) {
            warp = 8;
        }

        for (int j = 0; j < 5; ++j) {
            heading += dh;
            center += Point(util::roundToInt(util::squareInteger(warp) * std::sin(heading * (util::PI/180.0))),
                            util::roundToInt(util::squareInteger(warp) * std::cos(heading * (util::PI/180.0))));
            result.push_back(IonStorm::Forecast(center, radius, uncertainity));
        }
    }
}

const int game::map::IonStorm::UNCERTAINITY_LIMIT;

game::map::IonStorm::IonStorm(int id)
    : CircularObject(id),
      m_x(),
      m_y(),
      m_radius(),
      m_voltage(),
      m_speed(),
      m_heading(),
      m_isGrowing(false),
      m_name()
{ }

game::map::IonStorm::~IonStorm()
{ }

// Object:
String_t
game::map::IonStorm::getName(ObjectName which, afl::string::Translator& tx, const InterpreterInterface& /*iface*/) const
{
    // ex GIonStorm::getName
    if (m_name.empty()) {
        return getDefaultName(tx);
    } else {
        switch (which) {
         case PlainName:
            return m_name;
         case LongName:
         case DetailedName:
            return afl::string::Format("%s: %s", getDefaultName(tx), m_name);
        }
        return String_t();
    }
}

afl::base::Optional<int>
game::map::IonStorm::getOwner() const
{
    // GIonStorm::getOwner
    return 0;
}

afl::base::Optional<game::map::Point>
game::map::IonStorm::getPosition() const
{
    // ex GIonStorm::getPos
    int x, y;
    if (m_x.get(x) && m_y.get(y)) {
        return Point(x, y);
    } else {
        return afl::base::Nothing;
    }
}

// CircularObject:
afl::base::Optional<int>
game::map::IonStorm::getRadius() const
{
    // ex GIonStorm::getRadius
    return m_radius;
}

afl::base::Optional<int32_t>
game::map::IonStorm::getRadiusSquared() const
{
    // ex GIonStorm::getRadiusSquared
    int r;
    if (getRadius().get(r)) {
        return util::squareInteger(r);
    } else {
        return afl::base::Nothing;
    }
}

// IonStorm:
void
game::map::IonStorm::setName(String_t name)
{
    m_name = name;
}

void
game::map::IonStorm::setPosition(Point pos)
{
    m_x = pos.getX();
    m_y = pos.getY();
}

void
game::map::IonStorm::setRadius(int r)
{
    m_radius = r;
}

void
game::map::IonStorm::setVoltage(int voltage)
{
    m_voltage = voltage;
}

void
game::map::IonStorm::setWarpFactor(int speed)
{
    m_speed = speed;
}

void
game::map::IonStorm::setHeading(int heading)
{
    m_heading = heading;
}

void
game::map::IonStorm::setIsGrowing(bool flag)
{
    m_isGrowing = flag;
}

game::IntegerProperty_t
game::map::IonStorm::getClass() const
{
    // ex GIonStorm::getClass
    int volt;
    if (getVoltage().get(volt)) {
        if (volt > 200) {
            return 5;
        } else {
            return volt / 50 + 1;
        }
    } else {
        return afl::base::Nothing;
    }
}

game::IntegerProperty_t
game::map::IonStorm::getVoltage() const
{
    // ex GIonStorm::getVoltage
    return m_voltage;
}

game::IntegerProperty_t
game::map::IonStorm::getHeading() const
{
    // ex GIonStorm::getHeading
    return m_heading;
}

game::IntegerProperty_t
game::map::IonStorm::getWarpFactor() const
{
    // ex GIonStorm::getSpeed
    return m_speed;
}

bool
game::map::IonStorm::isGrowing() const
{
    // ex GIonStorm::isGrowing
    return m_isGrowing;
}

bool
game::map::IonStorm::isActive() const
{
    int volt;
    return getVoltage().get(volt)
        && volt > 0;
}

void
game::map::IonStorm::addMessageInformation(const game::parser::MessageInformation& info)
{
    // Allow voltage=0 to remove a storm. Otherwise, we need at minimum position, radius.
    namespace gp = game::parser;
    int32_t x, y, radius, voltage;
    if (info.getValue(gp::mi_IonVoltage).get(voltage)) {
        if (voltage == 0) {
            // Remove the storm
            setVoltage(voltage);
            setIsGrowing(false);
        } else {
            // Try to create the storm
            if (info.getValue(gp::mi_X).get(x) && info.getValue(gp::mi_Y).get(y) && info.getValue(gp::mi_Radius).get(radius)) {
                // Success
                setPosition(Point(x, y));
                setVoltage(voltage);
                setRadius(radius);

                // Try to set status. Either explicit from message, or implicit from voltage
                int status;
                if (info.getValue(gp::mi_IonStatus).get(status)) {
                    setIsGrowing(status != 0);
                } else {
                    setIsGrowing((voltage & 1) != 0);
                }

                // Try to set movement vector
                int speed, heading;
                if (info.getValue(gp::mi_WarpFactor).get(speed) && info.getValue(gp::mi_Heading).get(heading)) {
                    setWarpFactor(speed);
                    setHeading(heading);
                }

                // Try to set the name
                info.getValue(gp::ms_Name).get(m_name);
            }
        }
    }
}

void
game::map::IonStorm::getForecast(Forecast_t& result) const
{
    // ex WIonForecastChart::drawPre (part), chartdlg.pas:PredictStorm
    for (int it = 0; it <= UNCERTAINITY_LIMIT; ++it) {
        // Storm changes direction by [-10, +10] degrees. Plot all even changes.
        // Storm goes warp 2..4. Plot slow speed outside (for tightest turns), fast speed inside (for farthest reach).
        // We do not plot voltage or radius changes.
        int uncertainity = UNCERTAINITY_LIMIT - it;
        getSingleForecast(result, *this, -10 + 2*it, 2 + it/2, uncertainity);
        if (it != 5) {
            getSingleForecast(result, *this, +10 - 2*it, 2 + it/2, uncertainity);
        }
    }

    // Add current position as very certain
    Point center;
    int radius;
    if (getPosition().get(center) && getRadius().get(radius)) {
        result.push_back(IonStorm::Forecast(center, radius, 0));
    }
}

String_t
game::map::IonStorm::getDefaultName(afl::string::Translator& tx) const
{
    return afl::string::Format(tx.translateString("Ion storm #%d").c_str(), getId());
}
