/**
  *  \file game/map/ionstorm.hpp
  */
#ifndef C2NG_GAME_MAP_IONSTORM_HPP
#define C2NG_GAME_MAP_IONSTORM_HPP

#include "game/map/circularobject.hpp"
#include "game/map/point.hpp"
#include "game/types.hpp"

namespace game { namespace map {

    class IonStorm : public CircularObject {
     public:
        explicit IonStorm(int id);
        virtual ~IonStorm();

        // Object:
        virtual String_t getName(Name which, afl::string::Translator& tx, InterpreterInterface& iface) const;
        virtual Id_t getId() const;
        virtual bool getOwner(int& result) const;

        // MapObject:
        virtual bool getPosition(Point& result) const;

        // CircularObject:
        virtual bool getRadius(int& result) const;
        virtual bool getRadiusSquared(int32_t& result) const;

        // IonStorm:
        void setName(String_t name);
        void setPosition(Point pos);
        void setRadius(int r);
        void setVoltage(int voltage);
        void setSpeed(int speed);
        void setHeading(int heading);
        void setIsGrowing(bool flag);

        IntegerProperty_t getClass() const;
        IntegerProperty_t getVoltage() const;
        IntegerProperty_t getHeading() const;
        IntegerProperty_t getSpeed() const;
        bool isGrowing() const;
        bool isActive() const;

     private:
        const int m_id;

        IntegerProperty_t m_x;
        IntegerProperty_t m_y;
        IntegerProperty_t m_radius;
        IntegerProperty_t m_voltage;
        IntegerProperty_t m_speed;
        IntegerProperty_t m_heading;
        bool m_isGrowing;       // false if unknown
        String_t m_name;        // "" if unknown

        String_t getDefaultName(afl::string::Translator& tx) const;
    };

} }

#endif
