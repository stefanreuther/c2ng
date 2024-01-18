/**
  *  \file test/client/markertest.cpp
  *  \brief Test for client::Marker
  */

#include "client/marker.hpp"

#include "afl/test/testrunner.hpp"
#include "gfx/basecontext.hpp"
#include "gfx/rgbapixmap.hpp"

/** Test user marker access, basic litmus test. */
AFL_TEST("client.Marker:getUserMarker", a)
{
    // Type 0 must exist
    const client::Marker* p = client::getUserMarker(0, true);
    a.checkNonNull("01. getUserMarker", p);

    // Height must be nonzero
    a.checkDifferent("11. getMarkerHeight", client::getMarkerHeight(*p), 0);
}

/** Test ship markers.
    Own and enemy must not cancel out each other. */
AFL_TEST("client.Marker:drawShipIcon", a)
{
    afl::base::Ref<gfx::RGBAPixmap> pix = gfx::RGBAPixmap::create(20, 20);
    afl::base::Ref<gfx::Canvas> can(pix->makeCanvas());

    const gfx::ColorQuad_t ME = COLORQUAD_FROM_RGB(0, 0, 100);
    const gfx::ColorQuad_t THEM = COLORQUAD_FROM_RGB(0, 100, 0);
    const gfx::Point POS(10, 10);

    // Draw own ship
    gfx::BaseContext ctx(*can);
    ctx.setRawColor(ME);
    client::drawShipIcon(ctx, POS, true, true);

    // Draw enemy ship
    ctx.setRawColor(THEM);
    client::drawShipIcon(ctx, POS, false, true);

    // There must be both ME and THEM pixels
    size_t size = pix->pixels().size();
    a.checkDifferent("01. me", pix->pixels().find(ME), size);
    a.checkDifferent("02. them", pix->pixels().find(THEM), size);
}
