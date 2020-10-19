/**
  *  \file game/map/ionstorm.hpp
  *  \brief Class game::map::IonStorm
  */
#ifndef C2NG_GAME_MAP_IONSTORM_HPP
#define C2NG_GAME_MAP_IONSTORM_HPP

#include "game/map/circularobject.hpp"
#include "game/map/point.hpp"
#include "game/parser/messageinformation.hpp"
#include "game/types.hpp"

namespace game { namespace map {

    /** Ion storm.
        This is just a simple data container with no history or other logic.

        An ion storm is considered valid/existing (isValid()) if it has a known nonzero voltage.
        At this point, it should also have a position and radius. */
    class IonStorm : public CircularObject {
     public:
        /** Constructor.
            \param id Id */
        explicit IonStorm(int id);

        /** Destructor. */
        virtual ~IonStorm();

        // Object:
        virtual String_t getName(ObjectName which, afl::string::Translator& tx, InterpreterInterface& iface) const;
        virtual Id_t getId() const;
        virtual bool getOwner(int& result) const;
        virtual bool getPosition(Point& result) const;

        // CircularObject:
        virtual bool getRadius(int& result) const;
        virtual bool getRadiusSquared(int32_t& result) const;

        /** Set name of ion storm.
            \param name Name */
        void setName(String_t name);

        /** Set position.
            \param pos Position */
        void setPosition(Point pos);

        /** Set radius.
            \param r Radiu s */
        void setRadius(int r);

        /** Set voltage.
            Note that setting the voltage does not implicitly set the growing/weakening status.
            \param voltage Voltage */
        void setVoltage(int voltage);

        /** Set speed.
            \param speed Speed */
        void setSpeed(int speed);

        /** Set heading.
            \param heading Heading */
        void setHeading(int heading);

        /** Set growing/weakening status.
            \param flag True if growing, false if weakening */
        void setIsGrowing(bool flag);

        /** Get ion storm class.
            The class is derived from the voltage.
            \return class [1,5] if known */
        IntegerProperty_t getClass() const;

        /** Get voltage.
            \return voltage */
        IntegerProperty_t getVoltage() const;

        /** Get heading.
            \return heading */
        IntegerProperty_t getHeading() const;

        /** Get speed.
            \return speed */
        IntegerProperty_t getSpeed() const;

        /** Get growing/weakening status.
            \return true if growing, false if weakening */
        bool isGrowing() const;

        /** Check validity.
            \return true this ion storm is currently active/visible */
        bool isActive() const;

        /** Add report from a message.
            This will update internal members as required.
            \param info Message information */
        void addMessageInformation(const game::parser::MessageInformation& info);

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
