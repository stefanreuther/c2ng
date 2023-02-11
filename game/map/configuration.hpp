/**
  *  \file game/map/configuration.hpp
  *  \brief Class game::map::Configuration
  */
#ifndef C2NG_GAME_MAP_CONFIGURATION_HPP
#define C2NG_GAME_MAP_CONFIGURATION_HPP

#include "afl/string/string.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/config/userconfiguration.hpp"
#include "game/map/point.hpp"

namespace game { namespace map {

    /** Map configuration (wrap mode).
        Contains methods to transform coordinates for wrapped maps of all types. */
    class Configuration {
     public:
        /** Map mode. */
        enum Mode {
            Flat,               ///< Flat (regular) map.
            Wrapped,            ///< Rectangular wrap (Sphere, PWrap, PHost).
            Circular            ///< Circular wrap (PWrap).
        };

        /** Default constructor.
            Constructs an empty starchart configuration object.

            Change to PCC2: in PCC2, this function would have updated the user preferences.
            Use saveToConfiguration() to do that in c2ng. */
        Configuration();

        /** Get wrap mode.
            \return wrap mode */
        Mode getMode() const;

        /** Get center of map.
            \return center
            \see setConfiguration */
        Point getCenter() const;

        /** Get size of map.
            \return size. For Wrapped map: width/height. For Circular map: X component is radius.
            \see setConfiguration  */
        Point getSize() const;

        /** Get minimum coordinates (south-east).
            \return minimum coordinates */
        Point getMinimumCoordinates() const;

        /** Get maximum coordinates (south-east).
            \return maximum coordinates */
        Point getMaximumCoordinates() const;

        /** Get precision for circular wrap.
            \return precision (search depth for inside-out mapping) */
        int getCircularPrecision() const;

        /** Get circular excess (size of outside area).
            \return circular excess */
        int getCircularExcess() const;

        /** Set precision for circular wrap.
            \param n precision (search depth for inside-out mapping) */
        void setCircularPrecision(int n);

        /** Set circular excess (size of outside area).
            \param n circular excess */
        void setCircularExcess(int n);

        /*
         *  Configuration
         */

        /** Initialize from configuration.
            \param config Host configuration
            \param pref User configuration

            Change to PCC2: in PCC2, this function would have updated the user preferences.
            Use saveToConfiguration() to do that in c2ng. */
        void initFromConfiguration(const game::config::HostConfiguration& config,
                                   const game::config::UserConfiguration& pref);

        /** Save to configuration.
            This updates the specified user configuration (preferences) object.
            \param pref User configuration object to update
            \param config Host configuration */
        void saveToConfiguration(game::config::UserConfiguration& pref,
                                 const game::config::HostConfiguration& config);

        /** Set configuration.
            This overrides a previous configuration and marks it "not from host configuration".
            \param mode Wrap mode
            \param center Map center
            \param size Map size. For wrapped map: width/height. For Circular map: X component is radius.

            Change to PCC2: in PCC2, this function would have updated the user preferences.
            Use saveToConfiguration() to do that in c2ng. */
        void setConfiguration(Mode mode, Point center, Point size);

        /** Check for host configuration.
            \retval true the current map configuration is derived from the host configuration
            \retval false the current map configuration was set by the user */
        bool isSetFromHostConfiguration() const;

        /*
         *  Coordinate management
         */

        /** Check for point on map.

            Usage: This function is mostly used internally.

            \param pt point to check
            \retval true this point is on the map and accessible to players
            \retval false this point is not accessible to players */
        bool isOnMap(Point pt) const;

        /** Check for valid planet coordinates.
            Points may be on the map using isOnMap's rules, but by convention be treated as out-of-bounds.
            This is used by the ExploreMap add-on.

            Usage: to filter incoming planet coordinates

            \param pt point to check
            \return validity */
        bool isValidPlanetCoordinate(Point pt) const;

        /** Limit user coordinate location.
            Assuming @c represents a point the user wants to be at,
            returns the position they actually are at.

            \param pt point
            \return updater position */
        Point limitUserLocation(Point pt) const;

        /** Get canonical location.
            If any kind of wrap is active, this performs the "wrap" step normally performed by the host.

            Usage: any kind of "forward" prediction.
            For example, given a ship's after-movement coordinate (which could be outside the map),
            returns a new location on the map.

            \param pt point to check
            \return updated location */
        Point getCanonicalLocation(Point pt) const;

        /** Get canonical location of a point, simple version.
            This handles just rectangular wrap, where all instances of a location are equivalent.

            Usage: FIXME

            \param pt point to check
            \return updated location */
        Point getSimpleCanonicalLocation(Point pt) const;

        /** Get nearest alias of a point, simple version.
            This handles just rectangular wrap, where all instances of a location are equivalent.
            Returns the instance of \c pt that is closest to \c a (which might be outside the map).

            Usage: if \c pt is a ship's waypoint (e.g. a planet), and \c a is the ship's location,
            this function returns the desired waypoint.
            The waypoint will move the ship outside the map, but the host will move it in again.

            \param pt point to check
            \param a origin
            \return updated location */
        Point getSimpleNearestAlias(Point pt, Point a) const;

        /** Get number of map images that can map rectangles.
            \see getSimplePointAlias
            \return number of images */
        int  getNumRectangularImages() const;

        /** Get number of map images that can map points.
            \see getPointAlias
            \return number of images */
        int  getNumPointImages() const;

        /** Compute outside location for a point inside the map.
            This is an inverse operation to getCanonicalLocation.
            \param pt    [in] Point
            \param out   [out] Result will be produced here
            \param image [in] Index of map image to produce, [0,getNumPointImages()). 0=regular image.
            \param exact [in] true to request a perfect mapping, false to accept an inexact mapping
            \retval true this point could be mapped to the requested image
            \retval false this point could not be mapped */
        bool getPointAlias(Point pt, Point& out, int image, bool exact) const;

        /** Compute outside location for a point inside the map, simple version.
            This is well-suited to map known map objects in a fail-safe way, using pre-verified parameters,
            in code that is fully aware of the map geometry.

            This function does NOT map circular points to the outside.
            This is an inverse operation to getCanonicalLocation.

            If the given image parameter is invalid, pt is returned unmodified.

            \param pt    [in] Point
            \param image [in] Index of map image to produce, [0,getNumRectangularImages()). 0=regular image. */
        Point getSimplePointAlias(Point pt, int image) const;

        /** Get minimum distance between two points, considering map configuration.
            \param a,b points
            \return squared distance */
        int32_t getSquaredDistance(Point a, Point b) const;

        /*
         *  Sector numbers
         */

        /** Parse a sector number.
            \param s [in] user input
            \param result [out] result
            \retval true success; result has been updated
            \retval false failure; result was not modified */
        bool parseSectorNumber(const String_t& s, Point& result) const;

        /** Parse a sector number.
            \param n [in] user input
            \param result [out] result
            \retval true success; result has been updated
            \retval false failure; result was not modified */
        bool parseSectorNumber(int n, Point& result) const;

        /** Get sector number.
            The sector number is shown by PCC2 and PCC1.x, and agrees to Trevor Fuson's VGAMAP for a standard-sized map.
            \return sector number (100..499), zero if point is not in any numbered sector */
        int getSectorNumber(Point pt) const;

        /*
         *  Comparison
         */

        /** Compare for equality.
            \param other Other configuration
            \return true if both configurations are identical */
        bool operator==(const Configuration& other) const;

        /** Compare for inequality.
            \param other Other configuration
            \return true if both configurations are different */
        bool operator!=(const Configuration& other) const;

     private:
        void computeDerivedInformation();

        Mode m_mode;

        Point m_center;
        Point m_size;
        Point m_min;
        Point m_max;
        bool m_fromHostConfiguration;

        int m_circularPrecision;
        int m_circularExcess;
    };

} }

#endif
