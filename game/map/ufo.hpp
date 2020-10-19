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
        virtual Id_t getId() const;
        virtual bool getOwner(int& result) const;
        virtual bool getPosition(Point& result) const;

        // CircularObject:
        virtual bool getRadius(int& result) const;
        virtual bool getRadiusSquared(int32_t& result) const;

        // Inquiry:
        bool isValid() const;

        int getColorCode() const;
        void setColorCode(int n);
        IntegerProperty_t getSpeed() const;
        void setSpeed(IntegerProperty_t speed);
        IntegerProperty_t getHeading() const;
        void setHeading(IntegerProperty_t heading);
        IntegerProperty_t getPlanetRange() const;
        void setPlanetRange(IntegerProperty_t range);
        IntegerProperty_t getShipRange() const;
        void setShipRange(IntegerProperty_t range);
        IntegerProperty_t getTypeCode() const;
        void setTypeCode(IntegerProperty_t typeCode);

        String_t getInfo1() const;
        void setInfo1(String_t info);
        String_t getInfo2() const;
        void setInfo2(String_t info);
        int32_t getRealId() const;
        void setRealId(int32_t id);

        void setName(String_t name);
        void setPosition(Point pt);
        void setRadius(IntegerProperty_t r);

        String_t getPlainName() const;

        Point getLastPosition() const;

        int getLastTurn() const;
        void setLastTurn(int n);

        Point getMovementVector() const;
        void setMovementVector(Point vec);

        /*
         *  Links
         */

        /** Disconnect from other Ufo.
            \post getOtherEnd()=0 */
        void disconnect();

        /** Connect with another Ufo.
            This creates a bidirectional link.
            If either end is already connected, that connection is removed first.
            \param other Other Ufo
            \post getOtherEnd()=&other */
        void connectWith(Ufo& other);

        /** Get other end.
            \return Other end */
        Ufo* getOtherEnd() const;

        /*
         *  Loader interface
         */

        /** Add message information.
            \param info Message information addressed at this Ufo */
        void addMessageInformation(const game::parser::MessageInformation& info);

        /** Postprocess after loading.
            \param turn Current turn
            \param mapConfig Map config */
        void postprocess(int turn, const Configuration& mapConfig);

        bool isStoredInHistory() const;
        void setIsStoredInHistory(bool value);

        bool isSeenThisTurn() const;
        void setIsSeenThisTurn(bool value);

     private:
        enum Flag {
            UfoSeenThisTurn,
            UfoStoredInDatabase
        };

        /* Standard Ufo properties */
        Id_t m_id;                           // id
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
