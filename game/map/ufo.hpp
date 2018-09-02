/**
  *  \file game/map/ufo.hpp
  */
#ifndef C2NG_GAME_MAP_UFO_HPP
#define C2NG_GAME_MAP_UFO_HPP

#include "game/map/circularobject.hpp"
#include "game/types.hpp"
#include "afl/bits/smallset.hpp"
#include "game/parser/messageinformation.hpp"

namespace game { namespace map {

    class Ufo : public CircularObject {
     public:
        // Constructor and Destructor
        Ufo(Id_t id);
        //    GUfo(const GUfo& other);
        ~Ufo();
        //    GUfo& operator=(const GUfo& rhs);

        // Object:
        virtual String_t getName(Name which, afl::string::Translator& tx, InterpreterInterface& iface) const;
        virtual Id_t getId() const;
        virtual bool getOwner(int& result) const;

        // MapObject:
        virtual bool getPosition(Point& result) const;

        // CircularObject:
        virtual bool getRadius(int& result) const;
        virtual bool getRadiusSquared(int32_t& result) const;

        // Inquiry:
        bool isValid() const;

        int getColorCode() const;
        void setColorCode(int n);
        //    uint8    getColor() const;
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
        //    GPoint   getOtherEndPos() const;  <-- not used

        void setName(String_t name);
        void setPosition(Point pt);
        void setRadius(IntegerProperty_t r);

        String_t getPlainName() const;

        Point getLastPosition() const;

        int getLastTurn() const;
        void setLastTurn(int n);

        Point getMovementVector() const;
        void setMovementVector(Point vec);

        // Links:
        void disconnect();
        void connectWith(Ufo& other);
        Ufo* getOtherEnd() const;

        //    /* Loader interface */
        //    void addUfoData(const TUfo& data);
        //    void addObjectData(const TUtil33GO& obj);
        //    void addHistoryData(const TDbUfo& data);
        //    void addWormholeData(const TUtil14Wormhole& data, bool all, int turn);
        void addMessageInformation(const game::parser::MessageInformation& info);

        void postprocess(int turn);

        bool isStoredInHistory() const;
        //    void getHistoryData(TDbUfo& data) const;
        void setIsStoredInHistory(bool value);

        bool isSeenThisTurn() const;
        void setIsSeenThisTurn(bool value);

        //    /* Accessors */

        //    void     setMovementVector(GPoint movement_vector);

        //    /* Values for flags */
        //    enum {
        //        UfoSeenThisTurn     = 1,
        //        UfoStoredInDatabase = 2
        //    };

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

        //    void copyUfo(const TUfo& data);
    };

} }

#endif
