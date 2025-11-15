/**
  *  \file game/map/ionstorm.hpp
  *  \brief Class game::map::IonStorm
  */
#ifndef C2NG_GAME_MAP_IONSTORM_HPP
#define C2NG_GAME_MAP_IONSTORM_HPP

#include <vector>
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
        struct Forecast {
            Point center;
            int radius;
            int uncertainity;

            Forecast(Point center, int radius, int uncertainity)
                : center(center), radius(radius), uncertainity(uncertainity)
                { }
        };
        typedef std::vector<Forecast> Forecast_t;
        static const int UNCERTAINITY_LIMIT = 5;

        /** Constructor.
            \param id Id */
        explicit IonStorm(int id);

        /** Destructor. */
        virtual ~IonStorm();

        // Object:
        virtual String_t getName(ObjectName which, afl::string::Translator& tx, const InterpreterInterface& iface) const;
        virtual afl::base::Optional<int> getOwner() const;
        virtual afl::base::Optional<Point> getPosition() const;

        // CircularObject:
        virtual afl::base::Optional<int> getRadius() const;
        virtual afl::base::Optional<int32_t> getRadiusSquared() const;

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
        void setWarpFactor(int speed);

        /** Set heading.
            \param heading Heading */
        void setHeading(int heading);

        /** Set growing/weakening status.
            \param flag True if growing, false if weakening */
        void setIsGrowing(bool flag);

        /** Set parent storm Id (Nu).
            \param parentId Id; 0 if none */
        void setParentId(int parentId);

        /** Get name.
            \param tx Translator
            \return name (same as getName(PlainName), but with fewer dependencies) */
        String_t getName(afl::string::Translator& tx) const;

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
        IntegerProperty_t getWarpFactor() const;

        /** Get growing/weakening status.
            \return true if growing, false if weakening */
        bool isGrowing() const;

        /** Check validity.
            \return true this ion storm is currently active/visible */
        bool isActive() const;

        /** Get parent storm Id (Nu).
            \return Id; 0 if none */
        int getParentId() const;

        /** Add report from a message.
            This will update internal members as required.
            \param info Message information */
        void addMessageInformation(const game::parser::MessageInformation& info);

        /** Compute forecast.
            \param [out] result Forecast, sorted by descending uncertainity (most certain/lowest value last) */
        void getForecast(Forecast_t& result) const;

     private:
        IntegerProperty_t m_x;
        IntegerProperty_t m_y;
        IntegerProperty_t m_radius;
        IntegerProperty_t m_voltage;
        IntegerProperty_t m_speed;
        IntegerProperty_t m_heading;
        bool m_isGrowing;       // false if unknown
        String_t m_name;        // "" if unknown
        int m_parentId;

        String_t getDefaultName(afl::string::Translator& tx) const;
    };

} }

inline void
game::map::IonStorm::setName(String_t name)
{
    m_name = name;
}

inline void
game::map::IonStorm::setPosition(Point pos)
{
    m_x = pos.getX();
    m_y = pos.getY();
}

inline void
game::map::IonStorm::setRadius(int r)
{
    m_radius = r;
}

inline void
game::map::IonStorm::setVoltage(int voltage)
{
    m_voltage = voltage;
}

inline void
game::map::IonStorm::setWarpFactor(int speed)
{
    m_speed = speed;
}

inline void
game::map::IonStorm::setHeading(int heading)
{
    m_heading = heading;
}

inline void
game::map::IonStorm::setIsGrowing(bool flag)
{
    m_isGrowing = flag;
}

inline void
game::map::IonStorm::setParentId(int parentId)
{
    m_parentId = parentId;
}

inline game::IntegerProperty_t
game::map::IonStorm::getVoltage() const
{
    // ex GIonStorm::getVoltage
    return m_voltage;
}

inline game::IntegerProperty_t
game::map::IonStorm::getHeading() const
{
    // ex GIonStorm::getHeading
    return m_heading;
}

inline game::IntegerProperty_t
game::map::IonStorm::getWarpFactor() const
{
    // ex GIonStorm::getSpeed
    return m_speed;
}

inline bool
game::map::IonStorm::isGrowing() const
{
    // ex GIonStorm::isGrowing
    return m_isGrowing;
}

inline int
game::map::IonStorm::getParentId() const
{
    return m_parentId;
}

#endif
