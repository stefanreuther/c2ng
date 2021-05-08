/**
  *  \file client/widgets/vcrobjectinfo.cpp
  *  \brief Class client::widgets::VcrObjectInfo
  */

#include "client/widgets/vcrobjectinfo.hpp"
#include "afl/functional/stringtable.hpp"
#include "util/translation.hpp"

using game::vcr::Range_t;
using afl::string::Format;

namespace {
    gfx::Point getPreferredSize(bool fullInfo, gfx::ResourceProvider& provider)
    {
        // ex WVcrInfoMain::getLayoutInfo
        int lines = fullInfo ? 12 : 10;
        return provider.getFont(gfx::FontRequest())->getCellSize().scaledBy(35, lines);
    }

    /*
     *  Ship Page
     */

    struct ShipMetrics {
        int x1;
        int x2;
    };

    void addShipInfo(ui::rich::Document& doc, const ShipMetrics& m, String_t heading, const game::vcr::ShipInfoItem_t& info)
    {
        doc.add(heading);
        doc.addAt(m.x1, info.first);
        doc.addAt(m.x2, info.second);

        // Make sure line isn't entirely empty
        if (heading.empty() && info.first.empty() && info.second.empty()) {
            doc.addAt(m.x1, " ");
        }
        doc.addNewline();
    }

    /*
     *  Planet Page
     */

    struct PlanetMetrics {
        int x1;
    };

    void addPlanetInfo(ui::rich::Document& doc, const PlanetMetrics& m, String_t heading, Range_t range, Range_t maxRange, const util::NumberFormatter& fmt, afl::string::Translator& tx)
    {
        doc.add(heading);
        doc.addAt(m.x1, toString(range, maxRange, false, fmt, tx));
        doc.addNewline();
    }
}

client::widgets::VcrObjectInfo::VcrObjectInfo(bool fullInfo, util::NumberFormatter fmt, afl::string::Translator& tx, gfx::ResourceProvider& provider)
    : DocumentView(getPreferredSize(fullInfo, provider), 0, provider),
      m_fullInfo(fullInfo),
      m_formatter(fmt),
      m_translator(tx),
      m_provider(provider)
{
    // ex WVcrInfoMain::WVcrInfoMain
}

client::widgets::VcrObjectInfo::~VcrObjectInfo()
{ }

void
client::widgets::VcrObjectInfo::setShipInfo(const game::vcr::ShipInfo& info)
{
    // ex showShipInfo
    static const char*const headings[] = {
        N_("Primary"),
        N_("Secondary"),
        N_("Tech level"),
        N_("Mass"),
        N_("Fuel"),
        N_("Engines"),
        N_("Crew"),
        N_("Experience"),
        N_("Shield"),
        N_("Damage"),
    };

    afl::string::Translator& tx = m_translator;
    ui::rich::Document& doc = getDocument();
    doc.clear();

    afl::base::Ref<gfx::Font> font = m_provider.getFont(gfx::FontRequest());
    ShipMetrics m;
    const int em = font->getEmWidth();
    m.x1 = font->getMaxTextWidth(afl::functional::createStringTable(headings).map(m_translator)) + 2*em;
    m.x2 = m.x1 + 16*em;

    doc.addAt(m.x1, util::rich::Text(tx("Ship")).withStyle(util::rich::StyleAttribute::Underline));
    doc.addAt(m.x2, util::rich::Text(tx("Hull (max)")).withStyle(util::rich::StyleAttribute::Underline));
    doc.addNewline();

    addShipInfo(doc, m, tx(headings[0]), info.primary);
    addShipInfo(doc, m, tx(headings[1]), info.secondary);
    addShipInfo(doc, m, String_t(),      info.ammo);
    addShipInfo(doc, m, tx(headings[6]), info.crew);
    if (m_fullInfo && !info.experienceLevel.first.empty()) {
        addShipInfo(doc, m, tx(headings[7]), info.experienceLevel);
    }
    addShipInfo(doc, m, tx(headings[2]), info.techLevel);
    addShipInfo(doc, m, tx(headings[3]), info.mass);
    if (m_fullInfo) {
        addShipInfo(doc, m, tx(headings[8]), info.shield);
        addShipInfo(doc, m, tx(headings[9]), info.damage);
    }
    addShipInfo(doc, m, tx(headings[4]), info.fuel);
    addShipInfo(doc, m, tx(headings[5]), info.engine);

    doc.finish();
    handleDocumentUpdate();
}

void
client::widgets::VcrObjectInfo::setPlanetInfo(const game::vcr::PlanetInfo& info)
{
    // ex showPlanetInfo
    static const char*const planetHeadings[] = {
        N_("Combat Mass"),
        N_("Starbase"),
        N_("Planetary Defense"),
        N_("Starbase Defense"),
        N_("Starbase Fighters"),
        N_("Beam Tech"),
    };

    afl::base::Ref<gfx::Font> font = m_provider.getFont(gfx::FontRequest());
    PlanetMetrics m;
    const int em = font->getEmWidth();
    const int x0 = 2*em;
    m.x1 = font->getMaxTextWidth(afl::functional::createStringTable(planetHeadings).map(m_translator)) + x0 + em;

    afl::string::Translator& tx = m_translator;
    ui::rich::Document& doc = getDocument();
    util::NumberFormatter fmt = m_formatter;
    doc.clear();

    doc.add(tx(planetHeadings[0]));
    doc.addAt(m.x1, String_t(Format(tx("%d kt"), fmt.formatNumber(info.mass))));
    doc.addParagraph();

    if (!info.isValid) {
        doc.add(tx("Unable to determine derived information."));
        doc.addNewline();
    } else {
        doc.add(tx("Derived Information:"));
        doc.addNewline();
        doc.setLeftMargin(x0);

        doc.add(tx(planetHeadings[1]));
        doc.addAt(m.x1, info.hasBase ? tx("yes") : tx("no"));
        doc.addNewline();

        addPlanetInfo(doc, m, tx(planetHeadings[2]), info.defense, Range_t(0, game::MAX_NUMBER), fmt, tx);
        if (info.hasBase) {
            addPlanetInfo(doc, m, tx(planetHeadings[3]), info.baseDefense,     Range_t(0, info.maxBaseDefense),  fmt, tx);
            addPlanetInfo(doc, m, tx(planetHeadings[4]), info.numBaseFighters, Range_t(0, info.maxBaseFighters), fmt, tx);
            addPlanetInfo(doc, m, tx(planetHeadings[5]), info.baseBeamTech,    Range_t(1, 10),                   fmt, tx);
        }
    }

    doc.finish();
    handleDocumentUpdate();
}

void
client::widgets::VcrObjectInfo::clear()
{
    getDocument().clear();
    handleDocumentUpdate();
}
