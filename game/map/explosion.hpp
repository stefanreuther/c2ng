/**
  *  \file game/map/explosion.hpp
  *  \brief Class game::map::Explosion
  */
#ifndef C2NG_GAME_MAP_EXPLOSION_HPP
#define C2NG_GAME_MAP_EXPLOSION_HPP

#include "game/map/object.hpp"

namespace game { namespace map {

    /** Explosion.

        Explosions come in a variety of forms.
        Winplan clients receive up to 50 explosions that have an explosion Id.
        In addition, there are several messages and utildata records
        that just include facts, but no Id.
        We try to merge these in a clever way.
        However, when getting 10 reports "something exploded at (X,Y)",
        we cannot know whether these all pertain to the same object or to 10 different ones,
        and thus generate only one Explosion object.

        In PCC2 (and c2ng), explosions are a separate data type.
        In PCC 1.x, they were markers with a special tag.
        Implementing them as separate type allows to attach regular information
        in a more meaningful way than by recycling and abusing drawings.

        The disadvantage is that we need to process two info sources when iterating over markers and explosions.
        We therefore have a difference in the script interface:
        - PCC 1.x shows explosions during "ForEach Marker"
        - PCC2 (classic) has no explosion information for scripts as of 20201010.
        - c2ng has a seperate "ForEach Explosion". */
    class Explosion : public Object {
     public:
        /** Constructor.
            \param id Explosion Id (NOT ship Id), can be 0.
            \parmm pos Explosion position */
        Explosion(Id_t id, Point pos);

        /** Copy constructor.
            \param ex Other explosion */
        Explosion(const Explosion& ex);

        /** Destructor. */
        ~Explosion();

        // Object:
        virtual String_t getName(ObjectName which, afl::string::Translator& tx, InterpreterInterface& iface) const;
        virtual Id_t getId() const;
        virtual bool getOwner(int& result) const;
        virtual bool getPosition(Point& result) const;

        /** Get name of ship that exploded here.
            \return name; empty if unknown. */
        String_t getShipName() const;

        /** Get Id of ship that exploded here.
            \return Id; 0 if unknown. */
        Id_t getShipId() const;

        /** Set name of ship that exploded here.
            \param name Name */
        void setShipName(String_t name);

        /** Set Id of ship that exploded here.
            \param id Ship Id */
        void setShipId(Id_t id);

        /** Try to merge information of other explosion record.
            This tests whether these records potentially describe the same explosion and,
            if yes, merges other's information into this one.
            \param other other explosion record
            \return true if merge successful, false if both describe different explosions */
        bool merge(const Explosion& other);

     private:
        Id_t m_id;
        Point m_position;
        String_t m_shipName;
        Id_t m_shipId;
    };

} }

#endif
