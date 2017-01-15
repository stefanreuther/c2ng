/**
  *  \file game/map/drawing.hpp
  */
#ifndef C2NG_GAME_MAP_DRAWING_HPP
#define C2NG_GAME_MAP_DRAWING_HPP

#include "game/map/point.hpp"
#include "util/atomtable.hpp"
#include "afl/string/string.hpp"
#include "afl/base/types.hpp"

namespace game { namespace map {

    class Configuration;

    /** User Drawing. This class represents a single user drawing object.

        \todo should we derive this from GMapObject?
        \todo make four derived classes? */
    class Drawing {
     public:
        typedef util::Atom_t Atom_t;

        /** Kind of drawing.
            FIXME --> This matches the "type of element" field in chartX.cc. */
        enum Type {
            LineDrawing      = 0,       ///< Line from A to B.
            RectangleDrawing = 1,       ///< Rectangle between A and B.
            CircleDrawing    = 2,       ///< Circle with center and radius.
            MarkerDrawing    = 3        ///< Marker (tiny symbol) with optional comment; lockable.
        };

        Drawing(Point pos, Type type);

        void setPos(Point pos);
        void setPos2(Point pos);
        void setCircleRadius(int r);
        void setMarkerKind(int k);
        void setTag(Atom_t tag);
        void setColor(uint8_t color);
        void setComment(String_t comment);
        void setExpire(int expire);

        Type getType() const
            { return m_type; }
        Point getPos() const
            { return m_pos; }
        Point getPos2() const
            { return Point(m_x2, m_y2); }
        int getCircleRadius() const
            { return m_x2; }
        int getMarkerKind() const
            { return m_x2; }
        Atom_t getTag() const
            { return m_tag; }
        uint8_t getColor() const
            { return m_color; }
        String_t getComment() const
            { return m_comment; }
        int getExpire() const
            { return m_expire; }

        bool isVisible() const
            { return m_color != 0; }

        double getDistanceTo(Point pt, const Configuration& config) const;
        double getDistanceToWrap(Point pt, const Configuration& config) const;

        // static int getUserColorFromColor(uint8 color);
        // static uint8 getColorFromUserColor(int user_color);

        enum { NUM_USER_COLORS = 30 };
     private:
        Point m_pos;                 ///< Center position or left/top.
        Type m_type;                 ///< Kind of drawing.
        uint8_t m_color;             ///< Color.
        int m_x2, m_y2;              ///< Radius, kind, or bottom/right.
        Atom_t m_tag;                ///< User-defined marking.
        int m_expire;                ///< Turn of expiry.
        String_t m_comment;          ///< User comment.
    };

} }

#endif
