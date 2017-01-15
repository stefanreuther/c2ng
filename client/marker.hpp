/**
  *  \file client/marker.hpp
  */
#ifndef C2NG_CLIENT_MARKER_HPP
#define C2NG_CLIENT_MARKER_HPP

#include "gfx/context.hpp"

namespace client {

    struct Marker;

    const int NUM_USER_MARKERS = 8;

    void drawMarker(gfx::BaseContext& ctx, const Marker& marker, gfx::Point pt);
    void drawDottedCircle(gfx::BaseContext& ctx, gfx::Point pt, int r);
    void drawSelection(gfx::BaseContext& ctx, gfx::Point pt, int mult, int divi);
    // void drawShipIcon(gfx::BaseContext& ctx, GPlayerRelation relation, bool big, const GfxPoint pt);
    const Marker* getUserMarker(int id, bool big);
    int getMarkerHeight(const Marker& marker);

}

#endif
