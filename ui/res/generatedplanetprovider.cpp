/**
  *  \file ui/res/generatedplanetprovider.cpp
  */

#include "ui/res/generatedplanetprovider.hpp"
#include "ui/res/resid.hpp"
#include "util/randomnumbergenerator.hpp"
#include "gfx/gen/planetconfig.hpp"

ui::res::GeneratedPlanetProvider::GeneratedPlanetProvider()
{ }

ui::res::GeneratedPlanetProvider::~GeneratedPlanetProvider()
{ }

afl::base::Ptr<gfx::Canvas>
ui::res::GeneratedPlanetProvider::loadImage(String_t name, Manager& /*mgr*/)
{
    int a, b;
    if (matchResourceId(name, PLANET, a, b)) {
        return renderPlanet(a, b);
    } else if (matchResourceId(name, PLANET, a)) {
        return renderPlanet(a, 1);
    } else {
        return 0;
    }
}

afl::base::Ptr<gfx::Canvas>
ui::res::GeneratedPlanetProvider::renderPlanet(int temp, int id)
{
    util::RandomNumberGenerator rng(id);

    gfx::gen::PlanetConfig cfg;
    cfg.setSize(gfx::Point(100, 100));
    cfg.setPlanetRadius(40 + ((id ^ (id>>2) ^ (id>>4) ^ (id>>7)) & 7));
    cfg.setPlanetTemperature(temp);

    afl::base::Ref<gfx::RGBAPixmap> pix = cfg.render(rng);

    return pix->makeCanvas().asPtr();
}
