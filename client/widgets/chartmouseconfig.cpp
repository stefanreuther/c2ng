/**
  *  \file client/widgets/chartmouseconfig.cpp
  *  \brief Class client::widgets::ChartMouseConfig
  */

#include "client/widgets/chartmouseconfig.hpp"
#include "afl/base/countof.hpp"
#include "afl/base/staticassert.hpp"
#include "game/map/locker.hpp"
#include "ui/icons/image.hpp"
#include "ui/icons/stylableicon.hpp"
#include "util/translation.hpp"

using client::widgets::ChartMouseConfig;
using game::config::UserConfiguration;

namespace {
    struct Option {
        int flag;
        const char* name;
    };

    // Options for each mode
    const Option LOCK_FLAGS[] = {
        { game::map::MatchPlanets,       N_("Planets") },
        { game::map::MatchShips,         N_("Ships") },
        { game::map::MatchUfos,          N_("Ufos") },
        { game::map::MatchDrawings,      N_("Markers") },
        { game::map::MatchMinefields,    N_("Minefields") }
    };

    const char*const LOCK_MODES[] = {
        N_("Left mouse button / space bar locks on..."),
        N_("Right mouse button / Enter locks on..."),
    };
    const size_t NUM_LOCK_MODES = countof(LOCK_MODES);

    const char*const WHEEL_MODES[] = {
        N_("Zoom (+/-)"),
        N_("Browse list (Tab)"),
        N_("Cycle units (PgUp/PgDn)"),
    };
    static_assert(UserConfiguration::WheelZoom   == 0, "WheelZoom");
    static_assert(UserConfiguration::WheelBrowse == 1, "WheelBrowse");
    static_assert(UserConfiguration::WheelPage   == 2, "WheelPage");

    // Overall
    enum Mode {
        MODE_LEFT,
        MODE_RIGHT,
        MODE_WHEEL
    };

    // ex WChartLockConfig::drawItem [sort-of]
    const char*const IMAGE_NAMES[ChartMouseConfig::NUM_VALUES] = {
        "ui.cb0",               // Unchecked
        "ui.cb1",               // Checked
        "ui.radio0",            // Unselected
        "ui.radio1",            // Selected
        0,                      // None
    };
    static const int8_t IMAGE_PADDING[ChartMouseConfig::NUM_VALUES] = {
        2, 2, 0, 0, 0
    };


    /*
     *  Tree node Ids. A tree node has a 32-bit Id. This folds two pieces of information
     *  into one such value:
     *  - the mode index into the modes[]/opts[] array
     *  - the sub-index for the actual flag:
     *      0 - node
     *      1,2,3 - index into flags[], plus one, or wheel mode, plus one
     */

    int32_t makePair(Mode mode, size_t sub)
    {
        return (mode << 5) + int(sub);
    }

    Mode getModeFromPair(int32_t pair)
    {
        return Mode(pair >> 5);
    }

    size_t getSubIndexFromPair(int32_t pair)
    {
        return (pair & 31);
    }
}


client::widgets::ChartMouseConfig::ChartMouseConfig(ui::Root& root, afl::string::Translator& tx)
    : TreeListbox(root, 16 /* height */, 25*root.provider().getFont(gfx::FontRequest())->getEmWidth()),
      m_root(root),
      m_deleter(),
      m_leftLock(),
      m_rightLock(),
      m_wheelMode(),
      m_icons(),
      conn_imageChange()
{
    // ex WChartDisplayConfig::WChartDisplayConfig
    init(tx);

    // Images
    conn_imageChange = root.provider().sig_imageChange.add(this, &ChartMouseConfig::onImageChange);
    onImageChange();

    // Icon click (and space key)
    sig_iconClick.add(this, &ChartMouseConfig::onIconClick);
}

client::widgets::ChartMouseConfig::~ChartMouseConfig()
{ }

void
client::widgets::ChartMouseConfig::set(int leftLock, int rightLock, int wheelMode)
{
    m_leftLock = leftLock;
    m_rightLock = rightLock;
    m_wheelMode = wheelMode;
    render();
}

int
client::widgets::ChartMouseConfig::getLeftLock() const
{
    return m_leftLock;
}

int
client::widgets::ChartMouseConfig::getRightLock() const
{
    return m_rightLock;
}

int
client::widgets::ChartMouseConfig::getWheelMode() const
{
    return m_wheelMode;
}

void
client::widgets::ChartMouseConfig::init(afl::string::Translator& tx)
{
    // ex WChartLockConfig::init()

    // Configure tree
    // - lock modes
    for (size_t i = 0; i < NUM_LOCK_MODES; ++i) {
        addNode(makePair(Mode(i), 0), 0, tx(LOCK_MODES[i]), true);
        for (size_t j = 0; j < countof(LOCK_FLAGS); ++j) {
            addNode(makePair(Mode(i), j+1), 1, tx(LOCK_FLAGS[j].name), false);
        }
    }

    // - wheel
    addNode(makePair(MODE_WHEEL, 0), 0, tx("Mouse wheel does..."), true);
    for (size_t i = 0; i < countof(WHEEL_MODES); ++i) {
        addNode(makePair(MODE_WHEEL, i+1), 1, tx(WHEEL_MODES[i]), false);
    }
}

void
client::widgets::ChartMouseConfig::render()
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

client::widgets::ChartMouseConfig::Value
client::widgets::ChartMouseConfig::getValue(int32_t id) const
{
    // ex WChartLockConfig::drawItem (sort-of)
    Mode mode = getModeFromPair(id);
    size_t sub = getSubIndexFromPair(id);
    switch (mode) {
     case MODE_LEFT:
     case MODE_RIGHT:
        if (sub != 0) {
            int value = (mode == MODE_LEFT ? m_leftLock : m_rightLock);
            return (value & LOCK_FLAGS[sub-1].flag) != 0 ? Checked : Unchecked;
        }
        break;
     case MODE_WHEEL:
        if (sub != 0) {
            return (m_wheelMode == int(sub-1)) ? Selected : Unselected;
        }
        break;
    }
    return None;
}

void
client::widgets::ChartMouseConfig::onIconClick(int32_t id)
{
    // ex WChartLockConfig::onClick(int32_t id)
    Mode mode = getModeFromPair(id);
    size_t sub  = getSubIndexFromPair(id);
    if (sub != 0) {
        switch (mode) {
         case MODE_LEFT:
            m_leftLock ^= LOCK_FLAGS[sub-1].flag;
            break;

         case MODE_RIGHT:
            m_rightLock ^= LOCK_FLAGS[sub-1].flag;
            break;

         case MODE_WHEEL:
            m_wheelMode = int(sub-1);
            break;
        }
        render();
    }
}

void
client::widgets::ChartMouseConfig::onImageChange()
{
    // Check whether there are unloaded images, and if so, load them.
    bool did = false;
    for (size_t i = 0; i < NUM_VALUES; ++i) {
        if (m_icons[i] == 0) {
            if (IMAGE_NAMES[i] == 0) {
                // Special case for 'None': make an image icon with no image (for consistent vertical spacing)
                m_icons[i] = &m_deleter.addNew(new ui::icons::Image(gfx::Point(20, 20)));
                did = true;
            } else {
                afl::base::Ptr<gfx::Canvas> image = m_root.provider().getImage(IMAGE_NAMES[i]);
                if (image.get() != 0) {
                    // Make a padded image to give user more space for clicking (same as in PCC2)
                    ui::icons::Image& img = m_deleter.addNew(new ui::icons::Image(*image));
                    ui::icons::StylableIcon& icon = m_deleter.addNew(new ui::icons::StylableIcon(img, m_root.colorScheme()));
                    int pad = IMAGE_PADDING[i];
                    icon.setPaddingBefore(gfx::Point(pad, pad));
                    icon.setPaddingAfter(gfx::Point(pad, pad));
                    m_icons[i] = &icon;
                    did = true;
                }
            }
        }
    }

    // If we loaded images, render. Widget will redraw when this produces an actual change.
    if (did) {
        render();
    }
}
