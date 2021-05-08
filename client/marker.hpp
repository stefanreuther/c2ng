/**
  *  \file client/marker.hpp
  *  \brief Marker drawing
  *
  *  Markers are small symbols, mainly used for the marker drawings users can put
  *  into the starchart, but also some other places.
  */
#ifndef C2NG_CLIENT_MARKER_HPP
#define C2NG_CLIENT_MARKER_HPP

#include "gfx/context.hpp"

namespace client {

    struct Marker;

    const int NUM_USER_MARKERS = 8;

    /** Draw marker.
        \param ctx    Context (in particular, color selected)
        \param marker Marker to draw
        \param pt     Position */
    void drawMarker(gfx::BaseContext& ctx, const Marker& marker, gfx::Point pt);

    /** Draw dotted circle.
        Used for unowned planets.
        \param ctx    Context (in particular, color selected)
        \param pt     Position
        \param r      Radius */
    void drawDottedCircle(gfx::BaseContext& ctx, gfx::Point pt, int r);

    /** Draw selection marker.
        Supports all zoom levels, using lines for large levels,
        and a handmade marker for small ones.
        \param ctx    Context (in particular, color selected)
        \param pt     Position
        \param mult   Zoom multiplier
        \param divi   Zoom divider */
    void drawSelection(gfx::BaseContext& ctx, gfx::Point pt, int mult, int divi);

    /** Draw message marker.
        Supports all zoom levels, using lines for large levels,
        and a handmade marker for small ones.
        \param ctx    Context (in particular, color selected)
        \param pt     Position
        \param mult   Zoom multiplier
        \param divi   Zoom divider */
    void drawMessageMarker(gfx::BaseContext& ctx, const gfx::Point pt, int mult, int divi);

    /** Draw ship marker (for ships-are-triangles mode).
        \param ctx    Context (in particular, color selected)
        \param pt     Position
        \param isMe   true for our ships, false for others
        \param big    true for normal/big symbol, false for small one */
    void drawShipIcon(gfx::BaseContext& ctx, const gfx::Point pt, bool isMe, bool big);

    /** Get user marker definition.
        \param kind   Marker kind; see game::map::Drawing::getMarkerKind
        \param big    true for normal/big symbol, false for small one
        \return marker; null if kind is unknown */
    const Marker* getUserMarker(int kind, bool big);

    /** Get height of a marker.
        \param marker Marker
        \return height, i.e. amount to add to Y coordinate to place label text below the marker */
    int getMarkerHeight(const Marker& marker);

}

#endif
