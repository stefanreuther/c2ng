/**
  *  \file game/map/locker.hpp
  *  \brief Class game::map::Locker
  */
#ifndef C2NG_GAME_MAP_LOCKER_HPP
#define C2NG_GAME_MAP_LOCKER_HPP

#include "game/config/integeroption.hpp"
#include "game/map/point.hpp"
#include "game/reference.hpp"

namespace game { namespace map {

    class Configuration;
    class MapObject;
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
    typedef game::config::IntegerOptionDescriptorWithDefault LockOptionDescriptor_t;
    extern const LockOptionDescriptor_t Lock_Left;
    extern const LockOptionDescriptor_t Lock_Right;


    /*
      c2ng changes, 20180906:
      - removed setIgnore()/isIgnore().
        It is needed to ignore drawings, but drawings currently are not MapObject's.
        Therefore, add special parameter to addDrawings(), addUniverse().
      - changed GMapObject* -> Reference
      - changed two constructors -> once ctor + setRangeLimit, setMarkedOnly
     */
    class Locker {
     public:
        explicit Locker(Point target, const Configuration& config);

        void setRangeLimit(Point min, Point max);
        void setMarkedOnly(bool flag);

        void addPoint(Point pt, bool marked, Reference obj = Reference());
        void addObject(const MapObject& obj, Reference::Type type);

        void addPlanets(const Universe& univ);
        void addShips(const Universe& univ);
        void addUfos(const Universe& univ);
        void addMinefields(const Universe& univ);
        void addDrawings(const Universe& univ, const Drawing* ignore);

        void addUniverse(const Universe& univ, int32_t items, const Drawing* ignoreDrawing);

        Point getFoundPoint() const;
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
    };

} }

#endif
