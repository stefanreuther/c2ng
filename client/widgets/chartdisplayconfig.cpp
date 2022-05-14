/**
  *  \file client/widgets/chartdisplayconfig.cpp
  *  \brief Class client::widgets::ChartDisplayConfig
  */

#include "client/widgets/chartdisplayconfig.hpp"
#include "afl/base/countof.hpp"
#include "ui/icons/image.hpp"
#include "ui/icons/stylableicon.hpp"
#include "util/translation.hpp"

using game::map::RenderOptions;

namespace {
    struct Option {
        RenderOptions::Option option;
        const char* name;
    };

    const Option OPTIONS[] = {
        { RenderOptions::ShowIonStorms,  N_("Ion storms")        },
        { RenderOptions::ShowMinefields, N_("Minefields")        },
        { RenderOptions::ShowUfos,       N_("Ufos")              },
        { RenderOptions::ShowGrid,       N_("Sector borders")    },
        { RenderOptions::ShowBorders,    N_("Starchart borders") },
        { RenderOptions::ShowDrawings,   N_("Own drawings")      },
        { RenderOptions::ShowSelection,  N_("Selection")         },
        { RenderOptions::ShowLabels,     N_("Object labels")     },
        { RenderOptions::ShowTrails,     N_("Ship trails")       },
        { RenderOptions::ShowShipDots,   N_("Ships are dots")    },
        { RenderOptions::ShowWarpWells,  N_("Warp wells")        },
        { RenderOptions::ShowMessages,   N_("Message flag")      },
    };

    // ex WChartDisplayConfig::drawItem [sort-of]
    const char*const IMAGE_NAMES[client::widgets::ChartDisplayConfig::NUM_VALUES] = {
        "ui.cb0",               // Unchecked
        "ui.cb1",               // Checked
        "ui.cbf",               // Filled
        "ui.cbi",               // Inside
        "ui.cbm",               // Mixed
    };


    /*
     *  Tree node Ids. A tree node has a 32-bit Id. This folds two pieces of information
     *  into one such value:
     *  - the option index into the above flags[] array
     *  - the sub-index for the actual option:
     *      0 - node
     *      1,2,3 - small/regular/scanner leaf
     */

    int32_t makePair(size_t index, int sub)
    {
        return static_cast<int32_t>((index << 3) + sub);
    }

    size_t getIndexFromPair(int32_t pair)
    {
        return (pair >> 3);
    }

    int getSubIndexFromPair(int32_t pair)
    {
        return (pair & 7);
    }
}


client::widgets::ChartDisplayConfig::ChartDisplayConfig(ui::Root& root, afl::string::Translator& tx)
    : TreeListbox(root, 16 /* height */, 25*root.provider().getFont(gfx::FontRequest())->getEmWidth()),
      m_root(root),
      m_deleter(),
      m_options(),
      m_icons(),
      conn_imageChange()
{
    // ex WChartDisplayConfig::WChartDisplayConfig
    init(tx);

    // Images
    conn_imageChange = root.provider().sig_imageChange.add(this, &ChartDisplayConfig::onImageChange);
    onImageChange();

    // Icon click (and space key)
    sig_iconClick.add(this, &ChartDisplayConfig::onIconClick);
}

client::widgets::ChartDisplayConfig::~ChartDisplayConfig()
{ }

void
client::widgets::ChartDisplayConfig::set(game::map::RenderOptions::Area area, const game::map::RenderOptions& opts)
{
    size_t index = area;
    if (index < RenderOptions::NUM_AREAS) {
        m_options[index] = opts;
        render();
    }
}

game::map::RenderOptions
client::widgets::ChartDisplayConfig::get(game::map::RenderOptions::Area area) const
{
    size_t index = area;
    if (index < RenderOptions::NUM_AREAS) {
        return m_options[index];
    } else {
        return game::map::RenderOptions();
    }
}

void
client::widgets::ChartDisplayConfig::init(afl::string::Translator& tx)
{
    // ex WChartDisplayConfig::init, CSettingsGroup.Draw
    for (size_t i = 0; i < countof(OPTIONS); ++i) {
        addNode(makePair(i, 0), 0, tx(OPTIONS[i].name), false);
        addNode(makePair(i, RenderOptions::Normal  + 1), 1, tx("Regular starchart"), false);
        addNode(makePair(i, RenderOptions::Small   + 1), 1, tx("Small starchart"), false);
        addNode(makePair(i, RenderOptions::Scanner + 1), 1, tx("Control screen scanner"), false);
    }
}

void
client::widgets::ChartDisplayConfig::render()
{
    // For all nodes, set a new icon. If this is a change, widget will redraw automatically.
    // If images are not available on the initial draw, this will cause the widget to re-layout, which is not an issue for now as far as I can tell.
    size_t i = 0;
    while (Node* n = getNodeByIndex(i)) {
        Value v = getValue(getIdFromNode(n));
        setIcon(n, m_icons[v]);
        ++i;
    }
}

client::widgets::ChartDisplayConfig::Value
client::widgets::ChartDisplayConfig::getValue(int32_t id) const
{
    // WChartDisplayConfig::getValue(int32_t id), CChartOptionCheckbox.GetImage [totally rewritten]
    size_t index = getIndexFromPair(id);
    int sub = getSubIndexFromPair(id);
    RenderOptions::Option opt = OPTIONS[index].option;

    RenderOptions::Value val;
    if (sub == 0) {
        val = m_options[0].getOption(opt);
        if (val != m_options[1].getOption(opt) || val != m_options[2].getOption(opt)) {
            return Mixed;
        }
    } else {
        val = m_options[sub-1].getOption(opt);
    }

    switch (val) {
     case RenderOptions::Disabled:
        return Unchecked;
     case RenderOptions::Enabled:
        return Checked;
     case RenderOptions::Filled:
        return (opt == RenderOptions::ShowGrid ? Inside : Filled);
    }
    return Unchecked;
}

void
client::widgets::ChartDisplayConfig::onIconClick(int32_t id)
{
    // ex WChartDisplayConfig::onClick
    size_t index = getIndexFromPair(id);
    int sub = getSubIndexFromPair(id);
    RenderOptions::Options_t option = RenderOptions::Options_t(OPTIONS[index].option);
    if (sub == 0) {
        // Node
        if (getValue(id) == Mixed) {
            // Mixed values: enable all to make them identical
            m_options[0].setOptions(option);
        } else {
            // Identical values: toggle
            m_options[0].toggleOptions(option);
        }
        m_options[1].copyOptions(m_options[0], option);
        m_options[2].copyOptions(m_options[0], option);
    } else {
        // Single option
        m_options[sub-1].toggleOptions(option);
    }
    render();
}

void
client::widgets::ChartDisplayConfig::onImageChange()
{
    // Check whether there are unloaded images, and if so, load them.
    bool did = false;
    for (size_t i = 0; i < NUM_VALUES; ++i) {
        if (m_icons[i] == 0) {
            afl::base::Ptr<gfx::Canvas> image = m_root.provider().getImage(IMAGE_NAMES[i]);
            if (image.get() != 0) {
                // Make a padded image to give user more space for clicking (same as in PCC2)
                ui::icons::Image& img = m_deleter.addNew(new ui::icons::Image(*image));
                ui::icons::StylableIcon& icon = m_deleter.addNew(new ui::icons::StylableIcon(img, m_root.colorScheme()));
                icon.setPaddingBefore(gfx::Point(2, 2));
                icon.setPaddingAfter(gfx::Point(2, 2));
                m_icons[i] = &icon;
                did = true;
            }
        }
    }

    // If we loaded images, render. Widget will redraw when this produces an actual change.
    if (did) {
        render();
    }
}
