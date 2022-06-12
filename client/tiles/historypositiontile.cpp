/**
  *  \file client/tiles/historypositiontile.cpp
  *  \brief Class client::tiles::HistoryPositionTile
  */

#include "client/tiles/historypositiontile.hpp"
#include "client/tiles/historyadaptor.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/scrollbarcontainer.hpp"
#include "ui/widgets/statictext.hpp"

using ui::widgets::AbstractListbox;
using ui::widgets::FrameGroup;
using ui::widgets::ScrollbarContainer;
using ui::widgets::StaticText;

namespace {
    const int NLINES = 10;
}

client::tiles::HistoryPositionTile::HistoryPositionTile(ui::Root& root, afl::string::Translator& tx)
    : Group(ui::layout::VBox::instance0),
      m_deleter(),
      m_internalColorScheme(ui::GRAY_COLOR_SET, root.colorScheme()),
      m_list(root, tx)
{
    // ex WHistoryShipPositionTile::WHistoryShipPositionTile, WHistoryShipPositionTile::drawData
    // Color scheme to fill unused list items
    m_list.setColorScheme(m_internalColorScheme);
    m_list.setFlag(AbstractListbox::NoPageKeys, true);
    m_list.setNumLines(NLINES);
    m_list.setWidth(305);

    // Title
    ui::Group& g = m_deleter.addNew(new ui::Group(ui::layout::HBox::instance5));
    g.add(m_deleter.addNew(new StaticText(tx("Turn: Location"), util::SkinColor::Static, gfx::FontRequest(), root.provider())));
    g.add(m_deleter.addNew(new ui::Spacer()));
    g.add(m_deleter.addNew(new StaticText(tx("Mass"), util::SkinColor::Static, gfx::FontRequest(), root.provider())));
    add(g);
    add(FrameGroup::wrapWidget(m_deleter, root.colorScheme(), ui::LoweredFrame, m_deleter.addNew(new ScrollbarContainer(m_list, root))));

    // List should start having focus
    m_list.requestFocus();
}

client::tiles::HistoryPositionTile::~HistoryPositionTile()
{ }

void
client::tiles::HistoryPositionTile::attach(HistoryAdaptor& adaptor)
{
    class ListChange : public afl::base::Closure<void()> {
     public:
        ListChange(HistoryPositionTile& parent, HistoryAdaptor& adaptor)
            : m_parent(parent), m_adaptor(adaptor)
            { }
        virtual void call()
            { m_parent.onListChange(m_adaptor); }
     private:
        HistoryPositionTile& m_parent;
        HistoryAdaptor& m_adaptor;
    };
    conn_listChange = adaptor.sig_listChange.addNewClosure(new ListChange(*this, adaptor));

    class ListScroll : public afl::base::Closure<void()> {
     public:
        ListScroll(HistoryPositionTile& parent, HistoryAdaptor& adaptor)
            : m_parent(parent), m_adaptor(adaptor)
            { }
        virtual void call()
            { m_parent.onListScroll(m_adaptor); }
     private:
        HistoryPositionTile& m_parent;
        HistoryAdaptor& m_adaptor;
    };
    conn_listScroll = m_list.sig_change.addNewClosure(new ListScroll(*this, adaptor));
}

void
client::tiles::HistoryPositionTile::onListChange(const HistoryAdaptor& adaptor)
{
    // Save turn number. setContent() may create events that destroy it, so we restore it later.
    const int turnNumber = adaptor.getTurnNumber();
    m_list.setContent(adaptor.getPositionList());
    m_list.setCurrentTurnNumber(turnNumber);
}

void
client::tiles::HistoryPositionTile::onListScroll(HistoryAdaptor& adaptor)
{
    // ex WHistoryShipPositionTile::onMove
    adaptor.setTurnNumber(m_list.getCurrentTurnNumber());
}
