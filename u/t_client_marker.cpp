/**
  *  \file u/t_client_marker.cpp
  *  \brief Test for client::Marker
  */

#include "client/marker.hpp"

#include "t_client.hpp"
#include "gfx/basecontext.hpp"
#include "gfx/rgbapixmap.hpp"

/** Test user marker access, basic litmus test. */
void
TestClientMarker::testUserAccess()
{
    // Type 0 must exist
    const client::Marker* p = client::getUserMarker(0, true);
    TS_ASSERT(p != 0);

    // Height must be nonzero
    TS_ASSERT(client::getMarkerHeight(*p) != 0);
}

/** Test ship markers.
    Own and enemy must not cancel out each other. */
void
TestClientMarker::testShip()
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
    TS_ASSERT_DIFFERS(pix->pixels().find(ME), size);
    TS_ASSERT_DIFFERS(pix->pixels().find(THEM), size);
}

