/**
  *  \file game/map/ufo.hpp
  *  \brief Class game::map::Ufo
  */
#ifndef C2NG_GAME_MAP_UFO_HPP
#define C2NG_GAME_MAP_UFO_HPP

#include "afl/bits/smallset.hpp"
#include "game/map/circularobject.hpp"
#include "game/parser/messageinformation.hpp"
#include "game/types.hpp"

namespace game { namespace map {

    class Configuration;

    /** Ufo.
        Represents an Ufo, General Object, or Wormhole.
        These objects have certain informative properties, and in addition:
        - optional connection to another object (Wormhole connection)
        - can be stored in history database

        \see UfoType for details. */
    class Ufo : public CircularObject {
     public:
        /** Constructor.
            \param id Id */
        explicit Ufo(Id_t id);

        /** Destructor. */
        ~Ufo();

        // Object:
        virtual String_t getName(ObjectName which, afl::string::Translator& tx, InterpreterInterface& iface) const;
        virtual afl::base::Optional<int> getOwner() const;
        virtual afl::base::Optional<Point> getPosition() const;

        // CircularObject:
        virtual afl::base::Optional<int> getRadius() const;
        virtual afl::base::Optional<int32_t> getRadiusSquared() const;


        /*
         *  Inquiry
         */

        /** Check whether this Ufo is valid.
            @return flag */
        bool isValid() const;

        /** Get color code.
            @return color */
        int getColorCode() const;

        /** Set color code.
            @param n color code (VGA color, [0,15]) */
        void setColorCode(int n);

        /** Get speed.
            @return warp factor */
        IntegerProperty_t getWarpFactor() const;

        /** Set speed.
            @param speed Warp factor */
        void setWarpFactor(IntegerProperty_t speed);

        /** Get heading.
            @return heading */
        IntegerProperty_t getHeading() const;

        /** Set heading.
            @param heading Heading */
        void setHeading(IntegerProperty_t heading);

        /** Get visibility range from planets.
            @return range */
        IntegerProperty_t getPlanetRange() const;

        /** Set visibility range from planets.
            @param range Range */
        void setPlanetRange(IntegerProperty_t range);

        /** Get visibility range from ships.
            @return range */
        IntegerProperty_t getShipRange() const;

        /** Set visibility range from ships.
            @param range Range */
        void setShipRange(IntegerProperty_t range);

        /** Get type code.
            @return code */
        IntegerProperty_t getTypeCode() const;

        /** Set type code.
            @param typeCode Code */
        void setTypeCode(IntegerProperty_t typeCode);

        /** Get information string 1.
            @return text */
        String_t getInfo1() const;

        /** Set information string 1.
            @param info Text */
        void setInfo1(String_t info);

        /** Get information string 2.
            @return text */
        String_t getInfo2() const;

        /** Set information string 2.
            @param info Text */
        void setInfo2(String_t info);

        /** Get real Id.
            @return Id */
        int32_t getRealId() const;

        /** Set real Id.
            @param id Id */
        void setRealId(int32_t id);

        /** Set name.
            @param name Name */
        void setName(String_t name);

        /** Set center position.
            @param pt Center position */
        void setPosition(Point pt);

        /** Set radius.
            @param r Radius */
        void setRadius(IntegerProperty_t r);

        /** Get plain name.
            Same as getName(PlainName), without the extra dependencies.
            @return name */
        String_t getPlainName() const;

        /** Get positiion at which Ufo was last seen.
            @return position */
        Point getLastPosition() const;

        /** Get turn number when Ufo was last seen.
            @return turn number */
        int getLastTurn() const;

        /** Get movement vector.
            @return average movement per turn */
        Point getMovementVector() const;

        /** Set movement vector.
            @param vec Vector */
        void setMovementVector(Point vec);


        /*
         *  Links
         */

        /** Disconnect from other Ufo.
            @post getOtherEnd()=0 */
        void disconnect();

        /** Connect with another Ufo.
            This creates a bidirectional link.
            If either end is already connected, that connection is removed first.
            @param other Other Ufo
            @post getOtherEnd()=&other */
        void connectWith(Ufo& other);

        /** Get other end.
            @return Other end */
        Ufo* getOtherEnd() const;

        /*
         *  Loader interface
         */

        /** Add message information.
            @param info Message information addressed at this Ufo */
        void addMessageInformation(const game::parser::MessageInformation& info);

        /** Postprocess after loading.
            @param turn Current turn
            @param mapConfig Map config */
        void postprocess(int turn, const Configuration& mapConfig);

        /** Get stored-in-history flag.
            @return flag */
        bool isStoredInHistory() const;

        /** Set whether Ufo is stored in history.
            @param value Flag */
        void setIsStoredInHistory(bool value);

        /** Check whether Ufo was seen this turn.
            @return flag */
        bool isSeenThisTurn() const;

        /** Set whether Ufo was seen this turn.
            @param value Flag */
        void setIsSeenThisTurn(bool value);

     private:
        enum Flag {
            UfoSeenThisTurn,
            UfoStoredInDatabase
        };

        /* Standard Ufo properties */
        int  m_colorCode;                    // vga_color
        Point m_position;                    // pos
        IntegerProperty_t m_speed;           // warp;
        IntegerProperty_t m_heading;         // heading
        IntegerProperty_t m_planetRange;     // planet_range
        IntegerProperty_t m_shipRange;       // ship_range
        IntegerProperty_t m_radius;          // radius
        IntegerProperty_t m_typeCode;        // type_code
        String_t m_name;                     // name
        String_t m_info1;                    // info1
        String_t m_info2;                    // info2

        /* Additional PCC properties */
        int32_t m_realId;                    // real_id;
        int m_turnLastSeen;                  // turn_last_seen;
        Point m_posLastSeen;                 // pos_last_seen;
        Point m_movementVector;              // movement_vector;
        afl::bits::SmallSet<Flag> m_flags;   // flags
        Ufo* m_otherEnd;                     // other_end
    };

} }

#endif
