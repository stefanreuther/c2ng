/**
  *  \file game/map/configuration.cpp
  *  \brief Class game::map::Configuration
  */

#include <cmath>
#include <algorithm>
#include "game/map/configuration.hpp"
#include "afl/string/parse.hpp"
#include "game/config/integeroption.hpp"
#include "game/config/integervalueparser.hpp"
#include "game/config/enumvalueparser.hpp"

namespace {
    /*
     *  Definition of configuration options
     */

    const int DEFAULT_CIRCULAR_PRECISION = 2;
    const int DEFAULT_CIRCULAR_EXCESS = 501;
    const int DEFAULT_MAP_CENTER = 2000;
    const int DEFAULT_MAP_SIZE = 2000;

    /** Circular Wrap Precision option.
        Determines how many iterations are performed to find the outside location of an object. */
    const game::config::IntegerOptionDescriptorWithDefault opt_circular_precision = { "Chart.Circle.Precision", &game::config::IntegerValueParser::instance, DEFAULT_CIRCULAR_PRECISION };

    /** Circular Wrap Excess option.
        Determines the maximum size of the outside area on the circular wrapped map.
        This is limited to avoid math problems due to the heavy distortion. */
    const game::config::IntegerOptionDescriptorWithDefault opt_circular_excess = { "Chart.Circle.Outside", &game::config::IntegerValueParser::instance, DEFAULT_CIRCULAR_EXCESS };

    /** Wrap mode. Default is 0, so no need to specify. */
    game::config::EnumValueParser opt_map_kind_parser("flat,wrapped,circular");
    const game::config::IntegerOptionDescriptor opt_map_kind = { "Chart.Geo.Mode", &opt_map_kind_parser };

    /** Map center. */
    const game::config::IntegerArrayOptionDescriptorWithDefault<2> opt_map_center = { "Chart.Geo.Center", &game::config::IntegerValueParser::instance, { DEFAULT_MAP_CENTER, DEFAULT_MAP_CENTER }};

    /** Map size. */
    const game::config::IntegerArrayOptionDescriptorWithDefault<2> opt_map_size = { "Chart.Geo.Size", &game::config::IntegerValueParser::instance, { DEFAULT_MAP_SIZE, DEFAULT_MAP_SIZE } };



    /* Map Images:
       1 2 3
       4 0 5
       6 7 8 */
    static const int8_t image_dx[] = {  0, -1,  0, +1, -1, +1, -1,  0, +1 };
    static const int8_t image_dy[] = {  0, -1, -1, -1,  0,  0, +1, +1, +1 };


    /** Get exact mapping for a point with circular wrap.

        Circular wrap cannot be inverted with a simple formula:

        Let x be the position where our ship is, x' be the position where PWrap maps it to, assuming that x is outside the wrap circle.
        Here, we know x' and want to know which x we need.

        We have
              x' = Trunc(CenterX + nr*sin(angle) + 0.5)
        with
              nr = radius - 2*Size
        and therefore
              x' = Trunc(CenterX + radius*sin(angle) - 2*Size*sin(angle) + 0.5)
        Because
              x = CenterX + radius*sin(angle)
        it follows that
              x' = Trunc(x - 2*Size*sin(angle) + 0.5)
        The term inside the Trunc() is always positive, so we're always rounding down.
        Therefore,
              x' = x - Floor(-2*Size*sin(angle) + 0.5)
        and thus
              x = x' + Floor(-2*Size*sin(angle) + 0.5)
        or, using Trunc,
              x = Trunc(x' - 2*Size*sin(angle) + 0.5)

        Now the problem is that we don't know then angle.
        Our ugly workaround, if an exact translation is requested, is to do brute force.
        The initial inexact hypothesis, computed using the above formula, is passed in as \c out.
        This iterates around the hypothesis until it finds a valid mapping.
        It may fail, though.
        This means that an inside point cannot be reached by moving to a particular outside point.

        \param config [in] Map config
        \param pt  [in] Point
        \param out [in] Initial hypothesis, [out] Result

        Derived from find.pas::CircularMoveOutside. */
    bool findExactOutsideLocation(const game::map::Configuration& config, const game::map::Point pt, game::map::Point& out)
    {
        // ex game/coord.cc:findExactOutsideLocation

        // Maybe the hypothesis already is exact?
        if (config.getCanonicalLocation(out) == pt) {
            return true;
        }

        // Brute force
        for (int i = 1, limit = config.getCircularPrecision(); i <= limit; ++i) {
            int dx = 1;
            int dy = 1;
            enum { MoveDown, MoveLeft, MoveUp, MoveRight } where = MoveDown;
            while (1) {
                // Check this location
                game::map::Point t(out + game::map::Point(dx, dy));
                if (config.getCanonicalLocation(t) == pt) {
                    out = t;
                    return true;
                }

                // Run around
                if (where == MoveDown) {
                    if (dy + i == 0) { where = MoveLeft;  } else { --dy; }
                } else if (where == MoveLeft) {
                    if (dx + i == 0) { where = MoveUp;    } else { --dx; }
                } else if (where == MoveUp) {
                    if (dy == i)     { where = MoveRight; } else { ++dy; }
                } else {
                    if (dx == i)     { break;             } else { ++dx; }
                }
            }
        }

        // Cannot find a mapping
        return false;
    }
}

game::map::Configuration::Configuration()
    : m_mode(Flat),
      m_center(2000, 2000),
      m_size(2000, 2000),
      m_min(),
      m_max(),
      m_fromHostConfiguration(false),
      m_circularPrecision(2),
      m_circularExcess(DEFAULT_CIRCULAR_EXCESS)
{
    // ex GChartConfiguration::GChartConfiguration
    computeDerivedInformation();
}

game::map::Configuration::Mode
game::map::Configuration::getMode() const
{
    // ex GChartConfiguration::getMode
    return m_mode;
}

game::map::Point
game::map::Configuration::getCenter() const
{
    // ex GChartConfiguration::getCenter
    return m_center;
}

game::map::Point
game::map::Configuration::getSize() const
{
    // ex GChartConfiguration::getSize
    return m_size;
}

game::map::Point
game::map::Configuration::getMinimumCoordinates() const
{
    // ex GChartConfiguration::getMinimumCoordinates
    return m_min;
}

game::map::Point
game::map::Configuration::getMaximumCoordinates() const
{
    // ex GChartConfiguration::getMaximumCoordinates
    return m_max;
}

int
game::map::Configuration::getCircularPrecision() const
{
    return m_circularPrecision;
}

int
game::map::Configuration::getCircularExcess() const
{
    return m_circularExcess;
}

// Initialize from configuration.
void
game::map::Configuration::initFromConfiguration(const HostVersion& host,
                                                const game::config::HostConfiguration& config,
                                                const game::config::UserConfiguration& pref)
{
    // ex GChartConfiguration::initFromConfig

    // Invariants (xref chartconfig.cc):
    // - center in range 500..4000
    // - size in range 500..4000
    // - size <= center

    // Load config file
    // - coordinates
    m_center = Point(pref[opt_map_center](1), pref[opt_map_center](2));
    m_size   = Point(pref[opt_map_size](1),   pref[opt_map_size](2));

    // - map kind
    int mapKind = pref[opt_map_kind]();
    if (mapKind == Flat || mapKind == Wrapped || mapKind == Circular) {
        m_mode = Mode(mapKind);
    }

    // - circular parameters. Out-of-range values will be corrected by computeDerivedInformation()
    m_circularPrecision = pref[opt_circular_precision]();
    m_circularExcess = pref[opt_circular_excess]();

    // Check host config
    if (host.getKind() == HostVersion::PHost
        && config[config.AllowWraparoundMap].getSource() != game::config::ConfigurationOption::Default
        && config[config.AllowWraparoundMap]() != 0)
    {
        // It's a PHost game, and the AllowWraparoundMap option is set.
        // Copy the settings from pconfig.
        m_fromHostConfiguration = true;
        m_mode = Wrapped;

        const game::config::IntegerArrayOption<4>& wrap = config[config.WraparoundRectangle];
        m_center.setX((wrap(3) + wrap(1)) / 2);
        m_center.setY((wrap(4) + wrap(2)) / 2);
        m_size.setX(wrap(3) + wrap(1));
        m_size.setY(wrap(4) + wrap(2));
    } else {
        // Not a PHost game, or AllowWraparoundMap not set or disabled.
        // It could use external wrap, so don't change anything.
        m_fromHostConfiguration = false;
    }
    computeDerivedInformation();
}

// Save to configuration.
void
game::map::Configuration::saveToConfiguration(game::config::UserConfiguration& pref)
{
    // ex GChartConfiguration::computeDerivedInformation (part)

    // Save to main config. loadUserPreferences() has set all options to srcUser.
    // This is wrong for map options which must always be srcGame.
    // However, to avoid creating game configuration files if users never even touched the settings,
    // we downgrade an option to srcDefault (which means it is not stored in a config file) if all of the following holds:
    //   - it is srcUser (=user did not set it to srcGame)
    //   - it has the default value
    //   - it is being set to the default value
    game::config::IntegerOption& map_kind = pref[opt_map_kind];
    if (map_kind.getSource() <= game::config::ConfigurationOption::User && map_kind() == Flat && m_mode == Flat) {
        map_kind.setSource(game::config::ConfigurationOption::Default);
    } else {
        map_kind.set(m_mode);
        map_kind.markUpdated(game::config::ConfigurationOption::Game);
    }

    game::config::IntegerArrayOption<2>& map_center = pref[opt_map_center];
    if (map_center.getSource() <= game::config::ConfigurationOption::User
        && map_center(1) == DEFAULT_MAP_CENTER && map_center(2) == DEFAULT_MAP_CENTER
        && m_center.getX() == DEFAULT_MAP_CENTER && m_center.getY() == DEFAULT_MAP_CENTER)
    {
        map_center.setSource(game::config::ConfigurationOption::Default);
    } else {
        map_center.set(1, m_center.getX());
        map_center.set(2, m_center.getY());
        map_center.markUpdated(game::config::ConfigurationOption::Game);
    }

    game::config::IntegerArrayOption<2>& map_size = pref[opt_map_size];
    if (map_size.getSource() <= game::config::ConfigurationOption::User
        && map_size(1) == DEFAULT_MAP_SIZE && map_size(2) == DEFAULT_MAP_SIZE
        && m_size.getX() == DEFAULT_MAP_SIZE && m_size.getY() == DEFAULT_MAP_SIZE)
    {
        map_size.setSource(game::config::ConfigurationOption::Default);
    } else {
        map_size.set(1, m_size.getX());
        map_size.set(2, m_size.getY());
        map_size.markUpdated(game::config::ConfigurationOption::Game);
    }

    // Save circular excess.
    // Use the same logic as above.
    // This differs from PCC2.
    game::config::IntegerOption& circular_excess = pref[opt_circular_excess];
    if (circular_excess.getSource() <= game::config::ConfigurationOption::User && circular_excess() == DEFAULT_CIRCULAR_EXCESS && m_circularExcess == DEFAULT_CIRCULAR_EXCESS) {
        circular_excess.setSource(game::config::ConfigurationOption::Default);
    } else {
        circular_excess.set(m_circularExcess);
        circular_excess.markUpdated(game::config::ConfigurationOption::Game);
    }

    // Update circular precision.
    // Do not mark it for the game configuration, so if this was a change because it was out of range,
    // it'll be updated in whatever config file it was.
    pref[opt_circular_precision].set(m_circularPrecision);
}

// Set configuration.
void
game::map::Configuration::setConfiguration(Mode mode, Point center, Point size)
{
    // ex GChartConfiguration::setConfig
    m_mode = mode;
    m_center = center;
    m_size = size;
    m_fromHostConfiguration = false;
    computeDerivedInformation();
}

// Check for host configuration.
bool
game::map::Configuration::isSetFromHostConfiguration() const
{
    // ex GChartConfiguration::isSetFromGameConfig
    return m_fromHostConfiguration;
}

// Check for point on map.
bool
game::map::Configuration::isOnMap(Point pt) const
{
    // ex GChartConfiguration::isOnMap, GPoint::isOnMap
    switch (m_mode) {
     case Flat:
        // Everything is on the map
        return true;

     case Wrapped:
        // Boundary behaviour consistent with PCC2, PCC 1.x, PHost 3.3c, and EchoView
        return pt.getX() >= m_min.getX() && pt.getX() < m_max.getX()
            && pt.getY() >= m_min.getY() && pt.getY() < m_max.getY();

     case Circular:
        // Check inside of bounding rectangle first, then distance.
        return pt.getX() >= m_min.getX() && pt.getX() <= m_max.getX()
            && pt.getY() >= m_min.getY() && pt.getY() <= m_max.getY()
            && pt.isCloserThan(m_center, m_size.getX());
    }
    return false;
}

// Check for valid planet coordinates.
bool
game::map::Configuration::isValidPlanetCoordinate(Point pt) const
{
    // ex GChartConfiguration::isValidPlanetCoordinate
    return pt.getX() > 0
        && pt.getY() > 0
        && pt.getX() <= 9000
        && pt.getY() <= 9000
        && isOnMap(pt);
}

// Get canonical location.
game::map::Point
game::map::Configuration::getCanonicalLocation(Point pt) const
{
    // ex GChartConfiguration::getCanonicalLocation, GPoint::getCanonicalLocation
    switch (m_mode) {
     case Flat:
        // No non-canonical locations
        break;

     case Wrapped:
        // Wrap into range
        if (pt.getX() < m_min.getX()) {
            pt.addX(m_size.getX());
        }
        if (pt.getX() >= m_max.getX()) {
            pt.addX(-m_size.getX());
        }
        if (pt.getY() < m_min.getY()) {
            pt.addY(m_size.getY());
        }
        if (pt.getY() >= m_max.getY()) {
            pt.addY(-m_size.getY());
        }
        break;

     case Circular:
        // Wrap into range
        int32_t dist = pt.getSquaredRawDistance(m_center);
        if (dist > int32_t(m_size.getX())*m_size.getX()) {
            // pwrap formulas
            double radius = 2*m_size.getX() - std::sqrt(double(dist));
            double angle  = std::atan2(double(m_center.getX() - pt.getX()), double(m_center.getY() - pt.getY()));

            pt.setX(int(radius * std::sin(angle) + m_center.getX() + 0.5));
            pt.setY(int(radius * std::cos(angle) + m_center.getY() + 0.5));
        }
        break;
    }

    return pt;
}

// Get canonical location of a point, simple version.
game::map::Point
game::map::Configuration::getSimpleCanonicalLocation(Point pt) const
{
    // ex GChartConfiguration::getSimpleCanonicalLocation, GPoint::getSimpleCanonicalLocation
    if (m_mode == Wrapped) {
        return getCanonicalLocation(pt);
    } else {
        return pt;
    }
}

// Get nearest alias of a point, simple version.
game::map::Point
game::map::Configuration::getSimpleNearestAlias(Point pt, Point a) const
{
    // ex GChartConfiguration::getSimpleNearestAlias, GPoint::getSimpleNearestAlias
    if (m_mode == Wrapped) {
        if (2*(a.getX() - pt.getX()) > m_size.getX()) {
            pt.addX(m_size.getX());
        }
        if (2*(a.getY() - pt.getY()) > m_size.getY()) {
            pt.addY(m_size.getY());
        }
        if (2*(pt.getX() - a.getX()) > m_size.getX()) {
            pt.addX(-m_size.getX());
        }
        if (2*(pt.getY() - a.getY()) > m_size.getY()) {
            pt.addY(-m_size.getY());
        }
    }
    return pt;
}

// Get number of map images that can map rectangles.
int
game::map::Configuration::getNumRectangularImages() const
{
    // ex int GChartConfiguration::getNumRectangularImages
    switch (m_mode) {
     case Flat:
        return 1;
     case Circular:
        return 1;
     case Wrapped:
        return 9;
    }
    return 0;
}

// Get number of map images that can map points.
int
game::map::Configuration::getNumPointImages() const
{
    // ex GChartConfiguration::getNumPointImages
    switch (m_mode) {
     case Flat:
        return 1;
     case Circular:
        return 2;
     case Wrapped:
        return 9;
    }
    return 0;
}

// Compute outside location for a point inside the map.
bool
game::map::Configuration::getPointAlias(Point pt, Point& out, int image, bool exact) const
{
    // ex GChartConfiguration::getPointAlias
    switch (m_mode) {
     case Flat:
        // We have only one image
        if (image == 0) {
            out = pt;
            return true;
        } else {
            return false;
        }

     case Wrapped:
        // Point must be inside, and a supported image.
        if (!isOnMap(pt) || image < 0 || image >= 9) {
            return false;
        } else {
            out.setX(pt.getX() + m_size.getX() * image_dx[image]);
            out.setY(pt.getY() + m_size.getY() * image_dy[image]);
            return true;
        }

     case Circular:
        // Find radius.
        long r = pt.getSquaredRawDistance(m_center);
        if (r > long(m_size.getX()) * m_size.getX()) {
            // It's outside the permitted range
            return false;
        } else if (image == 0) {
            // Standard image
            out = pt;
            return true;
        } else if (image == 1) {
            // Outside image
            double rr = std::sqrt(double(r));
            if (rr > m_size.getX() - m_circularExcess) {
                // We might be able to map it
                double angle = std::atan2(double(pt.getX() - m_center.getX()), double(pt.getY() - m_center.getY()));
                out.setX(int32_t(pt.getX() - 2*m_size.getX()*std::sin(angle) + 0.5));
                out.setY(int32_t(pt.getY() - 2*m_size.getX()*std::cos(angle) + 0.5));
                if (exact) {
                    return findExactOutsideLocation(*this, pt, out);
                }
                return true;
            } else {
                // It's too far inside, so its mapping would be too far outside
                return false;
            }
        } else {
            // Error
            return false;
        }
    }
    return false;
}


// Compute outside location for a point inside the map, simple version.
game::map::Point
game::map::Configuration::getSimplePointAlias(Point pt, int image) const
{
    // ex GChartConfiguration::getSimplePointAlias
    switch (m_mode) {
     case Flat:
     case Circular:
        // No change for these
        break;

     case Wrapped:
        // Regular remapping
        pt.addX(m_size.getX() * image_dx[image]);
        pt.addY(m_size.getY() * image_dy[image]);
        break;
    }
    return pt;
}

// Get minimum distance between two points, considering map configuration.
int32_t
game::map::Configuration::getSquaredDistance(Point a, Point b) const
{
    // ex game/coord.cc:distanceSquared
    return getSimpleNearestAlias(a, b).getSquaredRawDistance(b);
}

// Parse a sector number.
bool
game::map::Configuration::parseSectorNumber(const String_t& s, Point& result)
{
    // ex GPoint::parseSectorNumber
    int sec;
    return afl::string::strToInteger(s, sec) && parseSectorNumber(sec, result);
}

// Parse a sector number.
bool
game::map::Configuration::parseSectorNumber(int n, Point& result)
{
    // ex GPoint::parseSectorNumber
    // Valid range is 100 .. 499
    if (n < 100 || n > 499) {
        return false;
    }

    // Compute location
    static const int secX[] = { -950,   50, -950,   50 };
    static const int secY[] = {  950,  950,  -50,  -50 };
    int newX = m_center.getX() + secX[n / 100 - 1] + 100*((n % 100) / 10);
    int newY = m_center.getY() + secY[n / 100 - 1] - 100*(n % 10);

    // On map?
    if (!isOnMap(Point(newX, newY))) {
        return false;
    }

    // Use it
    result = Point(newX, newY);
    return true;
}

// Get sector number.
int
game::map::Configuration::getSectorNumber(Point pt) const
{
    // ex GPoint::getSectorNumber
    // outside map?
    if (!isOnMap(pt)) {
        return 0;
    }

    int x = pt.getX() - m_center.getX() + 1000;
    int y = pt.getY() - m_center.getY() + 1000;

    /* outside known region? */
    if (x < 0 || y < 0 || x >= 2000 || y >= 2000) {
        return 0;
    }

    int major = 1;
    if (x >= 1000) {
        major += 1;
    }
    if (y < 1000) {
        major += 2;
    }

    return 100 * major
        + 10 * ((x % 1000) / 100)
        + (999 - (y % 1000)) / 100;
}

void
game::map::Configuration::computeDerivedInformation()
{
    // ex GChartConfiguration::computeDerivedInformation

    // Check ranges and force out-of-range values into range
    // xref chartconfig.cc
    if (m_center.getX() < 500 || m_center.getX() > 4000) {
        m_center.setX(2000);
        m_fromHostConfiguration = false;
    }
    if (m_center.getY() < 500 || m_center.getX() > 4000) {
        m_center.setY(2000);
        m_fromHostConfiguration = false;
    }
    if (m_size.getX() < 500 || m_size.getX() > m_center.getX()) {
        m_size.setX(std::min(m_center.getX(), 2000));
        m_fromHostConfiguration = false;
    }
    if (m_size.getY() < 500 || m_size.getY() > m_center.getY()) {
        m_size.setY(std::min(m_center.getY(), 2000));
        m_fromHostConfiguration = false;
    }

    // Compute derived information
    switch (m_mode) {
     case Flat:
     case Wrapped:
        m_min.setX(m_center.getX() - m_size.getX() / 2);
        m_min.setY(m_center.getY() - m_size.getY() / 2);
        m_max = m_min + m_size;
        break;

     case Circular:
        m_size.setY(m_size.getX());
        m_min = m_center - m_size;
        m_max = m_center + m_size;

        // Fix up circular parameters.
        // We limit wrap excess to 2/3 of the size, which is 666 ly in the normal configuration.
        m_circularExcess = std::min(m_circularExcess, m_size.getX() * 2/3);

        // Range 0..20 is consistent with PCC 1.x/2.x.
        m_circularPrecision = std::max(0, std::min(20, m_circularPrecision));
        break;
    }
}
