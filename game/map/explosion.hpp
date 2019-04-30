/**
  *  \file game/map/explosion.hpp
  */
#ifndef C2NG_GAME_MAP_EXPLOSION_HPP
#define C2NG_GAME_MAP_EXPLOSION_HPP

#include "game/map/mapobject.hpp"

namespace game { namespace map {

    class Explosion : public MapObject {
     public:
        Explosion(Id_t id, Point pos);
        Explosion(const Explosion& ex);
        ~Explosion();

        // MapObject:
        virtual bool getPosition(Point& result) const;

        // Object:
        virtual String_t getName(ObjectName which, afl::string::Translator& tx, InterpreterInterface& iface) const;
        virtual Id_t getId() const;
        virtual bool getOwner(int& result) const;

        // Explosion:
        String_t getShipName() const;
        Id_t getShipId() const;

        void setShipName(String_t name);
        void setShipId(Id_t id);

        bool merge(const Explosion& other);

     private:
        Id_t m_id;
        Point m_position;
        String_t m_shipName;
        Id_t m_shipId;
    };

} }

#endif
