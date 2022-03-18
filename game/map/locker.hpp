/**
  *  \file game/map/locker.hpp
  *  \brief Class game::map::Locker
  */
#ifndef C2NG_GAME_MAP_LOCKER_HPP
#define C2NG_GAME_MAP_LOCKER_HPP

#include "game/config/integeroption.hpp"
#include "game/map/point.hpp"
#include "game/reference.hpp"
#include "game/hostversion.hpp"

namespace game { namespace map {

    class Configuration;
    class Object;
    class Universe;
    class Drawing;

    /*
     *  Configuration
     */

    const int32_t MatchPlanets    = 1;     // ex li_Planet
    const int32_t MatchShips      = 2;     // ex li_Ship
    const int32_t MatchUfos       = 4;     // ex li_Ufo
    const int32_t MatchDrawings   = 8;     // ex li_Marker
    const int32_t MatchMinefields = 16;    // ex li_Minefield

    // ex lock_config, setLockMode, getLockMode, getLockModeConfigOption, lm_Left, lm_Right: just access pref[Lock_Left] etc.
    typedef game::config::IntegerOptionDescriptor LockOptionDescriptor_t;

    /** Locking on objects on starchart.
        When users click at/near an object, we want to lock the cursor onto that object.
        To implement that,
        - construct a Locker
        - call setMarkedOnly(), setRangeLimit() to configure parameters
        - call one or more of the add() functions to add candidates
        - call getFoundPoint(), getFoundObject() to retrieve the result

        c2ng changes, 20180906:
        - removed setIgnore()/isIgnore().
          It is needed to ignore drawings, but drawings currently are not Object's.
          Therefore, add special parameter to addDrawings(), addUniverse().
        - changed GMapObject* -> Reference
        - changed two constructors -> once ctor + setRangeLimit, setMarkedOnly */
    class Locker {
     public:
        /** Constructor.
            \param target Clicked point
            \param config Map configuration. Saved by reference; must live longer than Locker. */
        Locker(Point target, const Configuration& config);

        /** Set range limit.
            If set, only points within this range are considered.
            \param min Minimum (bottom-left) coordinate, inclusive.
            \param max Maximum (top-right) coordinate, inclusive. */
        void setRangeLimit(Point min, Point max);

        /** Set limitation to marked objects.
            If set, only marked objects are considered.
            \param flag Flag */
        void setMarkedOnly(bool flag);

        /** Add single point candidate.
            \param pt     point
            \param marked true if that object is marked
            \param obj    reference to object */
        void addPoint(Point pt, bool marked, Reference obj = Reference());

        /** Add object candidate.
            \param obj  object
            \param type type of object for constructing references */
        void addObject(const Object& obj, Reference::Type type);

        /** Add planets.
            Adds all planets from the given universe.
            \param univ Universe */
        void addPlanets(const Universe& univ);

        /** Add ships.
            Adds all ships from the given universe.
            \param univ Universe */
        void addShips(const Universe& univ);

        /** Add Ufos.
            Adds all Ufos from the given universe.
            \param univ Universe */
        void addUfos(const Universe& univ);

        /** Add minefields.
            Adds all minefields from the given universe.
            \param univ Universe */
        void addMinefields(const Universe& univ);

        /** Add drawings.
            Adds all drawings and explosions from the given universe.
            \param univ Universe
            \param ignore Drawing to ignore. To use when moving a marker to ignore locking that marker onto itself. */
        void addDrawings(const Universe& univ, const Drawing* ignore);

        /** Add universe (main entry point).
            Adds all objects from the given universe that are selected by the \c items bitfield.
            \param univ Universe
            \param items Bitfield of object
            \param ignoreDrawing Drawing to ignore */
        void addUniverse(const Universe& univ, int32_t items, const Drawing* ignoreDrawing);

        /** Find warp-well edge.
            Call instead of getFoundObject() to find a point that minimizes the movement distance to reach the found point.
            \param origin         Origin of movement (minimize movement starting here)
            \param isHyperdriving true for hyperdrive, false for normal movement
            \param univ           Universe
            \param config         Host configuration (for warp well parameters)
            \param host           Host version (for warp well rules/shape)
            \return point in warp well if applicable, otherwise, same as getFoundPoint(). */
        Point findWarpWellEdge(Point origin, bool isHyperdriving, const Universe& univ, const game::config::HostConfiguration& config, const HostVersion& host) const;

        /** Get found point.
            If the found object is across a map border, this will return the coordinates nearest to the original target.
            Do not assume that this is equal to one of the points added.
            In case no object was in range/acceptable, the original target is returned as-is.
            \return found point */
        Point getFoundPoint() const;

        /** Get found object.
            May be a null reference if the found point does not correspond to an object.
            \return reference */
        Reference getFoundObject() const;

     private:
        Point m_target;
        Point m_min;
        Point m_max;
        Point m_foundPoint;
        Reference m_foundObject;
        bool m_markedOnly;
        int32_t m_minDistance;
        const Configuration& m_config;

        void addPointRaw(Point pt, Reference obj);
        int32_t getWarpWellDistanceMetric(Point origin, Point pt, bool isHyperdriving, const HostVersion& host) const;
    };

} }

#endif
