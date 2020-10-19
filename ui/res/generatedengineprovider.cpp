/**
  *  \file ui/res/generatedengineprovider.cpp
  *  \brief Class ui::res::GeneratedEngineProvider
  */

#include "ui/res/generatedengineprovider.hpp"
#include "gfx/basecontext.hpp"
#include "gfx/complex.hpp"
#include "gfx/palettizedpixmap.hpp"
#include "ui/colorscheme.hpp"
#include "ui/res/resid.hpp"
#include "util/stringparser.hpp"

ui::res::GeneratedEngineProvider::GeneratedEngineProvider(afl::base::Ref<gfx::Font> font, afl::string::Translator& tx)
    : m_font(font),
      m_translator(tx)
{ }

ui::res::GeneratedEngineProvider::~GeneratedEngineProvider()
{ }

afl::base::Ptr<gfx::Canvas>
ui::res::GeneratedEngineProvider::loadImage(String_t name, Manager& /*mgr*/)
{
    // name is "engine.TYPE.FAC|FAC|FAC|FAC..."; see client::PictureNamer.
    util::StringParser p(name);
    int id, value;
    if (p.parseString(RESOURCE_ID("engine."))
        && p.parseInt(id)
        && p.parseString(".")
        && p.parseInt(value))
    {
        std::vector<int> fuelUsage;
        do {
            fuelUsage.push_back(value);
        } while (p.parseString("|") && p.parseInt(value));
        if (p.parseEnd()) {
            return renderEngineDiagram(fuelUsage, gfx::Point(100, 100), *m_font, m_translator).asPtr();
        }
    }
    return 0;
}

afl::base::Ref<gfx::Canvas>
ui::res::renderEngineDiagram(const std::vector<int>& fuelUsage, gfx::Point size, gfx::Font& font, afl::string::Translator& tx)
{
    // ex WEngineInfoDiagram::drawContent (roughly)
    int diagHeight = size.getY() - font.getLineHeight() - 1;       // -1 because drawVLine includes the final coordinate

    // Create canvas
    afl::base::Ref<gfx::PalettizedPixmap> result(gfx::PalettizedPixmap::create(size.getX(), size.getY()));
    afl::base::Ref<gfx::Canvas> can(result->makeCanvas());

    result->setPalette(0, ui::STANDARD_COLORS);
    gfx::BaseContext ctx(*can);

    // Legend (bottom)
    ctx.setTextAlign(1, 0);
    ctx.useFont(font);
    ctx.setRawColor(ui::Color_White);
    outText(ctx, gfx::Point(size.getX()/2, diagHeight + 1), tx("Warp \xE2\x86\x92"));

    ctx.setTextAlign(0, 0);
    outText(ctx, gfx::Point(), tx("Fuel used per ly"));

    // Diagram
    int prevFF = 0;
    int prevX = 0;
    int max = int(fuelUsage.size());
    for (int i = 0; i <= max; ++i) {
        // For a normal engine, we have 9 speeds in fuelUsage, but we need to draw 10 segments
        // using 11 points (warp 0 = 0, warp 10 = 1000).
        const int newX = size.getX()*(i+1) / (max+1);
        const int newFF = std::min(999, (i >= max ? 1000 : fuelUsage[i]));
        const int deltaX = newX - prevX;

        // Draw the bars
        for (int dx = 1; dx < deltaX; ++dx) {
            const int thisFF    = (dx*newFF + (deltaX-dx)*prevFF) / deltaX;
            const int thisColor = ui::Color_Status+15 - (16*thisFF)/1000;
            const int thisY     = thisFF*diagHeight/1000;
            ctx.setRawColor(thisColor);
            drawVLine(ctx, dx + prevX, diagHeight - thisY, diagHeight);
        }
        prevX = newX;
        prevFF = newFF;
    }

    return can;
}
