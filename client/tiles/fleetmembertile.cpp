/**
  *  \file client/tiles/fleetmembertile.cpp
  *  \brief Class client::tiles::FleetMemberTile
  */

#include "client/tiles/fleetmembertile.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/scrollbarcontainer.hpp"
#include "ui/widgets/statictext.hpp"

using game::proxy::FleetProxy;
using ui::widgets::AbstractListbox;
using ui::widgets::FrameGroup;
using ui::widgets::ScrollbarContainer;

namespace {
    const int NLINES = 10;

    void addButton(ui::Group& g, afl::base::Deleter& del, ui::Root& root, gfx::KeyEventConsumer& keyHandler, String_t title, util::Key_t key)
    {
        ui::widgets::Button& btn = del.addNew(new ui::widgets::Button(title, key, root));
        btn.dispatchKeyTo(keyHandler);
        btn.setCompact(true);
        g.add(btn);
    }
}

client::tiles::FleetMemberTile::FleetMemberTile(ui::Root& root, gfx::KeyEventConsumer& keyHandler, afl::string::Translator& tx)
    : Group(ui::layout::VBox::instance5),
      m_deleter(),
      m_internalColorScheme(ui::GRAY_COLOR_SET, root.colorScheme()),
      m_list(root, NLINES, 305),
      m_updating(false)
{
    // ex WFleetMemberTile::WFleetMemberTile, WFleetMemberTile::init, WFleetMemberTile::drawData
    // Color scheme to fill unused list items
    m_list.setColorScheme(m_internalColorScheme);
    m_list.setFlag(AbstractListbox::NoPageKeys, true);

    // Buttons
    ui::Group& g = m_deleter.addNew(new ui::Group(ui::layout::HBox::instance5));
    addButton(g, m_deleter, root, keyHandler, tx("Del"), util::Key_Delete);
    addButton(g, m_deleter, root, keyHandler, "B", 'b');
    addButton(g, m_deleter, root, keyHandler, "T", 't');
    addButton(g, m_deleter, root, keyHandler, "P", 'p'); // was 'S' in PCC 1.x, but that is taken by the spec sheet on ShipEquipment tile
    g.add(m_deleter.addNew(new ui::Spacer()));
    g.add(m_deleter.addNew(new ui::widgets::StaticText(tx("FCode"), util::SkinColor::Static, gfx::FontRequest(), root.provider())));
    addButton(g, m_deleter, root, keyHandler, "F", 'f');

    add(g);
    add(FrameGroup::wrapWidget(m_deleter, root.colorScheme(), ui::LoweredFrame, m_deleter.addNew(new ScrollbarContainer(m_list, root))));

    // List should start having focus
    m_list.requestFocus();
}

client::tiles::FleetMemberTile::~FleetMemberTile()
{ }

void
client::tiles::FleetMemberTile::attach(game::proxy::FleetProxy& proxy)
{
    // Fleet change: new data from game
    class FleetChange : public afl::base::Closure<void()> {
     public:
        FleetChange(FleetMemberTile& tile, FleetProxy& proxy)
            : m_tile(tile), m_proxy(proxy)
            { }
        virtual void call()
            { m_tile.onFleetChange(m_proxy.getFleetMemberList(), m_proxy.getSelectedFleetMember()); }
     private:
        FleetMemberTile& m_tile;
        FleetProxy& m_proxy;
    };
    conn_fleetChange = proxy.sig_change.addNewClosure(new FleetChange(*this, proxy));

    // List scroll: cursor moved
    class ListScroll : public afl::base::Closure<void()> {
     public:
        ListScroll(FleetMemberTile& tile, FleetProxy& proxy)
            : m_tile(tile), m_proxy(proxy)
            { }
        virtual void call()
            { m_tile.onListScroll(m_proxy); }
     private:
        FleetMemberTile& m_tile;
        FleetProxy& m_proxy;
    };
    conn_listScroll = m_list.sig_change.addNewClosure(new ListScroll(*this, proxy));
}

void
client::tiles::FleetMemberTile::onFleetChange(const game::ref::FleetMemberList& memList, game::Id_t memId)
{
    // ex WFleetMemberTile::onFleetChanged() (sort-of)

    // setContent() will probably change the current position and thus emit an onListScroll; suppress that.
    // It would generate a request to the game, causing both sides to battle for the current position.
    m_updating = true;
    m_list.setContent(memList);
    m_list.setCurrentFleetMember(memId);
    m_updating = false;
}

void
client::tiles::FleetMemberTile::onListScroll(game::proxy::FleetProxy& proxy)
{
    // ex WFleetMemberTile::onMove()
    if (!m_updating) {
        if (game::Id_t memId = m_list.getCurrentFleetMember()) {
            proxy.selectFleetMember(memId);
        }
    }
}
