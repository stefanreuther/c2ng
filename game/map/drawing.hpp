/**
  *  \file game/map/drawing.hpp
  *  \brief Class game::map::Drawing
  */
#ifndef C2NG_GAME_MAP_DRAWING_HPP
#define C2NG_GAME_MAP_DRAWING_HPP

#include "afl/base/types.hpp"
#include "afl/string/string.hpp"
#include "game/map/point.hpp"
#include "util/atomtable.hpp"           // Atom_t
#include "game/config/markeroption.hpp"

namespace game { namespace map {

    class Configuration;

    /** User Drawing.
        This class represents a single user drawing object.

        \todo should we derive this from Object? */
    class Drawing {
     public:
        typedef util::Atom_t Atom_t;

        /** Kind of drawing.
            The numerical values are part of external representation (chartX.cc, script). */
        enum Type {
            LineDrawing      = 0,       ///< Line from A to B.
            RectangleDrawing = 1,       ///< Rectangle between A and B.
            CircleDrawing    = 2,       ///< Circle with center and radius.
            MarkerDrawing    = 3        ///< Marker (tiny symbol) with optional comment; lockable.
        };

        /*
           Limits.
           Those are renderer/user limits, not internal data representation limits.
         */
        static const int NUM_USER_COLORS = 30;       ///< Maximum allowed color value.
        static const int NUM_USER_MARKERS = 8;       ///< Number of user marker kinds.
        static const int MAX_CIRCLE_RADIUS = 5000;   ///< Maximum allowed circle radius.

        /** Constructor.
            \param pos Position (starting point for line, rectangle; center for circle or marker)
            \param type Type */
        Drawing(Point pos, Type type);

        /** Construct marker from template (canned marker).
            \param pos Position
            \param tpl Template */
        Drawing(Point pos, const game::config::MarkerOption::Data& tpl);

        /** Set position.
            \param pos Position (starting point for line, rectangle; center for circle or marker) */
        void setPos(Point pos);

        /** Set other position.
            Valid for LineDrawing, RectangleDrawing.
            \param pos Other position (ending point) */
        void setPos2(Point pos);

        /** Set radius.
            Valid for CircleDrawing.
            \param r Radius */
        void setCircleRadius(int r);

        /** Set marker kind (shape).
            Valid for MarkerDrawing.
            \param k Kind [0,NUM_USER_MARKERS) */
        void setMarkerKind(int k);

        /** Set drawing tag.
            This is a general-purpose numeric value, conventionally an atom.
            \param tag Tag */
        void setTag(Atom_t tag);

        /** Set drawing color.
            In c2ng, we store the user-visible color value (0-30, 0 means invisible).
            PCC1 and PCC2 store internal palette values instead.
            \param color Color */
        void setColor(uint8_t color);

        /** Set comment.
            \param comment Comment */
        void setComment(String_t comment);

        /** Set time of expiry.
            Specifies a turn number. When a turn after that is seen, the drawing is deleted (not loaded).
            \param expire Turn, -1 for never, 0 for immediatly (next load) */
        void setExpire(int expire);

        /** Get type.
            \return Type */
        Type getType() const
            { return m_type; }

        /** Get position.
            \return position
            \see setPos() */
        Point getPos() const
            { return m_pos; }

        /** Get other position.
            Valid for LineDrawing, RectangleDrawing.
            \return other position.
            \see setPos2() */
        Point getPos2() const
            { return Point(m_x2, m_y2); }

        /** Get circle radius.
            Valid for CircleDrawing.
            \return radius
            \see setCircleRadius */
        int getCircleRadius() const
            { return m_x2; }

        /** Get marker kind (shape).
            Valid for MarkerDrawing.
            \return marker kind
            \see setMarkerKind() */
        int getMarkerKind() const
            { return m_x2; }

        /** Get tag.
            \return tag
            \see setTag(). */
        Atom_t getTag() const
            { return m_tag; }

        /** Get color.
            \return color
            \see setColor(). */
        uint8_t getColor() const
            { return m_color; }

        /** Get comment.
            \return comment
            \see setComment() */
        String_t getComment() const
            { return m_comment; }

        /** Get time of expiry.
            \return time of expiry (turn number)
            \see setExpire() */
        int getExpire() const
            { return m_expire; }

        /** Check visibility.
            \return true if drawing is visible */
        bool isVisible() const
            { return m_color != 0; }

        /** Compare for equality.
            \param other Other drawing to compare to
            \return true if both drawings are equivalent. */
        bool equals(const Drawing& other) const;

        /** Compute distance of this drawing to a given point in the game universe.

            Note: You should normally use getDistanceToWrap instead.

            \param pt Point
            \return distance in ly */
        double getDistanceTo(Point pt) const;

        /** Compute distance of this drawing to a given point in the game universe, honoring wrap.
            This is used to select a drawing for editing.

            \param pt Point
            \param config Map configuration
            \return distance in ly
            \see getDistanceTo */
        double getDistanceToWrap(Point pt, const Configuration& config) const;
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
